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

#ifndef I_GEOCODE_CALLBACK_H
#define I_GEOCODE_CALLBACK_H

#include <list>
#include <string>

#include "iremote_broker.h"
#include "geo_address.h"

namespace OHOS {
namespace Location {
class IGeocodeCallback : public IRemoteBroker {
public:
    enum {
        RECEIVE_GEOCODE_INFO_EVENT = 1,
        ERROR_INFO_EVENT = 2,
    };
    DECLARE_INTERFACE_DESCRIPTOR(u"location.IGeocodeCallback");
    virtual void OnResults(std::list<std::shared_ptr<GeoAddress>> &results) = 0;
    virtual void OnErrorReport(const int errorCode) = 0;
};
} // namespace Location
} // namespace OHOS
#endif // I_GEOCODE_CALLBACK_H
