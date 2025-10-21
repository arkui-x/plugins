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

#ifndef LOCATION_NAPI_ADAPTER_H
#define LOCATION_NAPI_ADAPTER_H

#include "common_utils.h"
#include "locator.h"
#include "napi_util.h"
#include "geofence_request.h"
#include "constant_definition.h"
#include "location_gnss_geofence_callback_napi.h"
#include "geofence_async_context.h"
#include "beacon_fence_request.h"
#include "geo_code_request.h"

namespace OHOS {
namespace Location {
napi_value GetLastLocation(napi_env env, napi_callback_info info);
napi_value IsLocationEnabled(napi_env env, napi_callback_info info);
napi_value IsGeoServiceAvailable(napi_env env, napi_callback_info info);
napi_value GetAddressesFromLocation(napi_env env, napi_callback_info info);
napi_value GetAddressesFromLocationName(napi_env env, napi_callback_info info);
napi_value GetCachedGnssLocationsSize(napi_env env, napi_callback_info info);
napi_value FlushCachedGnssLocations(napi_env env, napi_callback_info info);
void SetExecuteFuncForFlushCachedGnssLocationsContext(CachedAsyncContext* asyncContext);
void SetCompleteFuncForFlushCachedGnssLocationsContext(CachedAsyncContext* asyncContext);
void SetExecuteFuncForGetCachedGnssLocationsSizeContext(CachedAsyncContext* asyncContext);
void SetCompleteFuncForGetCachedGnssLocationsSizeContext(CachedAsyncContext* asyncContext);
#ifdef ENABLE_NAPI_MANAGER
napi_value GetIsoCountryCode(napi_env env, napi_callback_info info);
LocationErrCode CheckLocationSwitchState();
napi_value IsWlanBssidMatched(napi_env env, napi_callback_info info);
napi_value AddGnssGeofence(napi_env env, napi_callback_info info);
void SetExecuteFuncForAddGnssGeofenceContext(GnssGeofenceAsyncContext* asyncContext);
void SetCompleteFuncForAddGnssGeofenceContext(GnssGeofenceAsyncContext* asyncContext);
napi_value RemoveGnssGeofence(napi_env env, napi_callback_info info);
void SetExecuteFuncForRemoveGnssGeofenceContext(GnssGeofenceAsyncContext* asyncContext);
void SetCompleteFuncForRemoveGnssGeofenceContext(GnssGeofenceAsyncContext* asyncContext);
napi_value GetGeofenceSupportedCoordTypes(napi_env env, napi_callback_info info);
void AddCallbackToGnssGeofenceCallbackHostMap(int fenceId, sptr<LocationGnssGeofenceCallbackNapi> callbackHost);
void RemoveCallbackToGnssGeofenceCallbackHostMap(int fenceId);
sptr<LocationGnssGeofenceCallbackNapi> FindCallbackInGnssGeofenceCallbackHostMap(int fenceId);
napi_value GetCurrentWifiBssidForLocating(napi_env env, napi_callback_info info);
napi_value GetPoiInfo(napi_env env, napi_callback_info info);
napi_value GetDistanceBetweenLocations(napi_env env, napi_callback_info info);
napi_value AddBeaconFence(napi_env env, napi_callback_info info);
napi_value RemoveBeaconFence(napi_env env, napi_callback_info info);
void CreateAsyncContextForAddBeaconFence(GnssGeofenceAsyncContext* asyncContext);
void CreateAsyncContextForRemoveBeaconFence(GnssGeofenceAsyncContext* asyncContext);
void RemoveCallbackToBeaconFenceCallbackHostMap(std::shared_ptr<BeaconFence>& beaconFence);
sptr<LocationGnssGeofenceCallbackNapi> FindCallbackInBeaconFenceCallbackHostMap(
    std::shared_ptr<BeaconFence>& beaconFence);
void SetExecuteFuncForCountryCodeContext(CountryCodeContext* asyncContext);
void SetCompleteFuncForCountryCodeContext(CountryCodeContext* asyncContext);
#endif
}  // namespace Location
}  // namespace OHOS
#endif // LOCATION_NAPI_ADAPTER_H
