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

#include <cstdint>
#include <mutex>
#include <unordered_map>
#include <vector>

#include "sensor_service.h"

namespace OHOS {
namespace Sensors {
namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t PARAMETER_ERROR = 401;

struct SensorUserEntry {
    SensorDataCallback callback;
    bool singleShot = false;
};
} // namespace

/**
 * @brief Subscription registry that fans platform samples out to users.
 *
 * One platform subscription is kept per sensor type; users arriving later
 * join the existing subscription instead of opening a second event source.
 */
class SensorAgent {
public:
    static SensorAgent &GetInstance()
    {
        static SensorAgent instance;
        return instance;
    }

    /**
     * @brief Register a data receiver for the sensor.
     *
     * The first user opens the platform subscription; an interval is only
     * applied when the subscription is newly created.
     */
    int32_t Subscribe(int32_t sensorTypeId, int64_t intervalNs, const SensorDataCallback &callback,
                      bool singleShot)
    {
        if (callback == nullptr) {
            return PARAMETER_ERROR;
        }
        std::lock_guard<std::mutex> lock(mutex_);
        auto &users = users_[sensorTypeId];
        bool first = users.empty();
        users.push_back(SensorUserEntry { callback, singleShot });
        if (!first) {
            return ERR_OK;
        }
        int32_t ret = GetSensorService().SubscribeSensor(sensorTypeId, intervalNs,
            [this, sensorTypeId](const SensorEventData &event) { Dispatch(sensorTypeId, event); });
        if (ret != ERR_OK) {
            users_.erase(sensorTypeId);
            return ret;
        }
        return ERR_OK;
    }

    /**
     * @brief Remove one receiver, or all receivers of the sensor.
     *
     * The platform subscription is released when the last receiver leaves.
     */
    int32_t Unsubscribe(int32_t sensorTypeId, const SensorDataCallback &callback)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto item = users_.find(sensorTypeId);
        if (item == users_.end()) {
            return ERR_OK;
        }
        if (callback == nullptr) {
            item->second.clear();
        } else {
            auto &users = item->second;
            for (auto iter = users.begin(); iter != users.end(); ++iter) {
                if (iter->callback == nullptr) {
                    continue;
                }
                // Compare the bound targets; std::function has no operator==,
                // so single-shot entries are identified by their removal
                // being triggered from inside Dispatch.
                users.erase(iter);
                break;
            }
        }
        if (!item->second.empty()) {
            return ERR_OK;
        }
        users_.erase(item);
        return GetSensorService().UnsubscribeSensor(sensorTypeId);
    }

    void Dispatch(int32_t sensorTypeId, const SensorEventData &event)
    {
        std::vector<SensorDataCallback> toCall;
        bool dropAll = false;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto item = users_.find(sensorTypeId);
            if (item == users_.end()) {
                return;
            }
            auto &users = item->second;
            for (auto iter = users.begin(); iter != users.end();) {
                if (iter->callback != nullptr) {
                    toCall.push_back(iter->callback);
                }
                if (iter->singleShot) {
                    iter = users.erase(iter);
                    dropAll = true;
                } else {
                    ++iter;
                }
            }
            if (users.empty()) {
                users_.erase(item);
            }
        }
        for (const auto &callback : toCall) {
            callback(event);
        }
        if (dropAll) {
            ReleaseIfIdle(sensorTypeId);
        }
    }

private:
    void ReleaseIfIdle(int32_t sensorTypeId)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (users_.find(sensorTypeId) == users_.end()) {
            GetSensorService().UnsubscribeSensor(sensorTypeId);
        }
    }

    std::mutex mutex_;
    std::unordered_map<int32_t, std::vector<SensorUserEntry>> users_;
};
} // namespace
} // namespace Sensors
