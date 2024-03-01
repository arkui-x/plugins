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

#include <new>

#include "wifi_napi_device.h"
#include "inner_api/plugin_utils_napi.h"
#include "log.h"
#include "wifi_callback.h"
#include "wifi_device.h"
#include "wifi_napi_errcode.h"

namespace OHOS::Plugin {
std::shared_ptr<WifiDevice> wifiDevicePtr = WifiDevice::GetInstance(0);

static std::set<std::string> g_supportEventList = {
    EVENT_STA_POWER_STATE_CHANGE,
    EVENT_STA_CONN_STATE_CHANGE,
};

std::map<std::string, std::int32_t> g_EventSysCapMap = {
    { EVENT_STA_POWER_STATE_CHANGE, SYSCAP_WIFI_STA },
    { EVENT_STA_CONN_STATE_CHANGE, SYSCAP_WIFI_STA },
};

static void LinkedInfoToJs(const napi_env& env, WifiLinkedInfo& linkedInfo, napi_value& result)
{
    SetValueUtf8String(env, "ssid", linkedInfo.ssid.c_str(), result);
    SetValueUtf8String(env, "bssid", linkedInfo.bssid.c_str(), result);
    SetValueInt32(env, "networkId", linkedInfo.networkId, result);
    SetValueInt32(env, "rssi", linkedInfo.rssi, result);
    SetValueInt32(env, "band", linkedInfo.band, result);
    SetValueInt32(env, "linkSpeed", linkedInfo.linkSpeed, result);
    SetValueInt32(env, "frequency", linkedInfo.frequency, result);
    SetValueBool(env, "isHidden", linkedInfo.ifHiddenSSID, result);
    SetValueBool(env, "isRestricted", linkedInfo.isDataRestricted, result);
    SetValueInt32(env, "chload", linkedInfo.chload, result);
    SetValueInt32(env, "snr", linkedInfo.snr, result);
    SetValueUtf8String(env, "macAddress", linkedInfo.macAddress.c_str(), result);
    SetValueInt32(env, "macType", linkedInfo.macType, result);
    SetValueUnsignedInt32(env, "ipAddress", linkedInfo.ipAddress, result);
    SetValueInt32(env, "suppState", static_cast<int>(linkedInfo.supplicantState), result);
    SetValueInt32(env, "connState", static_cast<int>(linkedInfo.connState), result);
    SetValueInt32(env, "WifiStandard", static_cast<int>(linkedInfo.wifiStandard), result);
    SetValueInt32(env, "maxSupportedRxLinkSpeed", static_cast<int>(linkedInfo.maxSupportedRxLinkSpeed), result);
    SetValueInt32(env, "maxSupportedTxLinkSpeed", static_cast<int>(linkedInfo.maxSupportedTxLinkSpeed), result);
    SetValueInt32(env, "rxLinkSpeed", static_cast<int>(linkedInfo.rxLinkSpeed), result);
    SetValueInt32(env, "txLinkSpeed", static_cast<int>(linkedInfo.txLinkSpeed), result);
    SetValueInt32(env, "channelWidth", static_cast<int>(linkedInfo.channelWidth), result);
}

napi_value GetLinkedInfo(napi_env env, napi_callback_info info)
{
    TRACE_FUNC_CALL;
    size_t argc = 2;
    napi_value argv[argc];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    WIFI_NAPI_ASSERT(env, wifiDevicePtr != nullptr, WIFI_OPT_FAILED, SYSCAP_WIFI_STA);

    LinkedInfoAsyncContext* asyncContext = new LinkedInfoAsyncContext(env);
    WIFI_NAPI_ASSERT(env, asyncContext != nullptr, WIFI_OPT_FAILED, SYSCAP_WIFI_STA);
    napi_create_string_latin1(env, "getLinkedInfo", NAPI_AUTO_LENGTH, &asyncContext->resourceName);

    asyncContext->executeFunc = [&](void* data) -> void {
        LinkedInfoAsyncContext* context = static_cast<LinkedInfoAsyncContext*>(data);
        TRACE_FUNC_CALL_NAME("wifiDevicePtr->GetLinkedInfo");
        context->errorCode = wifiDevicePtr->GetLinkedInfo(context->linkedInfo);
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        LinkedInfoAsyncContext* context = static_cast<LinkedInfoAsyncContext*>(data);
        napi_create_object(context->env, &context->result);
        LinkedInfoToJs(context->env, context->linkedInfo, context->result);
        LOGI("Push get linkedInfo result to client");
    };

    size_t nonCallbackArgNum = 0;
    asyncContext->sysCap = SYSCAP_WIFI_STA;
    return DoAsyncWork(env, asyncContext, argc, argv, nonCallbackArgNum);
}

napi_value IsWifiActive(napi_env env, napi_callback_info info)
{
    TRACE_FUNC_CALL;
    WIFI_NAPI_ASSERT(env, wifiDevicePtr != nullptr, WIFI_OPT_FAILED, SYSCAP_WIFI_STA);
    bool activeStatus = false;
    ErrCode ret = wifiDevicePtr->IsWifiActive(activeStatus);
    if (ret != WIFI_OPT_SUCCESS) {
        LOGE("Get wifi active status fail: %{public}d", ret);
        WIFI_NAPI_ASSERT(env, ret == WIFI_OPT_SUCCESS, ret, SYSCAP_WIFI_STA);
    }
    napi_value result;
    napi_get_boolean(env, activeStatus, &result);
    return result;
}

napi_value IsConnected(napi_env env, napi_callback_info info)
{
    TRACE_FUNC_CALL;
    WIFI_NAPI_ASSERT(env, wifiDevicePtr != nullptr, WIFI_OPT_FAILED, SYSCAP_WIFI_STA);
    bool isConnected = false;
    ErrCode ret = wifiDevicePtr->IsConnected(isConnected);
    if (ret != WIFI_OPT_SUCCESS) {
        LOGE("IsConnected return error: %{public}d", ret);
        WIFI_NAPI_ASSERT(env, ret == WIFI_OPT_SUCCESS, ret, SYSCAP_WIFI_STA);
    }
    napi_value result;
    napi_get_boolean(env, isConnected, &result);
    return result;
}

napi_value On(napi_env env, napi_callback_info cbinfo)
{
    TRACE_FUNC_CALL;
    size_t requireArgc = 2;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = 0;
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr));

    if (argc >= 2) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        if (valuetype != napi_function) {
            LOGI("callback Wrong argument type. Function expected.");
            return nullptr;
        }
    }

    if (requireArgc > argc) {
        LOGI("requireArgc:%{public}zu, argc:%{public}zu", requireArgc, argc);
        WIFI_NAPI_RETURN(env, false, WIFI_OPT_INVALID_PARAM, 0);
    }

    napi_valuetype eventName = napi_undefined;
    napi_typeof(env, argv[0], &eventName);
    if (eventName != napi_string) {
        LOGI("first argv != napi_string");
        WIFI_NAPI_RETURN(env, false, WIFI_OPT_INVALID_PARAM, 0);
    }

    napi_valuetype handler = napi_undefined;
    napi_typeof(env, argv[1], &handler);
    if (handler != napi_function) {
        LOGI("second argv != napi_function");
        WIFI_NAPI_RETURN(env, false, WIFI_OPT_INVALID_PARAM, 0);
    }
    std::string type = PluginUtilsNApi::GetStringFromValueUtf8(env, argv[0]);

    if (type != EVENT_STA_POWER_STATE_CHANGE && type != EVENT_STA_CONN_STATE_CHANGE) {
        LOGI("wifi event type error !");
        return nullptr;
    }

    bool isWifiRegister = WifiCallback::GetInstance().HasWifiRegister(type);
    WifiCallback::GetInstance().RegisterCallback(env, argv[1], type);

    if (!isWifiRegister) {
        ErrCode ret = wifiDevicePtr->On(type);
        if (ret != WIFI_OPT_SUCCESS) {
            LOGE("On return error: %{public}d", ret);
            WIFI_NAPI_ASSERT(env, ret == WIFI_OPT_SUCCESS, ret, SYSCAP_WIFI_STA);
            return nullptr;
        }
    }

    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value Off(napi_env env, napi_callback_info cbinfo)
{
    TRACE_FUNC_CALL;
    size_t requireArgc = 1;
    size_t requireArgcWithCb = 2;
    size_t argc = 2;
    napi_value argv[2] = { 0 };
    napi_value thisVar = 0;
    napi_valuetype valuetype;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr));
    if (argc >= WIFI_NAPI_MAX_PARA) {
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype));
        if (valuetype != napi_function) {
            LOGI("callback Wrong argument type. Function expected.");
            return nullptr;
        }
    }
    if (requireArgc > argc) {
        LOGI("requireArgc:%{public}zu, argc:%{public}zu", requireArgc, argc);
        WIFI_NAPI_RETURN(env, false, WIFI_OPT_INVALID_PARAM, 0);
    }

    napi_valuetype eventName = napi_undefined;
    napi_typeof(env, argv[0], &eventName);
    if (eventName != napi_string) {
        LOGI("first argv != napi_string");
        WIFI_NAPI_RETURN(env, false, WIFI_OPT_INVALID_PARAM, 0);
    }

    napi_valuetype handler = napi_undefined;
    if (argc >= requireArgcWithCb) {
        napi_typeof(env, argv[1], &handler);
        if (handler != napi_function && handler != napi_null) {
            LOGI("second argv != napi_function");
            WIFI_NAPI_RETURN(env, false, WIFI_OPT_INVALID_PARAM, 0);
        }
    }
    std::string type = PluginUtilsNApi::GetStringFromValueUtf8(env, argv[0]);

    if (type != EVENT_STA_POWER_STATE_CHANGE && type != EVENT_STA_CONN_STATE_CHANGE) {
        LOGI("wifi event type error !");
        return nullptr;
    }
    WifiCallback::GetInstance().UnRegisterCallback(env, argv[1], type);
    bool isWifiRegister = WifiCallback::GetInstance().HasWifiRegister(type);

    if (!isWifiRegister) {
        ErrCode ret = wifiDevicePtr->Off(type);
        if (ret != WIFI_OPT_SUCCESS) {
            LOGE("Off return error: %{public}d", ret);
            WIFI_NAPI_ASSERT(env, ret == WIFI_OPT_SUCCESS, ret, SYSCAP_WIFI_STA);
            return nullptr;
        }
    }
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}
} // namespace OHOS::Plugin
