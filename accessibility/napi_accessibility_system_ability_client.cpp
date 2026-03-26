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

#include <set>

#include "accessibility_system_ability_event_callback.h"
#include "log.h"

#ifdef IOS_PLATFORM
#include "ios/accessibilitySystemAbilityBridge.h"
#endif
#include "plugins/interfaces/native/inner_api/plugin_utils_napi.h"

using namespace OHOS::Ace::Framework;

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
        result = PluginUtilsNApi::CreateUndefined(env);
    } else {
        NAccessibilityErrMsg errMsg = QueryRetMsg(errCode);
        result = PluginUtilsNApi::CreateErrorMessage(env, static_cast<int32_t>(errMsg.errCode), errMsg.message);
    }
    return result;
}

void GetStringProperty(napi_env env, napi_value obj, const char* propName, std::string& out)
{
    napi_value prop = nullptr;
    if (napi_get_named_property(env, obj, propName, &prop) == napi_ok) {
        out = PluginUtilsNApi::GetStringFromValueUtf8(env, prop);
    }
}

void GetInt32Property(napi_env env, napi_value obj, const char* propName, int32_t& out)
{
    napi_value prop = nullptr;
    if (napi_get_named_property(env, obj, propName, &prop) == napi_ok) {
        out = PluginUtilsNApi::GetCInt32(prop, env);
    }
}

OHOS::Ace::Framework::AccessibilityEventInfo ParseAccessibilityEventInfo(napi_env env, napi_value obj)
{
    OHOS::Ace::Framework::AccessibilityEventInfo eventInfo;
    GetStringProperty(env, obj, "type", eventInfo.type);
    GetStringProperty(env, obj, "windowUpdateType", eventInfo.windowUpdateType);
    GetStringProperty(env, obj, "bundleName", eventInfo.bundleName);
    GetStringProperty(env, obj, "componentType", eventInfo.componentType);
    GetStringProperty(env, obj, "description", eventInfo.description);
    GetStringProperty(env, obj, "triggerAction", eventInfo.triggerAction);
    GetStringProperty(env, obj, "textMoveUnit", eventInfo.textMoveUnit);
    GetStringProperty(env, obj, "lastContent", eventInfo.lastContent);
    GetStringProperty(env, obj, "textAnnouncedForAccessibility", eventInfo.textAnnouncedForAccessibility);
    GetStringProperty(env, obj, "customId", eventInfo.customId);
    GetInt32Property(env, obj, "pageId", eventInfo.pageId);
    GetInt32Property(env, obj, "beginIndex", eventInfo.beginIndex);
    GetInt32Property(env, obj, "currentIndex", eventInfo.currentIndex);
    GetInt32Property(env, obj, "endIndex", eventInfo.endIndex);
    GetInt32Property(env, obj, "itemCount", eventInfo.itemCount);
    GetInt32Property(env, obj, "elementId", eventInfo.elementId);
    return eventInfo;
}

napi_value NAccessibilityClient::IsOpenAccessibilitySync(napi_env env, napi_callback_info info)
{
    bool status = false;
#ifdef IOS_PLATFORM
    status = getAccessibilityEnabledOC();
#endif

#ifdef ANDROID_PLATFORM
    status = AccessibilitySystemAbilityClientImpl::IsTouchExplorationEnable();
#endif
    return PluginUtilsNApi::CreateBoolean(env, status);
}

napi_value NAccessibilityClient::IsOpenStateAccessibilitySync(napi_env env, napi_callback_info info)
{
    bool status = false;
#ifdef IOS_PLATFORM
    status = getAccessibilityEnabledOC();
#endif

#ifdef ANDROID_PLATFORM
    status = AccessibilitySystemAbilityClientImpl::IsEnable();
#endif

    return PluginUtilsNApi::CreateBoolean(env, status);
}

napi_value NAccessibilityClient::GetTouchModeSync(napi_env env, napi_callback_info info)
{
    bool status = false;
#ifdef IOS_PLATFORM
    status = getAccessibilityEnabledOC();
#endif

#ifdef ANDROID_PLATFORM
    status = AccessibilitySystemAbilityClientImpl::IsTouchExplorationEnable();
#endif
    std::string touchMode = status ? "doubleTouchMode" : "none";
    return PluginUtilsNApi::CreateStringUtf8(env, touchMode);
}

