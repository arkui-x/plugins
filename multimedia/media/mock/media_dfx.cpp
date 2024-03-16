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

#include <media_dfx.h>
#include <unistd.h>
#include "securec.h"
#include "media_log.h"
#include "media_errors.h"
#include "hitrace_meter.h"
#include "ipc_skeleton.h"
#include "media_utils.h"

namespace {
    constexpr uint32_t MAX_STRING_SIZE = 256;
}

namespace OHOS {
namespace Media {
bool MediaEvent::CreateMsg(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char msg[MAX_STRING_SIZE] = {0};
    auto ret = vsnprintf_s(msg, sizeof(msg), sizeof(msg) - 1, format, args);
    va_end(args);
    msg_ = msg;
    return ret < 0 ? false : true;
}

void MediaEvent::EventWrite(std::string eventName, OHOS::HiviewDFX::HiSysEvent::EventType type,
    std::string module)
{
    int32_t pid = getpid();
    uint32_t uid = getuid();
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::MULTI_MEDIA, eventName, type,
        "PID", pid,
        "UID", uid,
        "MODULE", module,
        "MSG", msg_);
}

void MediaEvent::EventWriteWithAppInfo(std::string eventName, OHOS::HiviewDFX::HiSysEvent::EventType type,
    std::string module, std::string status, int32_t appUid, int32_t appPid)
{
    int32_t pid = getpid();
    uint32_t uid = getuid();
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::MULTI_MEDIA, eventName, type,
        "PID", pid,
        "UID", uid,
        "MODULE", module,
        "MSG", msg_,
        "APP_PID", appPid,
        "APP_UID", appUid,
        "STATUS", status);
}

void MediaEvent::EventWriteBundleName(std::string eventName, OHOS::HiviewDFX::HiSysEvent::EventType type,
    std::string module, std::string status, int32_t appUid, int32_t appPid, std::string bundleName)
{
    int32_t pid = getpid();
    uint32_t uid = getuid();
    HiSysEventWrite(OHOS::HiviewDFX::HiSysEvent::Domain::MULTI_MEDIA, eventName, type,
                    "PID", pid,
                    "UID", uid,
                    "MODULE", module,
                    "MSG", msg_,
                    "APP_PID", appPid,
                    "APP_UID", appUid,
                    "STATUS", status,
                    "BUNDLE", bundleName);
}

MediaTrace::MediaTrace(const std::string &funcName)
{
    isSync_ = true;
}

void MediaTrace::TraceBegin(const std::string &funcName, int32_t taskId)
{
}

void MediaTrace::TraceEnd(const std::string &funcName, int32_t taskId)
{
}

void MediaTrace::CounterTrace(const std::string &varName, int32_t val)
{
}

MediaTrace::~MediaTrace()
{
}
} // namespace Media
} // namespace OHOS
