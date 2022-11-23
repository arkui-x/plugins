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

#include "common_event.h"

#include <strstream>

#include "log.h"
#include "plugin_c_utils.h"
#include "plugin_utils.h"

namespace OHOS {
namespace Plugin {
CommonEvent* CommonEvent::instance_ = nullptr;
CommonEvent::CommonEvent()
{
    subscribers_.clear();
    LOGI("CommonEventPluginJniRegister");
    commonEventInterface_ = CommonEventInterface::Create();
}

void CommonEvent::PublishCommonEvent(const std::string &event, OH_Plugin_AsyncCallbackInfo* ptr)
{
    LOGI("PublishCommonEvent called.");
    commonEventInterface_->PublishCommonEvent(event, ptr);
}

void CommonEvent::SubscribeCommonEvent(
    const std::shared_ptr<CommonEventSubscriber> &subscriber)
{
    LOGI("SubscribeCommonEvent called.");
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (subscribers_.find(subscriber) != subscribers_.end()) {
            LOGE("ERROR");
            return;
        }
    }
    std::string key;
    std::strstream ss;
    ss << &subscriber;
    ss >> key;
    std::vector<std::string> events = subscriber->GetSubscribeInfo().GetEvents();
    for (auto it = events.begin(); it != events.end(); it++) {
        key += *it;
    }
    LOGI("subscriber key is %{public}s", key.c_str());
    {
        std::lock_guard<std::mutex> lock(mutex_);
        subscribers_[subscriber] = key;
    }
    commonEventInterface_->SubscribeCommonEvent(key, events);
}

void CommonEvent::UnSubscribeCommonEvent(
    const std::shared_ptr<CommonEventSubscriber> &subscriber, OH_Plugin_AsyncCallbackInfo* ptr)
{
    LOGI("UnSubscribeCommonEvent called.");
    std::string key;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto iter = subscribers_.find(subscriber);
        if (iter == subscribers_.end()) {
            LOGE("ERROR");
            return;
        }
        key = iter->second;
        subscribers_.erase(iter);
    }
    commonEventInterface_->UnSubscribeCommonEvent(key, ptr);
}

void CommonEvent::ReceiveCommonEvent(const std::string &event, const std::string &data)
{
    LOGI("ReceiveCommonEvent called.");
    CommonEventData commonEventData;
    commonEventData.SetEvent(event);
    commonEventData.SetData(data);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto it = subscribers_.begin(); it != subscribers_.end(); it++) {
            if (it->second.find(event)) {
                LOGI("Start RunTaskOnUI");
                it->first->OnReceiveEvent(commonEventData);
            }
        }
    }
}
}  // namespace Plugin
}  // namespace OHOS