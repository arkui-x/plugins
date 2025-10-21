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

#include "location_napi_adapter.h"
#include "location_log.h"
#include "location_napi_errcode.h"
#include "constant_definition.h"
#include "geofence_sdk.h"
#include "geofence_napi.h"
#include "geofence_async_context.h"
#include "beacon_fence_request.h"
#include "location_hiappevent.h"
#include <thread>
#include <chrono>

namespace OHOS {
namespace Location {
auto g_locatorClient = Locator::GetInstance();
auto g_geofenceClient = GeofenceManager::GetInstance();
auto g_hiAppEventClient = LocationHiAppEvent::GetInstance();
std::map<int, sptr<LocationGnssGeofenceCallbackNapi>> g_gnssGeofenceCallbackHostMap;
std::map<std::shared_ptr<BeaconFence>, sptr<LocationGnssGeofenceCallbackNapi>> g_beaconFenceRequestMap;
std::mutex g_gnssGeofenceCallbackHostMutex;
std::mutex g_beaconFenceRequestMutex;

napi_value GetLastLocation(napi_env env, napi_callback_info info)
{
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;

    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");

    auto asyncContext = new (std::nothrow) LocationAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "getLastLocation",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<LocationAsyncContext*>(data);
        context->loc = g_locatorClient->IsLocationEnabled() ? g_locatorClient->GetCachedLocation() : nullptr;
        if (context->loc != nullptr) {
            context->errCode = SUCCESS;
        } else {
            context->errCode = LAST_KNOWN_LOCATION_ERROR;
        }
    };

    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<LocationAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env, napi_create_object(context->env, &context->result[PARAM1]));
        g_hiAppEventClient->WriteEndEvent(
            context->beginTime, context->errCode == ERRCODE_SUCCESS ? 0 : 1, context->errCode, "getLastLocation");
        if (context->loc != nullptr) {
            LocationToJs(context->env, context->loc, context->result[PARAM1]);
        } else {
            LBSLOGE(LOCATOR_STANDARD, "loc is nullptr!");
        }
        LBSLOGI(LOCATOR_STANDARD, "Push last location result to client");
    };

    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

napi_value IsLocationEnabled(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "IsLocationEnabled enter.");
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
#ifdef ENABLE_NAPI_MANAGER
    napi_value res;
    bool isEnabled = false;
    int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
    LocationErrCode errorCode = g_locatorClient->IsLocationEnabledV9(isEnabled);
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return UndefinedNapiValue(env);
    }
    NAPI_CALL(env, napi_get_boolean(env, isEnabled, &res));
    return res;
#else
    auto asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "isLocationEnabled", NAPI_AUTO_LENGTH,
        &asyncContext->resourceName) != napi_ok) {
        LBSLOGE(LOCATOR_STANDARD, "copy string failed");
    }
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<SwitchAsyncContext*>(data);
        context->enable = g_locatorClient->IsLocationEnabled();
        context->errCode = SUCCESS;
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<SwitchAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env, napi_get_boolean(context->env, context->enable, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push IsLocationEnabled result to client");
    };

    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
#endif
}

#ifdef ENABLE_NAPI_MANAGER
napi_value GetCurrentWifiBssidForLocating(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "GetCurrentWifiBssidForLocating called.");
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    napi_value res;
    std::string bssid;
    int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
    LocationErrCode errorCode = g_locatorClient->GetCurrentWifiBssidForLocating(bssid);
    g_hiAppEventClient->WriteEndEvent(
        beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "getCurrentWifiBssidForLocating");
    if (errorCode != ERRCODE_SUCCESS) {
        ThrowBusinessError(env, errorCode);
        return UndefinedNapiValue(env);
    }
    NAPI_CALL(env, napi_create_string_utf8(env, bssid.c_str(), NAPI_AUTO_LENGTH, &res));
    return res;
}