napi_value NAccessibilityClient::SendAccessibilityEvent(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    size_t argc = PluginUtilsNApi::ARG_NUM_2;
    napi_value argv[PluginUtilsNApi::ARG_NUM_2] = { 0 };
    napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr);

    if (argc != PluginUtilsNApi::ARG_NUM_1 && argc != PluginUtilsNApi::ARG_NUM_2) {
        napi_throw(env, CreateBusinessError(env, RET_ERR_INVALID_PARAM));
        return nullptr;
    }

    if (PluginUtilsNApi::IsMatchType(argv[PluginUtilsNApi::ARG_NUM_0], napi_object, env) != TYPE_CHECK_SUCCESS) {
        LOGE("SendAccessibilityEvent: The type of the first argument is incorrect.");
        napi_throw(env, CreateBusinessError(env, RET_ERR_INVALID_PARAM));
        return nullptr;
    }

    AsyncAbilityContext* asyncContext = new (std::nothrow) AsyncAbilityContext { .env = env };
    if (asyncContext == nullptr) {
        return nullptr;
    }
    asyncContext->eventInfo = ParseAccessibilityEventInfo(env, argv[PluginUtilsNApi::ARG_NUM_0]);

    napi_value promise = nullptr;
    if (argc == PluginUtilsNApi::ARG_NUM_1) {
        napi_create_promise(env, &asyncContext->deferred, &promise);
    } else {
        if (PluginUtilsNApi::IsMatchType(argv[PluginUtilsNApi::ARG_NUM_1], napi_function, env) != TYPE_CHECK_SUCCESS) {
            delete asyncContext;
            napi_throw(env, CreateBusinessError(env, RET_ERR_INVALID_PARAM));
            return nullptr;
        }
        napi_create_reference(env, argv[PluginUtilsNApi::ARG_NUM_1], 1, &asyncContext->callbackRef);
        napi_get_undefined(env, &promise);
    }

    napi_value resourceName;
    napi_create_string_utf8(env, "SendAccessibilityEvent", NAPI_AUTO_LENGTH, &resourceName);
    napi_status status = napi_create_async_work(env, nullptr, resourceName,
        [](napi_env env, void* data) {},
        [](napi_env env, napi_status status, void* data) {
            AsyncAbilityContext* context = static_cast<AsyncAbilityContext*>(data);
            if (context == nullptr) {
                return;
            }
            OHOS::Ace::Framework::AccessibilityEventManager::SendEventInfo(context->eventInfo);
            if (context->deferred) {
                napi_value result;
                napi_get_undefined(env, &result);
                napi_resolve_deferred(env, context->deferred, result);
            } else {
                napi_value result[PluginUtilsNApi::ARG_NUM_2] = { 0 };
                napi_get_undefined(env, &result[0]);
                napi_get_undefined(env, &result[1]);

                napi_value callback = nullptr;
                napi_value cbResult = nullptr;
                napi_get_reference_value(env, context->callbackRef, &callback);
                napi_call_function(env, nullptr, callback, PluginUtilsNApi::ARG_NUM_2, &result[0], &cbResult);
                napi_delete_reference(env, context->callbackRef);
            }
            napi_delete_async_work(env, context->asyncWork);
            delete context;
        },
        static_cast<void*>(asyncContext), &asyncContext->asyncWork);
    if (status != napi_ok) {
        delete asyncContext;
        return nullptr;
    }
    status = napi_queue_async_work(env, asyncContext->asyncWork);
    if (status != napi_ok) {
        delete asyncContext;
        return nullptr;
    }
    return promise;
}

napi_value NAccessibilityClient::SubscribeState(napi_env env, napi_callback_info info)
{
    size_t argc = PluginUtilsNApi::ARG_NUM_2;
    napi_value args[PluginUtilsNApi::ARG_NUM_2] = { 0 };
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("SubscribeState Failed to get event type");
        napi_value err = CreateBusinessError(env, RET_ERR_FAILED);
        napi_throw(env, err);
        return nullptr;
    }

    if (argc < PluginUtilsNApi::ARG_NUM_2 ||
        PluginUtilsNApi::IsMatchType(args[PluginUtilsNApi::ARG_NUM_1], napi_function, env) != TYPE_CHECK_SUCCESS) {
        LOGE("SubscribeState invalid param");
        napi_value err = CreateBusinessError(env, RET_ERR_INVALID_PARAM);
        napi_throw(env, err);
        return nullptr;
    }

    napi_ref subscribeState_callback_ = nullptr;
    napi_create_reference(env, args[PluginUtilsNApi::ARG_NUM_1], PluginUtilsNApi::ARG_NUM_1, &subscribeState_callback_);
