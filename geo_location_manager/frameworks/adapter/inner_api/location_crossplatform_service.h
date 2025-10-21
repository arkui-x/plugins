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

#ifndef LOCATION_CROSSPLATFORM_SERVICE_H
#define LOCATION_CROSSPLATFORM_SERVICE_H

#include <cstdint>
#include <vector>

#include "bluetooth_scan_result_callback_napi.h"
#include "country_code.h"
#include "country_code_callback_napi.h"
#include "geofence_request.h"
#include "geo_address.h"
#include "geo_code_request.h"
#include "geo_convert_callback_host.h"
#include "gnss_status_callback_napi.h"
#include "location.h"
#include "location_error_callback_napi.h"
#include "location_gnss_geofence_callback_napi.h"
#include "location_switch_callback_napi.h"
#include "locator_callback_napi.h"
#include "nmea_message_callback_napi.h"
#include "request_config.h"

using OHOS::Location::LocationErrCode;
using OHOS::Location::Location;
using OHOS::Location::RequestConfig;
using OHOS::Location::GeofenceRequest;

namespace OHOS {
namespace Plugin {
class LocationCrossplatformService {
public:
    LocationCrossplatformService() = default;
    virtual ~LocationCrossplatformService() = default;
    static std::shared_ptr<LocationCrossplatformService> Get();

    virtual LocationErrCode GetSwitchState(int32_t& state) = 0;
    virtual LocationErrCode StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<Location::LocatorCallbackNapi>& locatorCallbackHost) = 0;
    virtual LocationErrCode StopLocating(sptr<Location::LocatorCallbackNapi>& locatorCallbackHost) = 0;
    virtual LocationErrCode IsGeoConvertAvailable(bool& isAvailable) = 0;
    virtual LocationErrCode GetAddressByLocationName(std::unique_ptr<Location::GeoCodeRequest> &request,
        std::list<std::shared_ptr<Location::GeoAddress>>& replyList) = 0;
    virtual LocationErrCode GetAddressByCoordinate(std::unique_ptr<Location::ReverseGeoCodeRequest> &request,
        std::list<std::shared_ptr<Location::GeoAddress>>& replyList) = 0;
    virtual std::shared_ptr<Location::CountryCode> GetIsoCountryCode() = 0;
    virtual LocationErrCode AddGnssGeofence(GeofenceRequest& request,
        sptr<Location::LocationGnssGeofenceCallbackNapi>& callback) = 0;
    virtual LocationErrCode RemoveGnssGeofence(int32_t fenceId,
        sptr<Location::LocationGnssGeofenceCallbackNapi>& callback) = 0;
    virtual LocationErrCode GetCurrentWifiBssidForLocating(std::string& bssid) = 0;
    virtual LocationErrCode RegisterSwitchCallback(sptr<Location::LocationSwitchCallbackNapi>& switchCallbackHost) = 0;
    virtual LocationErrCode UnregisterSwitchCallback(
        sptr<Location::LocationSwitchCallbackNapi>& switchCallbackHost) = 0;
    virtual LocationErrCode RegisterCountryCodeCallback(sptr<Location::CountryCodeCallbackNapi>& callbackHost) = 0;
    virtual LocationErrCode UnregisterCountryCodeCallback(sptr<Location::CountryCodeCallbackNapi>& callbackHost) = 0;
    virtual LocationErrCode SubscribeLocationError(
        sptr<Location::LocationErrorCallbackNapi>& locationErrorCallbackHost) = 0;
    virtual LocationErrCode UnSubscribeLocationError(
        sptr<Location::LocationErrorCallbackNapi>& locationErrorCallbackHost) = 0;
    virtual LocationErrCode SubscribeBluetoothScanResultChange(
        sptr<Location::BluetoothScanResultCallbackNapi>& bluetoothScanResultCallbackHost) = 0;
    virtual LocationErrCode UnSubscribeBluetoothScanResultChange(
        sptr<Location::BluetoothScanResultCallbackNapi>& bluetoothScanResultCallbackHost) = 0;
    virtual LocationErrCode RegisterGnssStatusCallback(sptr<Location::GnssStatusCallbackNapi>& callback) = 0;
    virtual LocationErrCode UnregisterGnssStatusCallback(sptr<Location::GnssStatusCallbackNapi>& callback) = 0;
    virtual LocationErrCode RegisterNmeaMessageCallback(sptr<Location::NmeaMessageCallbackNapi>& cb) = 0;
    virtual LocationErrCode UnregisterNmeaMessageCallback(sptr<Location::NmeaMessageCallbackNapi>& cb) = 0;
};
} // namespace Plugin
} // namespace OHOS
#endif // LOCATION_CROSSPLATFORM_SERVICE_H
