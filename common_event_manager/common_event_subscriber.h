/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_COMMON_EVENT_MANAGER_COMMON_EVENT_SUBSCRIBER_H
#define PLUGINS_COMMON_EVENT_MANAGER_COMMON_EVENT_SUBSCRIBER_H

#include "common_event_data.h"
#include "common_event_subscribe_info.h"

namespace OHOS {
namespace Plugin {
class CommonEventSubscriber {
public:
    CommonEventSubscriber();
    explicit CommonEventSubscriber(const CommonEventSubscribeInfo& subscribeInfo);
    virtual ~CommonEventSubscriber();
    virtual void OnReceiveEvent(const CommonEventData& data) = 0;
    const CommonEventSubscribeInfo& GetSubscribeInfo() const;

private:
    CommonEventSubscribeInfo subscribeInfo_;
};
} // namespace Plugin
} // namespace OHOS
#endif // PLUGINS_COMMON_EVENT_MANAGER_COMMON_EVENT_SUBSCRIBER_H