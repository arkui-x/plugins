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

#ifndef PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEB_COOKIE_MANAGER_JNI_H
#define PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEB_COOKIE_MANAGER_JNI_H

#include <jni.h>

#include "plugins/web/webview/android/java/jni/web_cookie_manager_android.h"

namespace OHOS::Plugin {
class WebCookieManagerJni final {
public:
    WebCookieManagerJni() = delete;
    ~WebCookieManagerJni() = delete;
    static bool Register(void* env);
    static void NativeInit(JNIEnv* env, jobject jobj);
    static void OnReceiveFetchCookieValue(JNIEnv* env, jobject jobj, jstring jResult, jint jId);
    static void OnReceiveConfigCookieValue(JNIEnv* env, jobject jobj, jboolean jResult, jint jId);
    static void OnReceiveClearSessionCookieValue(JNIEnv* env, jobject jobj, jint jId);
    static void OnReceiveClearAllCookiesValue(JNIEnv* env, jobject jobj, jint jId);
    static void ConfigCookie(const std::string& url, const std::string& value, int32_t asyncCallbackInfoId);
    static void FetchCookie(const std::string& url, int32_t asyncCallbackInfoId);
    static void ClearAllCookies(int32_t asyncCallbackInfoId);
    static bool ExistCookie(bool incognito);
    static void ClearSessionCookie(int32_t asyncCallbackInfoId);
};
} // namespace OHOS::Plugin

#endif