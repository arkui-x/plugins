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

#include "log.h"
#include "common_event.h"
#include "common_event_manager.h"

namespace OHOS {
namespace Plugin {
void CommonEventManager::PublishCommonEvent(const std::string &event, AsyncCallbackInfo* ptr)
{
    LOGI("PublishCommonEvent called event is %{public}s.", event.c_str());
    CommonEvent::GetInstance()->PublishCommonEvent(event, ptr);
}
void CommonEventManager::SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    LOGI("SubscribeCommonEvent called.");
    CommonEvent::GetInstance()->SubscribeCommonEvent(subscriber);
}
void CommonEventManager::UnSubscribeCommonEvent(
    const std::shared_ptr<CommonEventSubscriber> &subscriber, AsyncCallbackInfo* ptr)
{
    LOGI("UnSubscribeCommonEvent called.");
    CommonEvent::GetInstance()->UnSubscribeCommonEvent(subscriber, ptr);
}
}  // namespace Plugin
}  // namespace OHOS