#ifdef ANDROID_PLATFORM
    if (PluginUtilsNApi::GetStringFromValueUtf8(env, args[PluginUtilsNApi::ARG_NUM_0]) == "accessibilityStateChange") {
        AccessibilitySystemAbilityEventCallback::InsetStateEventCallback(env, subscribeState_callback_,
            PluginUtilsNApi::GetStringFromValueUtf8(env, args[PluginUtilsNApi::ARG_NUM_0]));
        AccessibilitySystemAbilityClientImpl::RegisterAccessibilityStateListener();
    } else {
        AccessibilitySystemAbilityEventCallback::InsetEventCallback(env, subscribeState_callback_,
            PluginUtilsNApi::GetStringFromValueUtf8(env, args[PluginUtilsNApi::ARG_NUM_0]));
        AccessibilitySystemAbilityClientImpl::RegisterTouchExplorationListener();
    }
#endif
#ifdef IOS_PLATFORM
    AccessibilitySystemAbilityEventCallback::InsetEventCallback(
        env, subscribeState_callback_, PluginUtilsNApi::GetStringFromValueUtf8(env, args[PluginUtilsNApi::ARG_NUM_0]));
    subscribeStateOC();
#endif
    return nullptr;
}

napi_value NAccessibilityClient::UnsubscribeState(napi_env env, napi_callback_info info)
{
    size_t argc = PluginUtilsNApi::ARG_NUM_2;
    napi_value args[PluginUtilsNApi::ARG_NUM_2] = { 0 };
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        LOGE("UnsubscribeState Failed to get event type");
        napi_value err = CreateBusinessError(env, RET_ERR_FAILED);
        napi_throw(env, err);
        return nullptr;
    }

    if (argc < PluginUtilsNApi::ARG_NUM_2 ||
        PluginUtilsNApi::IsMatchType(args[PluginUtilsNApi::ARG_NUM_1], napi_function, env) != TYPE_CHECK_SUCCESS) {
        napi_value err = CreateBusinessError(env, RET_ERR_INVALID_PARAM);
        LOGE("UnsubscribeState invalid param");
        napi_throw(env, err);
        return nullptr;
    }
    std::string eventName = PluginUtilsNApi::GetStringFromValueUtf8(env, args[PluginUtilsNApi::ARG_NUM_0]);
    bool hasCallback = false;
#ifdef ANDROID_PLATFORM
    if (eventName == "accessibilityStateChange") {
        AccessibilitySystemAbilityEventCallback::DeleteStateEventCallback(eventName);
        hasCallback = AccessibilitySystemAbilityEventCallback::HasStateCallback();
        if (!hasCallback) {
            AccessibilitySystemAbilityClientImpl::UnregisterAccessibilityStateListener();
        }
    } else {
        AccessibilitySystemAbilityEventCallback::DeleteEventCallback(eventName);
        hasCallback = AccessibilitySystemAbilityEventCallback::HasCallback();
        if (!hasCallback) {
            AccessibilitySystemAbilityClientImpl::UnregisterTouchExplorationListener();
        }
    }
#endif
#ifdef IOS_PLATFORM
    AccessibilitySystemAbilityEventCallback::DeleteEventCallback(eventName);
    hasCallback = AccessibilitySystemAbilityEventCallback::HasCallback();
    if (!hasCallback) {
        unsubscribeStateOC();
    }
#endif
    return nullptr;
}

napi_value NAccessibilityClient::GetAccessibilityExtensionListSync(napi_env env, napi_callback_info info)
{
#ifdef ANDROID_PLATFORM
    std::string abilityTypeStr;
    std::string stateTypeStr;
    if (!ValidateGetAccessibilityExtensionListParams(env, info, abilityTypeStr, stateTypeStr)) {
        napi_value resultArray;
        napi_create_array_with_length(env, PluginUtilsNApi::ARG_NUM_0, &resultArray);
        return resultArray;
    }

    std::vector<AccessibilityAbilityInfo> abilityList =
        AccessibilitySystemAbilityClientImpl::GetAccessibilityExtensionListSync(abilityTypeStr, stateTypeStr);
    return CreateAccessibilityExtensionListResult(env, abilityList);
#endif
    return nullptr;
}

