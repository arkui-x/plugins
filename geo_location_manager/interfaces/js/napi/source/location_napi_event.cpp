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
#include "location_napi_event.h"

#include "callback_manager.h"
#include "common_utils.h"
#include "location_log.h"
#include "location_napi_errcode.h"
#include "country_code_callback_napi.h"
#include "locator.h"
#include "geofence_sdk.h"
#include "napi_util.h"
#ifdef SUPPORT_JSSTACK
#include "xpower_event_js.h"
#endif
#include "location_hiappevent.h"

namespace OHOS {
namespace Location {
CallbackManager<LocationSwitchCallbackNapi> g_switchCallbacks;
CallbackManager<LocatorCallbackNapi> g_locationCallbacks;
CallbackManager<GnssStatusCallbackNapi> g_gnssStatusInfoCallbacks;
CallbackManager<NmeaMessageCallbackNapi> g_nmeaCallbacks;
CallbackManager<CountryCodeCallbackNapi> g_countryCodeCallbacks;
CallbackManager<LocationErrorCallbackNapi> g_locationErrorCallbackHosts;
CallbackManager<BluetoothScanResultCallbackNapi> g_bluetoothScanResultCallbackHosts;
CallbackManager<CachedLocationsCallbackNapi> g_cachedLocationCallbacks;

std::unique_ptr<CachedGnssLocationsRequest> g_cachedRequest = std::make_unique<CachedGnssLocationsRequest>();
auto g_locatorProxy = Locator::GetInstance();
auto g_geofenceProxy = GeofenceManager::GetInstance();
auto g_locationHiAppEvent = LocationHiAppEvent::GetInstance();

std::mutex g_FuncMapMutex;
std::map<std::string, bool(*)(const napi_env &)> g_offAllFuncMap;
std::map<std::string, bool(*)(const napi_env &, const napi_value &)> g_offFuncMap;
std::map<std::string, bool(*)(const napi_env &, const size_t, const napi_value *)> g_onFuncMap;

static constexpr int LASTLOCATION_CACHED_TIME = 10 * 60;
const int MIN_TIMEOUTMS_FOR_LOCATIONONCE = 1000;

void InitOnFuncMap()
{
    std::unique_lock<std::mutex> lock(g_FuncMapMutex);
    if (g_onFuncMap.size() != 0) {
        return;
    }
#ifdef ENABLE_NAPI_MANAGER
    g_onFuncMap.insert(std::make_pair("locationEnabledChange", &OnLocationServiceStateCallback));
    g_onFuncMap.insert(std::make_pair("cachedGnssLocationsChange", &OnCachedGnssLocationsReportingCallback));
    g_onFuncMap.insert(std::make_pair("satelliteStatusChange", &OnGnssStatusChangeCallback));
    g_onFuncMap.insert(std::make_pair("nmeaMessage", &OnNmeaMessageChangeCallback));
    g_onFuncMap.insert(std::make_pair("locationError", &OnLocationErrorCallback));
    g_onFuncMap.insert(std::make_pair("bluetoothScanResultChange", &OnBluetoothScanResultChangeCallback));
#else
    g_onFuncMap.insert(std::make_pair("locationServiceState", &OnLocationServiceStateCallback));
    g_onFuncMap.insert(std::make_pair("cachedGnssLocationsReporting", &OnCachedGnssLocationsReportingCallback));
    g_onFuncMap.insert(std::make_pair("gnssStatusChange", &OnGnssStatusChangeCallback));
    g_onFuncMap.insert(std::make_pair("nmeaMessageChange", &OnNmeaMessageChangeCallback));
#endif
    g_onFuncMap.insert(std::make_pair("locationChange", &OnLocationChangeCallback));
    g_onFuncMap.insert(std::make_pair("countryCodeChange", &OnCountryCodeChangeCallback));
}

void InitOffFuncMap()
{
    std::unique_lock<std::mutex> lock(g_FuncMapMutex);
    if (g_offAllFuncMap.size() != 0 || g_offFuncMap.size() != 0) {
        return;
    }
#ifdef ENABLE_NAPI_MANAGER
    g_offAllFuncMap.insert(std::make_pair("locationEnabledChange", &OffAllLocationServiceStateCallback));
    g_offAllFuncMap.insert(std::make_pair("cachedGnssLocationsChange", &OffAllCachedGnssLocationsReportingCallback));
    g_offAllFuncMap.insert(std::make_pair("satelliteStatusChange", &OffAllGnssStatusChangeCallback));
    g_offAllFuncMap.insert(std::make_pair("nmeaMessage", &OffAllNmeaMessageChangeCallback));
    g_offAllFuncMap.insert(std::make_pair("bluetoothScanResultChange", &OffAllBluetoothScanResultChangeCallback));
#else
    g_offAllFuncMap.insert(std::make_pair("locationServiceState", &OffAllLocationServiceStateCallback));
    g_offAllFuncMap.insert(std::make_pair("cachedGnssLocationsReporting", &OffAllCachedGnssLocationsReportingCallback));
    g_offAllFuncMap.insert(std::make_pair("gnssStatusChange", &OffAllGnssStatusChangeCallback));
    g_offAllFuncMap.insert(std::make_pair("nmeaMessageChange", &OffAllNmeaMessageChangeCallback));
#endif
    g_offAllFuncMap.insert(std::make_pair("locationChange", &OffAllLocationChangeCallback));
    g_offAllFuncMap.insert(std::make_pair("countryCodeChange", &OffAllCountryCodeChangeCallback));

#ifdef ENABLE_NAPI_MANAGER
    g_offFuncMap.insert(std::make_pair("locationEnabledChange", &OffLocationServiceStateCallback));
    g_offFuncMap.insert(std::make_pair("cachedGnssLocationsChange", &OffCachedGnssLocationsReportingCallback));
    g_offFuncMap.insert(std::make_pair("satelliteStatusChange", &OffGnssStatusChangeCallback));
    g_offFuncMap.insert(std::make_pair("nmeaMessage", &OffNmeaMessageChangeCallback));
    g_offFuncMap.insert(std::make_pair("locationError", &OffLocationErrorCallback));
    g_offFuncMap.insert(std::make_pair("bluetoothScanResultChange", &OffBluetoothScanResultChangeCallback));
#else
    g_offFuncMap.insert(std::make_pair("locationServiceState", &OffLocationServiceStateCallback));
    g_offFuncMap.insert(std::make_pair("cachedGnssLocationsReporting", &OffCachedGnssLocationsReportingCallback));
    g_offFuncMap.insert(std::make_pair("gnssStatusChange", &OffGnssStatusChangeCallback));
    g_offFuncMap.insert(std::make_pair("nmeaMessageChange", &OffNmeaMessageChangeCallback));
#endif
    g_offFuncMap.insert(std::make_pair("locationChange", &OffLocationChangeCallback));
    g_offFuncMap.insert(std::make_pair("countryCodeChange", &OffCountryCodeChangeCallback));
}

static void CleanUp(void* data)
{
    auto that = reinterpret_cast<NativeContext*>(data);
    OffAllLocationServiceStateCallback(that->env_);
    OffAllLocationChangeCallback(that->env_);
    OffAllGnssStatusChangeCallback(that->env_);
    OffAllNmeaMessageChangeCallback(that->env_);
    OffAllCachedGnssLocationsReportingCallback(that->env_);
    OffAllCountryCodeChangeCallback(that->env_);
    napi_delete_reference(that->env_, that->ref_);
    that->env_ = nullptr;
    that->ref_ = nullptr;
    delete that;
}

void GenerateExecuteContext(SingleLocationAsyncContext* context)
{
    LBSLOGI(LOCATION_NAPI, "GenerateExecuteContext enter");
    if (context == nullptr) {
        return;
    }
    auto callbackHost = context->callbackHost_;
    bool isNeedLocation = GetIsNeedLocation(context);
    if (callbackHost != nullptr) {
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = g_locatorProxy->StartLocating(context->request_, callbackHost);
        context->errCode = errorCode;
        if (errorCode != ERRCODE_SUCCESS) {
            callbackHost->SetCount(0);
        }
#else
        g_locatorProxy->StartLocating(context->request_, callbackHost);
#endif
        if (context->timeout_ > DEFAULT_TIMEOUT_30S) {
            callbackHost->Wait(DEFAULT_TIMEOUT_30S);
            if (callbackHost->GetSingleLocation() == nullptr) {
                callbackHost->Wait(context->timeout_ - DEFAULT_TIMEOUT_30S);
            }
        } else {
            callbackHost->Wait(context->timeout_);
        }
        if (callbackHost->GetCount() != 0 && callbackHost->GetSingleLocation() == nullptr) {
            std::unique_ptr<Location> location = nullptr;
            location = g_locatorProxy->GetCachedLocation();
            if (NeedReportLastLocation(context->request_, location)) {
                callbackHost->SetSingleLocation(location);
            } else if (isNeedLocation) {
                context->errCode = ERRCODE_LOCATING_FAIL;
                UpdateErrorCodeOfContext(context);
            }
        }
        UpdatePoiErrorCode(isNeedLocation, context);
        callbackHost->SetCount(1);
    }
}

void UpdateErrorCodeOfContext(SingleLocationAsyncContext* context)
{
    if (context == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "null context");
        return;
    }
    auto callbackHost = context->callbackHost_;
    if (callbackHost == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "null callbackHost");
        return;
    }
    int errorType = callbackHost->GetErrorType();
    if (errorType == LocationErrCode::ERRCODE_LOCATING_NETWORK_FAIL ||
        errorType == LocationErrCode::ERRCODE_LOCATING_ACC_FAIL) {
        context->errCode = errorType;
    }
}

