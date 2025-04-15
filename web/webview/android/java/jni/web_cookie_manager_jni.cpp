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

#include "plugins/web/webview/android/java/jni/web_cookie_manager_jni.h"

#include <jni.h>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
namespace {
const char WEB_WEBVIEW_CLASS_NAME[] = "ohos/ace/plugin/webviewplugin/webcookie/WebCookiePlugin";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(WebCookieManagerJni::NativeInit) },
    { "onReceiveCookieValue", "(Ljava/lang/String;J)V",
        reinterpret_cast<void*>(WebCookieManagerJni::OnReceiveFetchCookieValue) },
    { "onReceiveCookieValue", "(ZJ)V", reinterpret_cast<void*>(WebCookieManagerJni::OnReceiveConfigCookieValue) },
    { "onReceiveCookieValue", "(J)V", reinterpret_cast<void*>(WebCookieManagerJni::OnReceiveClearAllCookiesValue) },
    { "onReceiveCookieValue", "(J)V", reinterpret_cast<void*>(WebCookieManagerJni::OnReceiveClearSessionCookieValue) },
};
static const char METHOD_CONFIG_COOKIE[] = "configCookie";
static const char METHOD_FETCH_COOKIE[] = "fetchCookie";
static const char METHOD_CLEAR_ALL_COOKIE[] = "clearAllCookies";
static const char METHOD_EXIST_COOKIE[] = "existCookie";
static const char METHOD_CLEAR_SESSION_COOKIE[] = "clearSessionCookie";

static const char SIGNATURE_CONFIG_COOKIE[] = "(Ljava/lang/String;Ljava/lang/String;J)V";
static const char SIGNATURE_FETCH_COOKIE[] = "(Ljava/lang/String;J)V";
static const char SIGNATURE_CLEAR_ALL_COOKIE[] = "(J)V";
static const char SIGNATURE_EXIST_COOKIE[] = "(Z)Z";
static const char SIGNATURE_CLEAR_SESSION_COOKIE[] = "(J)V";
struct {
    jmethodID configCookie;
    jmethodID fetchCookie;
    jmethodID clearAllCookies;
    jmethodID existCookie;
    jmethodID clearSessionCookie;
    jobject globalRef;
} g_webWebviewClass;
} // namespace

bool WebCookieManagerJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(WEB_WEBVIEW_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("WebCookieManagerJni JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void WebCookieManagerJni::NativeInit(JNIEnv* env, jobject jobj)
{
    LOGI("WebCookieManagerJni JNI: NativeInit");
    CHECK_NULL_VOID(env);
    g_webWebviewClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_webWebviewClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);
    g_webWebviewClass.configCookie = env->GetMethodID(cls, METHOD_CONFIG_COOKIE, SIGNATURE_CONFIG_COOKIE);
    g_webWebviewClass.fetchCookie = env->GetMethodID(cls, METHOD_FETCH_COOKIE, SIGNATURE_FETCH_COOKIE);
    g_webWebviewClass.clearAllCookies = env->GetMethodID(cls, METHOD_CLEAR_ALL_COOKIE, SIGNATURE_CLEAR_ALL_COOKIE);
    g_webWebviewClass.existCookie = env->GetMethodID(cls, METHOD_EXIST_COOKIE, SIGNATURE_EXIST_COOKIE);
    g_webWebviewClass.clearSessionCookie = env->GetMethodID(cls, METHOD_CLEAR_SESSION_COOKIE, SIGNATURE_CLEAR_SESSION_COOKIE);
    env->DeleteLocalRef(cls);
}

void WebCookieManagerJni::OnReceiveFetchCookieValue(JNIEnv* env, jobject jobj, jstring jResult, jint jId)
{
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(jResult);
    std::string result;
    const char* content = env->GetStringUTFChars(jResult, nullptr);
    if (content != nullptr) {
        result.assign(content);
        env->ReleaseStringUTFChars(jResult, content);
    }
    if (jResult != nullptr) {
        env->DeleteLocalRef(jResult);
    }
    auto nativeId = static_cast<int32_t>(jId);
    WebCookieManager::OnFetchReceiveValue(result, nativeId);
}

void WebCookieManagerJni::OnReceiveConfigCookieValue(JNIEnv* env, jobject jobj, jboolean jResult, jint jId)
{
    auto nativeId = static_cast<int32_t>(jId);
    WebCookieManager::OnConfigReceiveValue(true, nativeId);
}

void WebCookieManagerJni::OnReceiveClearAllCookiesValue(JNIEnv* env, jobject jobj, jint jId)
{
    auto nativeId = static_cast<int32_t>(jId);
    WebCookieManager::OnClearReceiveValue(nativeId);
}

void WebCookieManagerJni::OnReceiveClearSessionCookieValue(JNIEnv* env, jobject jobj, jint jId)
{
    auto nativeId = static_cast<int32_t>(jId);
    WebCookieManager::OnClearReceiveValue(nativeId);
}

void WebCookieManagerJni::ConfigCookie(const std::string& url, const std::string& value, int32_t asyncCallbackInfoId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    jstring jUrl = env->NewStringUTF(url.c_str());
    CHECK_NULL_VOID(jUrl);
    jstring jValue = env->NewStringUTF(value.c_str());
    CHECK_NULL_VOID(jValue);
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.configCookie, jUrl, jValue, asyncCallbackInfoId);
    if (env->ExceptionCheck()) {
        LOGE("WebCookieManagerJni JNI: call ConfigCookie has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jUrl);
        env->DeleteLocalRef(jValue);
        return;
    }
    env->DeleteLocalRef(jUrl);
    env->DeleteLocalRef(jValue);
}

void WebCookieManagerJni::FetchCookie(const std::string& url, int32_t asyncCallbackInfoId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.fetchCookie)) {
        return;
    }
    jstring jUrl = env->NewStringUTF(url.c_str());
    CHECK_NULL_VOID(jUrl);
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.fetchCookie, jUrl, asyncCallbackInfoId);
    if (env->ExceptionCheck()) {
        LOGE("WebCookieManagerJni JNI: call FetchCookie has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jUrl);
        return;
    }
    env->DeleteLocalRef(jUrl);
}

void WebCookieManagerJni::ClearAllCookies(int32_t asyncCallbackInfoId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.clearAllCookies)) {
        return;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.clearAllCookies, asyncCallbackInfoId);
    if (env->ExceptionCheck()) {
        LOGE("WebCookieManagerJni JNI: call ClearAllCookies has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
}

void WebCookieManagerJni::ClearSessionCookie(int32_t asyncCallbackInfoId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.clearSessionCookie)) {
        return;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.clearSessionCookie, asyncCallbackInfoId);
    if (env->ExceptionCheck()) {
        LOGE("WebCookieManagerJni JNI: call ClearSessionCookie has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
}

bool WebCookieManagerJni::ExistCookie(bool incognito)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.existCookie)) {
        return false;
    }
    jboolean isCookie = env->CallBooleanMethod(g_webWebviewClass.globalRef, g_webWebviewClass.existCookie, incognito);
    if (env->ExceptionCheck()) {
        LOGE("WebCookieManagerJni JNI: call ExistCookie has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return isCookie;
}
} // namespace OHOS::Plugin