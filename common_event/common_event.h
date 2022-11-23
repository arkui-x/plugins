/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_COMMONEVENT_COMMON_EVENT_H
#define PLUGINS_COMMONEVENT_COMMON_EVENT_H

#include <memory>
#include <mutex>
#include <map>

#include "common_event_interface.h"
#include "common_event_subscriber.h"

namespace OHOS {
namespace Plugin {
class CommonEvent {
public:
    CommonEvent(const CommonEvent &commonEvent) = delete;
    CommonEvent &operator=(const CommonEvent &commonEvent) = delete;
    ~CommonEvent() = default;
    static CommonEvent* GetInstance()
    {
        if (instance_ == nullptr) {
            instance_ = new CommonEvent();
        }
        return instance_;
    }
    void ReceiveCommonEvent(const std::string &event, const std::string &data);
    void PublishCommonEvent(const std::string &event, OH_Plugin_AsyncCallbackInfo* ptr);
    void SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber> &subscriber);
    void UnSubscribeCommonEvent(
        const std::shared_ptr<CommonEventSubscriber> &subscriber, OH_Plugin_AsyncCallbackInfo* ptr);

private:
    CommonEvent();

private:
    static CommonEvent* instance_;
    std::map<std::shared_ptr<CommonEventSubscriber>, std::string> subscribers_;
    std::unique_ptr<CommonEventInterface> commonEventInterface_;
    std::mutex mutex_;
};
}  // namespace Plugin
}  // namespace OHOS
#endif  // PLUGINS_COMMONEVENT_COMMON_EVENT_H