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

#include "satellite_status.h"
#include <parcel.h>
#include "common_utils.h"

namespace OHOS {
namespace Location {
SatelliteStatus::SatelliteStatus()
{
    satellitesNumber_ = 0;
}

SatelliteStatus::SatelliteStatus(SatelliteStatus& satelliteStatus)
{
    satellitesNumber_ = satelliteStatus.GetSatellitesNumber();
    satelliteIds_ = satelliteStatus.GetSatelliteIds();
    carrierToNoiseDensitys_ = satelliteStatus.GetCarrierToNoiseDensitys();
    altitudes_ = satelliteStatus.GetAltitudes();
    azimuths_ = satelliteStatus.GetAzimuths();
    carrierFrequencies_ = satelliteStatus.GetCarrierFrequencies();
    constellationTypes_ = satelliteStatus.GetConstellationTypes();
    additionalInfoList_ = satelliteStatus.GetSatelliteAdditionalInfoList();
}

bool SatelliteStatus::Marshalling(Parcel& parcel) const
{
    return true;
}

bool SatelliteStatus::IsValidityDatas() const
{
    if (satellitesNumber_ != satelliteIds_.size()) {
        LBSLOGE(GNSS, "%{public}s: satelliteIds data length is incorrect.", __func__);
        return false;
    }
    if (satellitesNumber_ != carrierToNoiseDensitys_.size()) {
        LBSLOGE(GNSS, "%{public}s: carrierToNoiseDensitys data length is incorrect.", __func__);
        return false;
    }
    if (satellitesNumber_ != altitudes_.size()) {
        LBSLOGE(GNSS, "%{public}s: altitudes data length is incorrect.", __func__);
        return false;
    }
    if (satellitesNumber_ != azimuths_.size()) {
        LBSLOGE(GNSS, "%{public}s: azimuths data length is incorrect.", __func__);
        return false;
    }
    if (satellitesNumber_ != carrierFrequencies_.size()) {
        LBSLOGE(GNSS, "%{public}s: carrierFrequencies data length is incorrect.", __func__);
        return false;
    }
    if (satellitesNumber_ != constellationTypes_.size()) {
        LBSLOGE(GNSS, "%{public}s: constellationTypes data length is incorrect.", __func__);
        return false;
    }
    if (satellitesNumber_ != additionalInfoList_.size()) {
        LBSLOGE(GNSS, "%{public}s: additionalInfoList data length is incorrect.", __func__);
        return false;
    }
    return true;
}
} // namespace Location
} // namespace OHOS