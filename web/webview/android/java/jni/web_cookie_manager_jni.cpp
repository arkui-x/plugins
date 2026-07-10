/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
    { "onReceiveCookieError", "(IJ)V", reinterpret_cast<void*>(WebCookieManagerJni::OnReceiveConfigCookieError) },
    { "onReceiveCookieValue", "(J)V", reinterpret_cast<void*>(WebCookieManagerJni::OnReceiveClearAllCookiesValue) },
    { "onReceiveCookieValue", "(J)V", reinterpret_cast<void*>(WebCookieManagerJni::OnReceiveClearSessionCookieValue) },
    { "onReceiveSaveCookieValue", "(J)V", reinterpret_cast<void*>(WebCookieManagerJni::OnReceiveSaveCookieValue) },
};
static const char METHOD_CONFIG_COOKIE[] = "configCookie";
static const char METHOD_CONFIG_COOKIE_WITH_OPTIONS[] = "configCookie";
static const char METHOD_CONFIG_COOKIE_SYNC[] = "configCookieSync";
static const char METHOD_FETCH_COOKIE[] = "fetchCookie";
static const char METHOD_FETCH_COOKIE_SYNC[] = "fetchCookieSync";
static const char METHOD_PUT_ACCEPT_COOKIE_ENABLED[] = "putAcceptCookieEnabled";
static const char METHOD_PUT_ACCEPT_THIRD_PARTY_COOKIE_ENABLED[] = "putAcceptThirdPartyCookieEnabled";
static const char METHOD_CLEAR_ALL_COOKIE_SYNC[] = "clearAllCookiesSync";
static const char METHOD_CLEAR_SESSION_COOKIE_SYNC[] = "clearSessionCookieSync";
static const char METHOD_SAVE_COOKIE[] = "saveCookie";
static const char METHOD_CLEAR_ALL_COOKIE[] = "clearAllCookies";
static const char METHOD_EXIST_COOKIE[] = "existCookie";
static const char METHOD_CLEAR_SESSION_COOKIE[] = "clearSessionCookie";

