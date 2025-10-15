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

#ifndef GEOFENCE_DEFINITION_H
#define GEOFENCE_DEFINITION_H
#include "beacon_fence.h"

namespace OHOS {
namespace Location {
enum GeofenceTransitionEvent {
    GEOFENCE_TRANSITION_INIT = -1,
    GEOFENCE_TRANSITION_EVENT_ENTER = 1,
    GEOFENCE_TRANSITION_EVENT_EXIT = 2,
    GEOFENCE_TRANSITION_EVENT_DWELL = 4,
};

enum CoordinateSystemType {
    WGS84 = 1,
    GCJ02,
};

enum GnssGeofenceOperateResult {
    GNSS_GEOFENCE_OPERATION_SUCCESS = 0,
    GNSS_GEOFENCE_OPERATION_ERROR_UNKNOWN = -1,
    GNSS_GEOFENCE_OPERATION_ERROR_TOO_MANY_GEOFENCES = -2,
    GNSS_GEOFENCE_OPERATION_ERROR_GEOFENCE_ID_EXISTS = -3,
    GNSS_GEOFENCE_OPERATION_ERROR_PARAMS_INVALID = -4,
    GNSS_GEOFENCE_OPERATION_ERROR_GEOFENCE_ID_UNKNOWN = -5,
    GNSS_GEOFENCE_OPERATION_ERROR_NOT_SUPPORTED = -6,
    GNSS_GEOFENCE_OPERATION_ERROR_OPERATE_FAILED = -7,
};

enum GnssGeofenceOperateType {
    GNSS_GEOFENCE_OPT_TYPE_ADD = 1,
    GNSS_GEOFENCE_OPT_TYPE_DELETE = 2,
};

typedef struct {
    int fenceId;
    GeofenceTransitionEvent event;
    std::shared_ptr<BeaconFence> beaconFence;
} GeofenceTransition;
} // namespace Location
} // namespace OHOS
#endif // GEOFENCE_DEFINITION_H
