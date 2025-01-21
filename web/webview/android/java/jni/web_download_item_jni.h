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

#ifndef PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEB_DOWNLOAD_ITEM_JNI_H
#define PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEB_DOWNLOAD_ITEM_JNI_H

#include <jni.h>

#include "plugins/web/webview/web_download_manager.h"

namespace OHOS::Plugin {
class WebDownloadItemJni final {
public:
    WebDownloadItemJni() = delete;
    ~WebDownloadItemJni() = delete;
    static bool Register(void* env);
    static void NativeInit(JNIEnv* env, jobject jobj);
    static std::string GetGuid(void* object);
    static long GetCurrentSpeed(void* object);
    static int GetPercentComplete(void* object);
    static long GetTotalBytes(void* object);
    static int GetState(void* object);
    static int GetLastErrorCode(void* object);
    static std::string GetMethod(void* object);
    static std::string GetMimeType(void* object);
    static std::string GetUrl(void* object);
    static std::string GetSuggestedFileName(void* object);
    static long GetReceivedBytes(void* object);
    static std::string GetFullPath(void* object);

    static void Start(int webId, const std::string& guid, const std::string& path);
    static void Cancel(int webId, const std::string& guid);
};
}
#endif
