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

#include "locator_impl.h"
#include "country_code.h"
#include "location_crossplatform_service.h"

#include "location_log.h"
#include "geocode_convert_address_request.h"
#include "cached_location_manager.h"

#include <set>

namespace OHOS {
namespace Location {
std::shared_ptr<LocatorImpl> LocatorImpl::instance_ = nullptr;
std::mutex LocatorImpl::locatorMutex_;
std::mutex g_nmeaCallbacksMutex;
std::mutex g_gnssStatusInfoCallbacksMutex;
static std::set<sptr<IRemoteObject>> g_nmeaCallbacks;
static std::set<sptr<IRemoteObject>> g_gnssStatusInfoCallbacks;

std::shared_ptr<LocatorImpl> LocatorImpl::GetInstance()
{
    if (instance_ == nullptr) {
        std::unique_lock<std::mutex> lock(locatorMutex_);
        if (instance_ == nullptr) {
            std::shared_ptr<LocatorImpl> locator = std::make_shared<LocatorImpl>();
            instance_ = locator;
        }
    }
    return instance_;
}

LocatorImpl::LocatorImpl()
{
}

LocatorImpl::~LocatorImpl()
{
}

std::unique_ptr<Location> LocatorImpl::GetCachedLocation()
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl_GetCachedLocation");
    auto locationObj = std::make_unique<Location>();
    locationObj = CachedLocationManager::GetInstance().GetLastLocation();
    return locationObj;
}

bool LocatorImpl::IsLocationEnabled()
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl_IsLocationEnabled");
    int32_t state = DEFAULT_SWITCH_STATE;
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return false;
    }
    service->GetSwitchState(state);
    return state;
}

LocationErrCode LocatorImpl::StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
    sptr<LocatorCallbackNapi>& locatorCallbackHost)
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl_StartLocating enter");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return service->StartLocating(requestConfig, locatorCallbackHost);
}

LocationErrCode LocatorImpl::StopLocating(sptr<LocatorCallbackNapi>& locatorCallbackHost)
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl_StopLocating enter.");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return service->StopLocating(locatorCallbackHost);
}

LocationErrCode LocatorImpl::RegisterSwitchCallback(sptr<LocationSwitchCallbackNapi>& switchCallbackHost)
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl_RegisterSwitchCallback enter");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return service->RegisterSwitchCallback(switchCallbackHost);
}

LocationErrCode LocatorImpl::UnregisterSwitchCallback(sptr<LocationSwitchCallbackNapi>& switchCallbackHost)
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl_UnRegisterSwitchCallback enter");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return service->UnregisterSwitchCallback(switchCallbackHost);
}

bool LocatorImpl::RegisterGnssStatusCallback(sptr<GnssStatusCallbackNapi>& callback, pid_t uid)
{
    if (IsSatelliteStatusChangeCallbackRegistered(callback)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback has registered.", __func__);
        return false;
    }
    AddSatelliteStatusChangeCallBack(callback);
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    service->RegisterGnssStatusCallback(callback);
    LBSLOGI(LOCATOR, "RegisterGnssStatusCallback enter");
    return true;
}

bool LocatorImpl::UnregisterGnssStatusCallback(sptr<GnssStatusCallbackNapi>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return false;
    }
    RemoveSatelliteStatusChangeCallBack(callback);
    LBSLOGI(LOCATOR, "UnregisterGnssStatusCallback enter");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    service->UnregisterGnssStatusCallback(callback);
    return true;
}

bool LocatorImpl::RegisterNmeaMessageCallback(sptr<NmeaMessageCallbackNapi>& callback, pid_t uid)
{
    LBSLOGI(LOCATOR, "RegisterNmeaMessageCallback enter");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    service->RegisterNmeaMessageCallback(callback);
    return true;
}

bool LocatorImpl::UnregisterNmeaMessageCallback(sptr<NmeaMessageCallbackNapi>& callback)
{
    LBSLOGI(LOCATOR, "UnregisterNmeaMessageCallback enter");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    service->UnregisterNmeaMessageCallback(callback);
    return true;
}

LocationErrCode LocatorImpl::RegisterCountryCodeCallback(sptr<CountryCodeCallbackNapi>& callbackHost)
{
    LBSLOGI(LOCATOR, "RegisterCountryCodeCallback enter");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return service->RegisterCountryCodeCallback(callbackHost);
}

