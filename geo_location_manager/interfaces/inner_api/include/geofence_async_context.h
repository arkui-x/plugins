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

#ifndef GEOFENCE_ASYNC_CONTEXT_H
#define GEOFENCE_ASYNC_CONTEXT_H

#include <list>
#include <string>

#include "message_parcel.h"
#include "napi/native_api.h"

#include "async_context.h"
#include "constant_definition.h"
#include "location_log.h"
#include "location_gnss_geofence_callback_napi.h"
#include "geofence_definition.h"
#include "geofence_request.h"
#include "beacon_fence_request.h"
#include "beacon_fence.h"

namespace OHOS {
namespace Location {
class GnssGeofenceAsyncContext : public AsyncContext {
public:
    int code_{-1};
    std::shared_ptr<BeaconFence> beaconFence_;
    GeofenceTransition transition_{-1, GEOFENCE_TRANSITION_INIT, beaconFence_};
    sptr<LocationGnssGeofenceCallbackNapi> callbackHost_ = nullptr;
    std::shared_ptr<GeofenceRequest> request_;
    std::shared_ptr<BeaconFenceRequest> beaconRequest_;
    int fenceId_{-1};
    bool clearBeaconFence_ = false;

    explicit GnssGeofenceAsyncContext(
        napi_env env, napi_async_work work = nullptr, napi_deferred deferred = nullptr)
        : AsyncContext(env, work, deferred), fenceId_(-1) {}

    GnssGeofenceAsyncContext() = delete;

    ~GnssGeofenceAsyncContext() override {}
};
}  // namespace Location
}  // namespace OHOS
#endif // GEOFENCE_ASYNC_CONTEXT_H