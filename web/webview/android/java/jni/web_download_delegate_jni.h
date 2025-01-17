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

#ifndef PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEB_DOWNLOAD_DELEGATE_JNI_H
#define PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEB_DOWNLOAD_DELEGATE_JNI_H

#include <jni.h>

#include "plugins/web/webview/web_download_manager.h"

namespace OHOS::Plugin {
class WebDownloadDelegateJni final {
public:
    WebDownloadDelegateJni() = delete;
    ~WebDownloadDelegateJni() = delete;
    static bool Register(void* env);
    static void OnBeforeDownloadObject(JNIEnv* env, jobject clazz, jlong id, jobject object);
    static void OnDownloadUpdatedObject(JNIEnv* env, jobject clazz, jlong id, jobject object);
    static void OnDownloadFailedObject(JNIEnv* env, jobject clazz, jlong id, jobject object);
    static void OnDownloadFinishObject(JNIEnv* env, jobject clazz, jlong id, jobject object);
};
}
#endif
