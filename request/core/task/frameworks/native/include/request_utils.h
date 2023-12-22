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

#ifndef PLUGINS_REQUEST_UTILS_H
#define PLUGINS_REQUEST_UTILS_H

#include <string>
#include "constant.h"

namespace OHOS::Plugin::Request {
class RequestUtils {
public:
    static std::string GetVersionText(Version version);
    static std::string GetEventType(int64_t taskId, const std::string &type);
    static bool GetTaskId(const std::string &eventType, int64_t &taskId);
    static uint64_t GetTimeNow();
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_UTILS_H