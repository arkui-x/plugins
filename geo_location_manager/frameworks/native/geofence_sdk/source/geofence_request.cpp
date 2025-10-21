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

#include "geofence_request.h"
#include <parcel.h>
#include "common_utils.h"
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

namespace OHOS {
namespace Location {
GeofenceRequest::GeofenceRequest()
{
    callback_ = nullptr;
    scenario_ = -1;
    fenceId_ = -1;
    uid_ = 0;
    appAliveStatus_ = true;
}

GeofenceRequest::GeofenceRequest(GeofenceRequest& geofenceRequest)
{
    this->SetGeofence(geofenceRequest.GetGeofence());
    this->SetScenario(geofenceRequest.GetScenario());
    Parcel agentParcelData;
    geofenceRequest.GetWantAgentParcelData(agentParcelData);
    this->SetWantAgentParcelData(agentParcelData);
    this->SetGeofenceTransitionEventList(geofenceRequest.GetGeofenceTransitionEventList());
#ifdef NOTIFICATION_ENABLE
    this->SetNotificationRequestList(geofenceRequest.GetNotificationRequestList());
#endif
    this->SetGeofenceTransitionCallback(geofenceRequest.GetGeofenceTransitionCallback());
    this->SetFenceId(geofenceRequest.GetFenceId());
    this->SetBundleName(geofenceRequest.GetBundleName());
}

GeofenceRequest::~GeofenceRequest() {}

GeoFence GeofenceRequest::GetGeofence()
{
    return geofence_;
}

void GeofenceRequest::SetGeofence(GeoFence geofence)
{
    geofence_ = geofence;
}

int GeofenceRequest::GetScenario()
{
    return scenario_;
}

void GeofenceRequest::SetScenario(int scenario)
{
    scenario_ = scenario;
}

void GeofenceRequest::SetWantAgentParcelData(const Parcel& data)
{
    std::vector<char>().swap(wantAgentBuffer_);
    char *first = reinterpret_cast<char*>(data.GetData());
    char *last = first + data.GetDataSize();
    wantAgentBuffer_.assign(first, last);
}

bool GeofenceRequest::GetWantAgentParcelData(Parcel& data)
{
    if (wantAgentBuffer_.empty()) {
        return false;
    }
    void* tempBuffer = malloc(wantAgentBuffer_.size());
    if (tempBuffer == NULL) {
        return false;
    }
    errno_t ret = memcpy_s(tempBuffer, wantAgentBuffer_.size(), wantAgentBuffer_.data(), wantAgentBuffer_.size());
    if (ret != EOK) {
        LBSLOGE(LOCATOR, "memcpy_s failed, error code:%{public}d", ret);
        free(tempBuffer);
        return false;
    }
    bool result = data.ParseFrom(reinterpret_cast<uintptr_t>(tempBuffer), wantAgentBuffer_.size());
    if (!result) {
        LBSLOGE(LOCATOR, "ParseFrom failed");
        free(tempBuffer);
        return false;
    }
    return result;
}

std::vector<GeofenceTransitionEvent> GeofenceRequest::GetGeofenceTransitionEventList()
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    return transitionStatusList_;
}

void GeofenceRequest::SetGeofenceTransitionEvent(GeofenceTransitionEvent status)
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    transitionStatusList_.push_back(status);
}

void GeofenceRequest::SetGeofenceTransitionEventList(std::vector<GeofenceTransitionEvent> statusList)
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    for (auto it = statusList.begin(); it != statusList.end(); ++it) {
        transitionStatusList_.push_back(*it);
    }
}

#ifdef NOTIFICATION_ENABLE
std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>> GeofenceRequest::GetNotificationRequestList()
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    return notificationRequestList_;
}

void GeofenceRequest::SetNotificationRequest(std::shared_ptr<OHOS::Notification::NotificationRequest> request)
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    notificationRequestList_.push_back(request);
}

void GeofenceRequest::SetNotificationRequestList(
    std::vector<std::shared_ptr<OHOS::Notification::NotificationRequest>> requestList)
{
    std::unique_lock<std::mutex> lock(geofenceRequestMutex_);
    for (auto it = requestList.begin(); it != requestList.end(); ++it) {
        notificationRequestList_.push_back(*it);
    }
}
#endif

void GeofenceRequest::SetGeofenceTransitionCallback(const sptr<IRemoteObject>& callback)
{
    callback_ = callback;
}

sptr<IRemoteObject> GeofenceRequest::GetGeofenceTransitionCallback()
{
    return callback_;
}

int GeofenceRequest::GetFenceId()
{
    return fenceId_;
}

void GeofenceRequest::SetFenceId(int fenceId)
{
    fenceId_ = fenceId;
}

const std::string& GeofenceRequest::GetBundleName()
{
    return bundleName_;
}

void GeofenceRequest::SetBundleName(const std::string& bundleName)
{
    bundleName_ = bundleName;
}

int32_t GeofenceRequest::GetUid()
{
    return uid_;
}

void GeofenceRequest::SetUid(int32_t uid)
{
    uid_ = uid;
}

bool GeofenceRequest::GetAppAliveStatus()
{
    return appAliveStatus_;
}

int64_t GeofenceRequest::GetRequestExpirationTime()
{
    return requestExpirationTime_;
}

void GeofenceRequest::SetRequestExpirationTime(int64_t requestExpirationTime)
{
    requestExpirationTime_ = requestExpirationTime;
}

void GeofenceRequest::SetAppAliveStatus(bool appAliveStatus)
{
    appAliveStatus_ = appAliveStatus;
}

bool GeofenceRequest::Marshalling(Parcel& parcel) const
{
    return true;
}
} // namespace Location
} // namespace OHOS