bool GetIsNeedLocation(SingleLocationAsyncContext* context)
{
    bool isNeedLocation = true;
    if (context->request_ != nullptr) {
        isNeedLocation = context->request_->GetIsNeedLocation();
    }
    return isNeedLocation;
}

void UpdatePoiErrorCode(bool isNeedLocation, SingleLocationAsyncContext* context)
{
    if (isNeedLocation) {
        return;
    }
    if (context == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "null context");
        return;
    }
    auto callbackHost = context->callbackHost_;
    if (callbackHost == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "null callbackHost");
        return;
    }
    int errorType = callbackHost->GetErrorType();
    if (errorType == LocationErrCode::ERRCODE_PERMISSION_DENIED) {
        context->errCode = errorType;
    }
}

void GenerateCompleteContext(SingleLocationAsyncContext* context)
{
    if (context == nullptr) {
        return;
    }
    bool isNeedLocation = true;
    if (context->request_ != nullptr) {
        isNeedLocation = context->request_->GetIsNeedLocation();
    }
    NAPI_CALL_RETURN_VOID(context->env, napi_create_object(context->env, &context->result[PARAM1]));
    auto callbackHost = context->callbackHost_;
    if (callbackHost != nullptr && callbackHost->GetSingleLocation() != nullptr) {
        std::unique_ptr<Location> location = std::make_unique<Location>(*callbackHost->GetSingleLocation());
        if (isNeedLocation) {
            LocationToJs(context->env, location, context->result[PARAM1]);
        } else {
            PoiToJs(context->env, location, context->result[PARAM1]);
        }
    } else {
        LBSLOGE(LOCATOR_STANDARD, "m_singleLocation is nullptr!");
    }
    if (callbackHost != nullptr && !isNeedLocation && callbackHost->GetSingleLocation() == nullptr) {
        SetUndefinedResult(context->env, context->result[PARAM1]);
    }
    if (context->callbackHost_) {
        context->callbackHost_ = nullptr;
    }
}

SingleLocationAsyncContext* CreateSingleLocationAsyncContext(const napi_env& env,
    std::unique_ptr<RequestConfig>& config, sptr<LocatorCallbackNapi> callback)
{
    LBSLOGI(LOCATION_NAPI, "CreateSingleLocationAsyncContext enter");
    auto asyncContext = new (std::nothrow) SingleLocationAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "GetCurrentLocation",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->timeout_ = config->GetTimeOut();
    asyncContext->callbackHost_ = callback;
    asyncContext->request_ = std::move(config);
    asyncContext->executeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "data is nullptr!");
            return;
        }
        auto context = static_cast<SingleLocationAsyncContext*>(data);
        GenerateExecuteContext(context);
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "data is nullptr!");
            return;
        }
        auto context = static_cast<SingleLocationAsyncContext*>(data);
        GenerateCompleteContext(context);
        LBSLOGI(LOCATOR_STANDARD, "Push single location to client");
    };
    return asyncContext;
}

int GetObjectArgsNum(const napi_env& env, const size_t argc, const napi_value* argv)
{
    napi_valuetype valueType = napi_undefined;
    int objectArgsNum = PARAM0;
    if (argc == PARAM0) {
        objectArgsNum = PARAM0;
    } else if (argc == PARAM1) {
        NAPI_CALL_BASE(env, napi_typeof(env, argv[PARAM0], &valueType), objectArgsNum);
        if (valueType == napi_object) {
            objectArgsNum = PARAM1;
        } else if (valueType == napi_function) {
            objectArgsNum = PARAM0;
        }
    } else if (argc == PARAM2) {
        objectArgsNum = PARAM1;
    } else {
        LBSLOGI(LOCATION_NAPI, "argc of GetCurrentLocation is wrong.");
    }
    return objectArgsNum;
}

std::unique_ptr<RequestConfig> CreateRequestConfig(const napi_env& env,
    const napi_value* argv, const size_t& objectArgsNum)
{
    auto requestConfig = std::make_unique<RequestConfig>();
    if (objectArgsNum > 0) {
        JsObjToCurrentLocationRequest(env, argv[objectArgsNum - 1], requestConfig);
    } else {
        requestConfig->SetPriority(PRIORITY_FAST_FIRST_FIX);
    }
    requestConfig->SetFixNumber(1);
    requestConfig->SetTimeInterval(0);
    return requestConfig;
}

std::unique_ptr<RequestConfig> CreatePoiRequestConfig()
{
    auto requestConfig = std::make_unique<RequestConfig>();
    requestConfig->SetScenario(SCENE_DAILY_LIFE_SERVICE);
    requestConfig->SetFixNumber(1);
    requestConfig->SetTimeInterval(0);
    requestConfig->SetIsNeedLocation(false);
    requestConfig->SetIsNeedPoi(true);
    return requestConfig;
}

sptr<LocatorCallbackNapi> CreateSingleLocationCallbackHost()
{
    auto callbackHost =
        sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
    if (callbackHost) {
        callbackHost->SetFixNumber(1);
    }
    return callbackHost;
}

