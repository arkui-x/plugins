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

#ifndef PLUGINS_PASTEBOARD_MOCK_APP_EVENT_H
#define PLUGINS_PASTEBOARD_MOCK_APP_EVENT_H

#include <cstdint>
#include <string>
#include <vector>

namespace OHOS {
namespace HiviewDFX {
class AppEventPack;
namespace HiAppEvent {

struct ReportConfig {
    std::string name;
    std::string configName;
};

class Event {
public:
    Event(const std::string& /*domain*/, const std::string& /*name*/, EventType /*type*/) {}

    template<typename T>
    void AddParam(const std::string& /*key*/, const T& /*value*/)
    {}
};

class AppEventProcessorMgr {
public:
    static int64_t AddProcessor(const ReportConfig& /*config*/)
    {
        return -1;
    }
};
} // namespace HiAppEvent
} // namespace HiviewDFX
} // namespace OHOS
#endif // PLUGINS_PASTEBOARD_MOCK_APP_EVENT_H
