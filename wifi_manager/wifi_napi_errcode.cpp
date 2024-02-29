/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include <map>

#include "log.h"
#include "wifi_napi_errcode.h"
#include "wifi_errcode.h"

namespace OHOS::Plugin {
static std::map<int32_t, int32_t> errCodeMap = {
    { ErrCode::WIFI_OPT_SUCCESS, WifiNapiErrCode::WIFI_ERRCODE_SUCCESS },
    { ErrCode::WIFI_OPT_FAILED, WifiNapiErrCode::WIFI_ERRCODE_OPERATION_FAILED },
    { ErrCode::WIFI_OPT_NOT_SUPPORTED, WifiNapiErrCode::WIFI_ERRCODE_NOT_SUPPORTED },
    { ErrCode::WIFI_OPT_INVALID_PARAM, WifiNapiErrCode::WIFI_ERRCODE_INVALID_PARAM },
    { ErrCode::WIFI_OPT_FORBID_AIRPLANE, WifiNapiErrCode::WIFI_ERRCODE_OPERATION_FAILED },
    { ErrCode::WIFI_OPT_FORBID_POWSAVING, WifiNapiErrCode::WIFI_ERRCODE_OPERATION_FAILED },
    { ErrCode::WIFI_OPT_PERMISSION_DENIED, WifiNapiErrCode::WIFI_ERRCODE_PERMISSION_DENIED },
    { ErrCode::WIFI_OPT_NON_SYSTEMAPP, WifiNapiErrCode::WIFI_ERRCODE_NON_SYSTEMAPP },
    { ErrCode::WIFI_OPT_OPEN_FAIL_WHEN_CLOSING, WifiNapiErrCode::WIFI_ERRCODE_OPEN_FAIL_WHEN_CLOSING },
    { ErrCode::WIFI_OPT_OPEN_SUCC_WHEN_OPENED, WifiNapiErrCode::WIFI_ERRCODE_CLOSE_FAIL_WHEN_OPENING },
    { ErrCode::WIFI_OPT_CLOSE_FAIL_WHEN_OPENING, WifiNapiErrCode::WIFI_ERRCODE_CLOSE_FAIL_WHEN_OPENING },
    { ErrCode::WIFI_OPT_CLOSE_SUCC_WHEN_CLOSED, WifiNapiErrCode::WIFI_ERRCODE_OPERATION_FAILED },
    { ErrCode::WIFI_OPT_STA_NOT_OPENED, WifiNapiErrCode::WIFI_ERRCODE_WIFI_NOT_OPENED },
    { ErrCode::WIFI_OPT_SCAN_NOT_OPENED, WifiNapiErrCode::WIFI_ERRCODE_OPERATION_FAILED },
    { ErrCode::WIFI_OPT_AP_NOT_OPENED, WifiNapiErrCode::WIFI_ERRCODE_OPERATION_FAILED },
    { ErrCode::WIFI_OPT_INVALID_CONFIG, WifiNapiErrCode::WIFI_ERRCODE_OPERATION_FAILED },
    { ErrCode::WIFI_OPT_P2P_NOT_OPENED, WifiNapiErrCode::WIFI_ERRCODE_WIFI_NOT_OPENED },
    { ErrCode::WIFI_OPT_P2P_MAC_NOT_FOUND, WifiNapiErrCode::WIFI_ERRCODE_OPERATION_FAILED },
    { ErrCode::WIFI_OPT_P2P_ERR_MAC_FORMAT, WifiNapiErrCode::WIFI_ERRCODE_OPERATION_FAILED },
    { ErrCode::WIFI_OPT_P2P_ERR_INTENT, WifiNapiErrCode::WIFI_ERRCODE_OPERATION_FAILED },
    { ErrCode::WIFI_OPT_P2P_ERR_SIZE_NW_NAME, WifiNapiErrCode::WIFI_ERRCODE_OPERATION_FAILED },
    { ErrCode::WIFI_OPT_MOVING_FREEZE_CTRL, WifiNapiErrCode::WIFI_ERRCODE_OPERATION_FAILED },
};

static std::map<int32_t, std::string> napiErrMsgMap {
    { WifiNapiErrCode::WIFI_ERRCODE_OPERATION_FAILED, "Operation failed." },
    { WifiNapiErrCode::WIFI_ERRCODE_WIFI_NOT_OPENED, "WIFI doesn't open." },
    { WifiNapiErrCode::WIFI_ERRCODE_PERMISSION_DENIED, "Permission denied." },
    { WifiNapiErrCode::WIFI_ERRCODE_NON_SYSTEMAPP, "non-system application." },
    { WifiNapiErrCode::WIFI_ERRCODE_INVALID_PARAM, "Parameter error." },
    { WifiNapiErrCode::WIFI_ERRCODE_NOT_SUPPORTED, "Capability not supported." },
    { WifiNapiErrCode::WIFI_ERRCODE_OPEN_FAIL_WHEN_CLOSING, "Failed for wifi is closing." },
    { WifiNapiErrCode::WIFI_ERRCODE_CLOSE_FAIL_WHEN_OPENING, "Failed for wifi is opening." },
};

static napi_value NapiGetUndefined(const napi_env& env)
{
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    return undefined;
}

static int32_t GetNapiErrCode(const napi_env& env, const int32_t errCodeIn, const int32_t sysCap = 0)
{
    auto iter = errCodeMap.find(errCodeIn);
    if (iter == errCodeMap.end()) {
        return WifiNapiErrCode::WIFI_ERRCODE_OPERATION_FAILED + sysCap;
    }
    if (iter->second == WifiNapiErrCode::WIFI_ERRCODE_PERMISSION_DENIED ||
        iter->second == WifiNapiErrCode::WIFI_ERRCODE_INVALID_PARAM ||
        iter->second == WifiNapiErrCode::WIFI_ERRCODE_NOT_SUPPORTED ||
        iter->second == WifiNapiErrCode::WIFI_ERRCODE_NON_SYSTEMAPP) {
        return iter->second;
    }
    return iter->second + sysCap;
}

static std::string GetNapiErrMsg(const napi_env& env, const int32_t errCode, int sysCap)
{
    if (errCode == ErrCode::WIFI_OPT_SUCCESS) {
        return "";
    }

    int32_t napiErrCode = GetNapiErrCode(env, errCode);
    auto iter = napiErrMsgMap.find(napiErrCode);
    if (iter != napiErrMsgMap.end()) {
        std::string errMessage = "BussinessError ";
        napiErrCode = GetNapiErrCode(env, errCode, sysCap);
        errMessage.append(std::to_string(napiErrCode)).append(": ").append(iter->second);
        return errMessage;
    }
    return "Inner error.";
}

#ifdef ENABLE_NAPI_WIFI_MANAGER
static napi_value NapiGetNull(const napi_env& env)
{
    napi_value res = nullptr;
    napi_get_null(env, &res);
    return res;
}

static napi_value GetCallbackErrorValue(napi_env env, const int32_t errCode, const & std::string errMsg)
{
    napi_value businessError = nullptr;
    napi_value eCode = nullptr;
    napi_value eMsg = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &eCode));
    NAPI_CALL(env, napi_create_string_utf8(env, errMsg.c_str(), errMsg.length(), &eMsg));
    NAPI_CALL(env, napi_create_object(env, &businessError));
    NAPI_CALL(env, napi_set_named_property(env, businessError, "code", eCode));
    NAPI_CALL(env, napi_set_named_property(env, businessError, "message", eMsg));
    return businessError;
}
#endif

