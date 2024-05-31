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

#ifndef PLUGINS_COMMON_EVENT_MANAGER_COMMON_EVENT_DATA_H
#define PLUGINS_COMMON_EVENT_MANAGER_COMMON_EVENT_DATA_H

#include <string>

namespace OHOS {
namespace Plugin {
class CommonEventData {
public:
    CommonEventData() = default;
    CommonEventData(const std::string& event, const std::string& bunduleName, int code, const std::string& data);
    virtual ~CommonEventData() = default;
    std::string GetEvent() const;
    void SetEvent(const std::string& event);
    std::string GetBunduleName() const;
    void SetBunduleName(const std::string& bundleName);
    int GetCode() const;
    void SetCode(int code);
    std::string GetData() const;
    void SetData(const std::string& data);

private:
    std::string event_;
    std::string bunduleName_;
    int code_;
    std::string data_;
};
} // namespace Plugin
} // namespace OHOS
#endif // PLUGINS_COMMON_EVENT_MANAGER_COMMON_EVENT_DATA_H