/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "vibrator_napi_utils.h"

#include <string>
#include "hilog/log.h"
#include "securec.h"

#include "miscdevice_log.h"
#include "vibrator_napi_error.h"

#undef LOG_TAG
#define LOG_TAG "VibratorNapiUtils"

namespace OHOS {
namespace Sensors {
namespace {
constexpr int32_t RESULT_LENGTH = 2;
constexpr int32_t PARAMETER_TWO = 2;
} // namespace
AsyncCallbackInfo::~AsyncCallbackInfo()
{
    CALL_LOG_ENTER;
    if (asyncWork != nullptr) {
        MISC_HILOGD("Delete work");
        napi_delete_async_work(env, asyncWork);
    }
    for (int32_t i = 0; i < CALLBACK_NUM; ++i) {
        if (callback[i] != nullptr) {
            MISC_HILOGD("Delete reference, i:%{public}d", i);
            napi_delete_reference(env, callback[i]);
        }
    }
}

bool ClearVibratorPattern(VibratorPattern &vibratorPattern)
{
    CALL_LOG_ENTER;
    int32_t eventSize = vibratorPattern.eventNum;
    if ((eventSize <= 0) || (vibratorPattern.events == nullptr)) {
        MISC_HILOGW("events is not need to free, eventSize:%{public}d", eventSize);
        return false;
    }
    auto events = vibratorPattern.events;
    for (int32_t j = 0; j < eventSize; ++j) {
        if (events[j].points != nullptr) {
            free(events[j].points);
            events[j].points = nullptr;
        }
    }
    free(events);
    events = nullptr;
    return true;
}

bool CreateInt32Property(napi_env env, napi_value &eventObj, const char* name, int32_t value)
{
    CALL_LOG_ENTER;
    napi_value propValue = nullptr;
    napi_status status = napi_create_int32(env, value, &propValue);
    if (status != napi_ok) {
        MISC_HILOGE("napi_create_int32 fail");
        return false;
    }
    CHKCF((napi_set_named_property(env, eventObj, name, propValue) == napi_ok), "napi_set_named_property fail");
    return true;
}

bool CreateInt64Property(napi_env env, napi_value &eventObj, const char* name, int64_t value)
{
    CALL_LOG_ENTER;
    napi_value propValue = nullptr;
    napi_status status = napi_create_int64(env, value, &propValue);
    if (status != napi_ok) {
        MISC_HILOGE("napi_create_int64 fail");
        return false;
    }
    CHKCF((napi_set_named_property(env, eventObj, name, propValue) == napi_ok), "napi_set_named_property fail");
    return true;
}

bool CreateStringProperty(napi_env env, napi_value &eventObj, const char* name,
    const char* value, int32_t valueLength)
{
    CALL_LOG_ENTER;
    napi_value propValue = nullptr;
    napi_status status = napi_create_string_utf8(env, value, valueLength, &propValue);
    if (status != napi_ok) {
        MISC_HILOGE("napi_create_string_utf8 fail");
        return false;
    }
    CHKCF((napi_set_named_property(env, eventObj, name, propValue) == napi_ok), "napi_set_named_property fail");
    return true;
}

bool CreateBooleanProperty(napi_env env, napi_value &eventObj, const char* name, bool value)
{
    CALL_LOG_ENTER;
    napi_value propValue = nullptr;
    napi_status status = napi_get_boolean(env, value, &propValue);
    if (status != napi_ok) {
        MISC_HILOGE("napi_get_boolean fail");
        return false;
    }
    CHKCF((napi_set_named_property(env, eventObj, name, propValue) == napi_ok), "napi_set_named_property fail");
    return true;
}

bool IsSameValue(const napi_env &env, const napi_value &lhs, const napi_value &rhs)
{
    CALL_LOG_ENTER;
    bool result = false;
    CHKNRF(env, napi_strict_equals(env, lhs, rhs, &result), "napi_strict_equals");
    return result;
}

bool IsMatchArrayType(const napi_env &env, const napi_value &value)
{
    bool result = false;
    CHKCF((napi_is_array(env, value, &result) == napi_ok), "napi_is_array fail");
    return result;
}

bool IsMatchType(const napi_env &env, const napi_value &value, const napi_valuetype &type)
{
    napi_valuetype paramType = napi_undefined;
    napi_status ret = napi_typeof(env, value, &paramType);
    if ((ret != napi_ok) || (paramType != type)) {
        MISC_HILOGE("Type mismatch");
        return false;
    }
    return true;
}

bool GetNapiInt32(const napi_env &env, const int32_t value, napi_value &result)
{
    CALL_LOG_ENTER;
    napi_status ret = napi_create_int32(env, value, &result);
    if (ret != napi_ok) {
        MISC_HILOGE("GetNapiInt32 failed");
        return false;
    }
    return true;
}

bool GetInt32Value(const napi_env &env, const napi_value &value, int32_t &result)
{
    CALL_LOG_ENTER;
    napi_valuetype valuetype = napi_undefined;
    CHKCF(napi_typeof(env, value, &valuetype) == napi_ok, "napi_typeof failed");
    CHKCF((valuetype == napi_number), "Wrong argument type. Number expected");
    CHKCF(napi_get_value_int32(env, value, &result) == napi_ok, "napi_get_value_int32 failed");
    return true;
}

bool GetDoubleValue(const napi_env &env, const napi_value &value, double &result)
{
    CALL_LOG_ENTER;
    napi_valuetype valuetype = napi_undefined;
    CHKCF(napi_typeof(env, value, &valuetype) == napi_ok, "napi_typeof failed");
    CHKCF((valuetype == napi_number), "Wrong argument type. Number expected");
    CHKCF(napi_get_value_double(env, value, &result) == napi_ok, "napi_get_value_double failed");
    return true;
}

bool GetInt64Value(const napi_env &env, const napi_value &value, int64_t &result)
{
    CALL_LOG_ENTER;
    napi_valuetype valuetype = napi_undefined;
    CHKCF(napi_typeof(env, value, &valuetype) == napi_ok, "napi_typeof failed");
    CHKCF((valuetype == napi_number), "Wrong argument type. Number expected");
    CHKCF(napi_get_value_int64(env, value, &result) == napi_ok, "napi_get_value_int64 failed");
    return true;
}

bool GetBoolValue(const napi_env &env, const napi_value &value, bool &result)
{
    CALL_LOG_ENTER;
    napi_valuetype valuetype = napi_undefined;
    CHKCF(napi_typeof(env, value, &valuetype) == napi_ok, "napi_typeof failed");
    CHKCF((valuetype == napi_boolean), "Wrong argument type. bool expected");
    CHKCF(napi_get_value_bool(env, value, &result) == napi_ok, "napi_get_value_bool failed");
    return true;
}

bool GetStringValue(const napi_env &env, const napi_value &value, string &result)
{
    CALL_LOG_ENTER;
    napi_valuetype valuetype = napi_undefined;
    napi_status ret = napi_typeof(env, value, &valuetype);
    if (ret != napi_ok) {
        MISC_HILOGE("napi_typeof failed");
        return false;
    }
    CHKCF((valuetype == napi_string), "Wrong argument type. String or function expected");
    size_t bufLength = 0;
    ret = napi_get_value_string_utf8(env, value, nullptr, 0, &bufLength);
    if (ret != napi_ok) {
        MISC_HILOGE("napi_get_value_string_utf8 failed");
        return false;
    }
    bufLength = bufLength > STRING_LENGTH_MAX ? STRING_LENGTH_MAX : bufLength;
    char str[STRING_LENGTH_MAX + 1] = {0};
    size_t strLen = 0;
    ret = napi_get_value_string_utf8(env, value, str, bufLength + 1, &strLen);
    if (ret != napi_ok) {
        MISC_HILOGE("napi_get_value_string_utf8 failed");
        return false;
    }
    result = str;
    return true;
}

bool GetPropertyItem(const napi_env &env, const napi_value &value, const std::string &type, napi_value &item)
{
    bool exist = false;
    napi_status status = napi_has_named_property(env, value, type.c_str(), &exist);
    if ((status != napi_ok) || (!exist)) {
        MISC_HILOGE("Can not find %{public}s property", type.c_str());
        return false;
    }
    CHKCF((napi_get_named_property(env, value, type.c_str(), &item) == napi_ok), "napi get property fail");
    return true;
}

bool GetPropertyString(const napi_env &env, const napi_value &value, const std::string &type, std::string &result)
{
    bool exist = false;
    napi_status status = napi_has_named_property(env, value, type.c_str(), &exist);
    if ((status != napi_ok) || (!exist)) {
        MISC_HILOGE("Can not find %{public}s property", type.c_str());
        return false;
    }

    napi_value item = nullptr;
    CHKCF((napi_get_named_property(env, value, type.c_str(), &item) == napi_ok), "napi get property fail");
    if (!GetStringValue(env, item, result)) {
        return false;
    }
    return true;
}

bool GetPropertyInt32(const napi_env &env, const napi_value &value, const std::string &type, int32_t &result)
{
    napi_value item = nullptr;
    bool exist = false;
    napi_status status = napi_has_named_property(env, value, type.c_str(), &exist);
    if (status != napi_ok || !exist) {
        MISC_HILOGD("Can not find %{public}s property", type.c_str());
        return false;
    }
    CHKCF((napi_get_named_property(env, value, type.c_str(), &item) == napi_ok), "napi get property fail");
    if (!GetInt32Value(env, item, result)) {
        MISC_HILOGE("Get int value fail");
        return false;
    }
    return true;
}

bool GetPropertyInt64(const napi_env &env, const napi_value &value, const std::string &type, int64_t &result)
{
    napi_value item = nullptr;
    bool exist = false;
    napi_status status = napi_has_named_property(env, value, type.c_str(), &exist);
    if (status != napi_ok || !exist) {
        MISC_HILOGE("Can not find %{public}s property", type.c_str());
        return false;
    }
    CHKCF((napi_get_named_property(env, value, type.c_str(), &item) == napi_ok), "napi get property fail");
    if (!GetInt64Value(env, item, result)) {
        MISC_HILOGE("Get int value fail");
        return false;
    }
    return true;
}

bool GetPropertyBool(const napi_env &env, const napi_value &value, const std::string &type, bool &result)
{
    bool exist = false;
    napi_status status = napi_has_named_property(env, value, type.c_str(), &exist);
    if ((status != napi_ok) || (!exist)) {
        MISC_HILOGD("Can not find %{public}s property", type.c_str());
        return false;
    }
    napi_value item = nullptr;
    CHKCF((napi_get_named_property(env, value, type.c_str(), &item) == napi_ok), "napi get property fail");
    if (!GetBoolValue(env, item, result)) {
        MISC_HILOGE("Get bool value fail");
        return false;
    }
    return true;
}

std::map<int32_t, ConstructResultFunc> g_convertFuncList = {
    {COMMON_CALLBACK, ConstructCommonResult},
    {IS_SUPPORT_EFFECT_CALLBACK, ConstructIsSupportEffectResult},
    {VIBRATOR_STATE_CHANGE, ConstructVibratorPlugInfoResult},
};

bool ConvertErrorToResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo, napi_value &result)
{
    CHKPF(asyncCallbackInfo);
    int32_t code = asyncCallbackInfo->error.code;
    auto msg = GetNapiError(code);
    if (!msg) {
        MISC_HILOGE("ErrCode:%{public}d is invalid", code);
        return false;
    }
    result = CreateBusinessError(env, code, msg.value());
    return (result != nullptr);
}

bool ConstructCommonResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo, napi_value result[],
    int32_t length)
{
    CHKPF(asyncCallbackInfo);
    CHKCF(length == RESULT_LENGTH, "Array length is different");
    if (asyncCallbackInfo->error.code != SUCCESS) {
        CHKCF(ConvertErrorToResult(env, asyncCallbackInfo, result[0]), "Create napi err fail in async work");
        CHKCF((napi_get_undefined(env, &result[1]) == napi_ok), "napi_get_undefined fail");
    } else {
        CHKCF((napi_get_undefined(env, &result[0]) == napi_ok), "napi_get_undefined fail");
        CHKCF((napi_get_undefined(env, &result[1]) == napi_ok), "napi_get_undefined fail");
    }
    return true;
}

bool ConstructIsSupportEffectResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo,
    napi_value result[], int32_t length)
{
    CHKPF(asyncCallbackInfo);
    CHKCF(length == RESULT_LENGTH, "Array length is different");
    if (asyncCallbackInfo->error.code != SUCCESS) {
        CHKCF(ConvertErrorToResult(env, asyncCallbackInfo, result[0]), "Create napi err fail in async work");
        CHKCF((napi_get_undefined(env, &result[1]) == napi_ok), "napi_get_undefined fail");
    } else {
        CHKCF((napi_get_undefined(env, &result[0]) == napi_ok), "napi_get_undefined fail");
        CHKCF((napi_get_boolean(env, asyncCallbackInfo->isSupportEffect, &result[1]) == napi_ok),
            "napi_get_boolean fail");
    }
    return true;
}

napi_value ConvertToJsVibratorInfo(const napi_env &env, const VibratorInfos &vibratorInfo)
{
    CALL_LOG_ENTER;
    napi_value jsObject = nullptr;
    napi_status status = napi_create_object(env, &jsObject);
    if (status != napi_ok) {
        return jsObject;
    }
    MISC_HILOGD("VibratorInfos: [deviceId = %{public}d, vibratorId = %{public}d, deviceName = %{public}s,\
        isSupportHdHaptic = %{public}s, isLocalVibrator = %{public}s]", vibratorInfo.deviceId,
        vibratorInfo.vibratorId, vibratorInfo.deviceName.c_str(), (vibratorInfo.isSupportHdHaptic ? "true":"false"),
        (vibratorInfo.isLocalVibrator ? "true":"false"));
    if (!CreateInt32Property(env, jsObject, "deviceId", vibratorInfo.deviceId)) {
        MISC_HILOGE("Create vibratorInfo.deviceId failed");
        return jsObject;
    }
    if (!CreateInt32Property(env, jsObject, "vibratorId", vibratorInfo.vibratorId)) {
        MISC_HILOGE("Create vibratorInfo.vibratorId failed");
        return jsObject;
    }
    if (!CreateStringProperty(env, jsObject, "deviceName", vibratorInfo.deviceName.c_str(),
        vibratorInfo.deviceName.length())) {
            MISC_HILOGE("Create vibratorInfo.deviceName failed");
        return jsObject;
    }
    if (!CreateBooleanProperty(env, jsObject, "isHdHapticSupported", vibratorInfo.isSupportHdHaptic)) {
        MISC_HILOGE("Create vibratorInfo.isSupportHdHaptic failed");
        return jsObject;
    }
    if (!CreateBooleanProperty(env, jsObject, "isLocalVibrator", vibratorInfo.isLocalVibrator)) {
        MISC_HILOGE("Create vibratorInfo.isLocalVibrator failed");
        return jsObject;
    }
    return jsObject;
}

