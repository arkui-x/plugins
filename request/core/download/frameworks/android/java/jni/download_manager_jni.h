/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_DOWNLOADMANAGER_ANDROID_JAVA_JNI_DOWNLOADMANAGER_JNI_H
#define PLUGINS_DOWNLOADMANAGER_ANDROID_JAVA_JNI_DOWNLOADMANAGER_JNI_H

#include <jni.h>
#include <memory>

#include "plugin_utils.h"
#include "inner_api/plugin_utils_inner.h"
#include "download_config.h"

class AndroidDownloadAdpImpl;
namespace OHOS::Plugin::Request::Download {
class DownloadManagerJni final {
public:
    DownloadManagerJni() = default;
    virtual ~DownloadManagerJni() = default;
    static bool Register(void* env);

    // Called by java
    static void NativeInit(JNIEnv* env, jobject jobj);
    static void OnRequestDataCallback(JNIEnv* env, jobject jobj, jintArray intArray, jlong jDownloadProgress);
    static void OnRequestMimeTypeCallback(JNIEnv* env, jobject obj, jstring jMimeType, jlong jDownloadProgress);
    static void OnRequestNetworkTypeCallback(JNIEnv* env, jobject obj, jint jType, jlong jNetwork);

    // Called by C++
    static void Download(const DownloadConfig &config, void *downloadProgress);
    static bool ResumeDownload();
    static void PauseDownload();
    static void RemoveDownload();
    static void GetNetworkType(void *network);
};
} // namespace OHOS::Plugin::Request::Download

#endif // PLUGINS_DOWNLOADMANAGER_ANDROID_JAVA_JNI_DOWNLOADMANAGER_JNI_H