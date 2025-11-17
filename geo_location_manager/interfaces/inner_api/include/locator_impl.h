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
#ifndef LOCATOR_IMPL_H
#define LOCATOR_IMPL_H

#include <vector>

#include "iremote_object.h"

#include "constant_definition.h"
#include "country_code.h"
#include "geo_address.h"
#include "i_cached_locations_callback.h"
#include "request_config.h"
#include "ibluetooth_scan_result_callback.h"
#include "beacon_fence_request.h"
#include "beacon_fence.h"
#include "locator_callback_napi.h"
#include "country_code_callback_napi.h"
#include "location_switch_callback_napi.h"
#include "location_error_callback_napi.h"
#include "bluetooth_scan_result_callback_napi.h"
#include "geo_code_request.h"
#include "gnss_status_callback_napi.h"
#include "nmea_message_callback_napi.h"

namespace OHOS {
namespace Location {
class ICallbackResumeManager {
public:
    virtual ~ICallbackResumeManager() = default;
    virtual void ResumeCallback() = 0;
};

class LocatorImpl {
public:
    static std::shared_ptr<LocatorImpl> GetInstance();
    explicit LocatorImpl();
    ~LocatorImpl();

    /**
     * @brief Obtain current location switch status.
     *
     * @return Returns true if the location switch on, returns false otherwise.
     */
    bool IsLocationEnabled();

    /**
     * @brief Subscribe location changed.
     *
     * @param requestConfig Indicates the location request parameters.
     * @param callback Indicates the callback for reporting the location result.
     */
    LocationErrCode StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<LocatorCallbackNapi>& locatorCallbackHost);

    /**
     * @brief Unsubscribe location changed.
     *
     * @param callback Indicates the callback for reporting the location result.
     */
    LocationErrCode StopLocating(sptr<LocatorCallbackNapi>& locatorCallbackHost);

    /**
     * @brief Obtain last known location.
     *
     * @return The last known location information.
     */
    std::unique_ptr<Location> GetCachedLocation();

    /**
     * @brief Subscribe location switch changed.
     *
     * @param callback Indicates the callback for reporting the location switch status.
     * @param uid Indicates the calling uid
     * @return Return whether the registration is successful.
     */
    LocationErrCode RegisterSwitchCallback(sptr<LocationSwitchCallbackNapi>& switchCallbackHost);

    /**
     * @brief Unsubscribe location switch changed.
     *
     * @param callback Indicates the callback for reporting the location switch status.
     * @return Return whether the deregistration is successful.
     */
    LocationErrCode UnregisterSwitchCallback(sptr<LocationSwitchCallbackNapi>& switchCallbackHost);

    /**
     * @brief Subscribe satellite status changed.
     *
     * @param callback Indicates the callback for reporting the satellite status.
     * @param uid Indicates the calling uid
     * @return Return whether the registration is successful.
     */
    bool RegisterGnssStatusCallback(sptr<GnssStatusCallbackNapi>& callback, pid_t uid);

    /**
     * @brief Unsubscribe satellite status changed.
     *
     * @param callback Indicates the callback for reporting the satellite status.
     * @return Return whether the deregistration is successful.
     */
    bool UnregisterGnssStatusCallback(sptr<GnssStatusCallbackNapi>& callback);

    /**
     * @brief Subscribe nmea message changed.
     *
     * @param callback Indicates the callback for reporting the nmea message.
     * @param uid Indicates the calling uid
     * @return Return whether the registration is successful.
     */
    bool RegisterNmeaMessageCallback(sptr<NmeaMessageCallbackNapi>& callback, pid_t uid);

    /**
     * @brief Unsubscribe nmea message changed.
     *
     * @param callback Indicates the callback for reporting the nmea message.
     * @return Return whether the deregistration is successful.
     */
    bool UnregisterNmeaMessageCallback(sptr<NmeaMessageCallbackNapi>& callback);

    /**
     * @brief Registering the callback function for listening to country code changes.
     *
     * @param callback Indicates the callback for reporting country code changes.
     * @param uid Indicates the calling uid
     * @return Return whether the registration is successful.
     */
    LocationErrCode RegisterCountryCodeCallback(sptr<CountryCodeCallbackNapi>& callbackHost);

    /**
     * @brief Unregistering the callback function for listening to country code changes.
     *
     * @param callback Indicates the callback for reporting country code changes.
     * @return Return whether the deregistration is successful.
     */
    LocationErrCode UnregisterCountryCodeCallback(sptr<CountryCodeCallbackNapi>& callbackHost);