napi_value ConvertToJsEffectInfo(const napi_env &env, const EffectInfo &effectInfo)
{
    CALL_LOG_ENTER;
    napi_value jsObject = nullptr;
    napi_status status = napi_create_object(env, &jsObject);
    if (status != napi_ok) {
        return jsObject;
    }
    if (!CreateBooleanProperty(env, jsObject, "isEffectSupported", effectInfo.isSupportEffect)) {
        MISC_HILOGE("Create effectInfo.isSupportEffect failed");
        return jsObject;
    }
    return jsObject;
}

napi_value ConvertToJsVibratorPlungInfo(const napi_env &env, const VibratorStatusEvent &statusEvent)
{
    CALL_LOG_ENTER;
    MISC_HILOGD("statusEvent: [type = %{public}d, deviceId = %{public}d]", statusEvent.type, statusEvent.deviceId);
    bool plugFlag = false;
    napi_value jsObject = nullptr;
    napi_status status = napi_create_object(env, &jsObject);
    if (status != napi_ok) {
        napi_throw_error(env, nullptr, "Failed to create JS object");
        return jsObject;
    }
    if (statusEvent.type == PLUG_STATE_EVENT_PLUG_IN) {
        plugFlag = true;
    } else if (statusEvent.type == PLUG_STATE_EVENT_PLUG_OUT) {
        plugFlag = false;
    }
    MISC_HILOGD("plugFlag = %{public}s", plugFlag ? "true" : "false");
    if (!CreateBooleanProperty(env, jsObject, "isVibratorOnline", plugFlag)) {
        MISC_HILOGE("Create plugFlag failed");
        return jsObject;
    }
    if (!CreateInt32Property(env, jsObject, "deviceId", statusEvent.deviceId)) {
        MISC_HILOGE("Create statusEvent.deviceId failed");
        return jsObject;
    }
    if (!CreateInt64Property(env, jsObject, "timestamp", statusEvent.timestamp)) {
        MISC_HILOGE("Create statusEvent.timestamp failed");
        return jsObject;
    }
    if (!CreateInt32Property(env, jsObject, "vibratorCount", statusEvent.vibratorCnt)) {
        MISC_HILOGE("Create statusEvent.vibratorCnt failed");
        return jsObject;
    }

    return jsObject;
}

