/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "napi/native_node_api.h"
#include "sensor_service.h"

#include "i_sensor_service.h"
#include "sensor_napi_error.h"
#include "sensor_napi_utils.h"
#include "uv.h"

namespace OHOS {
namespace Sensors {
namespace {
constexpr int32_t ERR_OK = 0;

struct CallbackEntry {
    napi_env env = nullptr;
    napi_ref ref = nullptr;
    bool singleShot = false;
};

struct DispatchContext {
    napi_env env = nullptr;
    napi_ref ref = nullptr;
    SensorEventData event;
};

/**
 * @brief JS side registry of the on* subscribers.
 *
 * Sensor samples arrive on the platform sensor thread; the registry holds
 * the napi_ref of every subscriber and hops to the JS thread through the
 * uv event loop of the registering environment.
 */
class SensorCallbackManager {
public:
    static SensorCallbackManager &GetInstance()
    {
        static SensorCallbackManager instance;
        return instance;
    }

    void Register(int32_t sensorTypeId, napi_env env, napi_ref ref, bool singleShot)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        entries_[sensorTypeId].push_back(CallbackEntry { env, ref, singleShot });
    }

    /**
     * @brief Drops the subscriber holding the given reference, or all
     *        subscribers of the sensor when ref is null.
     *
     * @return true when at least one subscriber was removed.
     */
    bool Unregister(int32_t sensorTypeId, napi_env env, napi_ref ref)
    {
        std::vector<napi_ref> toRelease;
        bool removed = false;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto item = entries_.find(sensorTypeId);
            if (item == entries_.end()) {
                return false;
            }
            auto &list = item->second;
            for (auto iter = list.begin(); iter != list.end();) {
                bool match = (ref == nullptr) || (iter->ref == ref && iter->env == env);
                if (match) {
                    toRelease.push_back(iter->ref);
                    iter = list.erase(iter);
                    removed = true;
                } else {
                    ++iter;
                }
            }
            if (list.empty()) {
                entries_.erase(item);
            }
        }
        for (napi_ref refToRelease : toRelease) {
            uint32_t refCount = 0;
            napi_reference_unref(env, refToRelease, &refCount);
        }
        return removed;
    }

    /**
     * @brief Fans one sample out to every subscriber, on the JS thread.
     *
     * Called from the platform sensor thread; the event data is copied per
     * subscriber and delivered by uv_queue_work.
     */
    void Dispatch(int32_t sensorTypeId, const SensorEventData &event)
    {
        std::vector<CallbackEntry> receivers;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto item = entries_.find(sensorTypeId);
            if (item == entries_.end()) {
                return;
            }
            for (auto iter = item->second.begin(); iter != item->second.end();) {
                receivers.push_back(*iter);
                if (iter->singleShot) {
                    iter = item->second.erase(iter);
                } else {
                    ++iter;
                }
            }
            if (item->second.empty()) {
                entries_.erase(item);
            }
        }
        for (const CallbackEntry &receiver : receivers) {
            QueueToJs(receiver, event);
        }
    }

    bool IsEmpty(int32_t sensorTypeId)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto item = entries_.find(sensorTypeId);
        return item == entries_.end() || item->second.empty();
    }

private:
    static void QueueToJs(const CallbackEntry &receiver, const SensorEventData &event)
    {
        uv_loop_t *loop = nullptr;
        napi_get_uv_event_loop(receiver.env, &loop);
        if (loop == nullptr) {
            return;
        }
        auto *work = new (std::nothrow) uv_work_t;
        auto *context = new (std::nothrow) DispatchContext { receiver.env, receiver.ref, event };
        if (work == nullptr || context == nullptr) {
            delete work;
            delete context;
            return;
        }
        work->data = context;
        uv_queue_work(loop, work, [](uv_work_t *work) {},
            [](uv_work_t *work, int status) {
                auto *context = static_cast<DispatchContext *>(work->data);
                if (context == nullptr) {
                    delete work;
                    return;
                }
                if (status != 0) {
                    delete context;
                    delete work;
                    return;
                }
                napi_handle_scope scope = nullptr;
                napi_open_handle_scope(context->env, &scope);
                napi_value callback = nullptr;
                napi_get_reference_value(context->env, context->ref, &callback);
                if (callback != nullptr) {
                    napi_value response = CreateSensorResponse(context->env, context->event);
                    napi_value undefined = nullptr;
                    napi_get_undefined(context->env, &undefined);
                    napi_call_function(context->env, undefined, callback, 1, &response, nullptr);
                }
                napi_close_handle_scope(context->env, scope);
                delete context;
                delete work;
            });
    }

    std::mutex mutex_;
    std::unordered_map<int32_t, std::list<CallbackEntry>> entries_;
};

struct SensorApiDesc {
    const char *name;
    int32_t sensorTypeId;
    bool singleShot;
    bool hasOptions;
};

/**
 * @brief Shared body of every on and once export.
 *
 * Validates the callback, registers the JS receiver and opens the platform
 * subscription through the agent.
 */