LocationErrCode LocatorImpl::UnregisterCountryCodeCallback(sptr<CountryCodeCallbackNapi>& callbackHost)
{
    LBSLOGE(LOCATOR, "UNRegisterCountryCodeCallback enter");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return service->UnregisterCountryCodeCallback(callbackHost);
}

void LocatorImpl::RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
    sptr<ICachedLocationsCallback>& callback)
{
}

void LocatorImpl::UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback)
{
}

bool LocatorImpl::IsGeoServiceAvailable()
{
    LBSLOGI(LOCATOR_STANDARD, "IsGeoServiceAvailable called.");
    bool res = false;
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return false;
    }
    service->IsGeoConvertAvailable(res);
    return res;
}

void LocatorImpl::GetAddressByCoordinate(std::unique_ptr<ReverseGeoCodeRequest>& request,
    std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    LBSLOGI(LOCATOR_STANDARD, "GetAddressByCoordinate called.");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return;
    }
    service->GetAddressByCoordinate(request, replyList);
}

void LocatorImpl::GetAddressByLocationName(std::unique_ptr<GeoCodeRequest> &request,
    std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    LBSLOGI(LOCATOR_STANDARD, "GetAddressByLocationName called.");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return;
    }
    service->GetAddressByLocationName(request, replyList);
}

int LocatorImpl::GetCachedGnssLocationsSize()
{
    int size = -1;
    return size;
}

int LocatorImpl::FlushCachedGnssLocations()
{
    return -1;
}

std::shared_ptr<CountryCode> LocatorImpl::GetIsoCountryCode()
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl::GetIsoCountryCode()");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return nullptr;
    }
    return service->GetIsoCountryCode();
}

LocationErrCode LocatorImpl::IsLocationEnabledV9(bool &isEnabled)
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl::IsLocationEnabledV9()");
    int32_t state = DEFAULT_SWITCH_STATE;
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    LocationErrCode code = service->GetSwitchState(state);
    isEnabled = (state == 1);
    return code;
}

LocationErrCode LocatorImpl::RegisterGnssStatusCallbackV9(sptr<GnssStatusCallbackNapi>& callback)
{
    LBSLOGE(LOCATOR_STANDARD, "RegisterGnssStatusCallbackV9 enter");
    if (IsSatelliteStatusChangeCallbackRegistered(callback)) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback has registered.", __func__);
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    AddSatelliteStatusChangeCallBack(callback);
    LBSLOGI(LOCATOR_STANDARD, "RegisterGnssStatusCallbackV9 enter");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    return service->RegisterGnssStatusCallback(callback);
}

LocationErrCode LocatorImpl::UnregisterGnssStatusCallbackV9(sptr<GnssStatusCallbackNapi>& callback)
{
    if (callback == nullptr) {
        LBSLOGE(LOCATOR_STANDARD, "%{public}s callback is nullptr", __func__);
        return ERRCODE_INVALID_PARAM;
    }
    RemoveSatelliteStatusChangeCallBack(callback);
    LBSLOGI(LOCATOR_STANDARD, "UnregisterGnssStatusCallbackV9 enter");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    return service->UnregisterGnssStatusCallback(callback);
}

LocationErrCode LocatorImpl::RegisterNmeaMessageCallbackV9(sptr<NmeaMessageCallbackNapi>& callback)
{
    LBSLOGI(LOCATOR_STANDARD, "RegisterNmeaMessageCallbackV9 enter");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    return service->RegisterNmeaMessageCallback(callback);
}

LocationErrCode LocatorImpl::UnregisterNmeaMessageCallbackV9(sptr<NmeaMessageCallbackNapi>& callback)
{
    LBSLOGI(LOCATOR_STANDARD, "UnregisterNmeaMessageCallbackV9 enter");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    return service->UnregisterNmeaMessageCallback(callback);
}