bool ConstructVibratorPlugInfoResult(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo,
    napi_value result[], int32_t length)
{
    CALL_LOG_ENTER;
    CHKPF(asyncCallbackInfo);
    CHKCF(length == RESULT_LENGTH, "Array length is different");
    if (asyncCallbackInfo->error.code != SUCCESS) {
        CHKCF(ConvertErrorToResult(env, asyncCallbackInfo, result[0]), "Create napi err fail in async work");
        CHKCF((napi_get_undefined(env, &result[1]) == napi_ok), "napi_get_undefined fail");
    } else {
        napi_value jsDevicePlugInfo = nullptr;
        jsDevicePlugInfo = ConvertToJsVibratorPlungInfo(env, asyncCallbackInfo->statusEvent);
        CHKPF(jsDevicePlugInfo);
        result[0] = jsDevicePlugInfo;
        CHKCF((napi_get_undefined(env, &result[1]) == napi_ok), "napi_get_undefined fail");
    }
    return true;
}

void EmitSystemCallback(const napi_env &env, sptr<AsyncCallbackInfo> asyncCallbackInfo)
{
    CHKPV(asyncCallbackInfo);
    if (asyncCallbackInfo->error.code == SUCCESS) {
        CHKPV(asyncCallbackInfo->callback[0]);
        napi_value callback = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, asyncCallbackInfo->callback[0], &callback));
        napi_value result = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_get_undefined(env, &result));
        napi_value callResult = nullptr;
        NAPI_CALL_RETURN_VOID(env, napi_call_function(env, nullptr, callback, 1, &result, &callResult));
        return;
    }
    CHKPV(asyncCallbackInfo->callback[1]);
    napi_value callback = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_get_reference_value(env, asyncCallbackInfo->callback[1], &callback));
    napi_value result[2] = {0};
    NAPI_CALL_RETURN_VOID(env, napi_create_string_utf8(env, asyncCallbackInfo->error.message.data(),
        NAPI_AUTO_LENGTH, &result[0]));
    NAPI_CALL_RETURN_VOID(env, napi_create_int32(env, asyncCallbackInfo->error.code, &result[1]));
    napi_value callResult = nullptr;
    NAPI_CALL_RETURN_VOID(env, napi_call_function(env, nullptr, callback, 1, result, &callResult));
}

