/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "common.h"
#include "ans_inner_errors.h"
#include "log.h"
#include "js_native_api.h"
#include "js_native_api_types.h"
#include "napi_common_util.h"
#include <unordered_map>
#include <string>

namespace OHOS {
namespace NotificationNapi {
namespace {
static const std::unordered_map<int32_t, std::string> ERROR_CODE_MESSAGE {
    {ERROR_PERMISSION_DENIED, "Permission denied"},
    {ERROR_NOT_SYSTEM_APP, "The application isn't system application"},
    {ERROR_PARAM_INVALID, "Invalid parameter"},
    {ERROR_SYSTEM_CAP_ERROR, "SystemCapability not found"},
    {ERROR_INTERNAL_ERROR, "Internal error"},
    {ERROR_IPC_ERROR, "Marshalling or unmarshalling error"},
    {ERROR_SERVICE_CONNECT_ERROR, "Failed to connect to the service"},
    {ERROR_NOTIFICATION_CLOSED, "Notification disabled"},
    {ERROR_SLOT_CLOSED, "Notification slot disabled"},
    {ERROR_NOTIFICATION_UNREMOVABLE, "Notification deletion disabled"},
    {ERROR_NOTIFICATION_NOT_EXIST, "The notification does not exist"},
    {ERROR_USER_NOT_EXIST, "The user does not exist"},
    {ERROR_OVER_MAX_NUM_PER_SECOND, "The notification sending frequency reaches the upper limit"},
    {ERROR_DISTRIBUTED_OPERATION_FAILED, "Distributed operation failed"},
    {ERROR_READ_TEMPLATE_CONFIG_FAILED, "Failed to read the template configuration"},
    {ERROR_NO_MEMORY, "No memory space"},
    {ERROR_BUNDLE_NOT_FOUND, "The specified bundle name was not found"},
    {ERROR_NO_AGENT_SETTING, "There is no corresponding agent relationship configuration"},
};
}

napi_value Common::NapiGetNull(napi_env env)
{
    napi_value result = nullptr;
    napi_get_null(env, &result);
    return result;
}

napi_value Common::NapiGetUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value Common::CreateErrorValue(napi_env env, int32_t errCode, bool newType)
{
    LOGI("enter, errorCode[%{public}d]", errCode);
    napi_value error = Common::NapiGetNull(env);
    if (errCode == ERR_OK && newType) {
        return error;
    }

    napi_value code = nullptr;
    napi_create_int32(env, errCode, &code);

    auto iter = ERROR_CODE_MESSAGE.find(errCode);
    std::string errMsg = iter != ERROR_CODE_MESSAGE.end() ? iter->second : "";
    napi_value message = nullptr;
    napi_create_string_utf8(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &message);

    napi_create_error(env, nullptr, message, &error);
    napi_set_named_property(env, error, "code", code);
    return error;
}

napi_value Common::CreateErrorValue(napi_env env, int32_t errCode, std::string &msg)
{
    LOGI("enter, errorCode[%{public}d]", errCode);
    napi_value error = Common::NapiGetNull(env);
    if (errCode == ERR_OK) {
        return error;
    }

    napi_value code = nullptr;
    napi_create_int32(env, errCode, &code);

    auto iter = ERROR_CODE_MESSAGE.find(errCode);
    std::string errMsg = iter != ERROR_CODE_MESSAGE.end() ? iter->second : "";
    napi_value message = nullptr;
    napi_create_string_utf8(env, errMsg.append(" ").append(msg).c_str(), NAPI_AUTO_LENGTH, &message);

    napi_create_error(env, nullptr, message, &error);
    napi_set_named_property(env, error, "code", code);
    return error;
}

void Common::NapiThrow(napi_env env, int32_t errCode)
{
    LOGD("enter");

    napi_throw(env, CreateErrorValue(env, errCode, true));
}

void Common::NapiThrow(napi_env env, int32_t errCode, std::string &msg)
{
    LOGD("enter");

    napi_throw(env, CreateErrorValue(env, errCode, msg));
}

void Common::PaddingCallbackPromiseInfo(
    const napi_env &env, const napi_ref &callback, CallbackPromiseInfo &info, napi_value &promise)
{
    LOGD("enter");

    if (callback) {
        LOGD("Callback is not nullptr.");
        info.callback = callback;
        info.isCallback = true;
    } else {
        napi_deferred deferred = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_create_promise(env, &deferred, &promise));
        info.deferred = deferred;
        info.isCallback = false;
    }
}

void Common::SetCallback(
    const napi_env &env, const napi_ref &callbackIn, const int32_t &errorCode, const napi_value &result, bool newType)
{
    LOGD("enter");
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_get_reference_value(env, callbackIn, &callback);
    napi_value results[ARGS_TWO] = {nullptr};
    results[PARAM0] = CreateErrorValue(env, errorCode, newType);
    results[PARAM1] = result;
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_TWO, &results[PARAM0], &resultout));
    LOGD("end");
}

