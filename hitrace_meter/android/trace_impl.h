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

#ifndef PLUGINS_HITRACEMETER_ANDROID_TRACE_IMPL_H
#define PLUGINS_HITRACEMETER_ANDROID_TRACE_IMPL_H

#include <memory>

#include "plugins/hitrace_meter/trace.h"

namespace OHOS::Plugin {
class TraceImpl final : public Trace {
public:
    TraceImpl() = default;
    ~TraceImpl() override = default;

    void AsyncTraceBegin(int32_t taskId, const char* name) override;
    void AsyncTraceEnd(int32_t taskId, const char* name) override;
};
} // namespace OHOS::Plugin
#endif // PLUGINS_HITRACEMETER_ANDROID_TRACE_IMPL_H
