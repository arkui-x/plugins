/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_REQUEST_UPLOAD_UPLOAD_HILOG_WRAPPER_H
#define PLUGINS_REQUEST_UPLOAD_UPLOAD_HILOG_WRAPPER_H

#ifdef ANDROID_PLATFORM
#define CONFIG_DOWNLOAD_LOG
#endif


// param of log interface, such as UPLOAD_HILOGF.
enum UploadSubModule {
    UPLOAD_MODULE_INNERKIT = 0,
    UPLOAD_MODULE_FRAMEWORK,
    UPLOAD_MODULE_COMMON,
    UPLOAD_MODULE_JS_NAPI,
    UPLOAD_MODULE_TEST,
    UPLOAD_MODULE_BUTT,
};
// 0xD001C00: subsystem:miscservices module:upload_native, 8 bits reserved.
static constexpr unsigned int BASE_UPLOAD_DOMAIN_ID = 0xD001C00;
enum UploadDomainId {
    UPLOAD_INNERKIT_DOMAIN = BASE_UPLOAD_DOMAIN_ID + UPLOAD_MODULE_INNERKIT,
    UPLOAD_FRAMEWORK_DOMAIN,
    UPLOAD_COMMON_DOMAIN,
    UPLOAD_JS_NAPI,
    UPLOAD_TEST,
    UPLOAD_BUTT,
};

#ifdef CONFIG_DOWNLOAD_LOG
#include "hilog/log.h"

namespace  OHOS::Plugin::Request::Upload {

#define FILENAME            (__builtin_strrchr(__FILE__, '/') ? __builtin_strrchr(__FILE__, '/') + 1 : __FILE__)
#define FORMATED(fmt, ...)    "[%{public}s] %{public}s# " fmt, FILENAME, __FUNCTION__, ##__VA_ARGS__
static constexpr OHOS::HiviewDFX::HiLogLabel UPLOAD_MODULE_LABEL[UPLOAD_MODULE_BUTT] = {
    {LOG_CORE, UPLOAD_INNERKIT_DOMAIN,  "UploadInnerKit"},
    {LOG_CORE, UPLOAD_FRAMEWORK_DOMAIN, "UploadFramework"},
    {LOG_CORE, UPLOAD_COMMON_DOMAIN,    "UploadCommon"},
    {LOG_CORE, UPLOAD_JS_NAPI,  "UploadJSNAPI"},
    {LOG_CORE, UPLOAD_TEST,     "UploadTest"},
};


// In order to improve performance, do not check the module range.
// Besides, make sure module is less than UPLOAD_MODULE_BUTT.
#define UPLOAD_HILOGF(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Fatal(UPLOAD_MODULE_LABEL[module], FORMATED(__VA_ARGS__))
#define UPLOAD_HILOGE(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Error(UPLOAD_MODULE_LABEL[module], FORMATED(__VA_ARGS__))
#define UPLOAD_HILOGW(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Warn(UPLOAD_MODULE_LABEL[module], FORMATED(__VA_ARGS__))
#define UPLOAD_HILOGI(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Info(UPLOAD_MODULE_LABEL[module], FORMATED(__VA_ARGS__))
#define UPLOAD_HILOGD(module, ...) \
    (void)OHOS::HiviewDFX::HiLog::Debug(UPLOAD_MODULE_LABEL[module], FORMATED(__VA_ARGS__))
} // namespace OHOS::Plugin::Request::Upload

#else

#ifdef IOS_PLATFORM
    #include "plugins/interfaces/native/log.h"
    #define UPLOAD_HILOGF(module, fmt, ...) LOGF(fmt, ##__VA_ARGS__)
    #define UPLOAD_HILOGE(module, fmt, ...) LOGE(fmt, ##__VA_ARGS__)
    #define UPLOAD_HILOGW(module, fmt, ...) LOGW(fmt, ##__VA_ARGS__)
    #define UPLOAD_HILOGI(module, fmt, ...) LOGI(fmt, ##__VA_ARGS__)
    #define UPLOAD_HILOGD(module, fmt, ...) LOGD(fmt, ##__VA_ARGS__)
#else
    #define UPLOAD_HILOGF(module, fmt, ...)
    #define UPLOAD_HILOGE(module, fmt, ...)
    #define UPLOAD_HILOGW(module, fmt, ...)
    #define UPLOAD_HILOGI(module, fmt, ...)
    #define UPLOAD_HILOGD(module, fmt, ...)
#endif

#endif // CONFIG_DOWNLOAD_LOG

#endif // UPLOAD_HILOG_WRAPPER_H