void Common::SetCallback(
    const napi_env &env, const napi_ref &callbackIn, const napi_value &result)
{
    LOGD("enter");
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);

    napi_value callback = nullptr;
    napi_value resultout = nullptr;
    napi_get_reference_value(env, callbackIn, &callback);
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, undefined, callback, ARGS_ONE, &result, &resultout));
    LOGD("end");
}

void Common::SetPromise(const napi_env &env,
    const napi_deferred &deferred, const int32_t &errorCode, const napi_value &result, bool newType)
{
    LOGD("enter");
    if (errorCode == ERR_OK) {
        napi_resolve_deferred(env, deferred, result);
    } else {
        napi_reject_deferred(env, deferred, CreateErrorValue(env, errorCode, newType));
    }
    LOGD("end");
}

napi_value Common::JSParaError(const napi_env &env, const napi_ref &callback)
{
    if (callback) {
        return Common::NapiGetNull(env);
    }
    napi_value promise = nullptr;
    napi_deferred deferred = nullptr;
    napi_create_promise(env, &deferred, &promise);
    SetPromise(env, deferred, ERROR, Common::NapiGetNull(env), false);
    return promise;
}

napi_value Common::ParseParaOnlyCallback(const napi_env &env, const napi_callback_info &info, napi_ref &callback)
{
    LOGD("enter");

    size_t argc = ONLY_CALLBACK_MAX_PARA;
    napi_value argv[ONLY_CALLBACK_MAX_PARA] = {nullptr};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    if (argc < ONLY_CALLBACK_MIN_PARA) {
        LOGE("Wrong number of arguments");
        Common::NapiThrow(env, ERROR_PARAM_INVALID, MANDATORY_PARAMETER_ARE_LEFT_UNSPECIFIED);
        return nullptr;
    }

    // argv[0]:callback
    napi_valuetype valuetype = napi_undefined;
    if (argc >= ONLY_CALLBACK_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &valuetype));
        if (valuetype != napi_function) {
            LOGE("Callback is not function excute promise.");
            return Common::NapiGetNull(env);
        }
        napi_create_reference(env, argv[PARAM0], 1, &callback);
    }

    return Common::NapiGetNull(env);
}

void Common::CreateReturnValue(const napi_env &env, const CallbackPromiseInfo &info, const napi_value &result)
{
    LOGD("enter errorCode=%{public}d", info.errorCode);
    int32_t errorCode = info.errorCode == ERR_OK ? ERR_OK : ErrorToExternal(info.errorCode);
    if (info.isCallback) {
        SetCallback(env, info.callback, errorCode, result, true);
    } else {
        SetPromise(env, info.deferred, errorCode, result, true);
    }
    LOGD("end");
}

