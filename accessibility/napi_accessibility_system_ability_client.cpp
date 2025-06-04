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

#include "napi_accessibility_system_ability_client.h"

#include "accessibility_system_ability_event_callback.h"
#include "log.h"

#ifdef IOS_PLATFORM
#include "ios/accessibilitySystemAbilityBridge.h"
#endif
#ifdef ANDROID_PLATFORM
#include "android/java/jni/accessibility_system_ability_client_imlp.h"
#endif

namespace OHOS::Plugin {
NAccessibilityErrMsg QueryRetMsg(RetError errorCode)
{
    switch (errorCode) {
        case RetError::RET_OK:
            return { NAccessibilityErrorCode::ACCESSIBILITY_OK, "" };
        case RetError::RET_ERR_FAILED:
            return { NAccessibilityErrorCode::ACCESSIBILITY_ERROR_SYSTEM_ABNORMALITY,
                ERROR_MESSAGE_SYSTEM_ABNORMALITY };
        case RetError::RET_ERR_INVALID_PARAM:
            return { NAccessibilityErrorCode::ACCESSIBILITY_ERROR_INVALID_PARAM, ERROR_MESSAGE_PARAMETER_ERROR };
        default:
            return { NAccessibilityErrorCode::ACCESSIBILITY_ERROR_SYSTEM_ABNORMALITY,
                ERROR_MESSAGE_SYSTEM_ABNORMALITY };
    }
}

napi_value CreateBusinessError(napi_env env, RetError errCode)
{
    napi_value result = nullptr;
    if (errCode == RetError::RET_OK) {
        napi_get_undefined(env, &result);
    } else {
        NAccessibilityErrMsg errMsg = QueryRetMsg(errCode);
        napi_value eCode = nullptr;
        napi_create_int32(env, static_cast<int32_t>(errMsg.errCode), &eCode);
        napi_value eMsg = nullptr;
        napi_create_string_utf8(env, errMsg.message.c_str(), NAPI_AUTO_LENGTH, &eMsg);
        napi_create_error(env, nullptr, eMsg, &result);
        napi_set_named_property(env, result, "code", eCode);
    }
    return result;
}

std::string ParseString(napi_env env, napi_value args)
{
    std::string result;
    size_t size = 0;

    if (napi_get_value_string_utf8(env, args, nullptr, 0, &size) != napi_ok) {
        LOGE("can not get string size");
        return "";
    }
    result.reserve(size + 1);
    result.resize(size);
    if (napi_get_value_string_utf8(env, args, result.data(), (size + 1), &size) != napi_ok) {
        LOGE("can not get string value");
        return "";
    }
    return result;
}

napi_value NAccessibilityClient::IsOpenAccessibilitySync(napi_env env, napi_callback_info info)
{
    bool status = false;
#ifdef IOS_PLATFORM
    status = getAccessibilityEnabledOC();
#endif
#ifdef ANDROID_PLATFORM
    status = AccessibilitySystemAbilityClientImpl::IsEnable();
#endif
    napi_value result = nullptr;
    napi_get_boolean(env, status, &result);
    return result;
}

napi_value NAccessibilityClient::SubscribeState(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value args[ARGS_SIZE_TWO] = { 0 };
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("SubscribeState Failed to get event type");
        napi_value err = CreateBusinessError(env, RET_ERR_FAILED);
        napi_throw(env, err);
        return nullptr;
    }
    napi_valuetype valueType = napi_null;
    if (argc < ARGS_SIZE_TWO) {
        LOGE("SubscribeState invalid param");
        napi_value err = CreateBusinessError(env, RET_ERR_INVALID_PARAM);
        napi_throw(env, err);
        return nullptr;
    }
    napi_typeof(env, args[ARGS_SIZE_ONE], &valueType);
    if (valueType != napi_function) {
        napi_value err = CreateBusinessError(env, RET_ERR_INVALID_PARAM);
        LOGE("SubscribeState invalid param");
        napi_throw(env, err);
        return nullptr;
    }

    if (ParseString(env, args[ARGS_SIZE_ZERO]) == "accessibilityStateChange") {
        napi_ref subscribeState_callback_ = nullptr;
        napi_create_reference(env, args[ARGS_SIZE_ONE], ARGS_SIZE_ONE, &subscribeState_callback_);
        std::string eventName = "accessibilityStateChange";
        AccessibilitySystemAbilityEventCallback::InsetEventCallback(env, subscribeState_callback_, eventName);
#ifdef ANDROID_PLATFORM
        AccessibilitySystemAbilityClientImpl::RegisterAccessibilityStateListener();
#endif
#ifdef IOS_PLATFORM
        subscribeStateOC();
#endif
    }

    return nullptr;
}

napi_value NAccessibilityClient::UnsubscribeState(napi_env env, napi_callback_info info)
{
    size_t argc = ARGS_SIZE_TWO;
    napi_value args[ARGS_SIZE_TWO] = { 0 };
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("UnsubscribeState Failed to get event type");
        napi_value err = CreateBusinessError(env, RET_ERR_FAILED);
        napi_throw(env, err);
        return nullptr;
    }
    napi_valuetype valueType = napi_null;
    if (argc < ARGS_SIZE_TWO) {
        LOGE("SubscribeState invalid param");
        napi_value err = CreateBusinessError(env, RET_ERR_INVALID_PARAM);
        napi_throw(env, err);
        return nullptr;
    }
    napi_typeof(env, args[ARGS_SIZE_ONE], &valueType);
    if (valueType != napi_function) {
        napi_value err = CreateBusinessError(env, RET_ERR_INVALID_PARAM);
        LOGE("UnsubscribeState invalid param");
        napi_throw(env, err);
        return nullptr;
    }

    if ((ParseString(env, args[ARGS_SIZE_ZERO]) == "accessibilityStateChange")) {
        std::string eventName = "accessibilityStateChange";
        AccessibilitySystemAbilityEventCallback::DeleteEventCallback(eventName);
#ifdef ANDROID_PLATFORM
        AccessibilitySystemAbilityClientImpl::UnregisterAccessibilityStateListener();
#endif
#ifdef IOS_PLATFORM
        unsubscribeStateOC();
#endif
    }
    return nullptr;
}
} // namespace OHOS::Plugin