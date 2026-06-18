/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include <cinttypes>
#include <cstdint>
#include <strstream>

#include "log.h"
#include "plugin_utils.h"
#include "nlohmann/json.hpp"
namespace OHOS {
namespace Plugin {
namespace {
const char COMMON_EVENT_BUNDLE_NAME[] = "bundleName";
const char COMMON_EVENT_CODE[] = "code";
const char COMMON_EVENT_PUBLISH_DATA[] = "data";
const char COMMON_EVENT_PARAMETERS[] = "parameters";

bool ParseStringField(const nlohmann::json& jsonData, const char* field, std::string& value)
{
    if (!jsonData.contains(field)) {
        return true;
    }
    if (!jsonData[field].is_string()) {
        LOGE("Field %{public}s is not string.", field);
        return false;
    }
    value = jsonData[field].get<std::string>();
    return true;
}

bool ParseCodeField(const nlohmann::json& jsonData, CommonEventData& eventData)
{
    if (!jsonData.contains(COMMON_EVENT_CODE)) {
        return true;
    }
    if (!jsonData[COMMON_EVENT_CODE].is_number_integer()) {
        LOGE("Field %{public}s is not int.", COMMON_EVENT_CODE);
        return false;
    }
    int64_t rawCode = jsonData[COMMON_EVENT_CODE].get<int64_t>();
    if (rawCode > INT32_MAX) {
        LOGE("Field %{public}s value %{public}" PRId64 " exceeds INT32_MAX, clamp to INT32_MAX.",
            COMMON_EVENT_CODE, rawCode);
        eventData.SetCode(INT32_MAX);
    } else if (rawCode < INT32_MIN) {
        LOGE("Field %{public}s value %{public}" PRId64 " below INT32_MIN, clamp to INT32_MIN.",
            COMMON_EVENT_CODE, rawCode);
        eventData.SetCode(INT32_MIN);
    } else {
        eventData.SetCode(static_cast<int>(rawCode));
    }
    return true;
}

bool ParseParametersField(const nlohmann::json& jsonData, CommonEventData& eventData)
{
    if (!jsonData.contains(COMMON_EVENT_PARAMETERS)) {
        return true;
    }
    if (!jsonData[COMMON_EVENT_PARAMETERS].is_object()) {
        LOGE("Field %{public}s is not object.", COMMON_EVENT_PARAMETERS);
        return false;
    }
    eventData.SetParameters(jsonData[COMMON_EVENT_PARAMETERS].dump());
    return true;
}

bool ParseReceivePayload(const std::string& data, CommonEventData& eventData)
{
    auto jsonData = nlohmann::json::parse(data, nullptr, false);
    if (jsonData.is_discarded() || !jsonData.is_object()) {
        LOGE("Parse failed due to invalid payload.");
        return false;
    }
    std::string bundleName;
    std::string dataValue;
    bool isValidPayload = ParseStringField(jsonData, COMMON_EVENT_BUNDLE_NAME, bundleName) &&
        ParseStringField(jsonData, COMMON_EVENT_PUBLISH_DATA, dataValue) && ParseCodeField(jsonData, eventData) &&
        ParseParametersField(jsonData, eventData);
    if (!isValidPayload) {
        return false;
    }
    eventData.SetBundleName(bundleName);
    eventData.SetData(dataValue);
    return true;
}
} // namespace

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
    if (!data.empty() && !ParseReceivePayload(data, commonEventData)) {
        return;
    }
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