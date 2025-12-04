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

#ifndef CONSTANT_DEFINITION_H
#define CONSTANT_DEFINITION_H

#include <string>
#include <vector>

namespace OHOS {
namespace Location {
// error type
const int ERROR_PERMISSION_NOT_GRANTED = 0x0100;
const int ERROR_SWITCH_UNOPEN = 0x0101;
const int SESSION_START = 0x0002;
const int SESSION_STOP = 0x0003;
const int SUCCESS_CALLBACK = 0;
const int FAIL_CALLBACK = 1;
const int COMPLETE_CALLBACK = 2;
const int32_t PARAM0 = 0;
const int32_t PARAM1 = 1;
const int32_t PARAM2 = 2;
const int32_t PARAM3 = 3;
const int32_t MAXIMUM_JS_PARAMS = 10;
const int32_t MAX_CALLBACK_NUM = 3;
const size_t RESULT_SIZE = 2;
const int INPUT_WIFI_LIST_MAX_SIZE = 1000;
const int DEFAULT_TIMEOUT_30S = 30000;
const int DEFAULT_TIMEOUT_5S = 5000;
const double DEFAULT_APPROXIMATELY_ACCURACY = 5000.0;
const int STATE_OPEN = 1;
const int STATE_CLOSE = 0;
const int PRIVACY_TYPE_OTHERS = 0;
const int PRIVACY_TYPE_STARTUP = 1;
const int PRIVACY_TYPE_CORE_LOCATION = 2;
const int32_t UNKNOW_USER_ID = -1;
const int32_t SUBSCRIBE_TIME = 5;
const int32_t DEFAULT_TIME_INTERVAL = 30 * 60; // app receive location every 30 minutes in frozen state
const int32_t REQUESTS_NUM_MAX = 1;
constexpr const char* FEATURE_SWITCH_PROP = "ro.config.locator_background";
constexpr const char* TIME_INTERVAL_PROP = "ro.config.locator_background.timeInterval";
constexpr const char* PROC_NAME = "system";
constexpr const char* LOCALE_KEY = "persist.global.locale";
constexpr const char* LOCATION_SWITCH_MODE = "persist.location.switch_mode";
constexpr const char* LOCATION_LOCATIONHUB_STATE = "persist.location.locationhub_state";
constexpr const char* LOCATION_LAST_SIM_OPKEY = "persist.location.last_sim_opkey";
constexpr const char* LOCATION_SIM_CONFIGURE_SWITCH = "const.location.gnss_adapt_card";
const int MODE_STANDALONE = 1;
const int MODE_MS_BASED = 2;
const int MODE_MS_ASSISTED = 3;
const int DEFAULT_CALLBACK_WAIT_TIME = 10000;
const int COORDINATE_SYSTEM_TYPE_SIZE = 2;
// fence extension const start
constexpr const char *EXTENSION_SURVIVE_TIME = "EXTENSION_SURVIVE_TIME";
constexpr const char *EXTENSION_PARAM_KEY_FENCE_ID = "FENCE_ID";
constexpr const char *EXTENSION_PARAM_KEY_FENCE_EVENT = "FENCE_EVENT";
// fence extension const end

enum {
    SCENE_UNSET = 0x0300,
    SCENE_NAVIGATION = 0x0301,
    SCENE_TRAJECTORY_TRACKING = 0x0302,
    SCENE_CAR_HAILING = 0x0303,
    SCENE_DAILY_LIFE_SERVICE = 0x0304,
    SCENE_NO_POWER = 0x0305
};

enum {
    PRIORITY_UNSET = 0x0200,
    PRIORITY_ACCURACY = 0x0201,
    PRIORITY_LOW_POWER = 0x0202,
    PRIORITY_FAST_FIRST_FIX = 0x0203
};

enum {
    LOCATION_SCENE_NAVIGATION = 0x0401,
    LOCATION_SCENE_SPORT = 0x0402,
    LOCATION_SCENE_TRANSPORT = 0x0403,
    LOCATION_SCENE_DAILY_LIFE_SERVICE = 0x0404,
    LOCATION_SCENE_WALK = 0x0405,
    LOCATION_SCENE_RIDE = 0x0406,
    LOCATION_SCENE_INDOOR_POI = 0x0407,
};

enum {
    LOCATION_PRIORITY_ACCURACY = 0x0501,
    LOCATION_PRIORITY_LOCATING_SPEED = 0x0502,
};

enum {
    LOCATION_SCENE_HIGH_POWER_CONSUMPTION = 0x0601,
    LOCATION_SCENE_LOW_POWER_CONSUMPTION = 0x0602,
    LOCATION_SCENE_NO_POWER_CONSUMPTION = 0x0603,
};


enum {
    COUNTRY_CODE_FROM_LOCALE = 1,
    COUNTRY_CODE_FROM_SIM,
    COUNTRY_CODE_FROM_LOCATION,
    COUNTRY_CODE_FROM_NETWORK,
};

enum NlpRequestType {
    PRIORITY_TYPE_BALANCED_POWER_ACCURACY = 102,  // NLP
    PRIORITY_TYPE_INDOOR = 300,                   // indoor
    PRIORITY_TYPE_INDOOR_POI = 400,               // POI
};

enum LocationErrCode {
    ERRCODE_SUCCESS = 0,                      /* SUCCESS. */
    ERRCODE_PERMISSION_DENIED = 201,          /* Permission denied. */
    ERRCODE_SYSTEM_PERMISSION_DENIED = 202,   /* System API is not allowed called by third HAP. */
    ERRCODE_EDM_POLICY_ABANDON = 203,         /* This feature is prohibited by enterprise management policies. */
    ERRCODE_INVALID_PARAM = 401,              /* Parameter error. */
    ERRCODE_NOT_SUPPORTED = 801,              /* Capability not supported. */
    LOCATION_ERRCODE_MIN = 3300000,
    LOCATION_ERRCODE_PERMISSION_DENIED = 3300201,        /* Permission denied. */
    LOCATION_ERRCODE_SYSTEM_PERMISSION_DENIED = 3300202, /* System API is not allowed called by third HAP. */
    /* This feature is prohibited by enterprise management policies. */
    LOCATION_ERRCODE_EDM_POLICY_ABANDON = 3300203,
    LOCATION_ERRCODE_BACKGROUND_PERMISSION_DENIED = 3300204,
    LOCATION_ERRCODE_BACKGROUND_CONTINUE_PERMISSION_DENIED = 3300205,
    LOCATION_ERRCODE_NOT_CURRENT_USER_ID = 3300206,
    LOCATION_ERRCODE_REQUEST_TIMEOUT = 3300207,
    LOCATION_ERRCODE_DEVICE_IDLE = 3300208,
    LOCATION_ERRCODE_USING_PERMISSION = 3300209,
    LOCATION_ERRCODE_INVALID_PARAM = 3300401,            /* Parameter error. */
    LOCATION_ERRCODE_NOT_SUPPORTED = 3300801,            /* Capability not supported. */
    ERRCODE_SERVICE_UNAVAILABLE = 3301000,    /* Location service is unavailable. */
    ERRCODE_SWITCH_OFF = 3301100,             /* The location switch is off. */
    ERRCODE_LOCATING_FAIL = 3301200,          /* Failed to obtain the geographical location. */
    /* The network locating is failed because the network cannot be accessed. */
    ERRCODE_LOCATING_NETWORK_FAIL = 3301201,
    /* The positioning result does not meet the precision requirement (maxAccuracy) */
    /* in the positioning request parameters. */
    ERRCODE_LOCATING_ACC_FAIL = 3301203,
    ERRCODE_LOCATING_CACHE_FAIL = 3301204,    /* The system does not have a cache locaiton. */
    ERRCODE_LOCATING_FREEZE = 3301205,        /* This pid has been freezed by system */
    ERRCODE_REVERSE_GEOCODING_FAIL = 3301300, /* Reverse geocoding query failed */
    ERRCODE_GEOCODING_FAIL = 3301400,         /* Geocoding query failed */
    ERRCODE_COUNTRYCODE_FAIL  = 3301500,      /* Failed to query the area information */
    ERRCODE_GEOFENCE_FAIL = 3301600,          /* Failed to operate the geofence */
    ERRCODE_NO_RESPONSE = 3301700,            /* No response to the request */
    ERRCODE_SCAN_FAIL = 3301800,              /* Failed to start WiFi or Bluetooth scanning. */
    ERRCODE_WIFI_SCAN_FAIL = 3301801,              /* Failed to start WiFi scanning. */
    /* Failed to obtain the hotpot MAC address because the Wi-Fi is not connected. */
    ERRCODE_WIFI_IS_NOT_CONNECTED = 3301900,
    ERRCODE_GEOFENCE_EXCEED_MAXIMUM = 3301601, /* The number of geofences exceeds the maximum. */
    ERRCODE_GEOFENCE_INCORRECT_ID = 3301602, /* Failed to delete a geofence due to an incorrect ID. */
    /* Failed to add a beacon fence because the location switch is off. */
    ERRCODE_BEACONFENCE_LOCATION_SWITCH_OFF = 3501100,
    /* Failed to add a beacon fence because the bluetooth switch is off. */
    ERRCODE_BEACONFENCE_BLUETOOTH_SWITCH_OFF = 3501101,
    ERRCODE_BEACONFENCE_EXCEED_MAXIMUM = 3501601, /* The number of beacon fence exceeds the maximum. */
    /* Failed to delete the fence due to incorrect beacon fence information. */
    ERRCODE_BEACONFENCE_INCORRECT_ID = 3501602,
    ERRCODE_BEACONFENCE_DUPLICATE_INFORMATION = 3501603 /* Duplicate beacon fence information. */
};

enum FenceExtensionErrCode {
    EXTENSION_SUCCESS = 0,
    EXTENSION_IS_NOT_EXIST,
    CONNECT_ABILITY_FAILED,
    CONNECT_ABILITY_TIMEOUT,
    CONNECT_PROXY_IS_NULL,
    CONNECT_EXTENSION_PROXY_IS_NULL,
    PARAM_ERROR,
    NOT_FOUND_ABILITY_BY_TID,
    DISCONNECT_ABILITY_FAILED,
    DISCONNECT_ABILITY_TIMEOUT,
    EXTENSION_REMOTE_WRITE_FAILED,
    EXTENSION_REMOTE_SEND_FAILED,
    EXTENSION_REMOTE_STUB_IS_NULL,
    EXTENSION_JS_OBJ_IS_NULL,
    EXTENSION_JS_RUNTIME_IS_NULL,
    EXTENSION_JS_NOT_FOUND_METHOD,
    EXTENSION_JS_CREATE_PARAM_ERROR,
    EXTENSION_JS_CALL_FAILED
};

enum SatelliteConstellation {
    SV_CONSTELLATION_CATEGORY_UNKNOWN = 0,
    SV_CONSTELLATION_CATEGORY_GPS,
    SV_CONSTELLATION_CATEGORY_SBAS,
    SV_CONSTELLATION_CATEGORY_GLONASS,
    SV_CONSTELLATION_CATEGORY_QZSS,
    SV_CONSTELLATION_CATEGORY_BEIDOU,
    SV_CONSTELLATION_CATEGORY_GALILEO,
    SV_CONSTELLATION_CATEGORY_IRNSS,
};

enum SatelliteAdditionalInfo {
    SV_ADDITIONAL_INFO_NULL = 0,
    SV_ADDITIONAL_INFO_EPHEMERIS_DATA_EXIST = 1,
    SV_ADDITIONAL_INFO_ALMANAC_DATA_EXIST = 2,
    SV_ADDITIONAL_INFO_USED_IN_FIX = 4,
    SV_ADDITIONAL_INFO_CARRIER_FREQUENCY_EXIST = 8,
};

enum LocationErr {
    LOCATING_FAILED_DEFAULT = -1,
    LOCATING_FAILED_LOCATION_PERMISSION_DENIED = -2,
    LOCATING_FAILED_BACKGROUND_PERMISSION_DENIED = -3,
    LOCATING_FAILED_LOCATION_SWITCH_OFF = -4,
    LOCATING_FAILED_INTERNET_ACCESS_FAILURE = -5,
};

enum LocatingRequiredDataType {
    WIFI = 1,
    BLUE_TOOTH,
};

enum LocationSourceType {
    GNSS_TYPE = 1,
    NETWORK_TYPE = 2,
    INDOOR_TYPE = 3,
    RTK_TYPE = 4,
};

enum SportsType {
    RUNNING = 1,
    WALKING,
    CYCLING,
};

typedef struct {
    int reportingPeriodSec;
    bool wakeUpCacheQueueFull;
} CachedGnssLocationsRequest;

typedef struct {
    int scenario;
    std::string command;
} LocationCommand;

typedef struct {
    std::string locale;
    double latitude;
    double longitude;
    int maxItems;
} ReverseGeocodeRequest;

typedef struct {
    std::vector<std::string> wlanBssidArray;
    int32_t rssiThreshold;
    bool needStartScan;
} WlanRequestConfig;
} // namespace Location
} // namespace OHOS
#endif // CONSTANT_DEFINITION_H
