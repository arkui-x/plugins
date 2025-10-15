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

#include "geofence_sdk.h"
#include "location_log.h"
#include "location_crossplatform_service.h"

namespace OHOS {
namespace Location {
GeofenceManager* GeofenceManager::GetInstance()
{
    static GeofenceManager data;
    return &data;
}

GeofenceManager::GeofenceManager()
{
}

GeofenceManager::~GeofenceManager()
{}

LocationErrCode GeofenceManager::AddGnssGeofence(std::shared_ptr<GeofenceRequest>& request,
    sptr<LocationGnssGeofenceCallbackNapi> callback)
{
    LBSLOGI(GEOFENCE_SDK, "GeofenceManager_AddGnssGeofence called.");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    return service->AddGnssGeofence(*request, callback);
}

LocationErrCode GeofenceManager::RemoveGnssGeofence(std::shared_ptr<GeofenceRequest>& request,
    sptr<LocationGnssGeofenceCallbackNapi> callback)
{
    LBSLOGI(GEOFENCE_SDK, "GeofenceManager_RemoveGnssGeofence called.");
    std::shared_ptr<Plugin::LocationCrossplatformService> service = Plugin::LocationCrossplatformService::Get();
    return service->RemoveGnssGeofence(request->GetFenceId(), callback);
}

LocationErrCode GeofenceManager::GetGeofenceSupportedCoordTypes(
    std::vector<CoordinateSystemType>& coordinateSystemTypes)
{
    LBSLOGI(GEOFENCE_SDK, "GeofenceManager::%{public}s", __func__);
    coordinateSystemTypes.push_back(WGS84);
    return ERRCODE_SUCCESS;
}
}
}
