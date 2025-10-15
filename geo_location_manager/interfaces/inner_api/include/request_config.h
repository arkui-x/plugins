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

#ifndef REQUEST_CONFIG_H
#define REQUEST_CONFIG_H

#include <parcel.h>
#include <string>

namespace OHOS {
namespace Location {
class RequestConfig : public Parcelable {
public:
    RequestConfig();
    explicit RequestConfig(const int scenario);
    ~RequestConfig() override = default;

    inline int GetScenario() const
    {
        return scenario_;
    }

    inline void SetScenario(int scenario)
    {
        scenario_ = scenario;
    }

    inline void SetPriority(int priority)
    {
        priority_ = priority;
    }

    inline int GetPriority()
    {
        return priority_;
    }

    inline int GetTimeInterval() const
    {
        return timeInterval_;
    }

    inline void SetTimeInterval(int timeInterval)
    {
        timeInterval_ = timeInterval;
    }

    inline double GetDistanceInterval() const
    {
        return distanceInterval_;
    }

    inline void SetDistanceInterval(double distanceInterval)
    {
        distanceInterval_ = distanceInterval;
    }

    inline float GetMaxAccuracy() const
    {
        return maxAccuracy_;
    }

    inline void SetMaxAccuracy(float maxAccuracy)
    {
        maxAccuracy_ = maxAccuracy;
    }

    inline void SetFixNumber(int fixNumber)
    {
        fixNumber_ = fixNumber;
    }

    inline int GetFixNumber()
    {
        return fixNumber_;
    }

    inline void SetTimeOut(int time)
    {
        timeOut_ = time;
    }

    inline int GetTimeOut()
    {
        return timeOut_;
    }

    inline void SetTimeStamp(int64_t time)
    {
        timestamp_ = time;
    }

    inline int64_t GetTimeStamp()
    {
        return timestamp_;
    }

    inline void SetIsNeedPoi(bool isNeedPoi)
    {
        isNeedPoi_ = isNeedPoi;
    }
 
    inline bool GetIsNeedPoi()
    {
        return isNeedPoi_;
    }

    inline void SetIsNeedLocation(bool isNeedLocation)
    {
        isNeedLocation_ = isNeedLocation;
    }
 
    inline bool GetIsNeedLocation()
    {
        return isNeedLocation_;
    }

    bool Marshalling(Parcel& parcel) const override;
    std::string ToString() const;
    void Set(RequestConfig& requestConfig);
    bool IsSame(RequestConfig& requestConfig);
    bool IsRequestForAccuracy();
private:
    int scenario_;
    int timeInterval_; /* Units are seconds */
    double distanceInterval_ = 0.0;
    float maxAccuracy_;
    int fixNumber_;
    int priority_;
    int timeOut_;
    int64_t timestamp_;
    bool isNeedPoi_;
    bool isNeedLocation_;
};
} // namespace Location
} // namespace OHOS
#endif // REQUEST_CONFIG_H
