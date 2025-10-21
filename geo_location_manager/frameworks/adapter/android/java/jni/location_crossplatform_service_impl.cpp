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

#include <memory>

#include "constant_definition.h"
#include "country_code.h"
#include "location_crossplatform_service_impl.h"
#include "location.h"
#include "location_log.h"
#include "location_service_jni.h"

namespace OHOS {
namespace Plugin {
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATION_CROSSPLATFORM_SERVICE_IMPL = { LOG_CORE, LOCATION_LOG_DOMAIN,
    "LocationCrossplatformServiceImpl" };

std::shared_ptr<LocationCrossplatformService> LocationCrossplatformService::Get()
{
    return std::make_shared<LocatorCrossplatformServiceImpl>();
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::GetSwitchState(int32_t& state)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::GetSwitchState");
    return LocationServiceJni::GetSwitchState(state);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<Location::LocatorCallbackNapi>& locatorCallbackHost)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::StartLocating");
    return LocationServiceJni::StartLocating(requestConfig, locatorCallbackHost);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::StopLocating(
    sptr<Location::LocatorCallbackNapi>& locatorCallbackHost)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::StopLocating");
    return LocationServiceJni::StopLocating(locatorCallbackHost);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::IsGeoConvertAvailable(bool& isAvailable)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::IsGeoConvertAvailable");
    return LocationServiceJni::IsGeoConvertAvailable(isAvailable);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::GetAddressByCoordinate(
    std::unique_ptr<Location::ReverseGeoCodeRequest>& data,
    std::list<std::shared_ptr<Location::GeoAddress>>& replyList)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::GetAddressByCoordinate");
    return LocationServiceJni::GetAddressByCoordinate(data, replyList);
}

std::shared_ptr<Location::CountryCode> LocatorCrossplatformServiceImpl::GetIsoCountryCode()
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::GetIsoCountryCode");
    return LocationServiceJni::GetIsoCountryCode();
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::AddGnssGeofence(GeofenceRequest& request,
    sptr<Location::LocationGnssGeofenceCallbackNapi>& callback)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::AddGnssGeofence");
    return LocationServiceJni::AddGnssGeofence(request, callback);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::RemoveGnssGeofence(int32_t fenceId,
    sptr<Location::LocationGnssGeofenceCallbackNapi>& callback)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::RemoveGnssGeofence");
    return LocationServiceJni::RemoveGnssGeofence(fenceId, callback);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::GetCurrentWifiBssidForLocating(std::string& bssid)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::GetCurrentWifiBssidForLocating");
    return LocationServiceJni::GetCurrentWifiBssidForLocating(bssid);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::RegisterSwitchCallback(
    sptr<Location::LocationSwitchCallbackNapi>& switchCallbackHost)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::RegisterSwitchCallback");
    return LocationServiceJni::RegisterSwitchCallback(switchCallbackHost);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::UnregisterSwitchCallback(
    sptr<Location::LocationSwitchCallbackNapi>& switchCallbackHost)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::UnregisterSwitchCallback");
    return LocationServiceJni::UnregisterSwitchCallback(switchCallbackHost);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::RegisterCountryCodeCallback(
    sptr<Location::CountryCodeCallbackNapi>& callbackHost)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::RegisterCountryCodeCallback");
    return LocationServiceJni::RegisterCountryCodeCallback(callbackHost);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::UnregisterCountryCodeCallback(
    sptr<Location::CountryCodeCallbackNapi>& callbackHost)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::UnregisterCountryCodeCallback");
    return LocationServiceJni::UnregisterCountryCodeCallback(callbackHost);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::RegisterGnssStatusCallback(
    sptr<Location::GnssStatusCallbackNapi>& cb)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::RegisterGnssStatusCallback");
    return LocationServiceJni::RegisterGnssStatusCallback(cb);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::UnregisterGnssStatusCallback(
    sptr<Location::GnssStatusCallbackNapi>& callback)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::UnregisterGnssStatusCallback");
    return LocationServiceJni::UnregisterGnssStatusCallback(callback);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::RegisterNmeaMessageCallback(
    sptr<Location::NmeaMessageCallbackNapi>& cb)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::RegisterNmeaMessageCallback");
    return LocationServiceJni::RegisterNmeaMessageCallback(cb);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::UnregisterNmeaMessageCallback(
    sptr<Location::NmeaMessageCallbackNapi>& cb)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::UnregisterNmeaMessageCallback");
    return LocationServiceJni::UnregisterNmeaMessageCallback(cb);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::SubscribeLocationError(
    sptr<Location::LocationErrorCallbackNapi>& locationErrorCallbackHost)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::SubscribeLocationError");
    return LocationServiceJni::SubscribeLocationError(locationErrorCallbackHost);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::SubscribeBluetoothScanResultChange(
    sptr<Location::BluetoothScanResultCallbackNapi>& callback)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::SubscribeBluetoothScanResultChange");
    return LocationServiceJni::SubscribeBluetoothScanResultChange(callback);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::UnSubscribeLocationError(
    sptr<Location::LocationErrorCallbackNapi>& locationErrorCallbackHost)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::UnSubscribeLocationError");
    return LocationServiceJni::UnsubscribeLocationError(locationErrorCallbackHost);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::UnSubscribeBluetoothScanResultChange(
    sptr<Location::BluetoothScanResultCallbackNapi>& bluetoothScanResultCallbackHost)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL,
        "LocatorCrossplatformServiceImpl::UnSubscribeBluetoothScanResultChange");
    return LocationServiceJni::UnsubscribeBluetoothScanResultChange(bluetoothScanResultCallbackHost);
}

Location::LocationErrCode LocatorCrossplatformServiceImpl::GetAddressByLocationName(
    std::unique_ptr<Location::GeoCodeRequest>& request,
    std::list<std::shared_ptr<Location::GeoAddress>>& replyList)
{
    LBSLOGI(LOCATION_CROSSPLATFORM_SERVICE_IMPL, "LocatorCrossplatformServiceImpl::GetAddressByLocationName");
    return LocationServiceJni::GetAddressByLocationName(request, replyList);
}

} // namespace Plugin
} // namespace OHOS