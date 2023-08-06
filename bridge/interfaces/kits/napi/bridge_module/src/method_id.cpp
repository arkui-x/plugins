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

#include "plugins/bridge/interfaces/kits/napi/bridge_module/include/method_id.h"

namespace OHOS::Plugin::Bridge {
std::atomic<uint32_t> MethodID::id_(0);
static constexpr const uint32_t MDETHOD_ID_MAX = 0xffffffff - 1;
static constexpr const char* METHOD_ID_SEP = "$";

uint32_t MethodID::GetNextID(void)
{
    uint32_t id = id_;
    id_++;
    if (id_ > MDETHOD_ID_MAX) {
        id_ = 0;
    }
    return id;
}

std::string MethodID::MakeMethodNameID(const std::string& methodName)
{
    uint32_t methodID = GetNextID();
    std::string methodNameWithID(methodName);
    methodNameWithID.append(METHOD_ID_SEP);
    methodNameWithID.append(std::to_string(methodID));
    return methodNameWithID;
}

std::string MethodID::FetchMethodName(const std::string& methodNameWithID)
{
    std::string methodName;
    int pos = methodNameWithID.find(METHOD_ID_SEP);
    if (pos > 0) {
        methodName = methodNameWithID.substr(0, pos);
    } else {
        methodName = methodNameWithID;
    }
    return methodName;
}
} // namespace OHOS::Plugin::Bridge