napi_value RequestLocationOnce(const napi_env& env, const size_t argc, const napi_value* argv)
{
    LBSLOGI(LOCATION_NAPI, "RequestLocationOnce enter");
    size_t objectArgsNum = 0;

    objectArgsNum = static_cast<size_t>(GetObjectArgsNum(env, argc, argv));
    auto requestConfig = CreateRequestConfig(env, argv, objectArgsNum);
    NAPI_ASSERT(env, requestConfig != nullptr, "requestConfig is null.");
    auto singleLocatorCallbackHost = CreateSingleLocationCallbackHost();
    NAPI_ASSERT(env, singleLocatorCallbackHost != nullptr, "callbackHost is null.");

    auto asyncContext = CreateSingleLocationAsyncContext(env, requestConfig, singleLocatorCallbackHost);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

#ifdef ENABLE_NAPI_MANAGER
napi_value RequestLocationOnceV9(const napi_env& env, const size_t argc, const napi_value* argv)
{
    LBSLOGI(LOCATION_NAPI, "RequestLocationOnceV9 enter");
    size_t objectArgsNum = 0;
    objectArgsNum = static_cast<size_t>(GetObjectArgsNum(env, argc, argv));
    auto requestConfig = CreateRequestConfig(env, argv, objectArgsNum);
    if (!IsRequestConfigValid(requestConfig)) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    auto singleLocatorCallbackHost = CreateSingleLocationCallbackHost();
    if (singleLocatorCallbackHost == nullptr) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    singleLocatorCallbackHost->SetLocationPriority(
        requestConfig->IsRequestForAccuracy() ? LOCATION_PRIORITY_ACCURACY : LOCATION_PRIORITY_LOCATING_SPEED);
    auto asyncContext = CreateSingleLocationAsyncContext(env, requestConfig, singleLocatorCallbackHost);
    if (asyncContext == nullptr) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}
#endif

#ifdef ENABLE_NAPI_MANAGER
napi_value RequestPoiInfoOnce(const napi_env& env, const size_t argc, const napi_value* argv)
{
    size_t objectArgsNum = 0;
    objectArgsNum = static_cast<size_t>(GetObjectArgsNum(env, argc, argv));
    auto requestConfig = CreatePoiRequestConfig();
    auto singleLocatorCallbackHost = CreateSingleLocationCallbackHost();
    if (singleLocatorCallbackHost == nullptr) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    singleLocatorCallbackHost->SetLocationPriority(LOCATION_PRIORITY_LOCATING_SPEED);
    auto asyncContext = CreateSingleLocationAsyncContext(env, requestConfig, singleLocatorCallbackHost);
    if (asyncContext == nullptr) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}
#endif

bool OnGnssStatusChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv)
{
    #ifdef ENABLE_NAPI_MANAGER
    if (argc != PARAM2) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
    if (!CheckIfParamIsFunctionType(env, argv[PARAM1])) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT_BASE(env, argc == PARAM2, "number of parameters is wrong", false);
    NAPI_ASSERT_BASE(env, CheckIfParamIsFunctionType(env, argv[PARAM1]),
        "callback should be function, mismatch for param.", false);
#endif
    if (g_gnssStatusInfoCallbacks.IsCallbackInMap(env, argv[PARAM1])) {
        LBSLOGE(LOCATION_NAPI, "This request already exists");
        return false;
    }
    auto gnssCallbackHost =
        sptr<GnssStatusCallbackNapi>(new (std::nothrow) GnssStatusCallbackNapi());
    if (gnssCallbackHost != nullptr) {
        napi_ref handlerRef = nullptr;
        NAPI_CALL_BASE(env, napi_create_reference(env, argv[PARAM1], PARAM1, &handlerRef), false);
#ifdef ENABLE_NAPI_MANAGER
        int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
        LocationErrCode errorCode = SubscribeGnssStatusV9(env, handlerRef, gnssCallbackHost);
        g_locationHiAppEvent->WriteEndEvent(
            beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "satelliteStatusChangeOn");
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        SubscribeGnssStatus(env, handlerRef, gnssCallbackHost);
#endif
        g_gnssStatusInfoCallbacks.AddCallback(env, handlerRef, gnssCallbackHost);
    }
    return true;
}

bool IsCallbackEquals(const napi_env& env, const napi_value& handler, const napi_ref& savedCallback)
{
    napi_value handlerTemp = nullptr;
    if (savedCallback == nullptr || handler == nullptr) {
        return false;
    }
    NAPI_CALL_BASE(env, napi_get_reference_value(env, savedCallback, &handlerTemp), false);
    bool isEqual = false;
    NAPI_CALL_BASE(env, napi_strict_equals(env, handlerTemp, handler, &isEqual), false);
    return isEqual;
}

void SubscribeLocationChange(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<LocatorCallbackNapi>& locatorCallbackHost)
{
    locatorCallbackHost->SetFixNumber(0);
    locatorCallbackHost->SetEnv(env);
    locatorCallbackHost->SetHandleCb(handlerRef);
    auto requestConfig = std::make_unique<RequestConfig>();
    JsObjToLocationRequest(env, object, requestConfig);
    g_locatorProxy->StartLocating(requestConfig, locatorCallbackHost);
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode SubscribeLocationChangeV9(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<LocatorCallbackNapi>& locatorCallbackHost)
{
    locatorCallbackHost->SetFixNumber(0);
    locatorCallbackHost->SetEnv(env);
    locatorCallbackHost->SetHandleCb(handlerRef);
    auto requestConfig = std::make_unique<RequestConfig>();
    JsObjToLocationRequest(env, object, requestConfig);
    if (!IsRequestConfigValid(requestConfig)) {
        return ERRCODE_INVALID_PARAM;
    }
    return g_locatorProxy->StartLocating(requestConfig, locatorCallbackHost);
}
#endif

LocationErrCode UnSubscribeLocationChange(sptr<LocatorCallbackNapi>& locatorCallbackHost)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeLocationChange");
    return g_locatorProxy->StopLocating(locatorCallbackHost);
}

