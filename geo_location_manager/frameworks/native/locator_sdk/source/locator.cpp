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

#include "locator.h"
#include "location_log.h"

namespace OHOS {
namespace Location {
std::shared_ptr<LocatorImpl> Locator::instance_ = nullptr;
std::mutex Locator::mutex_;
Locator::~Locator()
{}

std::shared_ptr<LocatorImpl> Locator::GetInstance()
{
    if (instance_ == nullptr) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (instance_ == nullptr) {
            std::shared_ptr<LocatorImpl> locator = std::make_shared<LocatorImpl>();
            instance_ = locator;
        }
    }
    return instance_;
}
}  // namespace Location
}  // namespace OHOS
