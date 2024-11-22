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

#ifndef PLUGIN_INTERFACE_NATIVE_LOG_H
#define PLUGIN_INTERFACE_NATIVE_LOG_H

#include <cstdint>
#include <string>

#define __FILENAME__ strrchr(__FILE__, '/') + 1

#ifndef LOG_EXPORT
#ifdef WINDOWS_PLATFORM
#define LOG_EXPORT __declspec(dllexport)
#else
#define LOG_EXPORT __attribute__((visibility("default")))
#endif
#endif

enum class LogLevel : uint32_t {
    Debug = 0,
    Info,
    Warn,
    Error,
    Fatal,
};

LOG_EXPORT void LogPrint(LogLevel level, const char* fmt, ...);
LOG_EXPORT void LogPrint(LogLevel level, const char* fmt, va_list args);

#define LOG_PRINT(Level, fmt, ...) \
    LogPrint(LogLevel::Level, "[%-20s(%s)] " fmt, __FILENAME__, __FUNCTION__, ##__VA_ARGS__)

#define LOGF(fmt, ...) LOG_PRINT(Fatal, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) LOG_PRINT(Error, fmt, ##__VA_ARGS__)
#define LOGW(fmt, ...) LOG_PRINT(Warn, fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...) LOG_PRINT(Info, fmt, ##__VA_ARGS__)
#define LOGD(fmt, ...) LOG_PRINT(Debug, fmt, ##__VA_ARGS__)

#endif // PLUGIN_INTERFACE_NATIVE_LOG_H
