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

#ifndef PLUGINS_COMMON_EVENT_MANAGER_COMMON_EVENT_SUBSCRIBE_INFO_H
#define PLUGINS_COMMON_EVENT_MANAGER_COMMON_EVENT_SUBSCRIBE_INFO_H
#include <string>
#include <vector>

namespace OHOS {
namespace Plugin {
class CommonEventSubscribeInfo {
public:
    CommonEventSubscribeInfo();
    explicit CommonEventSubscribeInfo(const std::vector<std::string>& events);
    explicit CommonEventSubscribeInfo(const CommonEventSubscribeInfo& commonEventSubscribeInfo);
    CommonEventSubscribeInfo& operator=(const CommonEventSubscribeInfo& commonEventSubscribeInfo);
    ~CommonEventSubscribeInfo();
    void SetEvents(const std::vector<std::string>& events);
    std::vector<std::string> GetEvents() const;
    void SetPublisherPermission(const std::string& publisherPermission);
    std::string GetPublisherPermission() const;
    void SetPublisherDeviceId(const std::string& publisherDeviceId);
    std::string GetPPublisherDeviceId() const;
    void SetUserId(int32_t userId);
    int32_t GetUserId() const;
    void SetPriority(int32_t priority);
    int32_t GetPriority() const;

private:
    std::vector<std::string> events_;
    std::string publisherPermission_;
    std::string publisherDeviceId_;
    int32_t userId_;
    int32_t priority_;
};
} // namespace Plugin
} // namespace OHOS
#endif // PLUGINS_COMMON_EVENT_MANAGER_COMMON_EVENT_SUBSCRIBE_INFO_H