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

#ifndef SATELLITE_STATUS_H
#define SATELLITE_STATUS_H

#include <mutex>
#include <parcel.h>
#include <vector>

namespace OHOS {
namespace Location {
class SatelliteStatus : public Parcelable {
public:
    SatelliteStatus();
    explicit SatelliteStatus(SatelliteStatus &satelliteStatus);
    ~SatelliteStatus() override = default;

    inline int GetSatellitesNumber() const
    {
        return satellitesNumber_;
    }

    inline void SetSatellitesNumber(int num)
    {
        satellitesNumber_ = num;
    }

    inline std::vector<int> GetSatelliteIds() const
    {
        return satelliteIds_;
    }

    inline void SetSatelliteIds(std::vector<int> ids)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        for (std::vector<int>::iterator it = ids.begin(); it != ids.end(); ++it) {
            satelliteIds_.push_back(*it);
        }
    }

    inline void SetSatelliteId(int id)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        satelliteIds_.push_back(id);
    }

    inline std::vector<double> GetCarrierToNoiseDensitys() const
    {
        return carrierToNoiseDensitys_;
    }

    inline void SetCarrierToNoiseDensitys(std::vector<double> cn0)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        for (std::vector<double>::iterator it = cn0.begin(); it != cn0.end(); ++it) {
            carrierToNoiseDensitys_.push_back(*it);
        }
    }

    inline void SetCarrierToNoiseDensity(double cn0)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        carrierToNoiseDensitys_.push_back(cn0);
    }

    inline std::vector<double> GetAltitudes() const
    {
        return altitudes_;
    }

    inline void SetAltitudes(std::vector<double> altitudes)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        for (std::vector<double>::iterator it = altitudes.begin(); it != altitudes.end(); ++it) {
            altitudes_.push_back(*it);
        }
    }

    inline void SetAltitude(double altitude)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        altitudes_.push_back(altitude);
    }

    inline std::vector<double> GetAzimuths() const
    {
        return azimuths_;
    }

    inline void SetAzimuths(std::vector<double> azimuths)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        for (std::vector<double>::iterator it = azimuths.begin(); it != azimuths.end(); ++it) {
            azimuths_.push_back(*it);
        }
    }

    inline void SetAzimuth(double azimuth)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        azimuths_.push_back(azimuth);
    }

    inline std::vector<double> GetCarrierFrequencies() const
    {
        return carrierFrequencies_;
    }

    inline void SetCarrierFrequencies(std::vector<double> cfs)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        for (std::vector<double>::iterator it = cfs.begin(); it != cfs.end(); ++it) {
            carrierFrequencies_.push_back(*it);
        }
    }

    inline void SetCarrierFrequencie(double cf)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        carrierFrequencies_.push_back(cf);
    }

    inline std::vector<int> GetConstellationTypes() const
    {
        return constellationTypes_;
    }

    inline void SetConstellationTypes(std::vector<int> types)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        for (std::vector<int>::iterator it = types.begin(); it != types.end(); ++it) {
            constellationTypes_.push_back(*it);
        }
    }

    inline void SetConstellationType(int type)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        constellationTypes_.push_back(type);
    }

    inline std::vector<int> GetSatelliteAdditionalInfoList()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        return additionalInfoList_;
    }

    inline void SetSatelliteAdditionalInfo(int additionalInfo)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        additionalInfoList_.push_back(additionalInfo);
    }

    inline void SetSatelliteAdditionalInfoList(std::vector<int> additionalInfo)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        for (std::vector<int>::iterator it = additionalInfo.begin();
            it != additionalInfo.end(); ++it) {
            additionalInfoList_.push_back(*it);
        }
    }

    bool Marshalling(Parcel& parcel) const override;
private:
    bool IsValidityDatas() const;
    
    unsigned int satellitesNumber_;
    std::vector<int> satelliteIds_;
    std::vector<double> carrierToNoiseDensitys_;
    std::vector<double> altitudes_;
    std::vector<double> azimuths_;
    std::vector<double> carrierFrequencies_;
    std::vector<int> constellationTypes_;
    std::vector<int> additionalInfoList_;
    std::mutex mutex_;
};
} // namespace Location
} // namespace OHOS
#endif // SATELLITE_STATUS_H