napi_value SubscribeEntry(napi_env env, napi_callback_info info, const SensorApiDesc &desc)
{
    size_t argc = 2;
    napi_value args[2] = { nullptr, nullptr };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (argc < 1 || !IsFunctionArg(env, args[0])) {
        ThrowSensorErr(env, SENSOR_ERR_PARAM, "");
        return nullptr;
    }
    napi_ref ref = nullptr;
    napi_create_reference(env, args[0], 1, &ref);
    int64_t interval = ParseSensorOptions(env, desc.hasOptions && argc >= 2 ? args[1] : nullptr);
    auto &callbacks = SensorCallbackManager::GetInstance();
    callbacks.Register(desc.sensorTypeId, env, ref, desc.singleShot);
    auto dispatch = [&callbacks, sensorTypeId = desc.sensorTypeId](const SensorEventData &event) {
        callbacks.Dispatch(sensorTypeId, event);
    };
    int32_t ret = GetSensorService().SubscribeSensor(desc.sensorTypeId, interval, dispatch);
    if (ret != ERR_OK) {
        callbacks.Unregister(desc.sensorTypeId, env, ref);
        ThrowSensorErr(env, ToPublicErrCode(ret), "");
        return nullptr;
    }
    return nullptr;
}

/**
 * @brief Shared body of every off* export.
 *
 * Removes one subscriber or all subscribers of the sensor; the platform
 * subscription stops when the last subscriber leaves.
 */
napi_value UnsubscribeEntry(napi_env env, napi_callback_info info, int32_t sensorTypeId)
{
    size_t argc = 2;
    napi_value args[2] = { nullptr, nullptr };
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    napi_ref ref = nullptr;
    if (argc >= 2 && IsFunctionArg(env, args[1])) {
        napi_create_reference(env, args[1], 1, &ref);
    }
    auto &callbacks = SensorCallbackManager::GetInstance();
    bool removed = callbacks.Unregister(sensorTypeId, env, ref);
    if (ref != nullptr) {
        uint32_t refCount = 0;
        napi_reference_unref(env, ref, &refCount);
    }
    if (removed && callbacks.IsEmpty(sensorTypeId)) {
        GetSensorService().UnsubscribeSensor(sensorTypeId);
    }
    return nullptr;
}

template<int32_t SENSOR_TYPE, bool SINGLE, bool HAS_OPTIONS>
napi_value MakeSubscribe(napi_env env, napi_callback_info info)
{
    SensorApiDesc desc { nullptr, SENSOR_TYPE, SINGLE, HAS_OPTIONS };
    return SubscribeEntry(env, info, desc);
}

template<int32_t SENSOR_TYPE>
napi_value MakeUnsubscribe(napi_env env, napi_callback_info info)
{
    return UnsubscribeEntry(env, info, SENSOR_TYPE);
}
} // namespace

napi_value InitSensorModule(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("onAccelerometerChange", (&MakeSubscribe<SENSOR_TYPE_ID_ACCELEROMETER, false, true>)),
        DECLARE_NAPI_FUNCTION("onceAccelerometerChange", (&MakeSubscribe<SENSOR_TYPE_ID_ACCELEROMETER, true, false>)),
        DECLARE_NAPI_FUNCTION("offAccelerometerChange", (&MakeUnsubscribe<SENSOR_TYPE_ID_ACCELEROMETER>)),
        DECLARE_NAPI_FUNCTION("onGyroscopeChange", (&MakeSubscribe<SENSOR_TYPE_ID_GYROSCOPE, false, true>)),
        DECLARE_NAPI_FUNCTION("onceGyroscopeChange", (&MakeSubscribe<SENSOR_TYPE_ID_GYROSCOPE, true, false>)),
        DECLARE_NAPI_FUNCTION("offGyroscopeChange", (&MakeUnsubscribe<SENSOR_TYPE_ID_GYROSCOPE>)),
        DECLARE_NAPI_FUNCTION("onOrientationChange", (&MakeSubscribe<SENSOR_TYPE_ID_ORIENTATION, false, true>)),
        DECLARE_NAPI_FUNCTION("onceOrientationChange", (&MakeSubscribe<SENSOR_TYPE_ID_ORIENTATION, true, false>)),
        DECLARE_NAPI_FUNCTION("offOrientationChange", (&MakeUnsubscribe<SENSOR_TYPE_ID_ORIENTATION>)),
        DECLARE_NAPI_FUNCTION("onAmbientLightChange", (&MakeSubscribe<SENSOR_TYPE_ID_AMBIENT_LIGHT, false, true>)),
        DECLARE_NAPI_FUNCTION("onceAmbientLightChange", (&MakeSubscribe<SENSOR_TYPE_ID_AMBIENT_LIGHT, true, false>)),
        DECLARE_NAPI_FUNCTION("offAmbientLightChange", (&MakeUnsubscribe<SENSOR_TYPE_ID_AMBIENT_LIGHT>)),
        DECLARE_NAPI_FUNCTION("onProximityChange", (&MakeSubscribe<SENSOR_TYPE_ID_PROXIMITY, false, true>)),
        DECLARE_NAPI_FUNCTION("onceProximityChange", (&MakeSubscribe<SENSOR_TYPE_ID_PROXIMITY, true, false>)),
        DECLARE_NAPI_FUNCTION("offProximityChange", (&MakeUnsubscribe<SENSOR_TYPE_ID_PROXIMITY>)),
        DECLARE_NAPI_FUNCTION("onBarometerChange", (&MakeSubscribe<SENSOR_TYPE_ID_BAROMETER, false, true>)),
        DECLARE_NAPI_FUNCTION("onceBarometerChange", (&MakeSubscribe<SENSOR_TYPE_ID_BAROMETER, true, false>)),
        DECLARE_NAPI_FUNCTION("offBarometerChange", (&MakeUnsubscribe<SENSOR_TYPE_ID_BAROMETER>)),
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}

static napi_module sensorModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitSensorModule,
    .nm_modname = "sensor",
    .nm_priv = ((void *)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void SensorRegisterModule(void)
{
    napi_module_register(&sensorModule);
}
} // namespace Sensors
} // namespace OHOS
