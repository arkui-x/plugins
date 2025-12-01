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

#ifndef GEO_CODE_REQUEST_H
#define GEO_CODE_REQUEST_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace Location {
class GeoCodeRequest {
public:
    GeoCodeRequest()
        : locale_(), description_(), maxItems_(0),
          minLatitude_(0.0f), minLongitude_(0.0f),
          maxLatitude_(0.0f), maxLongitude_(0.0f),
          transId_(), country_() {}
    ~GeoCodeRequest() = default;

    std::string locale_;
    std::string description_;
    int32_t maxItems_;
    float minLatitude_;
    float minLongitude_;
    float maxLatitude_;
    float maxLongitude_;
    std::string transId_;
    std::string country_;
};

class ReverseGeoCodeRequest {
public:
    ReverseGeoCodeRequest()
        : locale_(), maxItems_(0), latitude_(0.0f), longitude_(0.0f), country_() {}
    ~ReverseGeoCodeRequest() = default;

    std::string locale_;
    int32_t maxItems_;
    float latitude_;
    float longitude_;
    std::string country_;
    std::string transId_;
};
}
} // namespace OHOS
#endif