LocationErrCode LocatorImpl::RegisterCachedLocationCallbackV9(std::unique_ptr<CachedGnssLocationsRequest>& request,
    sptr<ICachedLocationsCallback>& callback)
{
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::UnregisterCachedLocationCallbackV9(sptr<ICachedLocationsCallback>& callback)
{
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::IsGeoServiceAvailableV9(bool &isAvailable)
{
    LBSLOGI(LOCATOR_STANDARD, "IsGeoServiceAvailableV9 called.");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    service->IsGeoConvertAvailable(isAvailable);
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::GetAddressByCoordinateV9(std::unique_ptr<ReverseGeoCodeRequest>& request,
    std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    LBSLOGI(LOCATOR_STANDARD, "GetAddressByCoordinateV9 called.");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return service->GetAddressByCoordinate(request, replyList);
}

LocationErrCode LocatorImpl::GetAddressByLocationNameV9(std::unique_ptr<GeoCodeRequest> &request,
    std::list<std::shared_ptr<GeoAddress>>& replyList)
{
    LBSLOGI(LOCATOR_STANDARD, "GetAddressByLocationNameV9 called.");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return service->GetAddressByLocationName(request, replyList);
}

LocationErrCode LocatorImpl::GetCachedGnssLocationsSizeV9(int &size)
{
    size = -1;
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::FlushCachedGnssLocationsV9()
{
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::GetIsoCountryCodeV9(std::shared_ptr<CountryCode>& countryCode)
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl::GetIsoCountryCodeV9()");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    countryCode = service->GetIsoCountryCode();
    return ERRCODE_SUCCESS;
}

LocationErrCode LocatorImpl::SubscribeBluetoothScanResultChange(
    sptr<BluetoothScanResultCallbackNapi>& bluetoothScanResultCallbackHost)
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl::SubscribeBluetoothScanResultChange()");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return service->SubscribeBluetoothScanResultChange(bluetoothScanResultCallbackHost);
}

LocationErrCode LocatorImpl::UnSubscribeBluetoothScanResultChange(
    sptr<BluetoothScanResultCallbackNapi>& bluetoothScanResultCallbackHost)
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl::UnSubscribeBluetoothScanResultChange()");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return service->UnSubscribeBluetoothScanResultChange(bluetoothScanResultCallbackHost);
}

LocationErrCode LocatorImpl::SubscribeLocationError(sptr<LocationErrorCallbackNapi>& locationErrorCallbackHost)
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl::SubscribeLocationError()");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return service->SubscribeLocationError(locationErrorCallbackHost);
}

LocationErrCode LocatorImpl::UnSubscribeLocationError(sptr<LocationErrorCallbackNapi>& locationErrorCallbackHost)
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl::UnSubscribeLocationError()");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return service->UnSubscribeLocationError(locationErrorCallbackHost);
}

LocationErrCode LocatorImpl::GetCurrentWifiBssidForLocating(std::string& bssid)
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl::GetCurrentWifiBssidForLocating() enter");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    if (!service) {
        return ERRCODE_SERVICE_UNAVAILABLE;
    }
    return service->GetCurrentWifiBssidForLocating(bssid);
}

LocationErrCode LocatorImpl::GetDistanceBetweenLocations(const Location& location1,
    const Location& location2, double& distance)
{
    LBSLOGI(LOCATOR_STANDARD, "LocatorImpl::GetDistanceBetweenLocations() enter");
    distance = location1.GetDistanceBetweenLocations(location1.GetLatitude(), location1.GetLongitude(),
        location2.GetLatitude(), location2.GetLongitude());
    return ERRCODE_SUCCESS;
}

bool LocatorImpl::IsSatelliteStatusChangeCallbackRegistered(const sptr<IRemoteObject>& callback)
{
    if (callback == nullptr) {
        return true;
    }
    std::unique_lock<std::mutex> lock(g_gnssStatusInfoCallbacksMutex);
    for (auto gnssStatusCallback : g_gnssStatusInfoCallbacks) {
        if (gnssStatusCallback == callback) {
            return true;
        }
    }
    return false;
}

void LocatorImpl::AddSatelliteStatusChangeCallBack(const sptr<IRemoteObject>& callback)
{
    std::unique_lock<std::mutex> lock(g_gnssStatusInfoCallbacksMutex);
    g_gnssStatusInfoCallbacks.insert(callback);
}

void LocatorImpl::RemoveSatelliteStatusChangeCallBack(const sptr<IRemoteObject>& callback)
{
    std::unique_lock<std::mutex> lock(g_gnssStatusInfoCallbacksMutex);
    for (auto iter = g_gnssStatusInfoCallbacks.begin(); iter != g_gnssStatusInfoCallbacks.end(); iter++) {
        if (callback == *iter) {
            g_gnssStatusInfoCallbacks.erase(callback);
            break;
        }
    }
}

}  // namespace Location
}  // namespace OHOS
