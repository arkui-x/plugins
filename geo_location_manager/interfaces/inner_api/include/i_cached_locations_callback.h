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

#ifndef I_CACHED_LOCATIONS_CALLBACK_H
#define I_CACHED_LOCATIONS_CALLBACK_H

#include <vector>
#include "iremote_broker.h"
#include "location.h"

namespace OHOS {
namespace Location {
class ICachedLocationsCallback : public IRemoteBroker {
public:
    enum {
        RECEIVE_CACHED_LOCATIONS_EVENT = 1,
    };
    DECLARE_INTERFACE_DESCRIPTOR(u"location.ICachedLocationsCallback");
    virtual void OnCacheLocationsReport(const std::vector<std::unique_ptr<Location>>& locations) = 0;
};
} // namespace Location
} // namespace OHOS
#endif // I_CACHED_LOCATIONS_CALLBACK_H
