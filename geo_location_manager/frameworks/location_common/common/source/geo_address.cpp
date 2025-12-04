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

#include "geo_address.h"
#include "string_ex.h"
#include "common_utils.h"

namespace OHOS {
namespace Location {
GeoAddress::GeoAddress()
{
    latitude_ = 0.0;
    longitude_ = 0.0;
}

std::string GeoAddress::GetDescriptions(int index)
{
    if (index < 0) {
        return "";
    }
    if (descriptionsSize_ <= 0) {
        return "";
    }

    std::unique_lock<std::mutex> lock(mutex_);
    std::map<int, std::string>::iterator it = descriptions_.find(index);
    if (it == descriptions_.end()) {
        return "";
    }

    return it->second;
}

double GeoAddress::GetLatitude()
{
    return latitude_;
}

double GeoAddress::GetLongitude()
{
    return longitude_;
}

bool GeoAddress::GetIsSystemApp()
{
    return isSystemApp_;
}

void GeoAddress::SetIsSystemApp(bool isSystemApp)
{
    isSystemApp_ = isSystemApp;
}

bool GeoAddress::Marshalling(Parcel& parcel) const
{
    return true;
}
} // namespace Location
} // namespace OHOS
