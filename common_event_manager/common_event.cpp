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

#include "common_event.h"

#include <strstream>

#include "log.h"
#include "plugin_utils.h"
#include "nlohmann/json.hpp"
namespace OHOS {
namespace Plugin {
static const std::string COMMON_EVENT_PUBLISH_DATA = "data";
CommonEvent::CommonEvent()
{
    subscribers_.clear();
    commonEventInterface_ = CommonEventInterface::Create();
}

void CommonEvent::PublishCommonEvent(
    const std::string& event, const std::string& commonEventPublishData, AsyncCallbackInfo* ptr)
{
    LOGI("Called.");
    commonEventInterface_->PublishCommonEvent(event, commonEventPublishData, ptr);
}

void CommonEvent::SubscribeCommonEvent(const std::shared_ptr<CommonEventSubscriber>& subscriber)
{
    LOGI("Called.");
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (subscribers_.find(subscriber) != subscribers_.end()) {
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
    LOGI("Subscriber key is %s", key.c_str());
    {
        std::lock_guard<std::mutex> lock(mutex_);
        subscribers_[subscriber] = key;
    }
    commonEventInterface_->SubscribeCommonEvent(key, events);
}

void CommonEvent::UnSubscribeCommonEvent(
    const std::shared_ptr<CommonEventSubscriber>& subscriber, AsyncCallbackInfo* ptr)
{
    LOGI("Called.");
    std::string key;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto iter = subscribers_.find(subscriber);
        if (iter == subscribers_.end()) {
            return;
        }
        key = iter->second;
        subscribers_.erase(iter);
    }
    commonEventInterface_->UnSubscribeCommonEvent(key, ptr);
}

void CommonEvent::ReceiveCommonEvent(const std::string& key, const std::string& event, const std::string& data)
{
    LOGI("Called.");
    CommonEventData commonEventData;
    commonEventData.SetEvent(event);
    std::string dataValue = "";
    if (!data.empty()) {
        nlohmann::json jsonData = nlohmann::json::parse(data);
        if (jsonData.is_discarded()) {
            LOGE("Parse failed due to data is discarded.");
            return;
        }
        if (jsonData.contains(COMMON_EVENT_PUBLISH_DATA)) {
            dataValue = jsonData[COMMON_EVENT_PUBLISH_DATA];
        }
    }
    commonEventData.SetData(dataValue);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto it = subscribers_.begin(); it != subscribers_.end(); it++) {
            if (it->second == key) {
                LOGI("On receive event, key is %{public}s", key.c_str());
                if (it->first == nullptr) {
                    LOGE("CommonEventSubscriber is null.");
                    return;
                }
                it->first->OnReceiveEvent(commonEventData);
            }
        }
    }
}
} // namespace Plugin
} // namespace OHOS