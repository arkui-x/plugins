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

#ifndef PLUGINS_COMMONEVENT_COMMON_EVENT_INTERFACE_H
#define PLUGINS_COMMONEVENT_COMMON_EVENT_INTERFACE_H

#include <string>
#include <memory>

#include "plugin_c_utils.h"

namespace OHOS {
namespace Plugin {
class CommonEventInterface {
public:
    CommonEventInterface() = default;
    virtual ~CommonEventInterface() = default;

    static std::unique_ptr<CommonEventInterface> Create();

    virtual void PublishCommonEvent(const std::string &event, OH_Plugin_AsyncCallbackInfo* ptr) = 0;
    virtual void SubscribeCommonEvent(
        const std::string &key, const std::vector<std::string> events) = 0;
    virtual void UnSubscribeCommonEvent(const std::string &key, OH_Plugin_AsyncCallbackInfo* ptr) = 0;
};
}  // namespace Plugin
}  // namespace OHOS
#endif  // PLUGINS_COMMONEVENT_COMMON_EVENT_INTERFACE_H