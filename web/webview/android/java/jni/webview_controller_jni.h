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

#ifndef PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEBVIEW_CONTROLLER_JNI_H
#define PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEBVIEW_CONTROLLER_JNI_H

#include <jni.h>

#include "plugins/web/webview/android/java/jni/webview_controller_android.h"

namespace OHOS::Plugin {
class WebviewControllerJni final {
public:
    WebviewControllerJni() = delete;
    ~WebviewControllerJni() = delete;
    static bool Register(void* env);
    static void NativeInit(JNIEnv* env, jobject jobj);
    static ErrCode LoadUrl(int id, const std::string& url,
        const std::map<std::string, std::string>& httpHeaders);
    static ErrCode LoadData(int id, const std::string& data, const std::string& mimeType, const std::string& encoding,
        const std::string& baseUrl, const std::string& historyUrl);
    static std::string GetUrl(int id);
    static bool AccessForward(int id);
    static bool AccessBackward(int id);
    static void Forward(int id);
    static void Backward(int id);
    static void Refresh(int id);
    static void EvaluateJavaScript(int id, const std::string& script);
    static void OnReceiveValue(JNIEnv* env, jobject jobj, jstring jResult);
};
}

#endif
