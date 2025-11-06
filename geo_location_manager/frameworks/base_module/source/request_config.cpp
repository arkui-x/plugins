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

#include "request_config.h"

#include <parcel.h>

#include "constant_definition.h"

namespace OHOS {
namespace Location {
RequestConfig::RequestConfig()
{
    scenario_ = SCENE_UNSET;
    priority_ = PRIORITY_FAST_FIRST_FIX;
    timeInterval_ = 1; // no time interval limit for reporting location
    distanceInterval_ = 0.0; // no distance interval limit for reporting location
    maxAccuracy_ = 0.0; // no accuracy limit for reporting location
    fixNumber_ = 0; // no fix size limit for reporting location
    timeOut_ = DEFAULT_TIMEOUT_5S;
    timestamp_ = 0;
    isNeedPoi_ = false;
    isNeedLocation_ = true;
}

RequestConfig::RequestConfig(const int scenario) : scenario_(scenario)
{
    priority_ = PRIORITY_UNSET;
    timeInterval_ = 1; // no time interval limit for reporting location
    distanceInterval_ = 0.0; // no distance interval limit for reporting location
    maxAccuracy_ = 0.0; // no accuracy limit for reporting location
    fixNumber_ = 0; // no fix size limit for reporting location
    timeOut_ = DEFAULT_TIMEOUT_5S;
    timestamp_ = 0;
    isNeedPoi_ = false;
    isNeedLocation_ = true;
}

void RequestConfig::Set(RequestConfig& requestConfig)
{
    scenario_ = requestConfig.GetScenario();
    priority_ = requestConfig.GetPriority();
    timeInterval_ = requestConfig.GetTimeInterval();
    distanceInterval_ = requestConfig.GetDistanceInterval();
    maxAccuracy_ = requestConfig.GetMaxAccuracy();
    fixNumber_ = requestConfig.GetFixNumber();
    timeOut_ = requestConfig.GetTimeOut();
    timestamp_ = requestConfig.GetTimeStamp();
    isNeedPoi_ = requestConfig.GetIsNeedPoi();
    isNeedLocation_ = requestConfig.GetIsNeedLocation();
}

bool RequestConfig::IsSame(RequestConfig& requestConfig)
{
    if (scenario_ != requestConfig.GetScenario()) {
        return false;
    }
    if (scenario_ != SCENE_UNSET) {
        return true;
    }
    return priority_ == requestConfig.GetPriority();
}

bool RequestConfig::Marshalling(Parcel& parcel) const
{
    return true;
}

bool RequestConfig::IsRequestForAccuracy()
{
    if (priority_ == LOCATION_PRIORITY_ACCURACY ||
        (scenario_ == SCENE_UNSET && priority_ == PRIORITY_ACCURACY) ||
        scenario_ == SCENE_NAVIGATION ||
        scenario_ == SCENE_TRAJECTORY_TRACKING ||
        scenario_ == SCENE_CAR_HAILING) {
        return true;
    } else {
        return false;
    }
}

std::string RequestConfig::ToString() const
{
    std::string str = "scenario : " + std::to_string(scenario_) +
        ", location priority : " + std::to_string(priority_) +
        ", timeInterval : " + std::to_string(timeInterval_) +
        ", distanceInterval : " + std::to_string(distanceInterval_) +
        ", maxAccuracy : " + std::to_string(maxAccuracy_) +
        ", fixNumber : " + std::to_string(fixNumber_) +
        ", timeOut : " + std::to_string(timeOut_) +
        ", isNeedPoiInfomation : " + std::to_string(isNeedPoi_) +
        ", isNeedLocation : " + std::to_string(isNeedLocation_);
    return str;
}
} // namespace Location
} // namespace OHOS