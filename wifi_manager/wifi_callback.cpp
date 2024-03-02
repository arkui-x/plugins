/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include <uv.h>

#include "log.h"
#include "wifi_callback.h"

namespace OHOS::Plugin {
constexpr uint32_t INVALID_REF_COUNT = 0xff;
void UvQueueWorkOnReceive(uv_work_t* work, int status)
{
    if (work == nullptr || work->data == nullptr) {
        LOGE("UvQueueWorkOnReceive work or work->data is nullptr");
        return;
    }
    WifiCallbackWorker* wifiCallbackWorker = reinterpret_cast<WifiCallbackWorker*>(work->data);
    napi_handle_scope scope;
    napi_open_handle_scope(wifiCallbackWorker->env, &scope);
    napi_value undefined = nullptr;
    napi_get_undefined(wifiCallbackWorker->env, &undefined);
    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_get_reference_value(wifiCallbackWorker->env, wifiCallbackWorker->ref, &callback);
    napi_value results[1] = { nullptr };
    results[0] = PluginUtilsNApi::GetNapiInt32(wifiCallbackWorker->code, wifiCallbackWorker->env);
    napi_call_function(wifiCallbackWorker->env, undefined, callback, 1, &results[0], &resultout);
    napi_close_handle_scope(wifiCallbackWorker->env, scope);
    delete wifiCallbackWorker;
    wifiCallbackWorker = nullptr;
    delete work;
}

WifiCallback::WifiCallback()
{
    maps_.clear();
    std::vector<RegObj> wifiStateChangeVector = {};
    std::vector<RegObj> wifiConnectionChangeVector = {};
    maps_[EVENT_STA_POWER_STATE_CHANGE] = wifiStateChangeVector;
    maps_[EVENT_STA_CONN_STATE_CHANGE] = wifiConnectionChangeVector;
}

void WifiCallback::SendCallback(const std::string& key, int code)
{
    auto it = maps_.find(key);
    if (it == maps_.end()) {
        LOGE("send callback event is %{public}s, not in map", key.c_str());
        return;
    }
    auto vectorItem = it->second.begin();
    while (vectorItem != it->second.end()) {
        uv_loop_s* loop = nullptr;
        napi_get_uv_event_loop(vectorItem->m_regEnv, &loop);
        if (loop == nullptr) {
            LOGE("Loop instance is nullptr.");
            return;
        }
        uv_work_t* work = new (std::nothrow) uv_work_t;
        if (work == nullptr) {
            LOGE("Work is null.");
            return;
        }
        WifiCallbackWorker* wifiCallbackWorker = new (std::nothrow) WifiCallbackWorker();
        if (wifiCallbackWorker == nullptr) {
            LOGE("Common event data worker is null.");
            delete work;
            work = nullptr;
            return;
        }
        wifiCallbackWorker->code = code;
        wifiCallbackWorker->env = vectorItem->m_regEnv;
        wifiCallbackWorker->ref = vectorItem->m_regHanderRef;
        work->data = (void*)wifiCallbackWorker;
        uv_queue_work(
            loop, work, [](uv_work_t* work) {}, UvQueueWorkOnReceive);
        vectorItem++;
    }
}

void WifiCallback::RegisterCallback(const napi_env& env, napi_value callback, const std::string& name)
{
    napi_ref handlerRef = nullptr;
    napi_create_reference(env, callback, 1, &handlerRef);
    RegObj regObj(env, handlerRef);
    auto iter = maps_.find(name);
    if (iter == maps_.end()) {
        LOGE("Cannot find key %{public}s", name.c_str());
        return;
    }
    if (!HashExitCallback(env, callback, iter->second)) {
        std::lock_guard<std::mutex> lock(mutex_);
        iter->second.emplace_back(regObj);
    }
}

void WifiCallback::UnRegisterCallback(const napi_env& env, napi_value callback, const std::string& name)
{
    auto iter = maps_.find(name);
    if (iter == maps_.end()) {
        return;
    }
    if (callback != nullptr) {
        std::lock_guard<std::mutex> lock(mutex_);
        DeleteRegisterObj(env, iter->second, callback);
    } else {
        std::lock_guard<std::mutex> lock(mutex_);
        LOGW("Unregister all relevant subscribe for: %{public}s", name.c_str());
        DeleteAllRegisterObj(env, iter->second);
    }
}

void WifiCallback::DeleteRegisterObj(const napi_env& env, std::vector<RegObj>& vecRegObjs, napi_value& handler)
{
    auto iter = vecRegObjs.begin();
    for (; iter != vecRegObjs.end();) {
        if (env == iter->m_regEnv) {
            napi_value handlerTemp = nullptr;
            napi_get_reference_value(iter->m_regEnv, iter->m_regHanderRef, &handlerTemp);
            bool isEqual = false;
            napi_strict_equals(iter->m_regEnv, handlerTemp, handler, &isEqual);
            if (isEqual) {
                uint32_t refCount = INVALID_REF_COUNT;
                napi_reference_unref(iter->m_regEnv, iter->m_regHanderRef, &refCount);
                LOGI("delete ref, m_regEnv: %{private}p, m_regHanderRef: %{private}p, refCount: %{public}d",
                    iter->m_regEnv, iter->m_regHanderRef, refCount);
                if (refCount == 0) {
                    napi_delete_reference(iter->m_regEnv, iter->m_regHanderRef);
                }
                LOGI("Delete register object ref.");
                iter = vecRegObjs.erase(iter);
            } else {
                ++iter;
            }
        } else {
            LOGI("Unregister event, env is not equal %{private}p, : %{private}p", env, iter->m_regEnv);
            ++iter;
        }
    }
}

void WifiCallback::DeleteAllRegisterObj(const napi_env& env, std::vector<RegObj>& vecRegObjs)
{
    auto iter = vecRegObjs.begin();
    for (; iter != vecRegObjs.end();) {
        if (env == iter->m_regEnv) {
            uint32_t refCount = INVALID_REF_COUNT;
            napi_reference_unref(iter->m_regEnv, iter->m_regHanderRef, &refCount);
            LOGI("delete all ref, m_regEnv: %{private}p, m_regHanderRef: %{private}p, refCount: %{public}d",
                iter->m_regEnv, iter->m_regHanderRef, refCount);
            if (refCount == 0) {
                napi_delete_reference(iter->m_regEnv, iter->m_regHanderRef);
            }
            iter = vecRegObjs.erase(iter);
        } else {
            LOGI("Unregister all event, env is not equal %{private}p, : %{private}p", env, iter->m_regEnv);
            ++iter;
        }
    }
}

bool WifiCallback::HashExitCallback(const napi_env& env, napi_value callback, const std::vector<RegObj>& regObjs)
{
    auto iter = regObjs.begin();
    while (iter != regObjs.end()) {
        if (iter->m_regEnv != env) {
            iter++;
            break;
        }
        napi_value handlerTemp = nullptr;
        napi_get_reference_value(env, iter->m_regHanderRef, &handlerTemp);
        bool isEqual = false;
        napi_strict_equals(env, handlerTemp, callback, &isEqual);
        if (isEqual) {
            return isEqual;
        }
        iter++;
    }
    return false;
}

bool WifiCallback::HasWifiRegister(const std::string& name)
{
    auto iter = maps_.find(name);
    if (iter == maps_.end()) {
        return false;
    }
    return iter->second.size() == 0 ? false : true;
}
} // namespace OHOS::Plugin