void SubscribeCountryCodeChange(const napi_env& env,
    const napi_ref& handlerRef, sptr<CountryCodeCallbackNapi>& callbackHost)
{
    callbackHost->SetEnv(env);
    callbackHost->SetCallback(handlerRef);
    g_locatorProxy->RegisterCountryCodeCallback(callbackHost);
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode SubscribeCountryCodeChangeV9(const napi_env& env,
    const napi_ref& handlerRef, sptr<CountryCodeCallbackNapi>& callbackHost)
{
    callbackHost->SetEnv(env);
    callbackHost->SetCallback(handlerRef);
    return g_locatorProxy->RegisterCountryCodeCallback(callbackHost);
}
#endif

LocationErrCode UnsubscribeCountryCodeChange(sptr<CountryCodeCallbackNapi>& callbackHost)
{
    LBSLOGI(LOCATION_NAPI, "UnsubscribeCountryCodeChange");
    return g_locatorProxy->UnregisterCountryCodeCallback(callbackHost);
}

void SubscribeLocationServiceState(const napi_env& env,
    const napi_ref& handlerRef, sptr<LocationSwitchCallbackNapi>& switchCallbackHost)
{
    switchCallbackHost->SetEnv(env);
    switchCallbackHost->SetHandleCb(handlerRef);
    g_locatorProxy->RegisterSwitchCallback(switchCallbackHost);
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode SubscribeLocationServiceStateV9(const napi_env& env,
    const napi_ref& handlerRef, sptr<LocationSwitchCallbackNapi>& switchCallbackHost)
{
    switchCallbackHost->SetEnv(env);
    switchCallbackHost->SetHandleCb(handlerRef);
    return g_locatorProxy->RegisterSwitchCallback(switchCallbackHost);
}
#endif

LocationErrCode UnSubscribeLocationServiceState(sptr<LocationSwitchCallbackNapi>& switchCallbackHost)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeLocationServiceState");
    return g_locatorProxy->UnregisterSwitchCallback(switchCallbackHost);
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode SubscribeLocationError(const napi_env& env,
    const napi_ref& handlerRef, sptr<LocationErrorCallbackNapi>& locationErrorCallbackHost)
{
    locationErrorCallbackHost->SetEnv(env);
    locationErrorCallbackHost->SetHandleCb(handlerRef);
    return g_locatorProxy->SubscribeLocationError(locationErrorCallbackHost);
}

LocationErrCode UnSubscribeLocationError(sptr<LocationErrorCallbackNapi>& locationErrorCallbackHost)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeLocationError");
    return g_locatorProxy->UnSubscribeLocationError(locationErrorCallbackHost);
}

LocationErrCode SubscribeBluetoothScanResultChange(const napi_env& env,
    const napi_ref& handlerRef, sptr<BluetoothScanResultCallbackNapi>& bluetoothScanResultCallbackHost)
{
    bluetoothScanResultCallbackHost->SetEnv(env);
    bluetoothScanResultCallbackHost->SetHandleCb(handlerRef);
    return g_locatorProxy->SubscribeBluetoothScanResultChange(bluetoothScanResultCallbackHost);
}

LocationErrCode UnSubscribeBluetoothScanResultChange(
    sptr<BluetoothScanResultCallbackNapi>& bluetoothScanResultCallbackHost)
{
    return g_locatorProxy->UnSubscribeBluetoothScanResultChange(bluetoothScanResultCallbackHost);
}
#endif

void SubscribeCacheLocationChange(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<CachedLocationsCallbackNapi>& cachedCallbackHost)
{
    auto cachedCallback = sptr<ICachedLocationsCallback>(cachedCallbackHost);
    cachedCallbackHost->SetEnv(env);
    cachedCallbackHost->SetHandleCb(handlerRef);
    JsObjToCachedLocationRequest(env, object, g_cachedRequest);
    g_locatorProxy->RegisterCachedLocationCallback(g_cachedRequest, cachedCallback);
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode SubscribeCacheLocationChangeV9(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<CachedLocationsCallbackNapi>& cachedCallbackHost)
{
    auto cachedCallback = sptr<ICachedLocationsCallback>(cachedCallbackHost);
    cachedCallbackHost->SetEnv(env);
    cachedCallbackHost->SetHandleCb(handlerRef);
    JsObjToCachedLocationRequest(env, object, g_cachedRequest);
    g_locatorProxy->RegisterCachedLocationCallbackV9(g_cachedRequest, cachedCallback);
    return ERRCODE_NOT_SUPPORTED;
}
#endif

void UnSubscribeCacheLocationChange(sptr<ICachedLocationsCallback>& callback)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeCacheLocationChange");
    return g_locatorProxy->UnregisterCachedLocationCallback(callback);
}

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode UnSubscribeCacheLocationChangeV9(sptr<ICachedLocationsCallback>& callback)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeCacheLocationChangeV9");
    g_locatorProxy->UnregisterCachedLocationCallbackV9(callback);
    return ERRCODE_NOT_SUPPORTED;
}

LocationErrCode UnSubscribeGnssStatusV9(sptr<GnssStatusCallbackNapi>& gnssStatusCallbackHost)
{
    LBSLOGI(LOCATION_NAPI, "UnSubscribeGnssStatusV9");
    return g_locatorProxy->UnregisterGnssStatusCallbackV9(gnssStatusCallbackHost);
}

LocationErrCode SubscribeNmeaMessageV9(const napi_env& env, const napi_ref& handlerRef,
    sptr<NmeaMessageCallbackNapi>& nmeaMessageCallbackHost)
{
    LocationErrCode errorCode = CheckLocationSwitchEnable();
    nmeaMessageCallbackHost->SetEnv(env);
    nmeaMessageCallbackHost->SetHandleCb(handlerRef);
    return g_locatorProxy->RegisterNmeaMessageCallbackV9(nmeaMessageCallbackHost);
}

LocationErrCode SubscribeGnssStatusV9(const napi_env& env, const napi_ref& handlerRef,
    sptr<GnssStatusCallbackNapi>& gnssStatusCallbackHost)
{
    LBSLOGE(LOCATION_NAPI, "SubscribeGnssStatusV9");
    LocationErrCode errorCode = CheckLocationSwitchEnable();
    if (errorCode != ERRCODE_SUCCESS) {
        return errorCode;
    }
    gnssStatusCallbackHost->SetEnv(env);
    gnssStatusCallbackHost->SetHandleCb(handlerRef);
    return g_locatorProxy->RegisterGnssStatusCallbackV9(gnssStatusCallbackHost);
}

LocationErrCode UnSubscribeNmeaMessageV9(sptr<NmeaMessageCallbackNapi>& nmeaMessageCallbackHost)
{
    LBSLOGE(LOCATION_NAPI, "UnSubscribeNmeaMessageV9");
    return g_locatorProxy->UnregisterNmeaMessageCallbackV9(nmeaMessageCallbackHost);
}

LocationErrCode CheckLocationSwitchEnable()
{
    bool isEnabled = false;
    LocationErrCode errorCode = g_locatorProxy->IsLocationEnabledV9(isEnabled);
    if (errorCode != ERRCODE_SUCCESS) {
        return errorCode;
    }
    if (!isEnabled) {
        return ERRCODE_SWITCH_OFF;
    }
    return ERRCODE_SUCCESS;
}
#endif

bool OnLocationChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv)
{
#ifdef SUPPORT_JSSTACK
    HiviewDFX::ReportXPowerJsStackSysEvent(env, "GNSS_STATE");
#endif
#ifdef ENABLE_NAPI_MANAGER
    if (argc != PARAM3) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
    napi_valuetype valueType;
    NAPI_CALL_BASE(env, napi_typeof(env, argv[PARAM1], &valueType), false);
    if (valueType != napi_object || !CheckIfParamIsFunctionType(env, argv[PARAM2])) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
#else
    NAPI_ASSERT_BASE(env, argc == PARAM3, "number of parameters is wrong", false);
    NAPI_ASSERT_BASE(env, CheckIfParamIsFunctionType(env, argv[PARAM2]),
        "callback should be function, mismatch for param.", false);
    if (!g_locatorProxy->IsLocationEnabled()) {
        LBSLOGE(LOCATION_NAPI, "location switch is off, just return.");
        return false;
    }
#endif
    // the third params should be handler
    if (g_locationCallbacks.IsCallbackInMap(env, argv[PARAM2])) {
        LBSLOGE(LOCATION_NAPI, "This request already exists");
        return false;
    }
    auto locatorCallbackHost =
        sptr<LocatorCallbackNapi>(new (std::nothrow) LocatorCallbackNapi());
    if (locatorCallbackHost != nullptr) {
        napi_ref handlerRef = nullptr;
        NAPI_CALL_BASE(env, napi_create_reference(env, argv[PARAM2], 1, &handlerRef), false);
        // argv[1]:request params, argv[2]:handler
#ifdef ENABLE_NAPI_MANAGER
        int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
        LocationErrCode errorCode = SubscribeLocationChangeV9(env, argv[PARAM1], handlerRef, locatorCallbackHost);
        g_locationHiAppEvent->WriteEndEvent(
            beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "locationChangeOn");
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        SubscribeLocationChange(env, argv[PARAM1], handlerRef, locatorCallbackHost);
#endif
        g_locationCallbacks.AddCallback(env, handlerRef, locatorCallbackHost);
    }
    return true;
}

