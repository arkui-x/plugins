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

#ifndef LOCATOR_CROSSPLATFORM_SERVICE_IOS_IMPL_H
#define LOCATOR_CROSSPLATFORM_SERVICE_IOS_IMPL_H

#include "location_crossplatform_service.h"

using OHOS::Location::LocationErrCode;

namespace OHOS {
namespace Plugin {
class LocatorCrossplatformServiceIosImpl : public LocationCrossplatformService {
public:
    LocatorCrossplatformServiceIosImpl() = default;
    ~LocatorCrossplatformServiceIosImpl() override = default;

    LocationErrCode GetSwitchState(int32_t& state) override;
    LocationErrCode StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<Location::LocatorCallbackNapi>& locatorCallbackHost) override;
    LocationErrCode StopLocating(sptr<Location::LocatorCallbackNapi>& locatorCallbackHost) override;
    LocationErrCode IsGeoConvertAvailable(bool& isAvailable) override;
    LocationErrCode GetAddressByLocationName(std::unique_ptr<Location::GeoCodeRequest> &request,
        std::list<std::shared_ptr<Location::GeoAddress>>& replyList) override;
    LocationErrCode GetAddressByCoordinate(std::unique_ptr<Location::ReverseGeoCodeRequest> &request,
        std::list<std::shared_ptr<Location::GeoAddress>>& replyList) override;
    std::shared_ptr<Location::CountryCode> GetIsoCountryCode() override;
    LocationErrCode AddGnssGeofence(GeofenceRequest& request,
        sptr<Location::LocationGnssGeofenceCallbackNapi>& callback) override;
    LocationErrCode RemoveGnssGeofence(int32_t fenceId,
        sptr<Location::LocationGnssGeofenceCallbackNapi>& callback) override;
    LocationErrCode GetCurrentWifiBssidForLocating(std::string& bssid) override;
    LocationErrCode RegisterSwitchCallback(sptr<Location::LocationSwitchCallbackNapi>& switchCallbackHost) override;
    LocationErrCode UnregisterSwitchCallback(sptr<Location::LocationSwitchCallbackNapi>& switchCallbackHost) override;
    LocationErrCode RegisterCountryCodeCallback(sptr<Location::CountryCodeCallbackNapi>& callbackHost) override;
    LocationErrCode UnregisterCountryCodeCallback(sptr<Location::CountryCodeCallbackNapi>& callbackHost) override;
    LocationErrCode SubscribeLocationError(
        sptr<Location::LocationErrorCallbackNapi>& locationErrorCallbackHost) override;
    LocationErrCode UnSubscribeLocationError(
        sptr<Location::LocationErrorCallbackNapi>& locationErrorCallbackHost) override;
    LocationErrCode SubscribeBluetoothScanResultChange(
        sptr<Location::BluetoothScanResultCallbackNapi>& bluetoothScanResultCallbackHost) override;
    LocationErrCode UnSubscribeBluetoothScanResultChange(
        sptr<Location::BluetoothScanResultCallbackNapi>& bluetoothScanResultCallbackHost) override;
    LocationErrCode RegisterGnssStatusCallback(sptr<Location::GnssStatusCallbackNapi>& callback) override;
    LocationErrCode UnregisterGnssStatusCallback(sptr<Location::GnssStatusCallbackNapi>& callback) override;
    LocationErrCode RegisterNmeaMessageCallback(sptr<Location::NmeaMessageCallbackNapi>& cb) override;
    LocationErrCode UnregisterNmeaMessageCallback(sptr<Location::NmeaMessageCallbackNapi>& cb) override;
    LocationErrCode ClearAllGnssGeofences();
};
} // namespace Plugin
} // namespace OHOS
#endif // LOCATOR_CROSSPLATFORM_SERVICE_IOS_IMPL_H
