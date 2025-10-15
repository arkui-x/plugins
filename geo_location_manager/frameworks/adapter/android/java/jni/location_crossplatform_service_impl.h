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

#ifndef LOCATOR_CROSSPLATFORM_SERVICE_IMPL_H
#define LOCATOR_CROSSPLATFORM_SERVICE_IMPL_H

#include <string>

#include "location.h"
#include "location_crossplatform_service.h"

namespace OHOS {
namespace Plugin {

class LocatorCrossplatformServiceImpl : public LocationCrossplatformService {
public:
    LocatorCrossplatformServiceImpl() = default;
    ~LocatorCrossplatformServiceImpl() override = default;

    Location::LocationErrCode GetSwitchState(int32_t& state) override;
    Location::LocationErrCode StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<Location::LocatorCallbackNapi>& locatorCallbackHost) override;
    Location::LocationErrCode StopLocating(sptr<Location::LocatorCallbackNapi>& locatorCallbackHost) override;
    Location::LocationErrCode IsGeoConvertAvailable(bool& isAvailable) override;
    Location::LocationErrCode GetAddressByLocationName(
        std::unique_ptr<Location::GeoCodeRequest>& request,
        std::list<std::shared_ptr<Location::GeoAddress>>& replyList) override;
    Location::LocationErrCode GetAddressByCoordinate(std::unique_ptr<Location::ReverseGeoCodeRequest>& data,
        std::list<std::shared_ptr<Location::GeoAddress>>& replyList) override;
    std::shared_ptr<Location::CountryCode> GetIsoCountryCode() override;
    Location::LocationErrCode AddGnssGeofence(GeofenceRequest& request,
        sptr<Location::LocationGnssGeofenceCallbackNapi>& callback) override;
    Location::LocationErrCode RemoveGnssGeofence(int32_t fenceId,
        sptr<Location::LocationGnssGeofenceCallbackNapi>& callback) override;
    Location::LocationErrCode GetCurrentWifiBssidForLocating(std::string& bssid) override;
    Location::LocationErrCode RegisterSwitchCallback(
        sptr<Location::LocationSwitchCallbackNapi>& switchCallbackHost) override;
    Location::LocationErrCode UnregisterSwitchCallback(
        sptr<Location::LocationSwitchCallbackNapi>& switchCallbackHost) override;
    Location::LocationErrCode RegisterCountryCodeCallback(
        sptr<Location::CountryCodeCallbackNapi>& callbackHost) override;
    Location::LocationErrCode UnregisterCountryCodeCallback(
        sptr<Location::CountryCodeCallbackNapi>& callbackHost) override;
    Location::LocationErrCode RegisterGnssStatusCallback(sptr<Location::GnssStatusCallbackNapi>& cb) override;
    Location::LocationErrCode UnregisterGnssStatusCallback(sptr<Location::GnssStatusCallbackNapi>& callback) override;
    Location::LocationErrCode RegisterNmeaMessageCallback(sptr<Location::NmeaMessageCallbackNapi>& cb) override;
    Location::LocationErrCode UnregisterNmeaMessageCallback(sptr<Location::NmeaMessageCallbackNapi>& cb) override;
    Location::LocationErrCode SubscribeLocationError(
        sptr<Location::LocationErrorCallbackNapi>& locationErrorCallbackHost) override;
    Location::LocationErrCode SubscribeBluetoothScanResultChange(
        sptr<Location::BluetoothScanResultCallbackNapi>& callback) override;
    Location::LocationErrCode UnSubscribeLocationError(
        sptr<Location::LocationErrorCallbackNapi>& locationErrorCallbackHost) override;
    Location::LocationErrCode UnSubscribeBluetoothScanResultChange(
        sptr<Location::BluetoothScanResultCallbackNapi>& bluetoothScanResultCallbackHost) override;
};

} // namespace Plugin
} // namespace OHOS

#endif // LOCATOR_CROSSPLATFORM_SERVICE_IMPL_H
