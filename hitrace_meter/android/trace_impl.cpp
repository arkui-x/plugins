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

#include "trace_impl.h"
#include <android/trace.h>

#include "log.h"

namespace OHOS::Plugin {
std::unique_ptr<Trace> Trace::Create()
{
    return std::make_unique<TraceImpl>();
}

void TraceImpl::AsyncTraceBegin(int32_t taskId, const char* name)
{
#if defined(__ANDROID_API__) && __ANDROID_API__ >= 29
    ATrace_beginAsyncSection(name, taskId);
#else defined(__ANDROID_API__) && __ANDROID_API__>=23 && __ANDROID_API__ < 29
    ATrace_beginSection(name);
#endif
}

void TraceImpl::AsyncTraceEnd(int32_t taskId, const char* name)
{
#if defined(__ANDROID_API__) && __ANDROID_API__ >= 29
    ATrace_endAsyncSection(name, taskId);
#else defined(__ANDROID_API__) && __ANDROID_API__>=23 && __ANDROID_API__ < 29
    ATrace_endSection();
#endif
}
} // namespace OHOS::Ace