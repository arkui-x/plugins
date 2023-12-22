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

#include "request_utils.h"
#include <chrono>
#include "log.h"

namespace OHOS::Plugin::Request {
std::string RequestUtils::GetVersionText(Version version)
{
    switch (version) {
        case Version::API9:
            return "APIv9";

        case Version::API10:
            return "APIv10";

        default:
            break;
    }
    return "unknown_version";
}

std::string RequestUtils::GetEventType(int64_t tid, const std::string &type)
{
    return type + ":" + std::to_string(tid);
}

bool RequestUtils::GetTaskId(const std::string &eventType, int64_t &taskId)
{
    auto pos = eventType.find(':');
    if (pos == std::string::npos) {
        REQUEST_HILOGE("invalid event type, return invalid ");
        return false;
    }
    taskId = atoll(eventType.substr(pos + 1).c_str());
    return true;
}

uint64_t RequestUtils::GetTimeNow()
{
    using namespace std::chrono;
    return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
}
} // namespace OHOS::Plugin::Request