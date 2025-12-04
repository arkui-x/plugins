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
#ifndef LOCATOR_H
#define LOCATOR_H

#include <list>
#include <vector>

#include "iremote_object.h"

#include "constant_definition.h"
#include "country_code.h"
#include "geo_address.h"
#include "i_cached_locations_callback.h"
#include "i_locator_callback.h"
#include "location.h"
#include "locator_impl.h"
#include "request_config.h"

namespace OHOS {
namespace Location {
class Locator {
public:
    static std::shared_ptr<LocatorImpl> GetInstance();
    virtual ~Locator();
    virtual bool IsLocationEnabled() = 0;
    virtual void RequestEnableLocation() = 0;
    virtual void EnableAbility(bool enable) = 0;
    virtual void StartLocating(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback) = 0;
    virtual void StopLocating(sptr<ILocatorCallback>& callback) = 0;
    virtual std::unique_ptr<Location> GetCachedLocation() = 0;
    virtual bool RegisterSwitchCallback(const sptr<IRemoteObject>& callback, pid_t uid) = 0;
    virtual bool UnregisterSwitchCallback(const sptr<IRemoteObject>& callback) = 0;
    virtual void ShowNotification() = 0;
    virtual void RequestPermission() = 0;
    virtual bool IsGeoServiceAvailable() = 0;
    virtual bool IsLocationPrivacyConfirmed(const int type) = 0;
    virtual int SetLocationPrivacyConfirmStatus(const int type, bool isConfirmed) = 0;
    virtual bool RegisterGnssStatusCallback(const sptr<IRemoteObject>& callback, pid_t uid) = 0;
    virtual bool UnregisterGnssStatusCallback(const sptr<IRemoteObject>& callback) = 0;
    virtual bool RegisterNmeaMessageCallback(const sptr<IRemoteObject>& callback, pid_t uid) = 0;
    virtual bool UnregisterNmeaMessageCallback(const sptr<IRemoteObject>& callback) = 0;
    virtual bool RegisterCountryCodeCallback(const sptr<IRemoteObject>& callback, pid_t uid) = 0;
    virtual bool UnregisterCountryCodeCallback(const sptr<IRemoteObject>& callback) = 0;
    virtual void RegisterCachedLocationCallback(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback) = 0;
    virtual void UnregisterCachedLocationCallback(sptr<ICachedLocationsCallback>& callback) = 0;
    virtual int GetCachedGnssLocationsSize() = 0;
    virtual int FlushCachedGnssLocations() = 0;
    virtual bool EnableReverseGeocodingMock() = 0;
    virtual bool DisableReverseGeocodingMock() = 0;
    virtual std::shared_ptr<CountryCode> GetIsoCountryCode() = 0;
    virtual bool EnableLocationMock() = 0;
    virtual bool DisableLocationMock() = 0;
    virtual bool SetMockedLocations(
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location) = 0;

    virtual LocationErrCode IsLocationEnabledV9(bool &isEnabled) = 0;
    virtual LocationErrCode EnableAbilityV9(bool enable) = 0;
    virtual LocationErrCode StartLocatingV9(std::unique_ptr<RequestConfig>& requestConfig,
        sptr<ILocatorCallback>& callback) = 0;
    virtual LocationErrCode StopLocatingV9(sptr<ILocatorCallback>& callback) = 0;
    virtual LocationErrCode GetCachedLocationV9(std::unique_ptr<Location> &loc) = 0;
    virtual LocationErrCode RegisterSwitchCallbackV9(const sptr<IRemoteObject>& callback) = 0;
    virtual LocationErrCode UnregisterSwitchCallbackV9(const sptr<IRemoteObject>& callback) = 0;
    virtual LocationErrCode IsGeoServiceAvailableV9(bool &isAvailable) = 0;
    virtual LocationErrCode IsLocationPrivacyConfirmedV9(const int type, bool &isConfirmed) = 0;
    virtual LocationErrCode SetLocationPrivacyConfirmStatusV9(const int type, bool isConfirmed) = 0;
    virtual LocationErrCode RegisterGnssStatusCallbackV9(const sptr<IRemoteObject>& callback) = 0;
    virtual LocationErrCode UnregisterGnssStatusCallbackV9(const sptr<IRemoteObject>& callback) = 0;
    virtual LocationErrCode RegisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback) = 0;
    virtual LocationErrCode UnregisterNmeaMessageCallbackV9(const sptr<IRemoteObject>& callback) = 0;
    virtual LocationErrCode RegisterCountryCodeCallbackV9(const sptr<IRemoteObject>& callback) = 0;
    virtual LocationErrCode UnregisterCountryCodeCallbackV9(const sptr<IRemoteObject>& callback) = 0;
    virtual LocationErrCode RegisterCachedLocationCallbackV9(std::unique_ptr<CachedGnssLocationsRequest>& request,
        sptr<ICachedLocationsCallback>& callback) = 0;
    virtual LocationErrCode UnregisterCachedLocationCallbackV9(sptr<ICachedLocationsCallback>& callback) = 0;
    virtual LocationErrCode GetCachedGnssLocationsSizeV9(int &size) = 0;
    virtual LocationErrCode FlushCachedGnssLocationsV9() = 0;
    virtual LocationErrCode EnableReverseGeocodingMockV9() = 0;
    virtual LocationErrCode DisableReverseGeocodingMockV9() = 0;
    virtual LocationErrCode GetIsoCountryCodeV9(std::shared_ptr<CountryCode>& countryCode) = 0;
    virtual LocationErrCode EnableLocationMockV9() = 0;
    virtual LocationErrCode DisableLocationMockV9() = 0;
    virtual LocationErrCode SetMockedLocationsV9(
        const int timeInterval, const std::vector<std::shared_ptr<Location>> &location) = 0;
    virtual LocationErrCode ProxyForFreeze(std::vector<int32_t> pidList, bool isProxy) = 0;
    virtual LocationErrCode ResetAllProxy() = 0;
private:
    static std::shared_ptr<LocatorImpl> instance_;
    static std::mutex mutex_;
};
} // namespace Location
} // namespace OHOS

#endif // LOCATOR_H
