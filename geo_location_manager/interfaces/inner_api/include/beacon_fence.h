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

#ifndef LOCATION_BEACON_FENCE_H
#define LOCATION_BEACON_FENCE_H

#include <mutex>
#include <parcel.h>
#include <string>
#include "string_ex.h"
#include "iremote_object.h"

namespace OHOS {
namespace Location {
enum BeaconFenceInfoType {
    BEACON_MANUFACTURE_DATA = 1,
};

typedef struct {
    int32_t manufactureId;
    std::vector<uint8_t> manufactureData;
    std::vector<uint8_t> manufactureDataMask;
} BeaconManufactureData;

class BeaconFence : public Parcelable {
public:
    BeaconFence();
    BeaconFence(BeaconFence &BeaconFence);
    ~BeaconFence();

    inline std::string GetIdentifier() const
    {
        return identifier_;
    }

    inline void SetIdentifier(std::string identifier)
    {
        identifier_ = identifier;
    }

    inline BeaconFenceInfoType GetBeaconFenceInfoType() const
    {
        return type_;
    }

    inline void SetBeaconFenceInfoType(BeaconFenceInfoType type)
    {
        type_ = type;
    }

    inline BeaconManufactureData GetBeaconManufactureData() const
    {
        return manufactureData_;
    }

    inline void SetBeaconManufactureData(BeaconManufactureData manufactureData)
    {
        manufactureData_ = manufactureData;
    }

    bool Marshalling(Parcel& parcel) const override;
    mutable std::mutex beaconFenceMutex_;
    std::string identifier_;
    BeaconFenceInfoType type_;
    BeaconManufactureData manufactureData_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_BEACON_FENCE_REQUEST_H