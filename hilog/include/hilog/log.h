/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_HILOG_LOG_H
#define PLUGINS_HILOG_LOG_H

#include <cstdint>
#include <string>
#include "native/log.h"

#define HILOG_NUM_TWO 2
#define HILOG_NUM_THREE 3

typedef enum {
    LOG_TYPE_MIN = 0,
    LOG_APP = 0,
    LOG_INIT = 1,
    // Used by core service, framework.
    LOG_CORE = 3,
    LOG_KMSG = 4,
    LOG_TYPE_MAX
} LogType;

namespace OHOS {
namespace HiviewDFX {
using HiLogLabel = struct {
    LogType type;
    unsigned int domain;
    const char *tag;
};

static constexpr HiLogLabel PLUGIN_HILOG_LABEL = { LOG_CORE, 0, "hilog" };
class HiLog final {
public:
    static void Debug(const HiLogLabel &label, const char *fmt, ...)
        __attribute__((__format__(os_log, HILOG_NUM_TWO, HILOG_NUM_THREE)))
    {
        va_list args;
        va_start(args, fmt);
        LogPrint(LogLevel::Debug, fmt, args);
        va_end(args);
    };

    static void Info(const HiLogLabel &label, const char *fmt, ...)
        __attribute__((__format__(os_log, HILOG_NUM_TWO, HILOG_NUM_THREE)))
    {
        va_list args;
        va_start(args, fmt);
        LogPrint(LogLevel::Info, fmt, args);
        va_end(args);
    };
    static void Warn(const HiLogLabel &label, const char *fmt, ...)
        __attribute__((__format__(os_log, HILOG_NUM_TWO, HILOG_NUM_THREE)))
    {
        va_list args;
        va_start(args, fmt);
        LogPrint(LogLevel::Warn, fmt, args);
        va_end(args);
    };

    static void Error(const HiLogLabel &label, const char *fmt, ...)
        __attribute__((__format__(os_log, HILOG_NUM_TWO, HILOG_NUM_THREE)))
    {
        va_list args;
        va_start(args, fmt);
        LogPrint(LogLevel::Error, fmt, args);
        va_end(args);
    };

    static void Fatal(const HiLogLabel &label, const char *fmt, ...)
        __attribute__((__format__(os_log, HILOG_NUM_TWO, HILOG_NUM_THREE)))
    {
        va_list args;
        va_start(args, fmt);
        LogPrint(LogLevel::Fatal, fmt, args);
        va_end(args);
    };

    static void Level(const HiLogLabel &label, const char *fmt, ...)
        __attribute__((__format__(os_log, HILOG_NUM_TWO, HILOG_NUM_THREE)))
    {
        va_list args;
        va_start(args, fmt);
        LogPrint(LogLevel::Fatal, fmt, args);
        va_end(args);
    };
};
}   /* HiviewDFX */
}   /* OHOS */
#endif /* PLUGINS_HILOG_LOG_H */