napi_value GetDistanceBetweenLocations(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "GetDistanceBetweenLocations called.");
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    if (g_locatorClient == nullptr) {
        HandleSyncErrCode(env, ERRCODE_SERVICE_UNAVAILABLE);
        return UndefinedNapiValue(env);
    }
    if (argc != PARAM2) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    napi_valuetype valueType1;
    NAPI_CALL(env, napi_typeof(env, argv[1], &valueType1));
    if (valueType != napi_object || valueType1 != napi_object) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    Location location1;
    Location location2;
    if (!JsObjToLocation(env, argv[0], location1) || !JsObjToLocation(env, argv[1], location2)) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    if (!location1.isValidLatitude(location1.GetLatitude()) || !location1.isValidLongitude(location1.GetLongitude()) ||
        !location1.isValidLatitude(location2.GetLatitude()) || !location1.isValidLongitude(location2.GetLongitude())) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    napi_value res;
    double distance;
    int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
    LocationErrCode errorCode = g_locatorClient->GetDistanceBetweenLocations(location1, location2, distance);
    g_hiAppEventClient->WriteEndEvent(
        beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "getDistanceBetweenLocations");
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return UndefinedNapiValue(env);
    }
    NAPI_CALL(env, napi_create_double(env, distance, &res));
    return res;
}
#endif

napi_value IsGeoServiceAvailable(napi_env env, napi_callback_info info)
{
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
#ifdef ENABLE_NAPI_MANAGER
    napi_value res;
    bool isAvailable = false;
    int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
    LocationErrCode errorCode = g_locatorClient->IsGeoServiceAvailableV9(isAvailable);
    g_hiAppEventClient->WriteEndEvent(
        beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "isGeoServiceAvailable");
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return UndefinedNapiValue(env);
    }
    NAPI_CALL(env, napi_get_boolean(env, isAvailable, &res));
    return res;
#else
    auto asyncContext = new (std::nothrow) SwitchAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    NAPI_CALL(env,
        napi_create_string_latin1(env, "isGeoServiceAvailable", NAPI_AUTO_LENGTH, &asyncContext->resourceName));
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<SwitchAsyncContext*>(data);
        bool isAvailable = g_locatorClient->IsGeoServiceAvailable();
        context->enable = isAvailable;
        context->errCode = SUCCESS;
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<SwitchAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env, napi_get_boolean(context->env, context->enable, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push isGeoServiceAvailable result to client");
    };
    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
#endif
}

void CreateReverseGeocodeAsyncContext(ReverseGeoCodeAsyncContext* asyncContext)
{
    LBSLOGI(LOCATOR_STANDARD, "CreateReverseGeocodeAsyncContext called.");
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<ReverseGeoCodeAsyncContext*>(data);
#ifdef ENABLE_NAPI_MANAGER
        if (context->errCode != ERRCODE_SUCCESS) {
#else
        if (context->errCode != SUCCESS) {
#endif
            return;
        }
#ifdef ENABLE_NAPI_MANAGER
        bool isAvailable = false;
        LocationErrCode errorCode = g_locatorClient->IsGeoServiceAvailableV9(isAvailable);
        if (errorCode != ERRCODE_SUCCESS) {
            context->errCode = errorCode;
            return;
        }
        if (!isAvailable) {
            context->errCode = ERRCODE_REVERSE_GEOCODING_FAIL;
            return;
        }
        context->beginTime = CommonUtils::GetCurrentTimeMilSec();
        errorCode = g_locatorClient->GetAddressByCoordinateV9(context->reverseGeoCodeRequest, context->replyList);
        if (context->replyList.empty() || errorCode != ERRCODE_SUCCESS) {
            context->errCode = errorCode;
        }
#else
        if (!g_locatorClient->IsGeoServiceAvailable()) {
            context->errCode = REVERSE_GEOCODE_ERROR;
            return;
        }
        g_locatorClient->GetAddressByCoordinate(context->reverseGeoCodeRequest, context->replyList);
        if (context->replyList.empty()) {
            context->errCode = REVERSE_GEOCODE_ERROR;
        }
#endif
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<ReverseGeoCodeAsyncContext*>(data);
        if (context->errCode != SUCCESS) {
            napi_create_array_with_length(context->env, 0, &context->result[PARAM1]);
            return;
        }
        NAPI_CALL_RETURN_VOID(context->env,
            napi_create_array_with_length(context->env, context->replyList.size(), &context->result[PARAM1]));
        GeoAddressesToJsObj(context->env, context->replyList, context->result[PARAM1]);
    };
}

