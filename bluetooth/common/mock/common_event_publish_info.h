/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_BLUETOOTH_COMMON_MOCK_COMMON_EVENT_PUBLISH_INFO_H
#define PLUGINS_BLUETOOTH_COMMON_MOCK_COMMON_EVENT_PUBLISH_INFO_H

#include "parcel.h"

namespace OHOS {
namespace EventFwk {
class CommonEventPublishInfo : public Parcelable {
public:
    CommonEventPublishInfo() {}

    ~CommonEventPublishInfo() {}

    void SetSubscriberPermissions(const std::vector<std::string>& subscriberPermissions) {}

    virtual bool Marshalling(Parcel& parcel) const override
    {
        return true;
    }
};
} // namespace EventFwk
} // namespace OHOS

#endif // PLUGINS_BLUETOOTH_COMMON_MOCK_COMMON_EVENT_PUBLISH_INFO_H