bool OffAllLocationChangeCallback(const napi_env& env)
{
    std::map<napi_env, std::map<napi_ref, sptr<LocatorCallbackNapi>>> callbackMap =
        g_locationCallbacks.GetCallbackMap();
    auto iter = callbackMap.find(env);
    if (iter == callbackMap.end()) {
        return false;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto callbackHost = innerIter->second;
        if (callbackHost == nullptr) {
            continue;
        }
#ifdef ENABLE_NAPI_MANAGER
        int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
        LocationErrCode errorCode = UnSubscribeLocationChange(callbackHost);
        g_locationHiAppEvent->WriteEndEvent(
            beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "locationChangeOff");
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeLocationChange(callbackHost);
#endif
        callbackHost->DeleteAllCallbacks();
        callbackHost = nullptr;
    }
    g_locationCallbacks.DeleteCallbackByEnv(env);
    return true;
}

bool OffLocationChangeCallback(const napi_env& env, const napi_value& handler)
{
    auto locatorCallbackHost = g_locationCallbacks.GetCallbackPtr(env, handler);
    if (locatorCallbackHost) {
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = UnSubscribeLocationChange(locatorCallbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeLocationChange(locatorCallbackHost);
#endif
        g_locationCallbacks.DeleteCallback(env, handler);
        locatorCallbackHost->DeleteAllCallbacks();
        locatorCallbackHost = nullptr;
        return true;
    }
    return false;
}

bool OnNmeaMessageChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv)
{
    #ifdef ENABLE_NAPI_MANAGER
    if (argc != PARAM2) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
    if (!CheckIfParamIsFunctionType(env, argv[PARAM1])) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
#else
    NAPI_ASSERT_BASE(env, argc == PARAM2, "number of parameters is wrong", false);
    NAPI_ASSERT_BASE(env, CheckIfParamIsFunctionType(env, argv[PARAM1]),
        "callback should be function, mismatch for param.", false);
#endif
    if (g_nmeaCallbacks.IsCallbackInMap(env, argv[PARAM1])) {
        LBSLOGE(LOCATION_NAPI, "This request already exists");
        return false;
    }
    auto nmeaCallbackHost =
        sptr<NmeaMessageCallbackNapi>(new (std::nothrow) NmeaMessageCallbackNapi());
    if (nmeaCallbackHost != nullptr) {
        napi_ref handlerRef = nullptr;
        NAPI_CALL_BASE(env, napi_create_reference(env, argv[PARAM1], PARAM1, &handlerRef), false);
#ifdef ENABLE_NAPI_MANAGER
        int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
        LocationErrCode errorCode = SubscribeNmeaMessageV9(env, handlerRef, nmeaCallbackHost);
        g_locationHiAppEvent->WriteEndEvent(
            beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "nmeaMessageOn");
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        SubscribeNmeaMessage(env, handlerRef, nmeaCallbackHost);
#endif
        g_nmeaCallbacks.AddCallback(env, handlerRef, nmeaCallbackHost);
    }
    return true;
}

bool OnCountryCodeChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv)
{
#ifdef ENABLE_NAPI_MANAGER
    if (argc != PARAM2) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
    if (!CheckIfParamIsFunctionType(env, argv[PARAM1])) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
#else
    NAPI_ASSERT_BASE(env, argc == PARAM2, "number of parameters is wrong", false);
    NAPI_ASSERT_BASE(env, CheckIfParamIsFunctionType(env, argv[PARAM1]),
        "callback should be function, mismatch for param.", false);
#endif
    if (g_countryCodeCallbacks.IsCallbackInMap(env, argv[PARAM1])) {
        LBSLOGE(LOCATION_NAPI, "This request already exists");
        return false;
    }
    auto callbackHost =
        sptr<CountryCodeCallbackNapi>(new (std::nothrow) CountryCodeCallbackNapi());
    if (callbackHost) {
        napi_ref handlerRef = nullptr;
        NAPI_CALL_BASE(env, napi_create_reference(env, argv[PARAM1], 1, &handlerRef), false);
#ifdef ENABLE_NAPI_MANAGER
        int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
        LocationErrCode errorCode = SubscribeCountryCodeChangeV9(env, handlerRef, callbackHost);
        g_locationHiAppEvent->WriteEndEvent(
            beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "countryCodeChangeOn");
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        SubscribeCountryCodeChange(env, handlerRef, callbackHost);
#endif
        g_countryCodeCallbacks.AddCallback(env, handlerRef, callbackHost);
    }
    return true;
}