void CreateGeocodeAsyncContext(GeoCodeAsyncContext* asyncContext)
{
    LBSLOGI(LOCATOR_STANDARD, "CreateGeocodeAsyncContext called.");
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<GeoCodeAsyncContext*>(data);
        if (context->errCode != SUCCESS) {
            return;
        }
#ifdef ENABLE_NAPI_MANAGER
        bool isAvailable = false;
        LocationErrCode errorCode = g_locatorClient->IsGeoServiceAvailableV9(isAvailable);
        if (errorCode != ERRCODE_SUCCESS) {
            context->errCode = errorCode;
            return;
        }
        if (!isAvailable) {
            context->errCode = ERRCODE_GEOCODING_FAIL;
            return;
        }
        errorCode = g_locatorClient->GetAddressByLocationNameV9(context->geoCodeRequest, context->replyList);
        if (context->replyList.empty() || errorCode != ERRCODE_SUCCESS) {
            context->errCode = errorCode;
        }
#else
        if (!g_locatorClient->IsGeoServiceAvailable()) {
            context->errCode = GEOCODE_ERROR;
            return;
        }
        g_locatorClient->GetAddressByLocationName(context->geoCodeRequest, context->replyList);
        if (context->replyList.empty()) {
            context->errCode = GEOCODE_ERROR;
        }
#endif
    };
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<GeoCodeAsyncContext*>(data);
        NAPI_CALL_RETURN_VOID(context->env,
            napi_create_array_with_length(context->env, context->replyList.size(), &context->result[PARAM1]));
        GeoAddressesToJsObj(context->env, context->replyList, context->result[PARAM1]);
        LBSLOGI(LOCATOR_STANDARD, "Push GetAddressesFromLocationName result to client");
    };
}

