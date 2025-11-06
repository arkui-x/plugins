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

#ifndef GNSS_GEOFENCE_REQUEST_H
#define GNSS_GEOFENCE_REQUEST_H

#include <mutex>
#include <vector>
#include "geofence_definition.h"
#include <parcel.h>
#include "iremote_object.h"

namespace OHOS {
namespace Notification {
    class NotificationRequest;
}

namespace Location {
typedef struct {
    double latitude;
    double longitude;
    double radius;
    double expiration;
    CoordinateSystemType coordinateSystemType;
} GeoFence;

class GeofenceRequest : public Parcelable {
public:
    GeofenceRequest();

    GeofenceRequest(GeofenceRequest& geofenceRequest);

    ~GeofenceRequest();

    GeoFence GetGeofence();
    void SetGeofence(GeoFence geofence);

    int GetScenario();

    void SetScenario(int scenario);

    void SetWantAgentParcelData(const Parcel& data);

    bool GetWantAgentParcelData(Parcel& data);

    std::vector<GeofenceTransitionEvent> GetGeofenceTransitionEventList();

    void SetGeofenceTransitionEvent(GeofenceTransitionEvent status);

    void SetGeofenceTransitionEventList(std::vector<GeofenceTransitionEvent> statusList);

    std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>> GetNotificationRequestList();

    void SetNotificationRequest(std::shared_ptr<OHOS::Notification::NotificationRequest> request);

    void SetNotificationRequestList(std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>> requestList);

    void SetGeofenceTransitionCallback(const sptr<IRemoteObject>& callback);

    sptr<IRemoteObject> GetGeofenceTransitionCallback();

    int GetFenceId();

    void SetFenceId(int fenceId);

    const std::string& GetBundleName();

    void SetBundleName(const std::string& bundleName);

    int32_t GetUid();

    void SetUid(int32_t uid);

    bool GetAppAliveStatus();

    void SetAppAliveStatus(bool appAliveStatus);

    int64_t GetRequestExpirationTime();

    void SetRequestExpirationTime(int64_t requestExpirationTime);

    bool Marshalling(Parcel& parcel) const override;
private:
    std::vector<GeofenceTransitionEvent> transitionStatusList_;
    std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>> notificationRequestList_;
    sptr<IRemoteObject> callback_ = nullptr;
    GeoFence geofence_{0.0, 0.0, 0.0, WGS84};
    int scenario_;
    int fenceId_;
    int32_t uid_;
    std::vector<char> wantAgentBuffer_;
    std::string bundleName_;
    bool appAliveStatus_;
    int64_t requestExpirationTime_ = 0;
    mutable std::mutex geofenceRequestMutex_;
};
} // namespace Location
} // namespace OHOS
#endif // GNSS_GEOFENCE_REQUEST_H
