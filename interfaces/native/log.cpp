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

#include "base/log/log.h"

LOG_EXPORT void LogPrint(LogLevel level, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    LogPrint(level, fmt, args);
    va_end(args);
}

LOG_EXPORT void LogPrint(LogLevel level, const char* fmt, va_list args)
{
    OHOS::Ace::LogLevel aceLevel;
    switch (level) {
        case LogLevel::Debug:
            aceLevel = OHOS::Ace::LogLevel::DEBUG;
            break;
        case LogLevel::Info:
            aceLevel = OHOS::Ace::LogLevel::INFO;
            break;
        case LogLevel::Warn:
            aceLevel = OHOS::Ace::LogLevel::WARN;
            break;
        case LogLevel::Error:
            aceLevel = OHOS::Ace::LogLevel::ERROR;
            break;
        case LogLevel::Fatal:
            aceLevel = OHOS::Ace::LogLevel::FATAL;
            break;
        default:
            aceLevel = OHOS::Ace::LogLevel::DEBUG;
            break;
    }
    OHOS::Ace::LogWrapper::PrintLog(OHOS::Ace::LogDomain::JS_APP, aceLevel, OHOS::Ace::AceLogTag::ACE_DEFAULT_DOMAIN, fmt, args);
}