bool NAccessibilityClient::ValidateGetAccessibilityExtensionListParams(
    napi_env env, napi_callback_info info, std::string& abilityTypeStr, std::string& stateTypeStr)
{
    size_t argc = PluginUtilsNApi::ARG_NUM_2;
    napi_value args[PluginUtilsNApi::ARG_NUM_2] = { 0 };
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok) {
        return false;
    }

    if (argc != PluginUtilsNApi::ARG_NUM_2) {
        return false;
    }

    if ((PluginUtilsNApi::IsMatchType(args[PluginUtilsNApi::ARG_NUM_0], napi_string, env) != TYPE_CHECK_SUCCESS) ||
        (PluginUtilsNApi::IsMatchType(args[PluginUtilsNApi::ARG_NUM_1], napi_string, env) != TYPE_CHECK_SUCCESS)) {
        return false;
    }

    abilityTypeStr = PluginUtilsNApi::GetStringFromValueUtf8(env, args[PluginUtilsNApi::ARG_NUM_0]);
    stateTypeStr = PluginUtilsNApi::GetStringFromValueUtf8(env, args[PluginUtilsNApi::ARG_NUM_1]);

    bool isValidParam = !IsValidAbilityType(abilityTypeStr) || !IsValidStateType(stateTypeStr) ||
                        abilityTypeStr.empty() || stateTypeStr.empty();
    return !isValidParam;
}

napi_value NAccessibilityClient::CreateAccessibilityExtensionListResult(
    napi_env env, const std::vector<AccessibilityAbilityInfo>& abilityList)
{
    napi_value resultArray;
    napi_create_array_with_length(env, abilityList.size(), &resultArray);

    for (uint32_t i = 0; i < abilityList.size(); i++) {
        napi_value abilityObj = CreateAccessibilityAbilityObject(env, abilityList[i]);
        napi_set_element(env, resultArray, i, abilityObj);
    }
    return resultArray;
}

napi_value NAccessibilityClient::CreateAccessibilityAbilityObject(
    napi_env env, const AccessibilityAbilityInfo& abilityInfo)
{
    napi_value abilityObj;
    napi_create_object(env, &abilityObj);

    SetBasicProperties(env, abilityObj, abilityInfo);
    SetArrayProperties(env, abilityObj, abilityInfo);
    SetBooleanProperty(env, abilityObj, abilityInfo);
    return abilityObj;
}

void NAccessibilityClient::SetBasicProperties(
    napi_env env, napi_value abilityObj, const AccessibilityAbilityInfo& abilityInfo)
{
    napi_value idValue;
    napi_create_string_utf8(env, abilityInfo.id.c_str(), NAPI_AUTO_LENGTH, &idValue);
    napi_set_named_property(env, abilityObj, "id", idValue);
    napi_value nameValue;
    napi_create_string_utf8(env, abilityInfo.name.c_str(), NAPI_AUTO_LENGTH, &nameValue);
    napi_set_named_property(env, abilityObj, "name", nameValue);
    napi_value bundleNameValue;
    napi_create_string_utf8(env, abilityInfo.bundleName.c_str(), NAPI_AUTO_LENGTH, &bundleNameValue);
    napi_set_named_property(env, abilityObj, "bundleName", bundleNameValue);
    napi_value descriptionValue;
    napi_create_string_utf8(env, abilityInfo.description.c_str(), NAPI_AUTO_LENGTH, &descriptionValue);
    napi_set_named_property(env, abilityObj, "description", descriptionValue);
    napi_value labelValue;
    napi_create_string_utf8(env, abilityInfo.label.c_str(), NAPI_AUTO_LENGTH, &labelValue);
    napi_set_named_property(env, abilityObj, "label", labelValue);
}

void NAccessibilityClient::SetArrayProperties(
    napi_env env, napi_value abilityObj, const AccessibilityAbilityInfo& abilityInfo)
{
    napi_value targetBundleNamesArray = CreateStringArray(env, abilityInfo.targetBundleNames);
    napi_set_named_property(env, abilityObj, "targetBundleNames", targetBundleNamesArray);
    napi_value abilityTypesArray = CreateStringArray(env, abilityInfo.abilityTypes);
    napi_set_named_property(env, abilityObj, "abilityTypes", abilityTypesArray);
    napi_value capabilitiesArray = CreateStringArray(env, abilityInfo.capabilities);
    napi_set_named_property(env, abilityObj, "capabilities", capabilitiesArray);
    napi_value eventTypesArray = CreateStringArray(env, abilityInfo.eventTypes);
    napi_set_named_property(env, abilityObj, "eventTypes", eventTypesArray);
}

