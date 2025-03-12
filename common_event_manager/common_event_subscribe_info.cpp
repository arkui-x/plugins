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

#include "common_event_subscribe_info.h"
namespace OHOS {
namespace Plugin {
CommonEventSubscribeInfo::CommonEventSubscribeInfo() : userId_(-1), priority_(-1)
{
    publisherPermission_.clear();
    publisherDeviceId_.clear();
    events_.clear();
}

CommonEventSubscribeInfo::CommonEventSubscribeInfo(const std::vector<std::string>& events) : userId_(-1), priority_(-1)
{
    publisherDeviceId_.clear();
    publisherDeviceId_.clear();
    for (auto it = events.begin(); it != events.end(); it++) {
        events_.push_back(*it);
    }
}

CommonEventSubscribeInfo::CommonEventSubscribeInfo(const CommonEventSubscribeInfo& commonEventSubscribeInfo) :
publisherPermission_(commonEventSubscribeInfo.GetPublisherPermission()),
      publisherDeviceId_(commonEventSubscribeInfo.GetPPublisherDeviceId()),
      userId_(commonEventSubscribeInfo.GetUserId()), priority_(commonEventSubscribeInfo.GetPriority())
{
    std::vector<std::string> events = commonEventSubscribeInfo.GetEvents();
    for (auto it = events.begin(); it != events.end(); it++) {
        events_.push_back(*it);
}
}

CommonEventSubscribeInfo& CommonEventSubscribeInfo::operator=(const CommonEventSubscribeInfo& commonEventSubscribeInfo)
{
    this->events_ = commonEventSubscribeInfo.GetEvents();
    this->publisherPermission_ = commonEventSubscribeInfo.GetPublisherPermission();
    this->publisherDeviceId_ = commonEventSubscribeInfo.GetPPublisherDeviceId();
    this->userId_ = commonEventSubscribeInfo.GetUserId();
    this->priority_ = commonEventSubscribeInfo.GetPriority();
    return *this;
}

CommonEventSubscribeInfo::~CommonEventSubscribeInfo() {}

void CommonEventSubscribeInfo::SetEvents(const std::vector<std::string>& events)
{
    events_.assign(events.begin(), events.end());
}

std::vector<std::string> CommonEventSubscribeInfo::GetEvents() const
{
    return events_;
}

void CommonEventSubscribeInfo::SetPublisherPermission(const std::string& publisherPermission)
{
    publisherPermission_ = publisherPermission;
}

std::string CommonEventSubscribeInfo::GetPublisherPermission() const
{
    return publisherPermission_;
}

void CommonEventSubscribeInfo::SetPublisherDeviceId(const std::string& publisherDeviceId)
{
    publisherDeviceId_ = publisherDeviceId;
}

std::string CommonEventSubscribeInfo::GetPPublisherDeviceId() const
{
    return publisherDeviceId_;
}

void CommonEventSubscribeInfo::SetUserId(int32_t userId)
{
    userId_ = userId;
}

int32_t CommonEventSubscribeInfo::GetUserId() const
{
    return userId_;
}

void CommonEventSubscribeInfo::SetPriority(int32_t priority)
{
    priority_ = priority;
}

int32_t CommonEventSubscribeInfo::GetPriority() const
{
    return priority_;
}
} // namespace Plugin
} // namespace OHOS