napi_value GetAddressesFromLocation(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
#ifdef ENABLE_NAPI_MANAGER
    if (argc < PARAM1 || argc > PARAM2 || (argc == PARAM2 && !CheckIfParamIsFunctionType(env, argv[1]))) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");
#endif

    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
#ifdef ENABLE_NAPI_MANAGER
    if (valueType != napi_object) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type, object is expected for parameter 1.");
#endif
    auto asyncContext = new ReverseGeoCodeAsyncContext(env);
    asyncContext->reverseGeoCodeRequest = std::make_unique<ReverseGeoCodeRequest>();
    if (napi_create_string_latin1(env, "getAddressesFromLocation",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    int ret = JsObjToReverseGeoCodeRequest(env, argv[0], asyncContext->reverseGeoCodeRequest);
    asyncContext->errCode = (ret == SUCCESS) ? ERRCODE_SUCCESS : ERRCODE_INVALID_PARAM;
#ifdef ENABLE_NAPI_MANAGER
    if (asyncContext->errCode != SUCCESS) {
        int code = asyncContext->errCode;
        delete asyncContext;
        asyncContext = nullptr;
        HandleSyncErrCode(env, code);
        return UndefinedNapiValue(env);
    }
#endif
    CreateReverseGeocodeAsyncContext(asyncContext);
    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

napi_value GetAddressesFromLocationName(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
#ifdef ENABLE_NAPI_MANAGER
    if (argc < PARAM1 || argc > PARAM2 || (argc == PARAM2 && !CheckIfParamIsFunctionType(env, argv[1]))) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");
#endif

    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
#ifdef ENABLE_NAPI_MANAGER
    if (valueType != napi_object) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#else
    NAPI_ASSERT(env, valueType == napi_object, "Wrong argument type, object is expected for parameter 1.");
#endif
    auto asyncContext = new (std::nothrow) GeoCodeAsyncContext(env);
    asyncContext->geoCodeRequest = std::make_unique<GeoCodeRequest>();
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "GetAddressesFromLocationName",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->errCode = JsObjToGeoCodeRequest(env, argv[0], asyncContext->geoCodeRequest);
#ifdef ENABLE_NAPI_MANAGER
    if (asyncContext->errCode == INPUT_PARAMS_ERROR) {
        delete asyncContext;
        asyncContext = nullptr;
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#endif
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    CreateGeocodeAsyncContext(asyncContext);
    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

napi_value GetCachedGnssLocationsSize(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
#ifdef ENABLE_NAPI_MANAGER
    if (argc > PARAM1 || (argc == PARAM1 && !CheckIfParamIsFunctionType(env, argv[PARAM0]))) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#endif
    auto asyncContext = new (std::nothrow) CachedAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "GetCachedGnssLocationsSize",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    SetExecuteFuncForGetCachedGnssLocationsSizeContext(asyncContext);
    SetCompleteFuncForGetCachedGnssLocationsSizeContext(asyncContext);
    return DoAsyncWork(env, asyncContext, argc, argv, 0);
}

void SetExecuteFuncForGetCachedGnssLocationsSizeContext(CachedAsyncContext* asyncContext)
{
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<CachedAsyncContext*>(data);
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = CheckLocationSwitchState();
        if (errorCode != ERRCODE_SUCCESS) {
            context->errCode = errorCode;
            return;
        }
#endif

#ifdef ENABLE_NAPI_MANAGER
        int size = -1;
        g_locatorClient->GetCachedGnssLocationsSizeV9(size);
        context->errCode = ERRCODE_NOT_SUPPORTED;
        context->locationSize = size;
#else
        context->locationSize = g_locatorClient->GetCachedGnssLocationsSize();
        context->errCode = (context->locationSize >= 0) ? SUCCESS : NOT_SUPPORTED;
#endif
    };
}

void SetCompleteFuncForGetCachedGnssLocationsSizeContext(CachedAsyncContext* asyncContext)
{
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<CachedAsyncContext*>(data);
        g_hiAppEventClient->WriteEndEvent(context->beginTime, context->errCode == ERRCODE_SUCCESS ? 0 : 1,
            context->errCode, "getCachedGnssLocationsSize");
        NAPI_CALL_RETURN_VOID(context->env,
            napi_create_int32(context->env, context->locationSize, &context->result[PARAM1]));
        LBSLOGI(LOCATOR_STANDARD, "Push GetCachedGnssLocationsSize result to client");
    };
}

napi_value FlushCachedGnssLocations(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
#ifdef ENABLE_NAPI_MANAGER
    if (argc > PARAM1 || (argc == PARAM1 && !CheckIfParamIsFunctionType(env, argv[PARAM0]))) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
#endif
    auto asyncContext = new CachedAsyncContext(env);
    if (napi_create_string_latin1(env, "FlushCachedGnssLocations",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    SetExecuteFuncForFlushCachedGnssLocationsContext(asyncContext);
    SetCompleteFuncForFlushCachedGnssLocationsContext(asyncContext);
    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

void SetExecuteFuncForFlushCachedGnssLocationsContext(CachedAsyncContext* asyncContext)
{
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<CachedAsyncContext*>(data);
#ifdef ENABLE_NAPI_MANAGER
        LocationErrCode errorCode = CheckLocationSwitchState();
        if (errorCode != ERRCODE_SUCCESS) {
            context->errCode = errorCode;
            return;
        }
        g_locatorClient->FlushCachedGnssLocationsV9();
        context->errCode = ERRCODE_NOT_SUPPORTED;
#else
        if (g_locatorClient->IsLocationEnabled()) {
            g_locatorClient->FlushCachedGnssLocations();
        }
        context->errCode = NOT_SUPPORTED;
#endif
    };
}

void SetCompleteFuncForFlushCachedGnssLocationsContext(CachedAsyncContext* asyncContext)
{
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<CachedAsyncContext*>(data);
#ifdef ENABLE_NAPI_MANAGER
        g_hiAppEventClient->WriteEndEvent(context->beginTime, context->errCode == ERRCODE_SUCCESS ? 0 : 1,
            context->errCode, "flushCachedGnssLocations");
        NAPI_CALL_RETURN_VOID(context->env, napi_get_undefined(context->env, &context->result[PARAM1]));
#else
        NAPI_CALL_RETURN_VOID(context->env,
            napi_get_boolean(context->env, context->errCode == SUCCESS, &context->result[PARAM1]));
#endif
        LBSLOGI(LOCATOR_STANDARD, "Push FlushCachedGnssLocations result to client");
    };
}

#ifdef ENABLE_NAPI_MANAGER
napi_value GetIsoCountryCode(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    LBSLOGI(LOCATOR_STANDARD, "GetIsoCountryCode_adapter");
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "locator instance is null.");
    if (argc > PARAM1 || (argc == PARAM1 && !CheckIfParamIsFunctionType(env, argv[PARAM0]))) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    CountryCodeContext *asyncContext = new (std::nothrow) CountryCodeContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    if (napi_create_string_latin1(env, "CountryCodeContext", NAPI_AUTO_LENGTH,
        &asyncContext->resourceName) != napi_ok) {
        LBSLOGE(LOCATOR_STANDARD, "copy string failed");
    }
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    SetExecuteFuncForCountryCodeContext(asyncContext);
    SetCompleteFuncForCountryCodeContext(asyncContext);
    size_t objectArgsNum = 0;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

void SetExecuteFuncForCountryCodeContext(CountryCodeContext* asyncContext)
{
    asyncContext->executeFunc = [&](void *data) -> void {
        if (data == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "GetIsoCountryCode data == nullptr");
            return;
        }
        CountryCodeContext *context = static_cast<CountryCodeContext*>(data);
        std::shared_ptr<CountryCode> country = std::make_shared<CountryCode>();
        LocationErrCode errorCode = g_locatorClient->GetIsoCountryCodeV9(country);
        context->errCode = errorCode;
        if (errorCode == ERRCODE_SUCCESS) {
            context->country = country;
        }
    };
}

void SetCompleteFuncForCountryCodeContext(CountryCodeContext* asyncContext)
{
    asyncContext->completeFunc = [&](void *data) -> void {
        if (data == nullptr) {
            LBSLOGE(LOCATOR_STANDARD, "GetIsoCountryCode data == nullptr");
            return;
        }
        CountryCodeContext *context = static_cast<CountryCodeContext *>(data);
        g_hiAppEventClient->WriteEndEvent(
            context->beginTime, context->errCode == ERRCODE_SUCCESS ? 0 : 1, context->errCode, "getCountryCode");
        NAPI_CALL_RETURN_VOID(context->env, napi_create_object(context->env, &context->result[PARAM1]));
        if (context->country) {
            CountryCodeToJs(context->env, context->country, context->result[PARAM1]);
        } else {
            LBSLOGE(LOCATOR_STANDARD, "country is nullptr!");
        }
        LBSLOGI(LOCATOR_STANDARD, "Push GetIsoCountryCode result to client, time = %{public}s",
            std::to_string(CommonUtils::GetCurrentTimeMilSec()).c_str());
    };
}
#endif

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode CheckLocationSwitchState()
{
    bool isEnabled = false;
    LocationErrCode errorCode = g_locatorClient->IsLocationEnabledV9(isEnabled);
    if (errorCode != ERRCODE_SUCCESS) {
        return errorCode;
    }
    if (!isEnabled) {
        return ERRCODE_SWITCH_OFF;
    }
    return ERRCODE_SUCCESS;
}

napi_value AddGnssGeofence(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator ext SA failed");
    if (argc > PARAM1) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    std::shared_ptr<GeofenceRequest> gnssGeofenceRequest = std::make_shared<GeofenceRequest>();
    bool isValidParameter = ParseGnssGeofenceRequest(env, argv[0], gnssGeofenceRequest);
    if (!isValidParameter) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    auto locationGnssGeofenceCallbackHost =
        sptr<LocationGnssGeofenceCallbackNapi>(new LocationGnssGeofenceCallbackNapi());
    JsObjToGeofenceTransitionCallback(env, argv[0], locationGnssGeofenceCallbackHost);
    auto callbackPtr = sptr<IGnssGeofenceCallback>(locationGnssGeofenceCallbackHost);
    gnssGeofenceRequest->SetGeofenceTransitionCallback(callbackPtr->AsObject());
    auto asyncContext = new GnssGeofenceAsyncContext(env);
    if (napi_create_string_latin1(env, "addGnssGeofence",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->callbackHost_ = locationGnssGeofenceCallbackHost;
    asyncContext->request_ = gnssGeofenceRequest;
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    SetExecuteFuncForAddGnssGeofenceContext(asyncContext);
    SetCompleteFuncForAddGnssGeofenceContext(asyncContext);
    return DoAsyncWork(env, asyncContext, argc, argv, 1);
}

void SetExecuteFuncForAddGnssGeofenceContext(GnssGeofenceAsyncContext* asyncContext)
{
    LBSLOGI(LOCATOR_STANDARD, "SetExecuteFuncForAddGnssGeofenceContext called.");
    asyncContext->executeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            return;
        }
        auto context = static_cast<GnssGeofenceAsyncContext*>(data);
        auto callbackHost = context->callbackHost_;
        auto gnssGeofenceRequest = context->request_;
        if (callbackHost != nullptr && gnssGeofenceRequest != nullptr) {
            auto errCode = g_geofenceClient->AddGnssGeofence(gnssGeofenceRequest, callbackHost);
            if (errCode != ERRCODE_SUCCESS) {
                context->errCode = errCode;
            }
        }
    };
}

void SetCompleteFuncForAddGnssGeofenceContext(GnssGeofenceAsyncContext* asyncContext)
{
    asyncContext->completeFunc = [&](void* data) -> void {
        if (data == nullptr) {
            return;
        }
        auto context = static_cast<GnssGeofenceAsyncContext*>(data);
        auto callbackHost = context->callbackHost_;
        if (callbackHost != nullptr && context->errCode == ERRCODE_SUCCESS &&
            callbackHost->GetGeofenceOperationType() == GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_ADD) {
            LocationErrCode errCode = callbackHost->DealGeofenceOperationResult();
            if (errCode == ERRCODE_SUCCESS) {
                int fenceId = callbackHost->GetFenceId();
                napi_create_object(context->env, &context->result[PARAM1]);
                napi_create_int64(context->env, fenceId, &context->result[PARAM1]);
                AddCallbackToGnssGeofenceCallbackHostMap(fenceId, callbackHost);
            } else {
                context->errCode = errCode;
            }
        }
    };
}

napi_value RemoveGnssGeofence(napi_env env, napi_callback_info info)
{
    LBSLOGI(LOCATOR_STANDARD, "RemoveGnssGeofence called.");
    LBSLOGI(LOCATOR_STANDARD, "%{public}s called.", __func__);
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    if (argc > PARAM1) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    int fenceId = -1;
    napi_valuetype valueType1;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType1));
    if (valueType1 != napi_number) {
        HandleSyncErrCode(env, ERRCODE_INVALID_PARAM);
        return UndefinedNapiValue(env);
    }
    NAPI_CALL(env, napi_get_value_int32(env, argv[0], &fenceId));
    auto asyncContext = new (std::nothrow) GnssGeofenceAsyncContext(env);
    NAPI_ASSERT(env, asyncContext != nullptr, "asyncContext is null.");
    asyncContext->fenceId_ = fenceId;
    asyncContext->callbackHost_ = FindCallbackInGnssGeofenceCallbackHostMap(fenceId);
    if (napi_create_string_latin1(env, "removeGnssGeofence",
        NAPI_AUTO_LENGTH, &asyncContext->resourceName) != napi_ok) {
        GET_AND_THROW_LAST_ERROR(env);
        delete asyncContext;
        return nullptr;
    }
    asyncContext->beginTime = CommonUtils::GetCurrentTimeMilSec();
    SetExecuteFuncForRemoveGnssGeofenceContext(asyncContext);
    SetCompleteFuncForRemoveGnssGeofenceContext(asyncContext);
    size_t objectArgsNum = 1;
    return DoAsyncWork(env, asyncContext, argc, argv, objectArgsNum);
}

void SetExecuteFuncForRemoveGnssGeofenceContext(GnssGeofenceAsyncContext* asyncContext)
{
    LBSLOGI(LOCATOR_STANDARD, "SetExecuteFuncForRemoveGnssGeofenceContext called.");
    asyncContext->executeFunc = [&](void* data) -> void {
        auto context = static_cast<GnssGeofenceAsyncContext*>(data);
        std::shared_ptr<GeofenceRequest> request = std::make_shared<GeofenceRequest>();
        request->SetFenceId(context->fenceId_);
        if (context->callbackHost_ == nullptr) {
            context->errCode = ERRCODE_GEOFENCE_INCORRECT_ID;
            return;
        }
        context->errCode = g_geofenceClient->RemoveGnssGeofence(request, context->callbackHost_);
        auto callbackHost = context->callbackHost_;
        if (callbackHost != nullptr) {
            if (context->errCode != ERRCODE_SUCCESS) {
            }
        } else {
            context->errCode = ERRCODE_GEOFENCE_INCORRECT_ID;
        }
    };
}

void SetCompleteFuncForRemoveGnssGeofenceContext(GnssGeofenceAsyncContext* asyncContext)
{
    asyncContext->completeFunc = [&](void* data) -> void {
        auto context = static_cast<GnssGeofenceAsyncContext*>(data);
        auto callbackHost = context->callbackHost_;
        if (callbackHost != nullptr && context->errCode == ERRCODE_SUCCESS &&
            callbackHost->GetGeofenceOperationType() ==
            GnssGeofenceOperateType::GNSS_GEOFENCE_OPT_TYPE_DELETE) {
            LocationErrCode errCode = callbackHost->DealGeofenceOperationResult();
            if (errCode == ERRCODE_SUCCESS) {
                NAPI_CALL_RETURN_VOID(
                    context->env, napi_get_undefined(context->env, &context->result[PARAM1]));
                RemoveCallbackToGnssGeofenceCallbackHostMap(context->fenceId_);
            } else {
                context->errCode = errCode;
            }
        }
        g_hiAppEventClient->WriteEndEvent(
            context->beginTime, context->errCode == ERRCODE_SUCCESS ? 0 : 1, context->errCode, "remoceGnssGeofence");
        LBSLOGI(LOCATOR_STANDARD, "Push RemoveGnssGeofence result to client");
    };
}

napi_value GetGeofenceSupportedCoordTypes(napi_env env, napi_callback_info info)
{
    size_t argc = MAXIMUM_JS_PARAMS;
    napi_value argv[MAXIMUM_JS_PARAMS];
    napi_value thisVar = nullptr;
    void* data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, g_locatorClient != nullptr, "get locator SA failed");
    std::vector<CoordinateSystemType> coordinateSystemTypes;
    int64_t beginTime = CommonUtils::GetCurrentTimeMilSec();
    LocationErrCode errorCode =
        g_geofenceClient->GetGeofenceSupportedCoordTypes(coordinateSystemTypes);
    g_hiAppEventClient->WriteEndEvent(
        beginTime, errorCode == ERRCODE_SUCCESS ? 0 : 1, errorCode, "getGeofenceSupportedCoordTypes");
    if (errorCode != ERRCODE_SUCCESS) {
        HandleSyncErrCode(env, errorCode);
        return UndefinedNapiValue(env);
    }
    napi_value res;
    NAPI_CALL(env,
        napi_create_array_with_length(env, coordinateSystemTypes.size(), &res));
    uint32_t idx = 0;
    for (auto iter = coordinateSystemTypes.begin(); iter != coordinateSystemTypes.end(); ++iter) {
        auto coordType = *iter;
        napi_value eachObj;
        NAPI_CALL(env, napi_create_int32(env, static_cast<int>(coordType), &eachObj));
        NAPI_CALL(env, napi_set_element(env, res, idx++, eachObj));
    }
    return res;
}

