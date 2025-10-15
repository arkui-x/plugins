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

#ifndef LOCATION_BEACON_FENCE_REQUEST_H
#define LOCATION_BEACON_FENCE_REQUEST_H

#include <mutex>
#include <parcel.h>
#include <string>
#include "string_ex.h"
#include "iremote_object.h"
#include "geofence_definition.h"
#include "beacon_fence.h"

namespace OHOS {
namespace Location {

class BeaconFenceRequest : public Parcelable {
public:
    BeaconFenceRequest();
    BeaconFenceRequest(BeaconFenceRequest &beaconFenceRequest);
    ~BeaconFenceRequest();

    inline sptr<IRemoteObject> GetBeaconFenceTransitionCallback() const
    {
        return callback_;
    }

    inline void SetBeaconFenceTransitionCallback(const sptr<IRemoteObject>& callback)
    {
        callback_ = callback;
    }

    inline std::string GetFenceExtensionAbilityName() const
    {
        return fenceExtensionAbilityName_;
    }

    inline void SetFenceExtensionAbilityName(std::string fenceExtensionAbilityName)
    {
        fenceExtensionAbilityName_ = fenceExtensionAbilityName;
    }

    inline std::string GetServiceUuid() const
    {
        return serviceUuid_;
    }

    inline void SetServiceUuid(std::string serviceUuid)
    {
        serviceUuid_ = serviceUuid;
    }

    inline std::string GetServiceUuidMask() const
    {
        return serviceUuidMask_;
    }

    inline void SetServiceUuidMask(std::string serviceUuidMask)
    {
        serviceUuidMask_ = serviceUuidMask;
    }

    inline std::string GetBundleName() const
    {
        return bundleName_;
    }

    inline void SetBundleName(std::string bundleName)
    {
        bundleName_ = bundleName;
    }

    inline std::string GetFenceId() const
    {
        return fenceId_;
    }

    inline void SetFenceId(std::string fenceId)
    {
        fenceId_ = fenceId;
    }

    bool Marshalling(Parcel& parcel) const override;
    std::shared_ptr<BeaconFence> GetBeaconFence();
    void SetBeaconFence(std::shared_ptr<BeaconFence>& beacon);

private:
    mutable std::mutex beaconFenceRequestMutex_;
    std::shared_ptr<BeaconFence> beacon_;
    sptr<IRemoteObject> callback_ = nullptr;
    std::string fenceId_;
    std::string fenceExtensionAbilityName_;
    std::string serviceUuid_;
    std::string serviceUuidMask_;
    std::string bundleName_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_BEACON_FENCE_REQUEST_H