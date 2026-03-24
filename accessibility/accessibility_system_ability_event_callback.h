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

#ifndef PLUGINS_ACCESSIBILITY_ACCESSIBILITY_SYSTEM_ABILITY_EVENT_CALLBACK_H
#define PLUGINS_ACCESSIBILITY_ACCESSIBILITY_SYSTEM_ABILITY_EVENT_CALLBACK_H

#include <map>
#include <string>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

namespace OHOS::Plugin {
struct AccessibilitySystemAbilityEventCallbackInfo {
    explicit AccessibilitySystemAbilityEventCallbackInfo(napi_env env, napi_ref ref)
        : eventCallbackEnv(env), eventCallbackRef(ref) {}
    napi_env eventCallbackEnv = nullptr;
    napi_ref eventCallbackRef = nullptr;
};

class AccessibilitySystemAbilityEventCallback {
public:
    AccessibilitySystemAbilityEventCallback() = default;
    virtual ~AccessibilitySystemAbilityEventCallback() = default;

    static bool HasCallback();
    static void InsetEventCallback(const napi_env& env, const napi_ref& eventCallback, const std::string& eventName);
    static void DeleteEventCallback(const std::string& eventName);
    static void ExcuteEventCallback(const bool state);
    static bool HasStateCallback();
    static void InsetStateEventCallback(
        const napi_env& env, const napi_ref& eventCallback, const std::string& eventName);
    static void DeleteStateEventCallback(const std::string& eventName);
    static void ExcuteStateEventCallback(const bool state);

private:
    static std::map<std::string, std::shared_ptr<AccessibilitySystemAbilityEventCallbackInfo>> callbackMap_;
    static std::map<std::string, std::shared_ptr<AccessibilitySystemAbilityEventCallbackInfo>> stateCallbackMap_;
};
} // namespace OHOS::Plugin
#endif // PLUGINS_ACCESSIBILITY_ACCESSIBILITY_SYSTEM_ABILITY_EVENT_CALLBACK_H