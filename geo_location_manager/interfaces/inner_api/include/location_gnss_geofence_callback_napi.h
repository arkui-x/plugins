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

#ifndef LOCATION_GNSS_GEOFENCE_CALLBACK_NAPI_H
#define LOCATION_GNSS_GEOFENCE_CALLBACK_NAPI_H

#include "iremote_stub.h"
#include "napi/native_api.h"
#include "uv.h"
#include "common_utils.h"
#include "i_gnss_geofence_callback.h"
#include "geofence_definition.h"

namespace OHOS {
namespace Location {
bool FindGeofenceRegCallback(napi_ref cb);
void DeleteGeofenceRegCallback(napi_ref cb);
class LocationGnssGeofenceCallbackNapi : public IRemoteStub<IGnssGeofenceCallback> {
public:
    LocationGnssGeofenceCallbackNapi();
    virtual ~LocationGnssGeofenceCallbackNapi();
    bool IsRemoteDied();
    void DeleteHandler();
    void CountDown();
    void Wait(int time);
    int GetCount();
    void SetCount(int count);
    int GetFenceId();
    void ClearFenceId();
    void SetFenceId(int fenceId);
    LocationErrCode DealGeofenceOperationResult();
    GnssGeofenceOperateType GetGeofenceOperationType();
    void SetGeofenceOperationType(GnssGeofenceOperateType type);
    GnssGeofenceOperateResult GetGeofenceOperationResult();
    void SetGeofenceOperationResult(GnssGeofenceOperateResult result);
    void OnTransitionStatusChange(GeofenceTransition transition) override;
    void OnReportOperationResult(int fenceId, int type, int result) override;
    napi_ref GetHandleCb();
    void SetHandleCb(const napi_ref& handlerCb);
    napi_env GetEnv();
    void SetEnv(const napi_env& env);

    template <typename T>
    bool InitContext(T* context)
    {
        if (context == nullptr) {
            LBSLOGE(LOCATION_GNSS_GEOFENCE_CALLBACK, "context == nullptr.");
            return false;
        }
        context->env = env_;
        context->callback[SUCCESS_CALLBACK] = handlerCb_;
        return true;
    }

    inline bool GetRemoteDied() const
    {
        return remoteDied_;
    }

    inline void SetRemoteDied(const bool remoteDied)
    {
        remoteDied_ = remoteDied;
    }

private:
    void UvQueueWork(uv_loop_s* loop, uv_work_t* work);
    void InitLatch();

    napi_env env_;
    napi_ref handlerCb_;
    bool remoteDied_;
    std::mutex mutex_;
    std::mutex operationResultMutex_;
    CountDownLatch* latch_;
    int fenceId_;
    GnssGeofenceOperateType type_;
    GnssGeofenceOperateResult result_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_GNSS_GEOFENCE_CALLBACK_NAPI_H