void CompleteCallback(napi_env env, napi_status status, void *data)
{
    CALL_LOG_ENTER;
    sptr<AsyncCallbackInfo> asyncCallbackInfo(static_cast<AsyncCallbackInfo *>(data));
    /**
        * After the asynchronous task is created, the asyncCallbackInfo reference count is reduced
        * to 0 destruction, so you need to add 1 to the asyncCallbackInfo reference count when the
        * asynchronous task is created, and subtract 1 from the reference count after the naked
        * pointer is converted to a pointer when the asynchronous task is executed, the reference
        * count of the smart pointer is guaranteed to be 1.
        */
    asyncCallbackInfo->DecStrongRef(nullptr);
    if (asyncCallbackInfo->callbackType == SYSTEM_VIBRATE_CALLBACK) {
        EmitSystemCallback(env, asyncCallbackInfo);
        return;
    }
    if (asyncCallbackInfo->flag == "preset" && asyncCallbackInfo->info.vibratorPattern.events != nullptr) {
        CHKCV(ClearVibratorPattern(asyncCallbackInfo->info.vibratorPattern), "ClearVibratorPattern fail");
    }
    CHKPV(asyncCallbackInfo->callback[0]);
    napi_value callback = nullptr;
    napi_status ret = napi_get_reference_value(env, asyncCallbackInfo->callback[0], &callback);
    CHKCV((ret == napi_ok), "napi_get_reference_value fail");
    napi_value result[RESULT_LENGTH] = { 0 };
    CHKCV((g_convertFuncList.find(asyncCallbackInfo->callbackType) != g_convertFuncList.end()),
        "Callback type invalid in async work");
    bool state = g_convertFuncList[asyncCallbackInfo->callbackType](env, asyncCallbackInfo, result,
        sizeof(result) / sizeof(napi_value));
    CHKCV(state, "Create napi data fail in async work");
    napi_value callResult = nullptr;
    CHKCV((napi_call_function(env, nullptr, callback, PARAMETER_TWO, result, &callResult) == napi_ok),
        "napi_call_function fail");
}

