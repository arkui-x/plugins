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

#ifndef PLUGINS_BLUETOOTH_COMMON_MOCK_COMMON_EVENT_MANAGER_H
#define PLUGINS_BLUETOOTH_COMMON_MOCK_COMMON_EVENT_MANAGER_H

#include "common_event_data.h"
#include "common_event_publish_info.h"

namespace OHOS {
namespace EventFwk {
class CommonEventManager {
public:
    static bool PublishCommonEvent(const CommonEventData& data, const CommonEventPublishInfo& publishInfo)
    {
        return false;
    }
};
} // namespace EventFwk
} // namespace OHOS

#endif // PLUGINS_BLUETOOTH_COMMON_MOCK_COMMON_EVENT_MANAGER_H
