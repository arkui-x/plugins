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

#ifndef LOCATION_NAPI_EVENT_H
#define LOCATION_NAPI_EVENT_H

#include "cached_locations_callback_napi.h"
#include "gnss_status_callback_napi.h"
#include "location_switch_callback_napi.h"
#include "locator.h"
#include "locator_callback_napi.h"
#include "nmea_message_callback_napi.h"
#include "request_config.h"
#include "location_error_callback_napi.h"
#include "bluetooth_scan_result_callback_napi.h"

namespace OHOS {
namespace Location {
void InitOnFuncMap();
bool OnLocationServiceStateCallback(const napi_env& env, const size_t argc, const napi_value* argv);
bool OnCachedGnssLocationsReportingCallback(const napi_env& env, const size_t argc, const napi_value* argv);
bool OnGnssStatusChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv);
bool OnLocationChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv);
bool OnNmeaMessageChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv);
bool OnCountryCodeChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv);
bool OnFenceStatusChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv);
#ifdef ENABLE_NAPI_MANAGER
bool OnLocationErrorCallback(const napi_env& env, const size_t argc, const napi_value* argv);
bool OnBluetoothScanResultChangeCallback(const napi_env& env, const size_t argc, const napi_value* argv);
#endif

void InitOffFuncMap();
bool OffAllLocationServiceStateCallback(const napi_env& env);
bool OffAllLocationChangeCallback(const napi_env& env);
bool OffAllGnssStatusChangeCallback(const napi_env& env);
bool OffAllNmeaMessageChangeCallback(const napi_env& env);
bool OffAllCachedGnssLocationsReportingCallback(const napi_env& env);
bool OffAllCountryCodeChangeCallback(const napi_env& env);
bool OffLocationServiceStateCallback(const napi_env& env, const napi_value& handler);
bool OffLocationChangeCallback(const napi_env& env, const napi_value& handler);
bool OffGnssStatusChangeCallback(const napi_env& env, const napi_value& handler);
bool OffNmeaMessageChangeCallback(const napi_env& env, const napi_value& handler);
bool OffCachedGnssLocationsReportingCallback(const napi_env& env, const napi_value& handler);
bool OffCountryCodeChangeCallback(const napi_env& env, const napi_value& handler);
#ifdef ENABLE_NAPI_MANAGER
bool OffLocationErrorCallback(const napi_env& env, const napi_value& handler);
bool OffBluetoothScanResultChangeCallback(const napi_env& env, const napi_value& handler);
bool OffAllBluetoothScanResultChangeCallback(const napi_env& env);
#endif

void SubscribeLocationServiceState(const napi_env& env,
    const napi_ref& handlerRef, sptr<LocationSwitchCallbackNapi>& switchCallbackHost);
void SubscribeGnssStatus(const napi_env& env, const napi_ref& handlerRef,
    sptr<GnssStatusCallbackNapi>& gnssStatusCallbackHost);
void SubscribeNmeaMessage(const napi_env& env, const napi_ref& handlerRef,
    sptr<NmeaMessageCallbackNapi>& nmeaMessageCallbackHost);
void SubscribeLocationChange(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<LocatorCallbackNapi>& locatorCallbackHost);
void SubscribeCacheLocationChange(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<CachedLocationsCallbackNapi>& cachedCallbackHost);
void SubscribeFenceStatusChange(const napi_env& env, const napi_value& object, const napi_value& handler);
LocationErrCode UnSubscribeLocationChange(sptr<LocatorCallbackNapi>& locatorCallbackHost);
LocationErrCode UnSubscribeFenceStatusChange(const napi_env& env, const napi_value& object, const napi_value& handler);
void UnSubscribeCacheLocationChange(sptr<ICachedLocationsCallback>& callback);
LocationErrCode UnSubscribeLocationServiceState(sptr<LocationSwitchCallbackNapi>& switchCallbackHost);
LocationErrCode UnSubscribeGnssStatus(sptr<GnssStatusCallbackNapi>& gnssStatusCallbackHost);
LocationErrCode UnSubscribeNmeaMessage(sptr<NmeaMessageCallbackNapi>& nmeaMessageCallbackHost);
bool IsCallbackEquals(const napi_env& env, const napi_value& handler, const napi_ref& savedCallback);
void GenRequestConfig(const napi_env& env, const napi_value* argv,
    const size_t& objectArgsNum, std::unique_ptr<RequestConfig>& requestConfig);
bool IsRequestConfigValid(std::unique_ptr<RequestConfig>& config);
napi_value RequestLocationOnce(const napi_env& env, const size_t argc, const napi_value* argv);
napi_value On(napi_env env, napi_callback_info cbinfo);
napi_value Off(napi_env env, napi_callback_info cbinfo);
napi_value GetCurrentLocation(napi_env env, napi_callback_info cbinfo);
void UpdateErrorCodeOfContext(SingleLocationAsyncContext* context);
void UpdatePoiErrorCode(bool isNeedLocation, SingleLocationAsyncContext* context);
bool GetIsNeedLocation(SingleLocationAsyncContext* context);
bool NeedReportLastLocation(const std::unique_ptr<RequestConfig>& config, const std::unique_ptr<Location>& location);

#ifdef ENABLE_NAPI_MANAGER
LocationErrCode SubscribeLocationServiceStateV9(const napi_env& env,
    const napi_ref& handlerRef, sptr<LocationSwitchCallbackNapi>& switchCallbackHost);
LocationErrCode SubscribeGnssStatusV9(const napi_env& env, const napi_ref& handlerRef,
    sptr<GnssStatusCallbackNapi>& gnssStatusCallbackHost);
LocationErrCode SubscribeNmeaMessageV9(const napi_env& env, const napi_ref& handlerRef,
    sptr<NmeaMessageCallbackNapi>& nmeaMessageCallbackHost);
LocationErrCode SubscribeLocationChangeV9(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<LocatorCallbackNapi>& locatorCallbackHost);
LocationErrCode SubscribeCacheLocationChangeV9(const napi_env& env, const napi_value& object,
    const napi_ref& handlerRef, sptr<CachedLocationsCallbackNapi>& cachedCallbackHost);
LocationErrCode SubscribeFenceStatusChangeV9(const napi_env& env, const napi_value& object, const napi_value& handler);
LocationErrCode SubscribeLocationError(const napi_env& env,
    const napi_ref& handlerRef, sptr<LocationErrorCallbackNapi>& locationErrorCallbackHost);
LocationErrCode SubscribeBluetoothScanResultChange(const napi_env& env,
    const napi_ref& handlerRef, sptr<BluetoothScanResultCallbackNapi>& bluetoothScanResultCallbackHost);
LocationErrCode UnSubscribeBluetoothScanResultChange(
    sptr<BluetoothScanResultCallbackNapi>& bluetoothScanResultCallbackHost);
LocationErrCode UnSubscribeLocationError(sptr<LocationErrorCallbackNapi>& locationErrorCallbackHost);
napi_value RequestLocationOnceV9(const napi_env& env, const size_t argc, const napi_value* argv);
LocationErrCode CheckLocationSwitchEnable();
napi_value RequestPoiInfoOnce(const napi_env& env, const size_t argc, const napi_value* argv);
#endif
}  // namespace Location
}  // namespace OHOS
#endif // LOCATION_NAPI_EVENT_H
