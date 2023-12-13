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

#ifndef PLUGINS_BRIDGE_ERROR_INFO_H
#define PLUGINS_BRIDGE_ERROR_INFO_H

#include <string>

namespace OHOS::Plugin::Bridge {
enum class ErrorCode {
    BRIDGE_ERROR_NO = 0,
    BRIDGE_NAME_ERROR,
    BRIDGE_CREATE_ERROR,
    BRIDGE_INVALID,
    BRIDGE_METHOD_NAME_ERROR,
    BRIDGE_METHOD_RUNNING,
    BRIDGE_METHOD_UNIMPL,
    BRIDGE_METHOD_PARAM_ERROR,
    BRIDGE_METHOD_EXISTS,
    BRIDGE_DATA_ERROR,
    BRIDGE_METHOD_NOT_EXISTS,
    BRIDGE_END
};

static constexpr const char* CodeMessage[static_cast<int>(ErrorCode::BRIDGE_END)] = {
    "Correct!",
    "Bridge name error!",
    "Bridge creation failure!",
    "Bridge unavailable!",
    "Method name error!",
    "Method is running...",
    "Method not implemented!",
    "Method parameter error!",
    "Method already exists!",
    "Data error!",
    "Method is not exists!"
};
} // namespace OHOS::Plugin::Bridge
#endif