int32_t Common::ErrorToExternal(uint32_t errCode)
{
    static std::vector<std::pair<uint32_t, int32_t>> errorsConvert = {
        {ERR_ANS_PERMISSION_DENIED, ERROR_PERMISSION_DENIED},
        {ERR_ANS_NON_SYSTEM_APP, ERROR_NOT_SYSTEM_APP},
        {ERR_ANS_NOT_SYSTEM_SERVICE, ERROR_NOT_SYSTEM_APP},
        {ERR_ANS_INVALID_PARAM, ERROR_PARAM_INVALID},
        {ERR_ANS_INVALID_UID, ERROR_PARAM_INVALID},
        {ERR_ANS_ICON_OVER_SIZE, ERROR_PARAM_INVALID},
        {ERR_ANS_PICTURE_OVER_SIZE, ERROR_PARAM_INVALID},
        {ERR_ANS_PUSH_CHECK_EXTRAINFO_INVALID, ERROR_PARAM_INVALID},
        {ERR_ANS_NO_MEMORY, ERROR_NO_MEMORY},
        {ERR_ANS_TASK_ERR, ERROR_INTERNAL_ERROR},
        {ERR_ANS_PARCELABLE_FAILED, ERROR_IPC_ERROR},
        {ERR_ANS_TRANSACT_FAILED, ERROR_IPC_ERROR},
        {ERR_ANS_REMOTE_DEAD, ERROR_IPC_ERROR},
        {ERR_ANS_SERVICE_NOT_READY, ERROR_SERVICE_CONNECT_ERROR},
        {ERR_ANS_SERVICE_NOT_CONNECTED, ERROR_SERVICE_CONNECT_ERROR},
        {ERR_ANS_NOT_ALLOWED, ERROR_NOTIFICATION_CLOSED},
        {ERR_ANS_PREFERENCES_NOTIFICATION_SLOT_ENABLED, ERROR_SLOT_CLOSED},
        {ERR_ANS_NOTIFICATION_IS_UNREMOVABLE, ERROR_NOTIFICATION_UNREMOVABLE},
        {ERR_ANS_NOTIFICATION_NOT_EXISTS, ERROR_NOTIFICATION_NOT_EXIST},
        {ERR_ANS_GET_ACTIVE_USER_FAILED, ERROR_USER_NOT_EXIST},
        {ERR_ANS_INVALID_PID, ERROR_BUNDLE_NOT_FOUND},
        {ERR_ANS_INVALID_BUNDLE, ERROR_BUNDLE_NOT_FOUND},
        {ERR_ANS_OVER_MAX_ACTIVE_PERSECOND, ERROR_OVER_MAX_NUM_PER_SECOND},
        {ERR_ANS_OVER_MAX_UPDATE_PERSECOND, ERROR_OVER_MAX_NUM_PER_SECOND},
        {ERR_ANS_DISTRIBUTED_OPERATION_FAILED, ERROR_DISTRIBUTED_OPERATION_FAILED},
        {ERR_ANS_DISTRIBUTED_GET_INFO_FAILED, ERROR_DISTRIBUTED_OPERATION_FAILED},
        {ERR_ANS_PREFERENCES_NOTIFICATION_READ_TEMPLATE_CONFIG_FAILED, ERROR_READ_TEMPLATE_CONFIG_FAILED},
        {ERR_ANS_REPEAT_CREATE, ERROR_REPEAT_SET},
        {ERR_ANS_END_NOTIFICATION, ERROR_REPEAT_SET},
        {ERR_ANS_EXPIRED_NOTIFICATION, ERROR_EXPIRED_NOTIFICATION},
        {ERR_ANS_PUSH_CHECK_FAILED, ERROR_NO_RIGHT},
        {ERR_ANS_PUSH_CHECK_UNREGISTERED, ERROR_NO_RIGHT},
        {ERR_ANS_PUSH_CHECK_NETWORK_UNREACHABLE, ERROR_NETWORK_UNREACHABLE},
        {ERR_ANS_NO_AGENT_SETTING, ERROR_NO_AGENT_SETTING}
    };

    int32_t ExternalCode = ERROR_INTERNAL_ERROR;
    for (const auto &errorConvert : errorsConvert) {
        if (errCode == errorConvert.first) {
            ExternalCode = errorConvert.second;
            break;
        }
    }

    LOGI("internal errorCode[%{public}u] to external errorCode[%{public}d]", errCode, ExternalCode);
    return ExternalCode;
}
}  // namespace NotificationNapi
}  // namespace OHOS