bool OffAllCountryCodeChangeCallback(const napi_env& env)
{
    std::map<napi_env, std::map<napi_ref, sptr<CountryCodeCallbackNapi>>> callbackMap =
        g_countryCodeCallbacks.GetCallbackMap();
    auto iter = callbackMap.find(env);
    if (iter == callbackMap.end()) {
        return false;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto callbackHost = innerIter->second;
        if (callbackHost == nullptr) {
            continue;
        }
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = UnsubscribeCountryCodeChange(callbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnsubscribeCountryCodeChange(callbackHost);
#endif
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    }
    g_countryCodeCallbacks.DeleteCallbackByEnv(env);
    return true;
}

bool OffCountryCodeChangeCallback(const napi_env& env, const napi_value& handler)
{
    auto callbackHost = g_countryCodeCallbacks.GetCallbackPtr(env, handler);
    if (callbackHost) {
#ifdef ENABLE_NAPI_MANAGER
        int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
        LocationErrCode errorCode = UnsubscribeCountryCodeChange(callbackHost);
        g_locationHiAppEvent->WriteEndEvent(
            beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "countryCodeChangeOff");
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnsubscribeCountryCodeChange(callbackHost);
#endif
        g_countryCodeCallbacks.DeleteCallback(env, handler);
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
        return true;
    }
    return false;
}

bool OnLocationServiceStateCallback(const napi_env& env, const size_t argc, const napi_value* argv)
{
    auto data = new NativeContext;
    data->env_ = env;
    napi_add_env_cleanup_hook(env, CleanUp, data);
#ifdef ENABLE_NAPI_MANAGER
    if (argc != PARAM2) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
    if (!CheckIfParamIsFunctionType(env, argv[PARAM1])) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
#else
    NAPI_ASSERT_BASE(env, argc == PARAM2, "number of parameters is wrong", false);
    NAPI_ASSERT_BASE(env, CheckIfParamIsFunctionType(env, argv[PARAM1]),
        "callback should be function, mismatch for param.", false);
#endif
    if (g_switchCallbacks.IsCallbackInMap(env, argv[PARAM1])) {
        LBSLOGE(LOCATION_NAPI, "This request already exists");
        return false;
    }
    auto switchCallbackHost =
        sptr<LocationSwitchCallbackNapi>(new (std::nothrow) LocationSwitchCallbackNapi());
    if (switchCallbackHost != nullptr) {
        napi_ref handlerRef = nullptr;
        NAPI_CALL_BASE(env, napi_create_reference(env, argv[PARAM1], 1, &handlerRef), false);
#ifdef ENABLE_NAPI_MANAGER
        int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
        LocationErrCode errorCode = SubscribeLocationServiceStateV9(env, handlerRef, switchCallbackHost);
        g_locationHiAppEvent->WriteEndEvent(
            beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "locationEnabledChangeOn");
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        SubscribeLocationServiceState(env, handlerRef, switchCallbackHost);
#endif
        g_switchCallbacks.AddCallback(env, handlerRef, switchCallbackHost);
    }
    return true;
}

bool OffAllLocationServiceStateCallback(const napi_env& env)
{
    std::map<napi_env, std::map<napi_ref, sptr<LocationSwitchCallbackNapi>>> callbackMap =
        g_switchCallbacks.GetCallbackMap();
    auto iter = callbackMap.find(env);
    if (iter == callbackMap.end()) {
        return false;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto callbackHost = innerIter->second;
        if (callbackHost == nullptr) {
            continue;
        }
#ifdef ENABLE_NAPI_MANAGER
        int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
        LocationErrCode errorCode = UnSubscribeLocationServiceState(callbackHost);
        g_locationHiAppEvent->WriteEndEvent(
            beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "locationEnabledChangeOff");
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeLocationServiceState(callbackHost);
#endif
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    }
    g_switchCallbacks.DeleteCallbackByEnv(env);
    return true;
}

bool OffLocationServiceStateCallback(const napi_env& env, const napi_value& handler)
{
    auto data = new NativeContext;
    data->env_ = env;
    napi_add_env_cleanup_hook(env, CleanUp, data);
    auto switchCallbackHost = g_switchCallbacks.GetCallbackPtr(env, handler);
    if (switchCallbackHost) {
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = UnSubscribeLocationServiceState(switchCallbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeLocationServiceState(switchCallbackHost);
#endif
        g_switchCallbacks.DeleteCallback(env, handler);
        switchCallbackHost->DeleteHandler();
        switchCallbackHost = nullptr;
        return true;
    }
    return false;
}

#ifdef ENABLE_NAPI_MANAGER
bool OnLocationErrorCallback(const napi_env& env, const size_t argc, const napi_value* argv)
{
    if (argc != PARAM2) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        LBSLOGE(LOCATION_NAPI, "ERRCODE_INVALID_PARAM");
        return false;
    }
    if (!CheckIfParamIsFunctionType(env, argv[PARAM1])) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        LBSLOGE(LOCATION_NAPI, "ERRCODE_INVALID_PARAM");
        return false;
    }
    // the third params should be handler
    if (g_locationErrorCallbackHosts.IsCallbackInMap(env, argv[PARAM1])) {
        LBSLOGE(LOCATION_NAPI, "This request already exists");
        return false;
    }
    auto locationErrorCallbackHost =
        sptr<LocationErrorCallbackNapi>(new (std::nothrow) LocationErrorCallbackNapi());
    if (locationErrorCallbackHost != nullptr) {
        napi_ref handlerRef = nullptr;
        NAPI_CALL_BASE(env, napi_create_reference(env, argv[PARAM1], 1, &handlerRef), false);
        // argv[1]:request params, argv[2]:handler
        int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
        LocationErrCode errorCode = SubscribeLocationError(env, handlerRef, locationErrorCallbackHost);
        g_locationHiAppEvent->WriteEndEvent(
            beginTime, errorCode == ERRCODE_SUCCESS ? 1 : 0, errorCode, "locationErrorOn");
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
        g_locationErrorCallbackHosts.AddCallback(env, handlerRef, locationErrorCallbackHost);
    }
    return true;
}

bool OffLocationErrorCallback(const napi_env& env, const napi_value& handler)
{
    auto locationErrorCallbackHost = g_locationErrorCallbackHosts.GetCallbackPtr(env, handler);
    if (locationErrorCallbackHost) {
        int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
        LocationErrCode errorCode = UnSubscribeLocationError(locationErrorCallbackHost);
        g_locationHiAppEvent->WriteEndEvent(
            beginTime, errorCode == ERRCODE_SUCCESS ? 1 : 0, errorCode, "locationErrorOff");
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
        g_locationErrorCallbackHosts.DeleteCallback(env, handler);
        locationErrorCallbackHost = nullptr;
        return true;
    }
    return false;
}
#endif

napi_value On(napi_env env, napi_callback_info cbinfo)
{
    InitOnFuncMap();
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS] = {0};
    napi_value thisVar = nullptr;
    LBSLOGI(LOCATION_NAPI, "On function entry");
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr));
    napi_valuetype eventName = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, argv[PARAM0], &eventName));
#ifdef ENABLE_NAPI_MANAGER
    if (eventName != napi_string) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT(env, eventName == napi_string, "type mismatch for parameter 1");
#endif
    NAPI_ASSERT(env, g_locatorProxy != nullptr, "locator instance is null.");
    auto data = new NativeContext;
    data->env_ = env;
    napi_add_env_cleanup_hook(env, CleanUp, data);
    char type[64] = {0}; // max length
    size_t typeLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[PARAM0], type, sizeof(type), &typeLen));
    std::string event = type;
    LBSLOGI(LOCATION_NAPI, "Subscribe event: %{public}s", event.c_str());
    std::unique_lock<std::mutex> lock(g_FuncMapMutex);
    auto onCallbackFunc = g_onFuncMap.find(event);
    if (onCallbackFunc != g_onFuncMap.end() && onCallbackFunc->second != nullptr) {
        auto memberFunc = onCallbackFunc->second;
        (*memberFunc)(env, argc, argv);
    }
    return UndefinedNapiValue(env);
}

bool OnCachedGnssLocationsReportingCallback(const napi_env& env, const size_t argc, const napi_value* argv)
{
#ifdef ENABLE_NAPI_MANAGER
    if (argc != PARAM3) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return false;
    }
    napi_valuetype valueType;
    NAPI_CALL_BASE(env, napi_typeof(env, argv[PARAM1], &valueType), false);
    if (valueType != napi_object || !CheckIfParamIsFunctionType(env, argv[PARAM2])) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT_BASE(env, argc == PARAM3, "number of parameters is wrong", false);
    NAPI_ASSERT_BASE(env, CheckIfParamIsFunctionType(env, argv[PARAM2]),
        "callback should be function, mismatch for param.", false);
#endif
    if (g_cachedLocationCallbacks.IsCallbackInMap(env, argv[PARAM2])) {
        LBSLOGE(LOCATION_NAPI, "This request already exists");
        return false;
    }
    auto cachedCallbackHost =
        sptr<CachedLocationsCallbackNapi>(new (std::nothrow) CachedLocationsCallbackNapi());
    if (cachedCallbackHost != nullptr) {
        napi_ref handlerRef = nullptr;
        NAPI_CALL_BASE(env, napi_create_reference(env, argv[PARAM2], PARAM1, &handlerRef), false);
#ifdef ENABLE_NAPI_MANAGER
        int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
        LocationErrCode errorCode = SubscribeCacheLocationChangeV9(env, argv[PARAM1], handlerRef, cachedCallbackHost);
        g_locationHiAppEvent->WriteEndEvent(
            beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "cachedGnssLocationsChangeOn");
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        SubscribeCacheLocationChange(env, argv[PARAM1], handlerRef, cachedCallbackHost);
#endif
        g_cachedLocationCallbacks.AddCallback(env, handlerRef, cachedCallbackHost);
    }
    return true;
}

