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

#include "beacon_fence_request.h"

namespace OHOS {
namespace Location {
BeaconFenceRequest::BeaconFenceRequest()
{
    callback_ = nullptr;
    fenceId_ = "";
    fenceExtensionAbilityName_ = "";
    serviceUuid_ = "";
    serviceUuidMask_ = "";
}

BeaconFenceRequest::BeaconFenceRequest(BeaconFenceRequest& beaconFenceRequest)
{
    beacon_ =  beaconFenceRequest.GetBeaconFence();
    callback_ = beaconFenceRequest.GetBeaconFenceTransitionCallback();
    fenceExtensionAbilityName_ = beaconFenceRequest.GetFenceExtensionAbilityName();
    fenceId_ = beaconFenceRequest.GetFenceId();
}

BeaconFenceRequest::~BeaconFenceRequest() {}

std::shared_ptr<BeaconFence> BeaconFenceRequest::GetBeaconFence()
{
    return beacon_;
}

void BeaconFenceRequest::SetBeaconFence(std::shared_ptr<BeaconFence>& beacon)
{
    beacon_ = beacon;
}

bool BeaconFenceRequest::Marshalling(Parcel& parcel) const
{
    return true;
}
} // namespace Location
} // namespace OHOS