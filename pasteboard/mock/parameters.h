/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

#ifndef PLUGIN_PASTEBOARD_MOCK_PARAMETERS_H
#define PLUGIN_PASTEBOARD_MOCK_PARAMETERS_H

#include <limits>
#include <string>

namespace OHOS {
namespace system {

constexpr size_t MIN_RAW_SIZE = 32 * 1024; // 32k

static int64_t GetIntParameter(const std::string& parameterName, int64_t def)
{
    return MIN_RAW_SIZE;
}
} // namespace system
} // namespace OHOS

#endif // PLUGIN_PASTEBOARD_MOCK_PARAMETERS_H