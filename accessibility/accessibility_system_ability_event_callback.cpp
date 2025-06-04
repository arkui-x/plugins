/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "accessibility_system_ability_event_callback.h"

#include "log.h"

namespace OHOS::Plugin {
std::map<std::string, std::shared_ptr<AccessibilitySystemAbilityEventCallbackInfo>>
    AccessibilitySystemAbilityEventCallback::callbackMap_;

void AccessibilitySystemAbilityEventCallback::InsetEventCallback(
    const napi_env& env, const napi_ref& eventCallback, const std::string& eventName)
{
    if (eventCallback == nullptr) {
        LOGE("Event callback is null for event: %{public}s", eventName.c_str());
        return;
    }

    auto callbackInfo = std::make_shared<AccessibilitySystemAbilityEventCallbackInfo>(env, eventCallback);
    callbackMap_[eventName] = callbackInfo;
}

void AccessibilitySystemAbilityEventCallback::DeleteEventCallback(const std::string& eventName)
{
    auto it = callbackMap_.find(eventName);
    if (it != callbackMap_.end()) {
        napi_delete_reference(it->second->eventCallbackEnv, it->second->eventCallbackRef);
        callbackMap_.erase(it);
    }
}

void AccessibilitySystemAbilityEventCallback::ExcuteEventCallback(const std::string& eventName, const bool state)
{
    auto it = callbackMap_.find(eventName);
    if (it != callbackMap_.end()) {
        napi_value callbackFunc = nullptr;
        napi_status status =
            napi_get_reference_value(it->second->eventCallbackEnv, it->second->eventCallbackRef, &callbackFunc);
        if (status == napi_ok) {
            napi_value result = nullptr;
            napi_value stateValue = nullptr;
            napi_get_boolean(it->second->eventCallbackEnv, state, &stateValue);
            napi_call_function(it->second->eventCallbackEnv, nullptr, callbackFunc, 1, &stateValue, &result);
        }
    }
}
} // namespace OHOS::Plugin