bool OffAllCachedGnssLocationsReportingCallback(const napi_env& env)
{
    std::map<napi_env, std::map<napi_ref, sptr<CachedLocationsCallbackNapi>>> callbackMap =
        g_cachedLocationCallbacks.GetCallbackMap();
    auto iter = callbackMap.find(env);
    if (iter == callbackMap.end()) {
#ifdef ENABLE_NAPI_MANAGER
        HandleSyncErrCode(env, ERRCODE_NOT_SUPPORTED);
#endif
        return false;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto callbackHost = innerIter->second;
        if (callbackHost == nullptr) {
            continue;
        }
        auto cachedCallback = sptr<ICachedLocationsCallback>(callbackHost);
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = UnSubscribeCacheLocationChangeV9(cachedCallback);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeCacheLocationChange(cachedCallback);
#endif
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    }
    g_cachedLocationCallbacks.DeleteCallbackByEnv(env);
    return true;
}

bool OffCachedGnssLocationsReportingCallback(const napi_env& env, const napi_value& handler)
{
    auto cachedCallbackHost = g_cachedLocationCallbacks.GetCallbackPtr(env, handler);
    if (cachedCallbackHost) {
        auto cachedCallback = sptr<ICachedLocationsCallback>(cachedCallbackHost);
#ifdef ENABLE_NAPI_MANAGER
        int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
        LocationErrCode errorCode = UnSubscribeCacheLocationChangeV9(cachedCallback);
        g_locationHiAppEvent->WriteEndEvent(
            beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "cachedGnssLocationsChangeOff");
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeCacheLocationChange(cachedCallback);
#endif
        g_cachedLocationCallbacks.DeleteCallback(env, handler);
        cachedCallbackHost->DeleteHandler();
        cachedCallbackHost = nullptr;
        return true;
    } else {
        LBSLOGI(LOCATION_NAPI, "%{public}s, the callback is not in the map", __func__);
#ifdef ENABLE_NAPI_MANAGER
        HandleSyncErrCode(env, ERRCODE_NOT_SUPPORTED);
#endif
    }
    return false;
}

bool OffAllGnssStatusChangeCallback(const napi_env& env)
{
    std::map<napi_env, std::map<napi_ref, sptr<GnssStatusCallbackNapi>>> callbackMap =
        g_gnssStatusInfoCallbacks.GetCallbackMap();
    auto iter = callbackMap.find(env);
    if (iter == callbackMap.end()) {
        return false;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto callbackHost = innerIter->second;
        if (callbackHost == nullptr) {
            continue;
        }
#ifdef ENABLE_NAPI_MANAGER
        int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
        LocationErrCode errorCode = UnSubscribeGnssStatusV9(callbackHost);
        g_locationHiAppEvent->WriteEndEvent(
            beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "satelliteStatusChangeOff");
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeGnssStatus(callbackHost);
#endif
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    }
    g_gnssStatusInfoCallbacks.DeleteCallbackByEnv(env);
    return true;
}

bool OffAllNmeaMessageChangeCallback(const napi_env& env)
{
    std::map<napi_env, std::map<napi_ref, sptr<NmeaMessageCallbackNapi>>> callbackMap =
        g_nmeaCallbacks.GetCallbackMap();
    auto iter = callbackMap.find(env);
    if (iter == callbackMap.end()) {
        return false;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto callbackHost = innerIter->second;
        if (callbackHost == nullptr) {
            continue;
        }
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = UnSubscribeNmeaMessageV9(callbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeNmeaMessage(callbackHost);
#endif
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    }
    g_nmeaCallbacks.DeleteCallbackByEnv(env);
    return true;
}

bool OffGnssStatusChangeCallback(const napi_env& env, const napi_value& handler)
{
    auto gnssCallbackHost = g_gnssStatusInfoCallbacks.GetCallbackPtr(env, handler);
    if (gnssCallbackHost) {
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = UnSubscribeGnssStatusV9(gnssCallbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeGnssStatus(gnssCallbackHost);
#endif
        g_gnssStatusInfoCallbacks.DeleteCallback(env, handler);
        gnssCallbackHost->DeleteHandler();
        gnssCallbackHost = nullptr;
        return true;
    }
    return false;
}

bool OffNmeaMessageChangeCallback(const napi_env& env, const napi_value& handler)
{
    auto nmeaCallbackHost = g_nmeaCallbacks.GetCallbackPtr(env, handler);
    if (nmeaCallbackHost) {
#ifdef ENABLE_NAPI_MANAGER
        int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
        LocationErrCode errorCode = UnSubscribeNmeaMessageV9(nmeaCallbackHost);
        g_locationHiAppEvent->WriteEndEvent(
            beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "nmeaMessageOff");
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
#else
        UnSubscribeNmeaMessage(nmeaCallbackHost);
#endif
        g_nmeaCallbacks.DeleteCallback(env, handler);
        nmeaCallbackHost->DeleteHandler();
        nmeaCallbackHost = nullptr;
        return true;
    }
    return false;
}

bool VerifyOffFuncParam(napi_env env, napi_callback_info cbinfo, size_t& argc)
{
    napi_value argv[MAXIMUM_JS_PARAMS] = {0};
    napi_valuetype valueType[PARAM3] = {napi_undefined};
    napi_value thisVar = nullptr;
    NAPI_CALL_BASE(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr), false);
    NAPI_ASSERT_BASE(env, g_locatorProxy != nullptr, "locator instance is null.", false);
    argc = (argc > PARAM3) ? PARAM3 : argc;
    if (argc > 0) {
        for (int i = (int)(argc - 1); i >= 0; i--) {
            NAPI_CALL_BASE(env, napi_typeof(env, argv[i], &valueType[i]), false);
            /* If the type of the last input parameter is incorrect, ignore it. */
            if (valueType[i] != napi_function && valueType[i] != napi_object && valueType[i] != napi_string &&
                i == (int)(argc - 1)) {
                argc--;
            }
        }
    }
    if (argc == PARAM3 && valueType[argc - 1] != napi_object) {
        argc--;
    }
    if (argc == PARAM2 && valueType[argc - 1] != napi_function) {
        argc--;
    }
    if (argc < PARAM1 || valueType[PARAM0] != napi_string ||
        (argc == PARAM2 && valueType[PARAM1] != napi_function) ||
        (argc == PARAM3 && valueType[PARAM1] != napi_object && valueType[PARAM2] != napi_object)) {
#ifdef ENABLE_NAPI_MANAGER
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
#else
        NAPI_ASSERT_BASE(env, false, "ERRCODE_INVALID_PARAM", false);
#endif
        return false;
    }
    return true;
}

napi_value Off(napi_env env, napi_callback_info cbinfo)
{
    LBSLOGI(LOCATION_NAPI, "Off function entry");
    InitOffFuncMap();

    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS] = {0};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr));

    if (!VerifyOffFuncParam(env, cbinfo, argc)) {
        LBSLOGE(LOCATION_NAPI, "VerifyOffFuncParam fail");
        return UndefinedNapiValue(env);
    }

    char type[64] = {0};
    size_t typeLen = 0;
    NAPI_CALL(env, napi_get_value_string_utf8(env, argv[PARAM0], type, sizeof(type), &typeLen));
    std::string event = type;
    LBSLOGI(LOCATION_NAPI, "Unsubscribe event: %{public}s", event.c_str());
    if (argc == PARAM1) {
        std::unique_lock<std::mutex> lock(g_FuncMapMutex);
        auto offAllCallbackFunc = g_offAllFuncMap.find(event);
        if (offAllCallbackFunc != g_offAllFuncMap.end() && offAllCallbackFunc->second != nullptr) {
            auto memberFunc = offAllCallbackFunc->second;
            (*memberFunc)(env);
        }
    } else if (argc == PARAM2) {
        std::unique_lock<std::mutex> lock(g_FuncMapMutex);
        auto offCallbackFunc = g_offFuncMap.find(event);
        if (offCallbackFunc != g_offFuncMap.end() && offCallbackFunc->second != nullptr) {
            auto singleMemberFunc = offCallbackFunc->second;
            (*singleMemberFunc)(env, argv[PARAM1]);
        }
    }
    return UndefinedNapiValue(env);
}