void NAccessibilityClient::SetBooleanProperty(
    napi_env env, napi_value abilityObj, const AccessibilityAbilityInfo& abilityInfo)
{
    napi_value needHideValue;
    napi_get_boolean(env, abilityInfo.needHide, &needHideValue);
    napi_set_named_property(env, abilityObj, "needHide", needHideValue);
}

napi_value NAccessibilityClient::CreateStringArray(napi_env env, const std::vector<std::string>& stringVector)
{
    napi_value array;
    napi_create_array_with_length(env, stringVector.size(), &array);

    for (uint32_t i = 0; i < stringVector.size(); i++) {
        napi_value stringValue;
        napi_create_string_utf8(env, stringVector[i].c_str(), NAPI_AUTO_LENGTH, &stringValue);
        napi_set_element(env, array, i, stringValue);
    }
    return array;
}

napi_value NAccessibilityClient::GetAccessibilityExtensionList(napi_env env, napi_callback_info info)
{
#ifdef ANDROID_PLATFORM
    size_t argc = PluginUtilsNApi::ARG_NUM_3;
    napi_value args[PluginUtilsNApi::ARG_NUM_3] = { 0 };
    napi_status status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    if (status != napi_ok || argc < PluginUtilsNApi::ARG_NUM_2) {
        napi_value err = CreateBusinessError(env, RET_ERR_INVALID_PARAM);
        napi_throw(env, err);
        return nullptr;
    }

    std::string abilityTypeStr = PluginUtilsNApi::GetStringFromValueUtf8(env, args[PluginUtilsNApi::ARG_NUM_0]);
    std::string stateTypeStr = PluginUtilsNApi::GetStringFromValueUtf8(env, args[PluginUtilsNApi::ARG_NUM_1]);
    bool isValidParam = !IsValidAbilityType(abilityTypeStr) || !IsValidStateType(stateTypeStr) ||
                        abilityTypeStr.empty() || stateTypeStr.empty();
    if (argc == PluginUtilsNApi::ARG_NUM_2) {
        if (isValidParam) {
            napi_deferred deferred;
            napi_value promise;
            napi_create_promise(env, &deferred, &promise);
            napi_value err = CreateBusinessError(env, RET_ERR_INVALID_PARAM);
            napi_reject_deferred(env, deferred, err);
            return promise;
        }
        return HandlePromiseCall(env, abilityTypeStr, stateTypeStr);
    } else {
        return HandleCallbackCall(env, args, abilityTypeStr, stateTypeStr);
    }
#endif
    return nullptr;
}

napi_value NAccessibilityClient::HandlePromiseCall(
    napi_env env, const std::string& abilityTypeStr, const std::string& stateTypeStr)
{
    napi_deferred deferred;
    napi_value promise;
    napi_create_promise(env, &deferred, &promise);

    AsyncAbilityContext* asyncContext = new (std::nothrow) AsyncAbilityContext {
        .env = env, .deferred = deferred, .abilityType = abilityTypeStr, .stateType = stateTypeStr
    };
    if (asyncContext == nullptr) {
        return promise;
    }
    napi_value resourceName;
    napi_create_string_utf8(env, "GetAccessibilityExtensionListPromise", NAPI_AUTO_LENGTH, &resourceName);
    napi_status status = napi_create_async_work(env, nullptr, resourceName, ExecuteCallback, PromiseCompleteCallback,
        static_cast<void*>(asyncContext), &asyncContext->asyncWork);
    if (status != napi_ok) {
        delete asyncContext;
        napi_value err = CreateBusinessError(env, RET_ERR_FAILED);
        napi_reject_deferred(env, deferred, err);
        return promise;
    }

    status = napi_queue_async_work(env, asyncContext->asyncWork);
    if (status != napi_ok) {
        napi_delete_async_work(env, asyncContext->asyncWork);
        delete asyncContext;
        napi_value err = CreateBusinessError(env, RET_ERR_FAILED);
        napi_reject_deferred(env, deferred, err);
        return promise;
    }
    return promise;
}

