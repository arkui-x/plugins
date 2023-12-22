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

#ifndef PLUGINS_REQUEST_LOG_H
#define PLUGINS_REQUEST_LOG_H

#ifdef IOS_PLATFORM
    #include "plugins/interfaces/native/log.h"
#endif

#ifdef ANDROID_PLATFORM
#define CONFIG_REQUEST_LOG
#endif

#ifdef CONFIG_REQUEST_LOG
#include "hilog/log.h"

static constexpr OHOS::HiviewDFX::HiLogLabel REQUEST_LOG_LABEL = {LOG_CORE, REQUEST_LOG_DOMAIN, REQUEST_LOG_TAG};

#define MAKE_FILE_NAME (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define REQUEST_HILOGF(fmt, ...)                                                                       \
    (void)OHOS::HiviewDFX::HiLog::Fatal(REQUEST_LOG_LABEL, "[%{public}s %{public}s %{public}d] " fmt,  \
    MAKE_FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define REQUEST_HILOGE(fmt, ...)                                                                       \
    (void)OHOS::HiviewDFX::HiLog::Error(REQUEST_LOG_LABEL, "[%{public}s %{public}s %{public}d] " fmt,  \
    MAKE_FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define REQUEST_HILOGW(fmt, ...)                                                                       \
    (void)OHOS::HiviewDFX::HiLog::Warn(REQUEST_LOG_LABEL, "[%{public}s %{public}s %{public}d] " fmt,   \
    MAKE_FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define REQUEST_HILOGD(fmt, ...)                                                                       \
    (void)OHOS::HiviewDFX::HiLog::Debug(REQUEST_LOG_LABEL, "[%{public}s %{public}s %{public}d] " fmt,  \
    MAKE_FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define REQUEST_HILOGI(fmt, ...)                                                                       \
    (void)OHOS::HiviewDFX::HiLog::Info(REQUEST_LOG_LABEL, "[%{public}s %{public}s %{public}d] " fmt,   \
    MAKE_FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#else

#define REQUEST_HILOGF(fmt, ...) LOGF(fmt, ##__VA_ARGS__)
#define REQUEST_HILOGE(fmt, ...) LOGE(fmt, ##__VA_ARGS__)
#define REQUEST_HILOGW(fmt, ...) LOGW(fmt, ##__VA_ARGS__)
#define REQUEST_HILOGD(fmt, ...) LOGD(fmt, ##__VA_ARGS__)
#define REQUEST_HILOGI(fmt, ...) LOGI(fmt, ##__VA_ARGS__)
#endif // CONFIG_REQUEST_LOG

#endif // PLUGINS_REQUEST_LOG_H