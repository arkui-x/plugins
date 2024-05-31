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

#include "common_event_subscriber.h"

namespace OHOS {
namespace Plugin {
CommonEventSubscriber::CommonEventSubscriber() {}

CommonEventSubscriber::CommonEventSubscriber(const CommonEventSubscribeInfo& subscribeInfo)
    : subscribeInfo_(subscribeInfo)
{}

CommonEventSubscriber::~CommonEventSubscriber() {}

const CommonEventSubscribeInfo& CommonEventSubscriber::GetSubscribeInfo() const
{
    return subscribeInfo_;
}
} // namespace Plugin
} // namespace OHOS