void ExecuteCallBack(napi_env env, void *data)
{
    CALL_LOG_ENTER;
    sptr<AsyncCallbackInfo> asyncCallbackInfo(static_cast<AsyncCallbackInfo *>(data));
    if (asyncCallbackInfo->flag == "preset") {
        asyncCallbackInfo->error.code =
            PlayPrimitiveEffect(asyncCallbackInfo->info.effectId.c_str(), asyncCallbackInfo->info.intensity);
    }
}

void EmitUvEventLoop(sptr<AsyncCallbackInfo> asyncCallbackInfo)
{
    CHKPV(asyncCallbackInfo);
    asyncCallbackInfo->IncStrongRef(nullptr);
    auto event = asyncCallbackInfo.GetRefPtr();
    auto task = [event]() {
        sptr<AsyncCallbackInfo> asyncCallbackInfo(static_cast<AsyncCallbackInfo *>(event));
        /**
         * After the asynchronous task is created, the asyncCallbackInfo reference count is reduced
         * to 0 destruction, so you need to add 1 to the asyncCallbackInfo reference count when the
         * asynchronous task is created, and subtract 1 from the reference count after the naked
         * pointer is converted to a pointer when the asynchronous task is executed, the reference
         * count of the smart pointer is guaranteed to be 1.
         */
        asyncCallbackInfo->DecStrongRef(nullptr);
        napi_env env = asyncCallbackInfo->env;
        napi_value callback = nullptr;
        if (napi_get_reference_value(env, asyncCallbackInfo->callback[0], &callback) != napi_ok) {
            MISC_HILOGE("napi_get_reference_value fail");
            napi_throw_error(env, nullptr, "napi_get_reference_value fail");
            return;
        }
        napi_value callResult = nullptr;
        napi_value result[RESULT_LENGTH] = {0};
        if (!(g_convertFuncList.find(asyncCallbackInfo->callbackType) != g_convertFuncList.end())) {
            MISC_HILOGE("asyncCallbackInfo type is invalid");
            napi_throw_error(env, nullptr, "asyncCallbackInfo type is invalid");
            return;
        }
        bool state = g_convertFuncList[asyncCallbackInfo->callbackType](env, asyncCallbackInfo, result,
            sizeof(result) / sizeof(napi_value));
        CHKCV(state, "Create napi data fail in async work");
        if (napi_call_function(env, nullptr, callback, PARAMETER_TWO, result, &callResult) != napi_ok) {
            MISC_HILOGE("napi_call_function callback fail");
            napi_throw_error(env, nullptr, "napi_call_function callback fail");
            return;
        }
    };
    auto ret = napi_send_event(asyncCallbackInfo->env, task, napi_eprio_immediate);
    if (ret != napi_ok) {
        MISC_HILOGE("Failed to SendEvent, ret:%{public}d", ret);
        asyncCallbackInfo->DecStrongRef(nullptr);
    }
}

void DeleteWork(uv_work_t *work)
{
    CHKPV(work);
    delete work;
    work = nullptr;
}

