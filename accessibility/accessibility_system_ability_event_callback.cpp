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
const char ACCESSIBILITY_STATE_CHANGE_STRING[] = "accessibilityStateChange";
const char ACCESSIBILITY_TOUCH_CHANGE_STRING[] = "touchModeChange";

std::map<std::string, std::shared_ptr<AccessibilitySystemAbilityEventCallbackInfo>>
    AccessibilitySystemAbilityEventCallback::callbackMap_;

std::map<std::string, std::shared_ptr<AccessibilitySystemAbilityEventCallbackInfo>>
    AccessibilitySystemAbilityEventCallback::stateCallbackMap_;

bool AccessibilitySystemAbilityEventCallback::HasCallback()
{
    return !callbackMap_.empty();
}

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

void AccessibilitySystemAbilityEventCallback::ExcuteEventCallback(const bool state)
{
    for (auto it = callbackMap_.begin(); it != callbackMap_.end(); ++it) {
        napi_value callbackFunc = nullptr;
        napi_status status =
            napi_get_reference_value(it->second->eventCallbackEnv, it->second->eventCallbackRef, &callbackFunc);
        if (status != napi_ok) {
            continue;
        }
        napi_value result = nullptr;
        napi_value stateValue = nullptr;
        if (it->first == ACCESSIBILITY_TOUCH_CHANGE_STRING) {
            const char* modeStr = state ? "doubleTouchMode" : "none";
            napi_value strValue = nullptr;
            napi_create_string_utf8(it->second->eventCallbackEnv, modeStr, NAPI_AUTO_LENGTH, &strValue);
            napi_call_function(it->second->eventCallbackEnv, nullptr, callbackFunc, 1, &strValue, &result);
        } else {
            napi_get_boolean(it->second->eventCallbackEnv, state, &stateValue);
            napi_call_function(it->second->eventCallbackEnv, nullptr, callbackFunc, 1, &stateValue, &result);
        }
    }
}

bool AccessibilitySystemAbilityEventCallback::HasStateCallback()
{
    return !stateCallbackMap_.empty();
}

void AccessibilitySystemAbilityEventCallback::InsetStateEventCallback(
    const napi_env& env, const napi_ref& eventCallback, const std::string& eventName)
{
    if (eventCallback == nullptr) {
        LOGE("Event callback is null for event: %{public}s", eventName.c_str());
        return;
    }

    auto callbackInfo = std::make_shared<AccessibilitySystemAbilityEventCallbackInfo>(env, eventCallback);
    stateCallbackMap_[eventName] = callbackInfo;
}

void AccessibilitySystemAbilityEventCallback::DeleteStateEventCallback(const std::string& eventName)
{
    auto it = stateCallbackMap_.find(eventName);
    if (it != stateCallbackMap_.end()) {
        napi_delete_reference(it->second->eventCallbackEnv, it->second->eventCallbackRef);
        stateCallbackMap_.erase(it);
    }
}

void AccessibilitySystemAbilityEventCallback::ExcuteStateEventCallback(const bool state)
{
    auto it = stateCallbackMap_.find(ACCESSIBILITY_STATE_CHANGE_STRING);
    if (it != stateCallbackMap_.end()) {
        napi_value callbackFunc = nullptr;
        napi_status status =
            napi_get_reference_value(it->second->eventCallbackEnv, it->second->eventCallbackRef, &callbackFunc);
        if (status != napi_ok) {
            return;
        }
        napi_value result = nullptr;
        napi_value stateValue = nullptr;
        napi_get_boolean(it->second->eventCallbackEnv, state, &stateValue);
        napi_call_function(it->second->eventCallbackEnv, nullptr, callbackFunc, 1, &stateValue, &result);
    }
}
} // namespace OHOS::Plugin