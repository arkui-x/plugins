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

#include "plugins/interfaces/native/log.h"

[[maybe_unused]] static void StripFormatString(const std::string& prefix, std::string& str)
{
    for (auto pos = str.find(prefix, 0); pos != std::string::npos; pos = str.find(prefix, pos)) {
        str.erase(pos, prefix.size());
    }
}

LOG_EXPORT void LogPrint(LogLevel level, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LogPrint(level, fmt, args);
    va_end(args);
}

#if defined(ANDROID_PLATFORM)
#include <android/log.h>

constexpr int32_t LOG_LEVEL[] = { ANDROID_LOG_DEBUG, ANDROID_LOG_INFO, ANDROID_LOG_WARN, ANDROID_LOG_ERROR,
    ANDROID_LOG_FATAL };

LOG_EXPORT void LogPrint(LogLevel level, const char* fmt, va_list args)
{
    std::string newFmt(fmt);
    StripFormatString("{public}", newFmt);
    StripFormatString("{private}", newFmt);
    __android_log_vprint(LOG_LEVEL[static_cast<int>(level)], PLUGIN_LOG_TAG, newFmt.c_str(), args);
}

#elif defined(IOS_PLATFORM)
#include <securec.h>

constexpr uint32_t MAX_BUFFER_SIZE = 4096;

LOG_EXPORT void LogPrint(LogLevel level, const char* fmt, va_list args)
{
    std::string newFmt(fmt);
    StripFormatString("{public}", newFmt);
    StripFormatString("{private}", newFmt);
    char buf[MAX_BUFFER_SIZE] = { '\0' };
    int ret = vsnprintf_s(buf, sizeof(buf), sizeof(buf) - 1, newFmt.c_str(), args);
    if (ret < 0) {
        return;
    }
    printf("%s\r\n", buf);
    fflush(stdout);
}
#else
LOG_EXPORT void LogPrint(LogLevel level, const char* fmt, va_list args) {}
#endif