napi_value NAccessibilityClient::HandleCallbackCall(
    napi_env env, napi_value* args, const std::string& abilityTypeStr, const std::string& stateTypeStr)
{
    if (PluginUtilsNApi::IsMatchType(args[PluginUtilsNApi::ARG_NUM_2], napi_function, env) != TYPE_CHECK_SUCCESS) {
        return nullptr;
    }
    bool isValidParam = !IsValidAbilityType(abilityTypeStr) || !IsValidStateType(stateTypeStr) ||
                        abilityTypeStr.empty() || stateTypeStr.empty();
    RetError ret = isValidParam ? RET_ERR_INVALID_PARAM : RET_OK;
    AsyncAbilityContext* asyncContext = new (std::nothrow) AsyncAbilityContext {
        .env = env, .callbackRef = nullptr, .abilityType = abilityTypeStr, .stateType = stateTypeStr, .ret = ret
    };
    if (asyncContext == nullptr) {
        return nullptr;
    }
    napi_status status = napi_create_reference(env, args[PluginUtilsNApi::ARG_NUM_2], 1, &asyncContext->callbackRef);
    if (status != napi_ok) {
        delete asyncContext;
        napi_value err = CreateBusinessError(env, RET_ERR_FAILED);
        napi_throw(env, err);
        return nullptr;
    }

    napi_value resourceName;
    napi_create_string_utf8(env, "GetAccessibilityExtensionListCallback", NAPI_AUTO_LENGTH, &resourceName);
    status = napi_create_async_work(env, nullptr, resourceName, ExecuteCallback, CallbackCompleteCallback,
        static_cast<void*>(asyncContext), &asyncContext->asyncWork);
    if (status != napi_ok) {
        napi_delete_reference(env, asyncContext->callbackRef);
        delete asyncContext;
        napi_value err = CreateBusinessError(env, RET_ERR_FAILED);
        napi_throw(env, err);
        return nullptr;
    }

    status = napi_queue_async_work(env, asyncContext->asyncWork);
    if (status != napi_ok) {
        napi_delete_async_work(env, asyncContext->asyncWork);
        napi_delete_reference(env, asyncContext->callbackRef);
        delete asyncContext;
        napi_value err = CreateBusinessError(env, RET_ERR_FAILED);
        napi_throw(env, err);
        return nullptr;
    }
    return nullptr;
}

void NAccessibilityClient::ExecuteCallback(napi_env env, void* data)
{
    AsyncAbilityContext* context = static_cast<AsyncAbilityContext*>(data);
    if (context == nullptr) {
        return;
    }
#ifdef ANDROID_PLATFORM
    context->result = AccessibilitySystemAbilityClientImpl::GetAccessibilityExtensionListSync(
        context->abilityType, context->stateType);
#endif
}

void NAccessibilityClient::PromiseCompleteCallback(napi_env env, napi_status status, void* data)
{
    AsyncAbilityContext* context = static_cast<AsyncAbilityContext*>(data);
    if (context == nullptr) {
        return;
    }
    if (context->ret == RET_OK) {
        napi_value result = CreateAccessibilityExtensionListResult(env, context->result);
        napi_resolve_deferred(env, context->deferred, result);
    } else {
        napi_value businessError = CreateBusinessError(env, context->ret);
        napi_reject_deferred(env, context->deferred, businessError);
    }
    napi_delete_async_work(env, context->asyncWork);
    delete context;
}

void NAccessibilityClient::CallbackCompleteCallback(napi_env env, napi_status status, void* data)
{
    AsyncAbilityContext* context = static_cast<AsyncAbilityContext*>(data);
    if (context == nullptr) {
        return;
    }
    napi_value callback;
    napi_get_reference_value(env, context->callbackRef, &callback);
    napi_value result[PluginUtilsNApi::ARG_NUM_2];
    if (context->ret == RET_OK) {
        napi_get_null(env, &result[0]);
        result[1] = CreateAccessibilityExtensionListResult(env, context->result);
    } else {
        napi_value businessError = CreateBusinessError(env, context->ret);
        result[0] = businessError;
        napi_get_undefined(env, &result[1]);
    }
    napi_call_function(env, nullptr, callback, PluginUtilsNApi::ARG_NUM_2, result, nullptr);
    napi_delete_reference(env, context->callbackRef);
    napi_delete_async_work(env, context->asyncWork);
    delete context;
}

bool NAccessibilityClient::IsValidAbilityType(const std::string& abilityType)
{
    static const std::set<std::string> validAbilityTypes = { "audible", "spoken", "visual", "haptic", "generic",
        "all" };
    return validAbilityTypes.find(abilityType) != validAbilityTypes.end();
}

bool NAccessibilityClient::IsValidStateType(const std::string& stateType)
{
    static const std::set<std::string> validStateTypes = { "enable", "disable", "install" };
    return validStateTypes.find(stateType) != validStateTypes.end();
}
} // namespace OHOS::Plugin