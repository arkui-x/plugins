/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_COMMONEVENT_COMMON_EVENT_MANAGER_H
#define PLUGINS_COMMONEVENT_COMMON_EVENT_MANAGER_H

#include <memory>

#include "common_event_subscriber.h"
#include "inner_api/plugin_utils_napi.h"
#include "plugin_utils.h"

namespace OHOS {
namespace Plugin {
class CommonEventManager {
public:
    static void PublishCommonEvent(const std::string &event, AsyncCallbackInfo* ptr);
    static void SubscribeCommonEvent(
        const std::shared_ptr<CommonEventSubscriber> &subscriber);
    static void UnSubscribeCommonEvent(
        const std::shared_ptr<CommonEventSubscriber> &subscriber, AsyncCallbackInfo* ptr);
};
}  // namespace Plugin
}  // namespace OHOS

#endif  // PLUGINS_COMMONEVENT_COMMON_EVENT_MANAGER_H