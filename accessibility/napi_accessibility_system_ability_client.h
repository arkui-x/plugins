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

#ifndef PLUGIN_ACCESSIBILITY_NAPI_ACCESSIBILITY_SYSTEM_ABILITY_CLIENT_H
#define PLUGIN_ACCESSIBILITY_NAPI_ACCESSIBILITY_SYSTEM_ABILITY_CLIENT_H

#include <map>
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#ifdef ANDROID_PLATFORM
#include "android/java/jni/accessibility_system_ability_client_imlp.h"
#include "foundation/arkui/ace_engine/adapter/android/osal/accessibility_event_manager.h"
#endif
#ifdef IOS_PLATFORM
#include "foundation/arkui/ace_engine/adapter/ios/osal/accessibility_event_manager.h"
#endif

const std::string ERROR_MESSAGE_PARAMETER_ERROR =
    "Parameter error. Possible causes:"
    "1. Mandatory parameters are left unspecified; 2. Incorrect parameter types; 3. Parameter verification failed.";
const std::string ERROR_MESSAGE_SYSTEM_ABNORMALITY = "System abnormality";

enum RetError : int32_t {
    RET_OK = 0,
    RET_ERR_FAILED = -1,
    RET_ERR_INVALID_PARAM = 1001,
};

enum class NAccessibilityErrorCode : int32_t {
    ACCESSIBILITY_OK = 0,
    ACCESSIBILITY_ERROR_INVALID_PARAM = 401,
    ACCESSIBILITY_ERROR_SYSTEM_ABNORMALITY = 9300000,
};

struct NAccessibilityErrMsg {
    NAccessibilityErrorCode errCode;
    std::string message;
};

namespace OHOS::Plugin {
class NAccessibilityClient {
public:
    NAccessibilityClient() = default;
    ~NAccessibilityClient() = default;
    static napi_value IsOpenAccessibilitySync(napi_env env, napi_callback_info info);
    static napi_value IsOpenStateAccessibilitySync(napi_env env, napi_callback_info info);
    static napi_value GetTouchModeSync(napi_env env, napi_callback_info info);
    static napi_value SendAccessibilityEvent(napi_env env, napi_callback_info info);
    static napi_value SubscribeState(napi_env env, napi_callback_info info);
    static napi_value UnsubscribeState(napi_env env, napi_callback_info info);
    static napi_value GetAccessibilityExtensionListSync(napi_env env, napi_callback_info info);
    static napi_value GetAccessibilityExtensionList(napi_env env, napi_callback_info info);

private:
    struct AsyncAbilityContext {
        napi_env env;
        napi_async_work asyncWork;
        napi_deferred deferred;
        napi_ref callbackRef;
        std::string abilityType;
        std::string stateType;
        std::vector<OHOS::Ace::Framework::AccessibilityAbilityInfo> result;
        RetError ret = RET_OK;
        OHOS::Ace::Framework::AccessibilityEventInfo eventInfo;
    };

    static bool ValidateGetAccessibilityExtensionListParams(
        napi_env env, napi_callback_info info, std::string& abilityTypeStr, std::string& stateTypeStr);
    static napi_value CreateAccessibilityExtensionListResult(
        napi_env env, const std::vector<OHOS::Ace::Framework::AccessibilityAbilityInfo>& abilityList);
    static napi_value CreateAccessibilityAbilityObject(
        napi_env env, const OHOS::Ace::Framework::AccessibilityAbilityInfo& abilityInfo);
    static void SetBasicProperties(
        napi_env env, napi_value abilityObj, const OHOS::Ace::Framework::AccessibilityAbilityInfo& abilityInfo);
    static void SetArrayProperties(
        napi_env env, napi_value abilityObj, const OHOS::Ace::Framework::AccessibilityAbilityInfo& abilityInfo);
    static void SetBooleanProperty(
        napi_env env, napi_value abilityObj, const OHOS::Ace::Framework::AccessibilityAbilityInfo& abilityInfo);
    static napi_value CreateStringArray(napi_env env, const std::vector<std::string>& stringVector);
    static napi_value HandlePromiseCall(
        napi_env env, const std::string& abilityTypeStr, const std::string& stateTypeStr);
    static napi_value HandleCallbackCall(
        napi_env env, napi_value* args, const std::string& abilityTypeStr, const std::string& stateTypeStr);
    static void ExecuteCallback(napi_env env, void* data);
    static void PromiseCompleteCallback(napi_env env, napi_status status, void* data);
    static void CallbackCompleteCallback(napi_env env, napi_status status, void* data);
    static bool IsValidAbilityType(const std::string& abilityType);
    static bool IsValidStateType(const std::string& stateType);
};
} // namespace OHOS::Plugin
#endif // PLUGIN_ACCESSIBILITY_NAPI_ACCESSIBILITY_SYSTEM_ABILITY_CLIENT_H
