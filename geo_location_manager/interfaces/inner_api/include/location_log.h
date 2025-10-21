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

#ifndef LOCATION_LOG_H
#define LOCATION_LOG_H

#include "hilog/log.h"

namespace OHOS {
namespace Location {
#ifndef LOCATION_LOG_DOMAIN
#define LOCATION_LOG_DOMAIN 0xD002300
#endif

#undef LOCATION_LOG_TAG
#define LOCATION_LOG_TAG "Locator"

#ifndef LOCATION_FUNC_FMT
#define LOCATION_FUNC_FMT "[(%{public}s:%{public}d)]"
#endif

#ifndef LOCATION_FILE_NAME
#define LOCATION_FILE_NAME (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#ifndef LOCATION_FUNC_INFO
#define LOCATION_FUNC_INFO __FUNCTION__, __LINE__
#endif

#define LBSLOGE(label, fmt, ...) do { \
    (void)HILOG_IMPL(LOG_CORE, LOG_ERROR, LOCATION_LOG_DOMAIN, label.tag, \
    LOCATION_FUNC_FMT fmt, LOCATION_FUNC_INFO, ##__VA_ARGS__);            \
} while (0)

#define LBSLOGW(label, fmt, ...) do { \
    (void)HILOG_IMPL(LOG_CORE, LOG_WARN, LOCATION_LOG_DOMAIN, label.tag, \
    LOCATION_FUNC_FMT fmt, LOCATION_FUNC_INFO, ##__VA_ARGS__);           \
} while (0)

#define LBSLOGI(label, fmt, ...) do { \
    (void)HILOG_IMPL(LOG_CORE, LOG_INFO, LOCATION_LOG_DOMAIN, label.tag, \
    LOCATION_FUNC_FMT fmt, LOCATION_FUNC_INFO, ##__VA_ARGS__);           \
} while (0)

#define LBSLOGD(label, fmt, ...) do { \
    (void)HILOG_IMPL(LOG_CORE, LOG_DEBUG, LOCATION_LOG_DOMAIN, label.tag, \
    LOCATION_FUNC_FMT fmt, LOCATION_FUNC_INFO, ##__VA_ARGS__);            \
} while (0)

#define LBSLOGF(label, fmt, ...) do { \
    (void)HILOG_IMPL(LOG_CORE, HILOG_FATAL, LOCATION_LOG_DOMAIN, label.tag, \
    LOCATION_FUNC_FMT fmt, LOCATION_FUNC_INFO, ##__VA_ARGS__);              \
} while (0)

static constexpr OHOS::HiviewDFX::HiLogLabel GEO_CONVERT = {LOG_CORE, LOCATION_LOG_DOMAIN, "GeoConvert"};
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATOR = {LOG_CORE, LOCATION_LOG_DOMAIN, "Locator"};
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATOR_STANDARD = {LOG_CORE, LOCATION_LOG_DOMAIN, "Locator_standard"};
static constexpr OHOS::HiviewDFX::HiLogLabel SWITCH_CALLBACK = {LOG_CORE, LOCATION_LOG_DOMAIN, "SwitchCallback"};
static constexpr OHOS::HiviewDFX::HiLogLabel GNSS_STATUS_CALLBACK = {
    LOG_CORE, LOCATION_LOG_DOMAIN, "GnssStatusCallback"
};
static constexpr OHOS::HiviewDFX::HiLogLabel NMEA_MESSAGE_CALLBACK = {
    LOG_CORE, LOCATION_LOG_DOMAIN, "NmeaMessageCallback"
};
static constexpr OHOS::HiviewDFX::HiLogLabel CACHED_LOCATIONS_CALLBACK = {
    LOG_CORE, LOCATION_LOG_DOMAIN, "CachedLocationsCallback"
};
static constexpr OHOS::HiviewDFX::HiLogLabel COUNTRY_CODE_CALLBACK = {
    LOG_CORE, LOCATION_LOG_DOMAIN, "CountryCodeCallback"
};
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATING_DATA_CALLBACK = {
    LOG_CORE, LOCATION_LOG_DOMAIN, "LocatingDataCallback"
};
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATION_ERR_CALLBACK = {
    LOG_CORE, LOCATION_LOG_DOMAIN, "LocationErrorCallback"
};
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATOR_CALLBACK = {LOG_CORE, LOCATION_LOG_DOMAIN, "LocatorCallback"};
static constexpr OHOS::HiviewDFX::HiLogLabel GNSS = {LOG_CORE, LOCATION_LOG_DOMAIN, "GnssAbility"};
static constexpr OHOS::HiviewDFX::HiLogLabel GNSS_TEST = {LOG_CORE, LOCATION_LOG_DOMAIN, "GnssAbilityTest"};
static constexpr OHOS::HiviewDFX::HiLogLabel NETWORK = {LOG_CORE, LOCATION_LOG_DOMAIN, "NetworkAbility"};
static constexpr OHOS::HiviewDFX::HiLogLabel NETWORK_TEST = {LOG_CORE, LOCATION_LOG_DOMAIN, "NetworkAbilityTest"};
static constexpr OHOS::HiviewDFX::HiLogLabel PASSIVE = {LOG_CORE, LOCATION_LOG_DOMAIN, "PassiveAbility"};
static constexpr OHOS::HiviewDFX::HiLogLabel PASSIVE_TEST = {LOG_CORE, LOCATION_LOG_DOMAIN, "PassiveAbilityTest"};
static constexpr OHOS::HiviewDFX::HiLogLabel REQUEST_MANAGER = {LOG_CORE, LOCATION_LOG_DOMAIN, "RequestManager"};
static constexpr OHOS::HiviewDFX::HiLogLabel REPORT_MANAGER = {LOG_CORE, LOCATION_LOG_DOMAIN, "ReportManager"};
static constexpr OHOS::HiviewDFX::HiLogLabel COMMON_UTILS = {LOG_CORE, LOCATION_LOG_DOMAIN, "CommonUtils"};
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATOR_EVENT = {LOG_CORE, LOCATION_LOG_DOMAIN, "LocatorEvent"};
static constexpr OHOS::HiviewDFX::HiLogLabel COUNTRY_CODE = {LOG_CORE, LOCATION_LOG_DOMAIN, "CountryCode"};
static constexpr OHOS::HiviewDFX::HiLogLabel VISIBILITY_CONTROLLER = {
    LOG_CORE, LOCATION_LOG_DOMAIN, "VisibilityController"
};
static constexpr OHOS::HiviewDFX::HiLogLabel FUSION_CONTROLLER = {LOG_CORE, LOCATION_LOG_DOMAIN, "FusionController"};
static constexpr OHOS::HiviewDFX::HiLogLabel DISTRIBUTION_CONTROLLER = {
    LOG_CORE, LOCATION_LOG_DOMAIN, "Location_Distribution"
};
static constexpr OHOS::HiviewDFX::HiLogLabel HIVIEW_EVENT = {LOG_CORE, LOCATION_LOG_DOMAIN, "ChrEvent"};
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATOR_BACKGROUND_PROXY = {
    LOG_CORE, LOCATION_LOG_DOMAIN, "LocatorBackgroundProxy"
};
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATION_NAPI = {LOG_CORE, LOCATION_LOG_DOMAIN, "LocationNapi"};
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATION_GNSS_GEOFENCE_CALLBACK = {
    LOG_CORE, LOCATION_LOG_DOMAIN, "LocationGnssGeofenceCallback"
};
static constexpr OHOS::HiviewDFX::HiLogLabel NAPI_UTILS = {LOG_CORE, LOCATION_LOG_DOMAIN, "LocationNapiUtils"};
static constexpr OHOS::HiviewDFX::HiLogLabel GEOFENCE_SDK = {
    LOG_CORE, LOCATION_LOG_DOMAIN, "geofenceSdk"
};
static constexpr OHOS::HiviewDFX::HiLogLabel GEOFENCE_SDK_TEST = {LOG_CORE, LOCATION_LOG_DOMAIN, "GeofenceSdkTest"};
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATION_CAPI = {LOG_CORE, LOCATION_LOG_DOMAIN, "LocationCapi"};
static constexpr OHOS::HiviewDFX::HiLogLabel FENCE_EXTENSION = {
    LOG_CORE, LOCATION_LOG_DOMAIN, "FenceExtension"
};
static constexpr OHOS::HiviewDFX::HiLogLabel BLUETOOTH_CALLBACK = {LOG_CORE, LOCATION_LOG_DOMAIN, "BluetoothCallback"};
static constexpr OHOS::HiviewDFX::HiLogLabel PROXY_FREEZE_MANAGER = {
    LOG_CORE, LOCATION_LOG_DOMAIN, "ProxyFreezeManager"
};
static constexpr OHOS::HiviewDFX::HiLogLabel COMMON_EVENT_HELPER = {
    LOG_CORE, LOCATION_LOG_DOMAIN, "CommonEventHelper"
};
static constexpr OHOS::HiviewDFX::HiLogLabel BEACON_FENCE_MANAGER = {
    LOG_CORE, LOCATION_LOG_DOMAIN, "BeaconFenceManager"
};
static constexpr OHOS::HiviewDFX::HiLogLabel LOCATION_HIAPPEVENT = {
    LOG_CORE, LOCATION_LOG_DOMAIN, "LocationHiAppEvent"
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_LOG_H
