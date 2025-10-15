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

#ifndef LOCATION_H
#define LOCATION_H

#include <parcel.h>
#include <string>
#include <map>

namespace OHOS {
namespace Location {

typedef struct Poi {
    std::string id = "";
    double confidence = 0.0;
    std::string name = "";
    double latitude = 0.0;
    double longitude = 0.0;
    std::string administrativeArea = "";
    std::string subAdministrativeArea = "";
    std::string locality = "";
    std::string subLocality = "";
    std::string address = "";
} Poi;
 
typedef struct PoiInfo {
    std::vector<Poi> poiArray;
    uint64_t timestamp = 0;
} PoiInfo;

class Location : public Parcelable {
public:
    Location();
    explicit Location(const Location &location);
    ~Location() override = default;

    inline double GetLatitude() const
    {
        return latitude_;
    }

    inline void SetLatitude(double latitude)
    {
        latitude_ = latitude;
    }

    inline double GetLongitude() const
    {
        return longitude_;
    }

    inline void SetLongitude(double longitude)
    {
        longitude_ = longitude;
    }

    inline double GetAltitude() const
    {
        return altitude_;
    }

    inline void SetAltitude(double altitude)
    {
        altitude_ = altitude;
    }

    inline double GetAccuracy() const
    {
        return accuracy_;
    }

    inline void SetAccuracy(double accuracy)
    {
        accuracy_ = accuracy;
    }

    inline double GetSpeed() const
    {
        return speed_;
    }

    inline void SetSpeed(double speed)
    {
        speed_ = speed;
    }

    inline double GetDirection() const
    {
        return direction_;
    }

    inline void SetDirection(double direction)
    {
        direction_ = direction;
    }

    inline int64_t GetTimeStamp() const
    {
        return timeStamp_;
    }

    inline void SetTimeStamp(int64_t timeStamp)
    {
        timeStamp_ = timeStamp;
    }

    inline int64_t GetTimeSinceBoot() const
    {
        return timeSinceBoot_;
    }

    inline void SetTimeSinceBoot(int64_t timeStamp)
    {
        timeSinceBoot_ = timeStamp;
    }

    inline std::vector<std::string> GetAdditions() const
    {
        return additions_;
    }

    inline void SetAdditions(std::vector<std::string> additions, bool ifAppend)
    {
        if (!ifAppend) {
            std::vector<std::string>().swap(additions_);
        }
        for (auto it = additions.begin(); it != additions.end(); ++it) {
            additions_.push_back(*it);
        }
    }

    inline int64_t GetAdditionSize() const
    {
        return additionSize_;
    }

    inline void SetAdditionSize(int64_t size)
    {
        additionSize_ = size;
    }

    inline int GetIsFromMock() const
    {
        return isFromMock_;
    }

    inline void SetIsFromMock(int fromMock)
    {
        isFromMock_ = fromMock;
    }

    inline int32_t GetFloorNo() const
    {
        return floorNo_;
    }

    inline void SetFloorNo(int32_t floorNo)
    {
        floorNo_ = floorNo;
    }

    inline int32_t GetIsSystemApp() const
    {
        return isSystemApp_;
    }

    inline void SetIsSystemApp(int32_t isSystemApp)
    {
        isSystemApp_ = isSystemApp;
    }

    inline int64_t GetUncertaintyOfTimeSinceBoot() const
    {
        return uncertaintyOfTimeSinceBoot_;
    }

    inline void SetUncertaintyOfTimeSinceBoot(int64_t uncertaintyOfTimeSinceBoot)
    {
        uncertaintyOfTimeSinceBoot_ = uncertaintyOfTimeSinceBoot;
    }

    inline double GetFloorAccuracy() const
    {
        return floorAccuracy_;
    }

    inline void SetFloorAccuracy(double floorAccuracy)
    {
        floorAccuracy_ = floorAccuracy;
    }

    inline double GetAltitudeAccuracy() const
    {
        return altitudeAccuracy_;
    }

    inline void SetAltitudeAccuracy(double altitudeAccuracy)
    {
        altitudeAccuracy_ = altitudeAccuracy;
    }

    inline double GetSpeedAccuracy() const
    {
        return speedAccuracy_;
    }

    inline void SetSpeedAccuracy(double speedAccuracy)
    {
        speedAccuracy_ = speedAccuracy;
    }

    inline double GetDirectionAccuracy() const
    {
        return directionAccuracy_;
    }

    inline void SetDirectionAccuracy(double directionAccuracy)
    {
        directionAccuracy_ = directionAccuracy;
    }
    
    inline int32_t GetLocationSourceType() const
    {
        return locationSourceType_;
    }

    inline void SetLocationSourceType(int32_t locationSourceType)
    {
        locationSourceType_ = locationSourceType;
    }

    inline std::string GetUuid() const
    {
        return uuid_;
    }

    inline void SetUuid(std::string uuid)
    {
        uuid_ = uuid;
    }

    inline std::map<std::string, std::string> GetAdditionsMap() const
    {
        return additionsMap_;
    }

    inline int32_t GetFieldValidity() const
    {
        return fieldValidity_;
    }

    inline void SetFieldValidity(int32_t fieldValidity)
    {
        fieldValidity_ = fieldValidity;
    }

    inline PoiInfo GetPoiInfo() const
    {
        return poiInfo_;
    }
 
    inline void SetPoiInfo(PoiInfo poiInfo)
    {
        poiInfo_ = poiInfo;
    }

    bool Marshalling(Parcel& parcel) const override;
    std::string ToString() const;
    bool LocationEqual(const std::unique_ptr<Location>& location);
    bool AdditionEqual(const std::unique_ptr<Location>& location);
    static double GetDistanceBetweenLocations(const double lat1, const double lon1,
        const double lat2, const double lon2);
    static bool isValidLatitude(double latitude);
    static bool isValidLongitude(double longitude);
private:
    double latitude_;
    double longitude_;
    double altitude_;
    double accuracy_;
    double speed_;
    double direction_;
    int64_t timeStamp_;
    int64_t timeSinceBoot_;
    std::vector<std::string> additions_;
    int64_t additionSize_;
    bool isFromMock_;
    int32_t isSystemApp_;
    int32_t floorNo_;
    double floorAccuracy_;
    std::map<std::string, std::string> additionsMap_;
    double altitudeAccuracy_;
    double speedAccuracy_;
    double directionAccuracy_;
    int64_t uncertaintyOfTimeSinceBoot_;
    int32_t locationSourceType_;
    std::string uuid_;
    int32_t fieldValidity_;
    PoiInfo poiInfo_;
};
} // namespace Location
} // namespace OHOS
#endif // LOCATION_H