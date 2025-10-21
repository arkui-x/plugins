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

#ifndef GEOFENCE_NAPI_H
#define GEOFENCE_NAPI_H

#include <list>
#include <string>

#include "napi_util.h"
#include "geofence_request.h"
#if !defined(PLUGIN_INTERFACE_NATIVE_LOG_H)
#define LogLevel GEOLOC_PLUGIN_LOGLEVEL_REMAED__
#define GEOLOC_PLUGIN_LOGLEVEL_RENAMED
#endif
#ifdef NOTIFICATION_ENABLE
#include "notification_request.h"
#endif
#ifdef GEOLOC_PLUGIN_LOGLEVEL_RENAMED
#undef LogLevel
#endif
#include "beacon_fence.h"

namespace OHOS {
namespace Location {
bool JsObjToGeoFenceRequest(const napi_env& env, const napi_value& object,
    const std::shared_ptr<GeofenceRequest>& request);
bool ParseGnssGeofenceRequest(
    const napi_env& env, const napi_value& value, std::shared_ptr<GeofenceRequest>& request);
bool GenGnssGeofenceRequest(
    const napi_env& env, const napi_value& value, std::shared_ptr<GeofenceRequest>& reminder);
#ifdef NOTIFICATION_ENABLE
void JsObjToNotificationRequestList(const napi_env& env, const napi_value& object,
    std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>>& notificationRequestList);
void GetNotificationRequestArray(const napi_env& env, const napi_value& notificationRequest,
    std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>>& notificationRequestList);
#endif
void JsObjToGeofenceTransitionEventList(const napi_env& env, const napi_value& object,
    std::vector<GeofenceTransitionEvent>& geofenceTransitionStatusList);
void GetGeofenceTransitionEventArray(const napi_env& env, const napi_value& monitorGeofenceTransition,
    std::vector<GeofenceTransitionEvent>& geofenceTransitionStatusList);
#ifdef NOTIFICATION_ENABLE
void GenNotificationRequest(const napi_env& env, const napi_value& elementValue,
    std::shared_ptr<OHOS::Notification::NotificationRequest>& notificationRequest);
#endif
void GeofenceTransitionToJs(const napi_env& env,
    const GeofenceTransition geofenceTransition, napi_value& result);
void JsObjToGeofenceTransitionCallback(const napi_env& env, const napi_value& object,
    sptr<LocationGnssGeofenceCallbackNapi> callbackHost);
napi_status SetValueBeacon(const napi_env& env, const char* fieldStr, napi_value& value, napi_value& result);
napi_value CreateBeaconFenceJsObj(const napi_env& env, const std::shared_ptr<BeaconFence>& beaconFence);
}  // namespace Location
}  // namespace OHOS
#endif // GEOFENCE_NAPI_H
