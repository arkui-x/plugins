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

#ifndef GEOCODE_CONVERT_ADDRESS_REQUEST_H
#define GEOCODE_CONVERT_ADDRESS_REQUEST_H

#include <singleton.h>
#include <string>
#include <vector>

#include "iremote_object.h"
#include "common_utils.h"

namespace OHOS {
namespace Location {
class GeocodeConvertAddressRequest : public Parcelable {
public:
    GeocodeConvertAddressRequest();
    ~GeocodeConvertAddressRequest();
    std::string GetLocale();
    void SetLocale(std::string locale);
    int32_t GetMaxItems();
    void SetMaxItems(int32_t maxItems);
    std::string GetDescription();
    void SetDescription(std::string description);
    double GetMaxLatitude();
    void SetMaxLatitude(double maxLatitude);
    double GetMaxLongitude();
    void SetMaxLongitude(double maxLongitude);
    double GetMinLatitude();
    void SetMinLatitude(double minLatitude);
    double GetMinLongitude();
    void SetMinLongitude(double minLongitude);
    std::string GetTransId();
    void SetTransId(std::string transId);
    std::string GetCountry();
    void SetCountry(std::string country);

    bool Marshalling(Parcel& parcel) const;
private:
    std::string locale_;
    int64_t maxItems_;
    std::string description_;
    double maxLatitude_;
    double maxLongitude_;
    double minLatitude_;
    double minLongitude_;
    std::string transId_;
    std::string country_;
};
} // namespace OHOS
} // namespace Location
#endif // GEOCODE_CONVERT_ADDRESS_REQUEST_H
