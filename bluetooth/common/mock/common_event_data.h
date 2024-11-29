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

#ifndef PLUGINS_BLUETOOTH_COMMON_MOCK_COMMON_EVENT_DATA_H
#define PLUGINS_BLUETOOTH_COMMON_MOCK_COMMON_EVENT_DATA_H

#include "parcel.h"
#include "want.h"

namespace OHOS {
namespace EventFwk {
using Want = OHOS::AAFwk::Want;

class CommonEventData : public Parcelable {
public:
    CommonEventData() {}

    ~CommonEventData() {}

    void SetWant(const Want& want) {}

    virtual bool Marshalling(Parcel& parcel) const override
    {
        return true;
    }
};
} // namespace EventFwk
} // namespace OHOS

#endif // PLUGINS_BLUETOOTH_COMMON_MOCK_COMMON_EVENT_DATA_H