void EmitAsyncCallbackWork(sptr<AsyncCallbackInfo> asyncCallbackInfo)
{
    CALL_LOG_ENTER;
    CHKPV(asyncCallbackInfo);
    CHKPV(asyncCallbackInfo->env);
    napi_env env = asyncCallbackInfo->env;
    napi_value resourceName = nullptr;
    napi_status ret = napi_create_string_latin1(env, "AsyncCallback", NAPI_AUTO_LENGTH, &resourceName);
    CHKCV((ret == napi_ok), "napi_create_string_latin1 fail");
    asyncCallbackInfo->IncStrongRef(nullptr);
    napi_status status = napi_create_async_work(
        env, nullptr, resourceName, ExecuteCallBack, CompleteCallback,
        asyncCallbackInfo.GetRefPtr(), &asyncCallbackInfo->asyncWork);
    if (status != napi_ok
        || napi_queue_async_work_with_qos(
            asyncCallbackInfo->env, asyncCallbackInfo->asyncWork, napi_qos_user_initiated) != napi_ok) {
        MISC_HILOGE("Create async work fail");
        asyncCallbackInfo->DecStrongRef(nullptr);
    }
}

void EmitPromiseWork(sptr<AsyncCallbackInfo> asyncCallbackInfo)
{
    CALL_LOG_ENTER;
    CHKPV(asyncCallbackInfo);
    CHKPV(asyncCallbackInfo->env);
    napi_value resourceName = nullptr;
    napi_env env = asyncCallbackInfo->env;
    napi_status ret = napi_create_string_latin1(env, "Promise", NAPI_AUTO_LENGTH, &resourceName);
    CHKCV((ret == napi_ok), "napi_create_string_latin1 fail");
    // Make the reference count of asyncCallbackInfo add 1, and the function exits the non-destructor
    asyncCallbackInfo->IncStrongRef(nullptr);
    napi_status status = napi_create_async_work(
        env, nullptr, resourceName, ExecuteCallBack,
        [](napi_env env, napi_status status, void *data) {
            CALL_LOG_ENTER;
            sptr<AsyncCallbackInfo> asyncCallbackInfo(static_cast<AsyncCallbackInfo *>(data));
            /**
             * After the asynchronous task is created, the asyncCallbackInfo reference count is reduced
             * to 0 destruction, so you need to add 1 to the asyncCallbackInfo reference count when the
             * asynchronous task is created, and subtract 1 from the reference count after the naked
             * pointer is converted to a pointer when the asynchronous task is executed, the reference
             * count of the smart pointer is guaranteed to be 1.
             */
            asyncCallbackInfo->DecStrongRef(nullptr);
            CHKPV(asyncCallbackInfo->deferred);
            if (asyncCallbackInfo->callbackType == SYSTEM_VIBRATE_CALLBACK) {
                EmitSystemCallback(env, asyncCallbackInfo);
                return;
            }
            napi_value result[RESULT_LENGTH] = { 0 };
            CHKCV((g_convertFuncList.find(asyncCallbackInfo->callbackType) != g_convertFuncList.end()),
                "Callback type invalid in promise");
            bool ret = g_convertFuncList[asyncCallbackInfo->callbackType](env, asyncCallbackInfo, result,
                sizeof(result) / sizeof(napi_value));
            CHKCV(ret, "Callback type invalid in promise");
            if (asyncCallbackInfo->error.code != SUCCESS) {
                CHKCV((napi_reject_deferred(env, asyncCallbackInfo->deferred, result[0]) == napi_ok),
                    "napi_reject_deferred fail");
            } else {
                CHKCV((napi_resolve_deferred(env, asyncCallbackInfo->deferred, result[1]) == napi_ok),
                    "napi_resolve_deferred fail");
            }
        }, asyncCallbackInfo.GetRefPtr(), &asyncCallbackInfo->asyncWork);
    if (status != napi_ok
        || napi_queue_async_work_with_qos(
            asyncCallbackInfo->env, asyncCallbackInfo->asyncWork, napi_qos_default) != napi_ok) {
        MISC_HILOGE("Create async work fail");
        asyncCallbackInfo->DecStrongRef(nullptr);
    }
}
} // namespace Sensors
} // namespace OHOS
