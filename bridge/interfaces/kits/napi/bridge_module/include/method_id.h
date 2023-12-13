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

#ifndef PLUGINS_BRIDGE_METHOD_ID_H
#define PLUGINS_BRIDGE_METHOD_ID_H

#include <atomic>
#include <string>

namespace OHOS::Plugin::Bridge {
class MethodID final {
public:
    MethodID() = default;
    ~MethodID() = default;
    static std::string MakeMethodNameID(const std::string& methodName);
    static std::string FetchMethodName(const std::string& methodNameWithID);

private:
    static std::atomic<uint32_t> id_;
    static uint32_t GetNextID(void);
};
} // namespace OHOS::Plugin::Bridge
#endif