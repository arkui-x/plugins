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

#ifndef I_LOCATOR_CALLBACK_H
#define I_LOCATOR_CALLBACK_H

#include "iremote_broker.h"

#include "location.h"

namespace OHOS {
namespace Location {
class ILocatorCallback : public IRemoteBroker {
public:
    enum {
        RECEIVE_LOCATION_INFO_EVENT_V9 = 1,
        RECEIVE_LOCATION_STATUS_EVENT = 2,
        RECEIVE_ERROR_INFO_EVENT = 3,
        RECEIVE_LOCATION_INFO_EVENT = 4,
    };
    DECLARE_INTERFACE_DESCRIPTOR(u"location.ILocatorCallback");
    virtual void OnLocationReport(const std::unique_ptr<Location>& location) = 0;
    virtual void OnLocatingStatusChange(const int status) = 0;
    virtual void OnErrorReport(const int errorCode) = 0;
};
} // namespace Location
} // namespace OHOS
#endif // I_LOCATOR_CALLBACK_H
