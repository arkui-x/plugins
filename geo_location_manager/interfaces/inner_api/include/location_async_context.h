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

#ifndef LOCATION_ASYNC_CONTEXT_H
#define LOCATION_ASYNC_CONTEXT_H

#include <list>
#include <string>

#include "message_parcel.h"
#include "napi/native_api.h"

#include "async_context.h"
#include "constant_definition.h"
#include "country_code.h"
#include "geo_address.h"
#include "location.h"
#include "location_log.h"
#include "locator_callback_napi.h"
#include "request_config.h"
#include "satellite_status.h"
#include "location_gnss_geofence_callback_napi.h"
#include "bluetooth_scan_result.h"
#include "geo_code_request.h"

namespace OHOS {
namespace Location {
class LocationAsyncContext : public AsyncContext {
public:
    std::unique_ptr<Location> loc;

    explicit LocationAsyncContext(napi_env env, napi_async_work work = nullptr,
        napi_deferred deferred = nullptr) : AsyncContext(env, work, deferred), loc(nullptr) {}

    LocationAsyncContext() = delete;

    ~LocationAsyncContext() override {}
};

class LocationMockAsyncContext : public AsyncContext {
public:
    int32_t timeInterval;
    bool enable;

    std::vector<std::shared_ptr<Location>> LocationNapi;
    LocationMockAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), timeInterval(0), enable(false) {}

    LocationMockAsyncContext() = delete;

    ~LocationMockAsyncContext() override {}
};

class CountryCodeContext : public AsyncContext {
public:
    std::shared_ptr<CountryCode> country;

    explicit CountryCodeContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), country(nullptr) {}

    CountryCodeContext() = delete;

    ~CountryCodeContext() override {}
};

class LocationRequestAsyncContext : public AsyncContext {
public:
    std::unique_ptr<Location> loc;
    std::unique_ptr<RequestConfig> request;

    explicit LocationRequestAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), loc(nullptr), request(nullptr) {}

    LocationRequestAsyncContext() = delete;

    ~LocationRequestAsyncContext() override {}
};

class SwitchAsyncContext : public AsyncContext {
public:
    bool enable;

    explicit SwitchAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), enable(false) {}

    SwitchAsyncContext() = delete;

    ~SwitchAsyncContext() override {}
};

class NmeaAsyncContext : public AsyncContext {
public:
    std::string msg;

    explicit NmeaAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), msg("") {}

    NmeaAsyncContext() = delete;

    ~NmeaAsyncContext() override {}
};

class GnssStatusAsyncContext : public AsyncContext {
public:
    std::shared_ptr<SatelliteStatus> statusInfo;

    explicit GnssStatusAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), statusInfo(nullptr) {}

    GnssStatusAsyncContext() = delete;

    ~GnssStatusAsyncContext() override {}
};

class CachedLocationAsyncContext : public AsyncContext {
public:
    std::vector<std::unique_ptr<Location>> locationList;

    explicit CachedLocationAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred) {}

    CachedLocationAsyncContext() = delete;

    ~CachedLocationAsyncContext() override {}
};

class PrivacyAsyncContext : public AsyncContext {
public:
    int type;
    bool isConfirmed;

    explicit PrivacyAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), type(PRIVACY_TYPE_OTHERS), isConfirmed(false) {}

    PrivacyAsyncContext() = delete;

    ~PrivacyAsyncContext() override {}
};

class CachedAsyncContext : public AsyncContext {
public:
    bool enable;
    int locationSize;

    explicit CachedAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), enable(false), locationSize(0) {}

    CachedAsyncContext() = delete;

    ~CachedAsyncContext() override {}
};

class CommandAsyncContext : public AsyncContext {
public:
    bool enable;
    std::unique_ptr<LocationCommand> command;

    explicit CommandAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), enable(false), command(nullptr) {}

    CommandAsyncContext() = delete;

    ~CommandAsyncContext() override {}
};

class ReverseGeoCodeAsyncContext : public AsyncContext {
public:
    std::unique_ptr<ReverseGeoCodeRequest> reverseGeoCodeRequest;
    std::list<std::shared_ptr<GeoAddress>> replyList;

    explicit ReverseGeoCodeAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred) {}

    ReverseGeoCodeAsyncContext() = delete;

    ~ReverseGeoCodeAsyncContext() override {}
};

class GeoCodeAsyncContext : public AsyncContext {
public:
    std::unique_ptr<GeoCodeRequest> geoCodeRequest;
    std::list<std::shared_ptr<GeoAddress>> replyList;

    explicit GeoCodeAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred) {}

    GeoCodeAsyncContext() = delete;

    ~GeoCodeAsyncContext() override {}
};

class SingleLocationAsyncContext : public AsyncContext {
public:
    int timeout_;
    sptr<LocatorCallbackNapi> callbackHost_;
    std::unique_ptr<RequestConfig> request_;

    explicit SingleLocationAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), timeout_(0), callbackHost_(nullptr), request_(nullptr) {}

    SingleLocationAsyncContext() = delete;

    ~SingleLocationAsyncContext() override {}
};

class LocationErrorAsyncContext : public AsyncContext {
public:
    int32_t errCode;

    explicit LocationErrorAsyncContext(napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), errCode(-1) {}

    LocationErrorAsyncContext() = delete;

    ~LocationErrorAsyncContext() override {}
};

class BluetoothScanResultAsyncContext : public AsyncContext {
public:
    std::unique_ptr<BluetoothScanResult> bluetoothScanResult;

    explicit BluetoothScanResultAsyncContext(napi_env env, napi_async_work work = nullptr,
        napi_deferred deferred = nullptr) : AsyncContext(env, work, deferred), bluetoothScanResult(nullptr) {}

    BluetoothScanResultAsyncContext() = delete;

    ~BluetoothScanResultAsyncContext() override {}
};
}  // namespace Location
}  // namespace OHOS
#endif // LOCATION_ASYNC_CONTEXT_H