    /**
     * @brief Subscribe to cache GNSS locations update messages.
     *
     * @param request Indicates the cached GNSS locations request parameters.
     * @param callback Indicates the callback for reporting the cached GNSS locations.
     * @return Return whether the registration is successful.
     */
    void RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback);

    /**
     * @brief Unsubscribe to cache GNSS locations update messages.
     *
     * @param callback Indicates the callback for reporting the cached gnss locations.
     * @return Return whether the deregistration is successful.
     */
    void UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback);

    /**
     * @brief Obtain geocoding service status.
     *
     * @return Returns true if geocoding service is available, returns false otherwise.
     */
    bool IsGeoServiceAvailable();

    /**
     * @brief Obtain address info from location.
     *
     * @param data Indicates the reverse geocode query parameters.
     * @param replyList Indicates the result of the address info.
     */
    void GetAddressByCoordinate(std::unique_ptr<ReverseGeoCodeRequest>& request,
        std::list<std::shared_ptr<GeoAddress>>& replyList);

    /**
     * @brief Obtain latitude and longitude info from location address.
     *
     * @param data Indicates the geocode query parameters.
     * @param replyList Indicates the result of the address info.
     */
    void GetAddressByLocationName(std::unique_ptr<GeoCodeRequest> &request,
        std::list<std::shared_ptr<GeoAddress>>& replyList);

    /**
     * @brief Obtain the number of cached GNSS locations.
     *
     * @return Returns the result of the cached GNSS locations size.
     */
    int GetCachedGnssLocationsSize();

    /**
     * @brief All prepared GNSS locations are returned to the application through the callback function,
     * and the bottom-layer buffer is cleared.
     *
     * @return Returns 1 if the cached gnss location has been flushed, returns 0 otherwise.
     */
    int FlushCachedGnssLocations();

    /**
     * @brief Obtain the current country code.
     *
     * @return Returns the result of the country code.
     */
    std::shared_ptr<CountryCode> GetIsoCountryCode();

    /**
     * @brief Obtain current location switch status.
     *
     * @param isEnabled Indicates if the location switch on.
     * @return Returns ERRCODE_SUCCESS if obtain current location switch status succeed.
     */
    LocationErrCode IsLocationEnabledV9(bool &isEnabled);

    /**
     * @brief Subscribe satellite status changed.
     *
     * @param callback Indicates the callback for reporting the satellite status.
     * @return Return ERRCODE_SUCCESS if the registration is successful.
     */
    LocationErrCode RegisterGnssStatusCallbackV9(sptr<GnssStatusCallbackNapi>& callback);

    /**
     * @brief Unsubscribe satellite status changed.
     *
     * @param callback Indicates the callback for reporting the satellite status.
     * @return Return ERRCODE_SUCCESS if the deregistration is successful.
     */
    LocationErrCode UnregisterGnssStatusCallbackV9(sptr<GnssStatusCallbackNapi>& callback);

    /**
     * @brief Subscribe nmea message changed.
     *
     * @param callback Indicates the callback for reporting the nmea message.
     * @return Return ERRCODE_SUCCESS if the registration is successful.
     */
    LocationErrCode RegisterNmeaMessageCallbackV9(sptr<NmeaMessageCallbackNapi>& callback);

    /**
     * @brief Unsubscribe nmea message changed.
     *
     * @param callback Indicates the callback for reporting the nmea message.
     * @return Return ERRCODE_SUCCESS if the deregistration is successful.
     */
    LocationErrCode UnregisterNmeaMessageCallbackV9(sptr<NmeaMessageCallbackNapi>& callback);

    /**
     * @brief Subscribe to cache GNSS locations update messages.
     *
     * @param request Indicates the cached GNSS locations request parameters.
     * @param callback Indicates the callback for reporting the cached GNSS locations.
     * @return Return ERRCODE_SUCCESS if the registration is successful.
     */
    LocationErrCode RegisterCachedLocationCallbackV9(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback);

    /**
     * @brief Unsubscribe to cache GNSS locations update messages.
     *
     * @param callback Indicates the callback for reporting the cached gnss locations.
     * @return Return ERRCODE_SUCCESS if the deregistration is successful.
     */
    LocationErrCode UnregisterCachedLocationCallbackV9(sptr<ICachedLocationsCallback>& callback);

    /**
     * @brief Obtain geocoding service status.
     *
     * @param isAvailable Indicates if geocoding service is available
     * @return Return ERRCODE_SUCCESS if obtain geocoding service status is successful.
     */
    LocationErrCode IsGeoServiceAvailableV9(bool &isAvailable);

    /**
     * @brief Obtain address info from location.
     *
     * @param data Indicates the reverse geocode query parameters.
     * @param replyList Indicates the result of the address info.
     * @return Return ERRCODE_SUCCESS if obtain address info from location is successful.
     */
    LocationErrCode GetAddressByCoordinateV9(std::unique_ptr<ReverseGeoCodeRequest>& request,
        std::list<std::shared_ptr<GeoAddress>>& replyList);

    /**
     * @brief Obtain latitude and longitude info from location address.
     *
     * @param data Indicates the geocode query parameters.
     * @param replyList Indicates the result of the address info.
     * @return Return ERRCODE_SUCCESS if obtain latitude and longitude info from location address is successful.
     */
    LocationErrCode GetAddressByLocationNameV9(std::unique_ptr<GeoCodeRequest> &request,
        std::list<std::shared_ptr<GeoAddress>>& replyList);

    /**
     * @brief Obtain the number of cached GNSS locations.
     *
     * @param size Indicates the cached GNSS locations size
     * @return Return ERRCODE_SUCCESS if obtain the number of cached GNSS locations is successful.
     */
    LocationErrCode GetCachedGnssLocationsSizeV9(int &size);

    /**
     * @brief All prepared GNSS locations are returned to the application through the callback function,
     * and the bottom-layer buffer is cleared.
     *
     * @return Return ERRCODE_SUCCESS if flush cached gnss locations is successful.
     */
    LocationErrCode FlushCachedGnssLocationsV9();

    /**
     * @brief Obtain the current country code.
     *
     * @param countryCode the result of the country code
     * @return Returns ERRCODE_SUCCESS if obtain the current country code successfully.
     */
    LocationErrCode GetIsoCountryCodeV9(std::shared_ptr<CountryCode>& countryCode);

    /**
     * @brief Subscribe location error changed.
     *
     * @param callback Indicates the callback for reporting the location error result.
     * @return Returns ERRCODE_SUCCESS if subscribe error changed succeed.
     */
    LocationErrCode SubscribeLocationError(sptr<LocationErrorCallbackNapi>& locationErrorCallbackHost);

    /**
     * @brief Subscribe bluetooth scan result change.
     *
     * @param callback Indicates the callback for reporting the location error result.
     * @return Returns ERRCODE_SUCCESS if subscribe error changed succeed.
     */
    LocationErrCode SubscribeBluetoothScanResultChange(
        sptr<BluetoothScanResultCallbackNapi>& bluetoothScanResultCallbackHost);

    /**
     * @brief Unsubscribe bluetooth scan result change.
     *
     * @param callback Indicates the callback for reporting the bluetooth scan result.
     * @return Returns ERRCODE_SUCCESS if subscribe error changed succeed.
     */
    LocationErrCode UnSubscribeBluetoothScanResultChange(
        sptr<BluetoothScanResultCallbackNapi>& bluetoothScanResultCallbackHost);

    /**
     * @brief Unsubscribe location errorcode changed.
     *
     * @param callback Indicates the callback for reporting the location error result.
     * @return Returns ERRCODE_SUCCESS if Unsubscribe error changed succeed.
     */
    LocationErrCode UnSubscribeLocationError(sptr<LocationErrorCallbackNapi>& locationErrorCallbackHost);

    /**
     * @brief Obtain last known location.
     *
     * @param loc Indicates the last known location information.
     * @return Returns ERRCODE_SUCCESS if obtain last known location succeed.
     */
    LocationErrCode GetCurrentWifiBssidForLocating(std::string& bssid);

    /**
     * @brief Gets the distance between two positions.
     *
     * @param loc location 1.
     * @param loc location 2.
     * @param loc distance.
     * @return Returns ERRCODE_SUCCESS if obtain last known location succeed.
     */
    LocationErrCode GetDistanceBetweenLocations(const Location& location1,
        const Location& location2, double& distance);

    bool IsSatelliteStatusChangeCallbackRegistered(const sptr<IRemoteObject>& callback);
    bool IsNmeaCallbackRegistered(const sptr<IRemoteObject>& callback);
    void AddSatelliteStatusChangeCallBack(const sptr<IRemoteObject>& callback);
    void RemoveSatelliteStatusChangeCallBack(const sptr<IRemoteObject>& callback);
    void AddNmeaCallBack(const sptr<IRemoteObject>& callback);
    void RemoveNmeaCallBack(const sptr<IRemoteObject>& callback);

private:
    static std::mutex locatorMutex_;
    static std::shared_ptr<LocatorImpl> instance_;
};
}  // namespace Location
}  // namespace OHOS
#endif // LOCATOR_IMPL_H