static const char SIGNATURE_CONFIG_COOKIE[] = "(Ljava/lang/String;Ljava/lang/String;J)V";
static const char SIGNATURE_CONFIG_COOKIE_WITH_OPTIONS[] = "(Ljava/lang/String;Ljava/lang/String;ZZJ)V";
static const char SIGNATURE_CONFIG_COOKIE_SYNC[] = "(Ljava/lang/String;Ljava/lang/String;ZZ)I";
static const char SIGNATURE_FETCH_COOKIE[] = "(Ljava/lang/String;J)V";
static const char SIGNATURE_FETCH_COOKIE_SYNC[] = "(Ljava/lang/String;Z)Ljava/lang/String;";
static const char SIGNATURE_PUT_ACCEPT_COOKIE_ENABLED[] = "(Z)V";
static const char SIGNATURE_PUT_ACCEPT_THIRD_PARTY_COOKIE_ENABLED[] = "(Z)V";
static const char SIGNATURE_CLEAR_ALL_COOKIE_SYNC[] = "(Z)V";
static const char SIGNATURE_CLEAR_SESSION_COOKIE_SYNC[] = "()V";
static const char SIGNATURE_SAVE_COOKIE[] = "(J)V";
static const char SIGNATURE_CLEAR_ALL_COOKIE[] = "(J)V";
static const char SIGNATURE_EXIST_COOKIE[] = "(Z)Z";
static const char SIGNATURE_CLEAR_SESSION_COOKIE[] = "(J)V";
struct {
    jmethodID configCookie;
    jmethodID configCookieWithOptions;
    jmethodID configCookieSync;
    jmethodID fetchCookie;
    jmethodID fetchCookieSync;
    jmethodID putAcceptCookieEnabled;
    jmethodID putAcceptThirdPartyCookieEnabled;
    jmethodID clearAllCookiesSync;
    jmethodID clearSessionCookieSync;
    jmethodID saveCookie;
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
    g_webWebviewClass.configCookieWithOptions =
        env->GetMethodID(cls, METHOD_CONFIG_COOKIE_WITH_OPTIONS, SIGNATURE_CONFIG_COOKIE_WITH_OPTIONS);
    g_webWebviewClass.configCookieSync = env->GetMethodID(cls, METHOD_CONFIG_COOKIE_SYNC, SIGNATURE_CONFIG_COOKIE_SYNC);
    g_webWebviewClass.fetchCookie = env->GetMethodID(cls, METHOD_FETCH_COOKIE, SIGNATURE_FETCH_COOKIE);
    g_webWebviewClass.fetchCookieSync = env->GetMethodID(cls, METHOD_FETCH_COOKIE_SYNC, SIGNATURE_FETCH_COOKIE_SYNC);
    g_webWebviewClass.putAcceptCookieEnabled = env->GetMethodID(
        cls, METHOD_PUT_ACCEPT_COOKIE_ENABLED, SIGNATURE_PUT_ACCEPT_COOKIE_ENABLED);
    g_webWebviewClass.putAcceptThirdPartyCookieEnabled = env->GetMethodID(
        cls, METHOD_PUT_ACCEPT_THIRD_PARTY_COOKIE_ENABLED, SIGNATURE_PUT_ACCEPT_THIRD_PARTY_COOKIE_ENABLED);
    g_webWebviewClass.clearAllCookiesSync = env->GetMethodID(
        cls, METHOD_CLEAR_ALL_COOKIE_SYNC, SIGNATURE_CLEAR_ALL_COOKIE_SYNC);
    g_webWebviewClass.clearSessionCookieSync = env->GetMethodID(
        cls, METHOD_CLEAR_SESSION_COOKIE_SYNC, SIGNATURE_CLEAR_SESSION_COOKIE_SYNC);
    g_webWebviewClass.saveCookie = env->GetMethodID(cls, METHOD_SAVE_COOKIE, SIGNATURE_SAVE_COOKIE);
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

void WebCookieManagerJni::OnReceiveConfigCookieError(JNIEnv* env, jobject jobj, jint jErrorCode, jlong jId)
{
    auto nativeId = static_cast<int32_t>(jId);
    WebCookieManager::OnConfigReceiveError(static_cast<ErrCode>(jErrorCode), nativeId);
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

void WebCookieManagerJni::OnReceiveSaveCookieValue(JNIEnv* env, jobject jobj, jlong jId)
{
    auto nativeId = static_cast<int32_t>(jId);
    WebCookieManager::OnSaveReceiveValue(nativeId);
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
    }
}

void WebCookieManagerJni::ConfigCookie(
    const std::string& url, const std::string& value, bool incognito, bool includeHttpOnly, int32_t asyncCallbackInfoId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.configCookieWithOptions)) {
        WebCookieManager::OnConfigReceiveError(NWebError::INIT_ERROR, asyncCallbackInfoId);
        return;
    }
    jstring jUrl = env->NewStringUTF(url.c_str());
    if (jUrl == nullptr) {
        WebCookieManager::OnConfigReceiveError(NWebError::NWEB_ERROR, asyncCallbackInfoId);
        return;
    }
    jstring jValue = env->NewStringUTF(value.c_str());
    if (jValue == nullptr) {
        env->DeleteLocalRef(jUrl);
        WebCookieManager::OnConfigReceiveError(NWebError::NWEB_ERROR, asyncCallbackInfoId);
        return;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.configCookieWithOptions,
        jUrl, jValue, static_cast<jboolean>(incognito), static_cast<jboolean>(includeHttpOnly),
        static_cast<jlong>(asyncCallbackInfoId));
    if (env->ExceptionCheck()) {
        LOGE("WebCookieManagerJni JNI: call ConfigCookie with options has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jUrl);
        env->DeleteLocalRef(jValue);
        WebCookieManager::OnConfigReceiveError(NWebError::NWEB_ERROR, asyncCallbackInfoId);
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

void WebCookieManagerJni::SaveCookie(int32_t asyncCallbackInfoId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.saveCookie)) {
        WebCookieManager::OnSaveReceiveValue(asyncCallbackInfoId);
        return;
    }
    env->CallVoidMethod(
        g_webWebviewClass.globalRef, g_webWebviewClass.saveCookie, static_cast<jlong>(asyncCallbackInfoId));
    if (env->ExceptionCheck()) {
        LOGE("WebCookieManagerJni JNI: call SaveCookie has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        WebCookieManager::OnSaveReceiveValue(asyncCallbackInfoId);
    }
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

ErrCode WebCookieManagerJni::ConfigCookieSync(
    const std::string& url, const std::string& value, bool incognito, bool includeHttpOnly)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.configCookieSync)) {
        return NWebError::INIT_ERROR;
    }
    jstring jUrl = env->NewStringUTF(url.c_str());
    if (!jUrl) {
        return NWebError::NWEB_ERROR;
    }
    jstring jValue = env->NewStringUTF(value.c_str());
    if (!jValue) {
        env->DeleteLocalRef(jUrl);
        return NWebError::NWEB_ERROR;
    }
    jint ret = env->CallIntMethod(g_webWebviewClass.globalRef, g_webWebviewClass.configCookieSync,
        jUrl, jValue, static_cast<jboolean>(incognito), static_cast<jboolean>(includeHttpOnly));
    if (env->ExceptionCheck()) {
        LOGE("WebCookieManagerJni JNI: call ConfigCookieSync has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jUrl);
        env->DeleteLocalRef(jValue);
        return NWebError::NWEB_ERROR;
    }
    env->DeleteLocalRef(jUrl);
    env->DeleteLocalRef(jValue);
    return static_cast<ErrCode>(ret);
}