void AddCallbackToGnssGeofenceCallbackHostMap(int fenceId, sptr<LocationGnssGeofenceCallbackNapi> callbackHost)
{
    std::unique_lock<std::mutex> lock(g_gnssGeofenceCallbackHostMutex);
    g_gnssGeofenceCallbackHostMap.insert(std::make_pair(fenceId, callbackHost));
}

void RemoveCallbackToGnssGeofenceCallbackHostMap(int fenceId)
{
    std::unique_lock<std::mutex> lock(g_gnssGeofenceCallbackHostMutex);
    auto iterForDelete = g_gnssGeofenceCallbackHostMap.find(fenceId);
    if (iterForDelete != g_gnssGeofenceCallbackHostMap.end()) {
        g_gnssGeofenceCallbackHostMap.erase(iterForDelete);
    }
}

sptr<LocationGnssGeofenceCallbackNapi> FindCallbackInGnssGeofenceCallbackHostMap(int fenceId)
{
    std::unique_lock<std::mutex> lock(g_gnssGeofenceCallbackHostMutex);
    auto iter = g_gnssGeofenceCallbackHostMap.find(fenceId);
    if (iter != g_gnssGeofenceCallbackHostMap.end()) {
        return iter->second;
    }
    return nullptr;
}
#endif
} // namespace Location
} // namespace OHOS
