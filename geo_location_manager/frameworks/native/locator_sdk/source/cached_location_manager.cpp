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

#include "cached_location_manager.h"

namespace OHOS {
namespace Location {

CachedLocationManager& CachedLocationManager::GetInstance()
{
    static CachedLocationManager instance;
    return instance;
}

void CachedLocationManager::UpdateLocation(const std::unique_ptr<Location>& location)
{
    if (!location) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    cachedLocation_ = std::make_unique<Location>(*location);
}

std::unique_ptr<Location> CachedLocationManager::GetLastLocation()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!cachedLocation_) {
        return nullptr;
    }
    return std::make_unique<Location>(*cachedLocation_);
}

} // namespace Location
} // namespace OHOS
