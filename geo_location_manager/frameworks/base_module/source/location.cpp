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

#include "location.h"

#include <parcel.h>
#include <string>
#include "string_ex.h"
#include <iostream>
#include <cmath>

namespace OHOS {
namespace Location {
static constexpr double MIN_LATITUDE = -90.0;
static constexpr double MIN_LONGITUDE = -180.0;
static constexpr int MAX_POI_ARRAY_SIZE = 20;
static constexpr double MAX_LATITUDE = 90.0;
static constexpr double MAX_LONGITUDE = 180.0;
const double PI = 3.1415926;
const double DEGREE_PI = 180.0;
const double POW_PARAMETER_TOW = 2;
const double NUM_DOUBLE = 2;
const double EARTH_SEMI_AXIS = 6378137.0;
const double EARTH_FLATTENING = 6356752.3142;
const double EARTH_SEMI_MINOR = (EARTH_SEMI_AXIS - EARTH_FLATTENING) / EARTH_SEMI_AXIS;

Location::Location()
{
    latitude_ = MIN_LATITUDE - 1;
    longitude_ = MIN_LONGITUDE - 1;
    altitude_ = 0.0;
    accuracy_ = 0.0;
    speed_ = 0.0;
    direction_ = 0.0;
    timeStamp_ = 0;
    timeSinceBoot_ = 0;
    additionSize_ = 0;
    isFromMock_ = false;
    isSystemApp_ = 0;
    floorNo_ = 0;
    floorAccuracy_ = 0.0;
    altitudeAccuracy_ = 0.0;
    speedAccuracy_ = 0.0;
    directionAccuracy_ = 0.0;
    uncertaintyOfTimeSinceBoot_ = 0;
    locationSourceType_ = 0;
    uuid_ = "";
    fieldValidity_ = 0;
}

Location::Location(const Location& location)
{
    latitude_ = location.GetLatitude();
    longitude_ = location.GetLongitude();
    altitude_ = location.GetAltitude();
    accuracy_ = location.GetAccuracy();
    speed_ = location.GetSpeed();
    direction_ = location.GetDirection();
    timeStamp_ = location.GetTimeStamp();
    timeSinceBoot_ = location.GetTimeSinceBoot();
    floorNo_ = location.GetFloorNo();
    floorAccuracy_ = location.GetFloorAccuracy();
    additions_ = location.GetAdditions();
    additionsMap_ = location.GetAdditionsMap();
    additionSize_ = location.GetAdditionSize();
    isFromMock_ = location.GetIsFromMock();
    isSystemApp_ = location.GetIsSystemApp();
    altitudeAccuracy_ = location.GetAltitudeAccuracy();
    speedAccuracy_ = location.GetSpeedAccuracy();
    directionAccuracy_ = location.GetDirectionAccuracy();
    uncertaintyOfTimeSinceBoot_ = location.GetUncertaintyOfTimeSinceBoot();
    locationSourceType_ = location.GetLocationSourceType();
    uuid_ = location.GetUuid();
    fieldValidity_ = location.GetFieldValidity();
    poiInfo_ = location.GetPoiInfo();
}

bool Location::Marshalling(Parcel& parcel) const
{
    return true;
}


std::string Location::ToString() const
{
    std::string str =
        ", altitude : " + std::to_string(altitude_) +
        ", accuracy : " + std::to_string(accuracy_) +
        ", speed : " + std::to_string(speed_) +
        ", direction : " + std::to_string(direction_) +
        ", timeStamp : " + std::to_string(timeStamp_) +
        ", timeSinceBoot : " + std::to_string(timeSinceBoot_) +
        ", additionSize : " + std::to_string(additionSize_) +
        ", isFromMock : " + std::to_string(isFromMock_) +
        ", isSystemApp : " + std::to_string(isSystemApp_) +
        ", altitudeAccuracy : " + std::to_string(altitudeAccuracy_) +
        ", speedAccuracy : " + std::to_string(speedAccuracy_) +
        ", directionAccuracy : " + std::to_string(directionAccuracy_) +
        ", uncertaintyOfTimeSinceBoot : " + std::to_string(uncertaintyOfTimeSinceBoot_) +
        ", locationSourceType : " + std::to_string(locationSourceType_) +
        ", uuid : " + uuid_ +
        ", fieldValidity : " + std::to_string(fieldValidity_);
    return str;
}

bool Location::LocationEqual(const std::unique_ptr<Location>& location)
{
    if (location == nullptr) {
        return false;
    }
    if (this->GetLatitude() == location->GetLatitude() &&
        this->GetLongitude() == location->GetLongitude() &&
        this->GetAltitude() == location->GetAltitude() &&
        this->GetAccuracy() == location->GetAccuracy() &&
        this->GetSpeed() == location->GetSpeed() &&
        this->GetDirection() == location->GetDirection() &&
        this->GetTimeStamp() == location->GetTimeStamp() &&
        this->GetTimeSinceBoot() == location->GetTimeSinceBoot() &&
        this->AdditionEqual(location) &&
        this->GetAdditionSize() == location->GetAdditionSize() &&
        this->GetIsFromMock() == location->GetIsFromMock()) {
        return true;
    }
    return false;
}

bool Location::AdditionEqual(const std::unique_ptr<Location>& location)
{
    if (location == nullptr) {
        return false;
    }
    std::vector<std::string> additionA = this->GetAdditions();
    std::vector<std::string> additionB = location->GetAdditions();
    if (additionA.size() != additionB.size()) {
        return false;
    }
    for (size_t i = 0; i < additionA.size(); i++) {
        if (additionA[i].compare(additionB[i]) != 0) {
            return false;
        }
    }
    return true;
}

bool Location::isValidLatitude(double latitude)
{
    return latitude >= MIN_LATITUDE && latitude <= MAX_LATITUDE;
}

bool Location::isValidLongitude(double longitude)
{
    return longitude >= MIN_LONGITUDE && longitude <= MAX_LONGITUDE;
}

double Location::GetDistanceBetweenLocations(const double lat1, const double lon1, const double lat2, const double lon2)
{
    if (lat1 == lat2 && lon1 == lon2) {
        return 0;
    }
    double radLat1 = lat1 * PI / DEGREE_PI;
    double radLat2 = lat2 * PI / DEGREE_PI;
    double radLon1 = lon1 * PI / DEGREE_PI;
    double radLon2 = lon2 * PI / DEGREE_PI;

    double deltaLon = radLon2 - radLon1;
    double reducedLat1 = atan((1 - EARTH_SEMI_MINOR) * tan(radLat1));
    double reducedLat2 = atan((1 - EARTH_SEMI_MINOR) * tan(radLat2));

    double sinReducedLat1 = sin(reducedLat1);
    double cosReducedLat1 = cos(reducedLat1);
    double sinReducedLat2 = sin(reducedLat2);
    double cosReducedLat2 = cos(reducedLat2);

    double lambda = deltaLon;
    double lambdaP = 100;
    double iterLimit = 20;
    double sinAlpha = 0.0;
    double cosSqAlpha = 0.0;
    double sinSigma = 0.0;
    double cos2SigmaM = 0.0;
    double cosSigma = 0.0;
    double sigma = 0.0;
    double sinLambda = 0.0;
    double cosLambda = 0.0;

    for (int iter = 0; iter < iterLimit; iter++) {
        sinLambda = sin(lambda);
        cosLambda = cos(lambda);
        sinSigma = sqrt(pow(cosReducedLat2 * sinLambda, POW_PARAMETER_TOW) +
            pow(cosReducedLat1 * sinReducedLat2 - sinReducedLat1 * cosReducedLat2 * cosLambda, POW_PARAMETER_TOW));
        if (sinSigma == 0) {
            return 0;
        }
        cosSigma = sinReducedLat1 * sinReducedLat2 + cosReducedLat1 * cosReducedLat2 * cosLambda;
        sigma = atan2(sinSigma, cosSigma);
        sinAlpha = cosReducedLat1 * cosReducedLat2 * sinLambda / sinSigma;
        cosSqAlpha = 1 - sinAlpha * sinAlpha;
        cos2SigmaM = (cosSqAlpha != 0) ? (cosSigma - NUM_DOUBLE * sinReducedLat1 * sinReducedLat2 / cosSqAlpha) : 0;
        double correction = EARTH_SEMI_MINOR / 16 * cosSqAlpha * (4 + EARTH_SEMI_MINOR * (4 - 3 * cosSqAlpha));

        lambdaP = lambda;
        lambda = deltaLon + (1 - correction) * EARTH_SEMI_MINOR * sinAlpha  *
            (sigma + correction * sinSigma *
            (cos2SigmaM + correction * cosSigma * (-1 + NUM_DOUBLE * pow(cos2SigmaM, POW_PARAMETER_TOW))));
        if (fabs(lambda - lambdaP) < 1e-12) {
            break;
        }
    }
    double uSquared = cosSqAlpha * (EARTH_SEMI_AXIS * EARTH_SEMI_AXIS - EARTH_FLATTENING * EARTH_FLATTENING) /
        (EARTH_FLATTENING * EARTH_FLATTENING);
    double highOrderCorrection = 1 + uSquared / 16384 * (4096 + uSquared * (-768 + uSquared * (320 - 175 * uSquared)));
    double highOrderTerm = uSquared / 1024 * (256 + uSquared * (-128 + uSquared * (74 - 47 * uSquared)));
    double deltaSigma = highOrderTerm * sinSigma *
        (cos2SigmaM + highOrderTerm / 4 * (cosSigma * (-1 + 2 * pow(cos2SigmaM, POW_PARAMETER_TOW)) -
            highOrderTerm / 6 * cos2SigmaM * (-3 + 4 * pow(sinSigma, POW_PARAMETER_TOW)) *
            (-3 + 4 * pow(cos2SigmaM, POW_PARAMETER_TOW))));

    double distance = EARTH_FLATTENING * highOrderCorrection * (sigma - deltaSigma);
    return distance;
}
} // namespace Location
} // namespace OHOS
