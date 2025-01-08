/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEB_STORAGE_ANDROID_H
#define PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEB_STORAGE_ANDROID_H

#include "plugins/web/webview/web_storage.h"

namespace OHOS::Plugin {
class WebStorageAndroid final {
public:
    WebStorageAndroid() = default;
    ~WebStorageAndroid() = default;
    static void GetOriginQuota(const std::string& origin, int32_t asyncCallbackInfoId);
    static void GetOriginUsage(const std::string& origin, int32_t asyncCallbackInfoId);
    static void GetOrigins(int32_t asyncCallbackInfoId);
    static void DeleteAllData();
    static void DeleteOrigin(const std::string& origin);
    
};
} // namespace OHOS::Plugin

#endif