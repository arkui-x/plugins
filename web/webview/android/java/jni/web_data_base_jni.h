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

#ifndef PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEB_DATA_BASE_JNI_H
#define PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEB_DATA_BASE_JNI_H

#include <jni.h>

#include "plugins/web/webview/android/java/jni/web_data_base_android.h"

namespace OHOS::Plugin {
class WebDataBaseJni final {
public:
    WebDataBaseJni() = delete;
    ~WebDataBaseJni() = delete;
    static bool Register(void* env);
    static void NativeInit(JNIEnv* env, jobject jobj);
    static bool ExistHttpAuthCredentials();
    static ErrCode DeleteHttpAuthCredentials();
    static ErrCode SaveHttpAuthCredentials(const std::string& host, const std::string& realm,
        const std::string& username, const char* password);
    static ErrCode GetHttpAuthCredentials(const std::string& host, const std::string& realm, std::string& username,
        char* password, uint32_t passwordSize);
private:
    static std::string GetName(JNIEnv* env, jclass cls, jobject jobj);
    static std::string GetPassword(JNIEnv* env, jclass cls, jobject jobj);
};
}

#endif