std::string WebCookieManagerJni::FetchCookieSync(const std::string& url, bool incognito, ErrCode& errorCode)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.fetchCookieSync)) {
        errorCode = NWebError::INIT_ERROR;
        return "";
    }
    jstring jUrl = env->NewStringUTF(url.c_str());
    if (!jUrl) {
        errorCode = NWebError::NWEB_ERROR;
        return "";
    }
    auto jResult = static_cast<jstring>(env->CallObjectMethod(
        g_webWebviewClass.globalRef, g_webWebviewClass.fetchCookieSync, jUrl, static_cast<jboolean>(incognito)));
    if (env->ExceptionCheck()) {
        LOGE("WebCookieManagerJni JNI: call FetchCookieSync has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jUrl);
        errorCode = NWebError::NWEB_ERROR;
        return "";
    }
    env->DeleteLocalRef(jUrl);
    if (jResult == nullptr) {
        // Java returns null to indicate an invalid URL (isValidCookieUrl failed).
        errorCode = NWebError::INVALID_URL;
        return "";
    }
    std::string result;
    const char* content = env->GetStringUTFChars(jResult, nullptr);
    if (content != nullptr) {
        result.assign(content);
        env->ReleaseStringUTFChars(jResult, content);
    }
    env->DeleteLocalRef(jResult);
    errorCode = NWebError::NO_ERROR;
    return result;
}

ErrCode WebCookieManagerJni::PutAcceptCookieEnabled(bool accept)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.putAcceptCookieEnabled)) {
        return NWebError::INIT_ERROR;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.putAcceptCookieEnabled,
        static_cast<jboolean>(accept));
    if (env->ExceptionCheck()) {
        LOGE("WebCookieManagerJni JNI: call PutAcceptCookieEnabled has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return NWebError::NWEB_ERROR;
    }
    return NWebError::NO_ERROR;
}

ErrCode WebCookieManagerJni::PutAcceptThirdPartyCookieEnabled(bool accept)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.putAcceptThirdPartyCookieEnabled)) {
        return NWebError::INIT_ERROR;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.putAcceptThirdPartyCookieEnabled,
        static_cast<jboolean>(accept));
    if (env->ExceptionCheck()) {
        LOGE("WebCookieManagerJni JNI: call PutAcceptThirdPartyCookieEnabled has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return NWebError::NWEB_ERROR;
    }
    return NWebError::NO_ERROR;
}

ErrCode WebCookieManagerJni::ClearAllCookiesSync(bool incognito)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.clearAllCookiesSync)) {
        return NWebError::INIT_ERROR;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.clearAllCookiesSync,
        static_cast<jboolean>(incognito));
    if (env->ExceptionCheck()) {
        LOGE("WebCookieManagerJni JNI: call ClearAllCookiesSync has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return NWebError::NWEB_ERROR;
    }
    return NWebError::NO_ERROR;
}

ErrCode WebCookieManagerJni::ClearSessionCookieSync()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.clearSessionCookieSync)) {
        return NWebError::INIT_ERROR;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.clearSessionCookieSync);
    if (env->ExceptionCheck()) {
        LOGE("WebCookieManagerJni JNI: call ClearSessionCookieSync has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return NWebError::NWEB_ERROR;
    }
    return NWebError::NO_ERROR;
}
} // namespace OHOS::Plugin
