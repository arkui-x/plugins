/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#ifndef MOCK_UTILS_LOG_H
#define MOCK_UTILS_LOG_H
#include <android/log.h>
#define HILOG_INFO(...)  __android_log_print(ANDROID_LOG_INFO, "ArkUI-X-Test", __VA_ARGS__)
#define HILOG_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, "ArkUI-X-Test", __VA_ARGS__)
#define HILOG_DEBUG(...) __android_log_print(ANDROID_LOG_DEBUG, "ArkUI-X-Test", __VA_ARGS__)
#define HILOG_WARN(...)  __android_log_print(ANDROID_LOG_WARN, "ArkUI-X-Test", __VA_ARGS__)
#define HILOG_FATAL(...) __android_log_print(ANDROID_LOG_FATAL, "ArkUI-X-Test", __VA_ARGS__)
#endif