void HandleCallbackErrCode(const napi_env& env, const AsyncContext& info)
{
    LOGI("HandleCallbackErrCode, errCode = %{public}d", (int)info.errorCode);
    constexpr int RESULT_PARAMS_NUM = 2;
    napi_value undefine = NapiGetUndefined(env);
    napi_value callback = nullptr;
    napi_value result[RESULT_PARAMS_NUM] = { nullptr };
    result[1] = info.result;
    if (info.errorCode == ErrCode::WIFI_OPT_SUCCESS) {
#ifdef ENABLE_NAPI_WIFI_MANAGER
        result[0] = NapiGetUndefined(env);
#else
        napi_create_uint32(env, info.errorCode, &result[0]);
#endif
        napi_get_reference_value(env, info.callback[0], &callback);
        napi_call_function(env, nullptr, callback, RESULT_PARAMS_NUM, result, &undefine);
    } else {
        napi_ref errCb = info.callback[1];
        if (!errCb) {
            LOGE("Get callback func[1] is null");
            errCb = info.callback[0];
        }
        napi_get_reference_value(env, errCb, &callback);
#ifdef ENABLE_NAPI_WIFI_MANAGER
        std::string errMsg = GetNapiErrMsg(env, info.errorCode, info.sysCap);
        int32_t errCodeInfo = GetNapiErrCode(env, info.errorCode, info.sysCap);
        result[0] = GetCallbackErrorValue(env, errCodeInfo, errMsg);
#else
        napi_create_uint32(env, info.errorCode, &result[0]);
#endif
        napi_call_function(env, nullptr, callback, RESULT_PARAMS_NUM, result, &undefine);
    }
}

void HandlePromiseErrCode(const napi_env& env, const AsyncContext& info)
{
    LOGI("HandlePromiseErrCode, errCode = %{public}d", (int)info.errorCode);
    if (info.errorCode == ErrCode::WIFI_OPT_SUCCESS) {
        napi_resolve_deferred(env, info.deferred, info.result);
    } else {
#ifdef ENABLE_NAPI_WIFI_MANAGER
        int32_t errCodeInfo = GetNapiErrCode(env, info.errorCode, info.sysCap);
        std::string errMsg = GetNapiErrMsg(env, info.errorCode, info.sysCap);
        napi_value businessError = nullptr;
        napi_value eCode = nullptr;
        napi_value eMsg = nullptr;
        napi_value eData = NapiGetNull(env);
        napi_create_int32(env, errCodeInfo, &eCode);
        napi_create_string_utf8(env, errMsg.c_str(), errMsg.length(), &eMsg);
        napi_create_object(env, &businessError);
        napi_set_named_property(env, businessError, "code", eCode);
        napi_set_named_property(env, businessError, "message", eMsg);
        napi_set_named_property(env, businessError, "data", eData);
        napi_reject_deferred(env, info.deferred, businessError);
#else
        napi_reject_deferred(info.env, info.deferred, info.result);
#endif
    }
}

void HandleSyncErrCode(const napi_env& env, int32_t errCode, int32_t sysCap)
{
    LOGI("HandleSyncErrCode, errCode = %{public}d", (int)errCode);
    if (errCode == ErrCode::WIFI_OPT_SUCCESS) {
        return;
    }
    std::string errMsg = GetNapiErrMsg(env, errCode, sysCap);
    int32_t errCodeInfo = GetNapiErrCode(env, errCode, sysCap);
    if (errMsg != "") {
        napi_throw_error(env, std::to_string(errCodeInfo).c_str(), errMsg.c_str());
    }
}
} // namespace OHOS::Plugin
