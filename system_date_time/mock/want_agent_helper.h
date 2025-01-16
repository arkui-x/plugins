/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef PLUGIN_TIME_MOCK_WANT_AGENT_HELPER_H
#define PLUGIN_TIME_MOCK_WANT_AGENT_HELPER_H

#include <string>
#include <memory>
#include <set>

#include "want_agent.h"

namespace OHOS::AbilityRuntime::WantAgent {
class WantAgentHelper final : public std::enable_shared_from_this<WantAgentHelper> {
private:
    WantAgentHelper();
    virtual ~WantAgentHelper() = default;
};
}  // namespace OHOS::AbilityRuntime::WantAgent
#endif  // PLUGIN_TIME_MOCK_WANT_AGENT_HELPER_H
