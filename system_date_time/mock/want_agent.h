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

#ifndef PLUGIN_TIME_MOCK_WANT_AGENT_H
#define PLUGIN_TIME_MOCK_WANT_AGENT_H

#include <string>
#include <memory>

namespace OHOS::AbilityRuntime::WantAgent {
class WantAgent{
public:
    WantAgent() {};
    virtual ~WantAgent() = default;
};
}  // namespace OHOS::AbilityRuntime::WantAgent
#endif  // PLUGIN_TIME_MOCK_WANT_AGENT_H
