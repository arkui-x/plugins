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

#ifndef DOWNLOAD_LOG
#define DOWNLOAD_LOG

#ifdef ANDROID_PLATFORM
#define CONFIG_DOWNLOAD_LOG
#endif

#ifdef CONFIG_DOWNLOAD_LOG
#include "hilog/log.h"

#ifdef DOWNLOAD_HILOGF
#undef DOWNLOAD_HILOGF
#endif

#ifdef DOWNLOAD_HILOGE
#undef DOWNLOAD_HILOGE
#endif

#ifdef DOWNLOAD_HILOGW
#undef DOWNLOAD_HILOGW
#endif

#ifdef DOWNLOAD_HILOGD
#undef DOWNLOAD_HILOGD
#endif

#ifdef DOWNLOAD_HILOGI
#undef DOWNLOAD_HILOGI
#endif

#define DOWNLOAD_LOG_TAG "Downloadkit"
#define DOWNLOAD_LOG_DOMAIN 0xD001C00
static constexpr OHOS::HiviewDFX::HiLogLabel DOWNLOAD_LOG_LABEL = {LOG_CORE, DOWNLOAD_LOG_DOMAIN, DOWNLOAD_LOG_TAG};

#define MAKE_FILE_NAME (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)

#define DOWNLOAD_HILOGF(fmt, ...)                                        								\
    (void)OHOS::HiviewDFX::HiLog::Fatal(DOWNLOAD_LOG_LABEL, "[%{public}s %{public}s %{public}d] " fmt,	\
    MAKE_FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define DOWNLOAD_HILOGE(fmt, ...)                                      									\
    (void)OHOS::HiviewDFX::HiLog::Error(DOWNLOAD_LOG_LABEL, "[%{public}s %{public}s %{public}d] " fmt,	\
    MAKE_FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define DOWNLOAD_HILOGW(fmt, ...)                                                        				\
    (void)OHOS::HiviewDFX::HiLog::Warn(DOWNLOAD_LOG_LABEL, "[%{public}s %{public}s %{public}d] " fmt,	\
    MAKE_FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define DOWNLOAD_HILOGD(fmt, ...)                                                            			\
    (void)OHOS::HiviewDFX::HiLog::Debug(DOWNLOAD_LOG_LABEL, "[%{public}s %{public}s %{public}d] " fmt,	\
    MAKE_FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#define DOWNLOAD_HILOGI(fmt, ...)                                                     					\
    (void)OHOS::HiviewDFX::HiLog::Info(DOWNLOAD_LOG_LABEL, "[%{public}s %{public}s %{public}d] " fmt,	\
    MAKE_FILE_NAME, __FUNCTION__, __LINE__, ##__VA_ARGS__)

#else

#define DOWNLOAD_HILOGF(fmt, ...)
#define DOWNLOAD_HILOGE(fmt, ...)
#define DOWNLOAD_HILOGW(fmt, ...)
#define DOWNLOAD_HILOGD(fmt, ...)
#define DOWNLOAD_HILOGI(fmt, ...)
#endif // CONFIG_DOWNLOAD_LOG

#endif /* DOWNLOAD_LOG */