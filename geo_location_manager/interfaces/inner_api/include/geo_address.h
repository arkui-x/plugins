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

#ifndef GEO_ADDRESS_H
#define GEO_ADDRESS_H

#include <map>
#include <mutex>
#include <parcel.h>
#include <string>

namespace OHOS {
namespace Location {
class GeoAddress : public Parcelable {
public:
    GeoAddress();
    ~GeoAddress() override = default;
    bool Marshalling(Parcel& parcel) const override;
    std::string GetDescriptions(int index);
    double GetLatitude();
    double GetLongitude();
    bool GetIsSystemApp();
    void SetIsSystemApp(bool isSystemApp);

    double latitude_;
    double longitude_;
    std::string locale_;
    std::string placeName_;
    std::string countryCode_;
    std::string countryName_;
    std::string administrativeArea_;
    std::string subAdministrativeArea_;
    std::string locality_;
    std::string subLocality_;
    std::string roadName_;
    std::string subRoadName_;
    std::string premises_;
    std::string postalCode_;
    std::string phoneNumber_;
    std::string addressUrl_;
    std::mutex mutex_;
    std::map<int, std::string> descriptions_;
    int descriptionsSize_ = 0;
    bool isFromMock_ = false;
    bool isSystemApp_ = false;
    static constexpr double PARCEL_INT_SIZE = 64.0;
    static constexpr int MAX_RESULT = 10;
};
} // namespace Location
} // namespace OHOS
#endif // GEO_ADDRESS_H
