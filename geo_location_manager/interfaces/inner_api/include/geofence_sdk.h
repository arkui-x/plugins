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
#ifndef GEOFENCE_SDK_H
#define GEOFENCE_SDK_H

#include "geofence_request.h"
#include "constant_definition.h"
#include "location_gnss_geofence_callback_napi.h"

namespace OHOS {
namespace Location {
class GeofenceManager {
public:
    static GeofenceManager* GetInstance();
    explicit GeofenceManager();
    ~GeofenceManager();

    LocationErrCode AddGnssGeofence(std::shared_ptr<GeofenceRequest>& request,
        sptr<LocationGnssGeofenceCallbackNapi> callback);
    LocationErrCode RemoveGnssGeofence(std::shared_ptr<GeofenceRequest>& request,
        sptr<LocationGnssGeofenceCallbackNapi> callback);
    LocationErrCode GetGeofenceSupportedCoordTypes(
        std::vector<CoordinateSystemType>& coordinateSystemTypes);
private:
    std::mutex mutex_;
    bool isServerExist_ = false;
};
}  // namespace Location
}  // namespace OHOS
#endif // GEOFENCE_SDK_H