napi_value GetCurrentLocation(napi_env env, napi_callback_info cbinfo)
{
    LBSLOGI(LOCATION_NAPI, "GetCurrentLocation enter");

    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS] = {0};
    napi_valuetype valueType[MAXIMUM_JS_PARAMS] = {napi_undefined};
    napi_value thisVar = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, cbinfo, &argc, argv, &thisVar, nullptr));
    NAPI_ASSERT(env, g_locatorProxy != nullptr, "locator instance is null.");
    LBSLOGI(LOCATION_NAPI, "GetCurrentLocation enter");
#ifdef SUPPORT_JSSTACK
    HiviewDFX::ReportXPowerJsStackSysEvent(env, "GNSS_STATE");
#endif
    argc = (argc > PARAM2) ? PARAM2 : argc;
    if (argc > 0) {
        for (int i = (int)(argc - 1); i >= 0; i--) {
            NAPI_CALL(env, napi_typeof(env, argv[i], &valueType[i]));
            /* If the type of the last input parameter is incorrect, ignore it. */
            if (valueType[i] != napi_function && valueType[i] != napi_object &&
                i == (int)(argc - 1)) {
                argc--;
            }
        }
    }
    if (argc == PARAM2) {
        if (valueType[PARAM1] != napi_function) {
            argc--;
        } else if (valueType[PARAM0] != napi_object) {
            HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
            return UndefinedNapiValue(env);
        }
    }
    if (argc == PARAM1) {
        if (valueType[PARAM0] != napi_function && valueType[PARAM0] != napi_object) {
            argc--;
        }
    }
#ifdef ENABLE_NAPI_MANAGER
    return RequestLocationOnceV9(env, argc, argv);
#else
    return RequestLocationOnce(env, argc, argv);
#endif
}

bool IsRequestConfigValid(std::unique_ptr<RequestConfig>& config)
{
    if (config == nullptr) {
        return false;
    }
    if ((config->GetScenario() > SCENE_NO_POWER || config->GetScenario() < SCENE_UNSET) &&
        (config->GetScenario() > LOCATION_SCENE_INDOOR_POI ||
        config->GetScenario() < LOCATION_SCENE_NAVIGATION) &&
        (config->GetScenario() > LOCATION_SCENE_NO_POWER_CONSUMPTION ||
        config->GetScenario() < LOCATION_SCENE_HIGH_POWER_CONSUMPTION)) {
        return false;
    }
    if ((config->GetPriority() > PRIORITY_FAST_FIRST_FIX || config->GetPriority() < PRIORITY_UNSET) &&
        (config->GetPriority() > LOCATION_PRIORITY_LOCATING_SPEED ||
        config->GetPriority() < LOCATION_PRIORITY_ACCURACY)) {
        return false;
    }
    if (config->GetTimeOut() < MIN_TIMEOUTMS_FOR_LOCATIONONCE) {
        return false;
    }
    if (config->GetTimeInterval() < 0) {
        return false;
    }
    if (config->GetDistanceInterval() < 0) {
        return false;
    }
    if (config->GetMaxAccuracy() < 0) {
        return false;
    }
    return true;
}

#ifdef ENABLE_NAPI_MANAGER
bool OnBluetoothScanResultChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv)
{
    if (argc != PARAM2) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        LBSLOGE(LOCATION_NAPI, "ERRCODE_INVALID_PARAM");
        return false;
    }
    if (!CheckIfParamIsFunctionType(env, argv[PARAM1])) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        LBSLOGE(LOCATION_NAPI, "ERRCODE_INVALID_PARAM");
        return false;
    }
    if (g_bluetoothScanResultCallbackHosts.IsCallbackInMap(env, argv[PARAM1])) {
        LBSLOGE(LOCATION_NAPI, "This request already exists");
        return false;
    }
    auto bluetoothScanResultCallbackHost =
        sptr<BluetoothScanResultCallbackNapi>(new (std::nothrow) BluetoothScanResultCallbackNapi());
    if (bluetoothScanResultCallbackHost != nullptr) {
        napi_ref handlerRef = nullptr;
        NAPI_CALL_BASE(env, napi_create_reference(env, argv[PARAM1], 1, &handlerRef), false);
        // argv[1]:request params, argv[2]:handler
        int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
        LocationErrCode errorCode = SubscribeBluetoothScanResultChange(env, handlerRef,
            bluetoothScanResultCallbackHost);
        g_locationHiAppEvent->WriteEndEvent(
            beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "bluetoothScanResultChangeOn");
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
        g_bluetoothScanResultCallbackHosts.AddCallback(env, handlerRef, bluetoothScanResultCallbackHost);
    }
    return true;
}

bool OffBluetoothScanResultChangeCallback(const napi_env& env, const napi_value& handler)
{
    auto bluetoothScanResultCallbackHost = g_bluetoothScanResultCallbackHosts.GetCallbackPtr(env, handler);
    if (bluetoothScanResultCallbackHost) {
        int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
        LocationErrCode errorCode = UnSubscribeBluetoothScanResultChange(bluetoothScanResultCallbackHost);
        g_locationHiAppEvent->WriteEndEvent(
            beginTime, errorCode == ERRCODE_SUCCESS ? 1 : 0, errorCode, "bluetoothScanResultChangeOff");
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
        g_bluetoothScanResultCallbackHosts.DeleteCallback(env, handler);
        bluetoothScanResultCallbackHost = nullptr;
        return true;
    }
    return false;
}

bool OffAllBluetoothScanResultChangeCallback(const napi_env& env)
{
    std::map<napi_env, std::map<napi_ref, sptr<BluetoothScanResultCallbackNapi>>> callbackMap =
        g_bluetoothScanResultCallbackHosts.GetCallbackMap();
    auto iter = callbackMap.find(env);
    if (iter == callbackMap.end()) {
        return false;
    }
    for (auto innerIter = iter->second.begin(); innerIter != iter->second.end(); innerIter++) {
        auto callbackHost = innerIter->second;
        if (callbackHost == nullptr) {
            continue;
        }
        LocationErrCode errorCode = UnSubscribeBluetoothScanResultChange(callbackHost);
        if (errorCode != ERRCODE_SUCCESS) {
            HandleSyncErrCode(env, errorCode);
            return false;
        }
        callbackHost->DeleteHandler();
        callbackHost = nullptr;
    }
    g_bluetoothScanResultCallbackHosts.DeleteCallbackByEnv(env);
    return true;
}
#endif

bool NeedReportLastLocation(const std::unique_ptr<RequestConfig>& config, const std::unique_ptr<Location>& location)
{
    if (config->GetScenario() == SCENE_UNSET && config->GetPriority() == PRIORITY_UNSET) {
        return false;
    }
    int64_t curTime = CommonUtils::GetCurrentTimeStamp();
    float maxAcc = config->GetMaxAccuracy();
    if (location != nullptr &&
        (curTime - location->GetTimeStamp() / MILLI_PER_SEC) <= LASTLOCATION_CACHED_TIME &&
        (location->GetAccuracy() == DEFAULT_APPROXIMATELY_ACCURACY ||
        !(maxAcc > 0 && location->GetAccuracy() > maxAcc))) {
        return true;
    } else {
        return false;
    }
}
}  // namespace Location
}  // namespace OHOS
