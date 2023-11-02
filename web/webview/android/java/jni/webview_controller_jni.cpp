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

#include "plugins/web/webview/android/java/jni/webview_controller_jni.h"

#include <jni.h>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
namespace {
const char WEB_WEBVIEW_CLASS_NAME[] = "ohos/ace/adapter/capability/web/AceWebPluginBase";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(WebviewControllerJni::NativeInit) },
    { "onReceiveValue", "(Ljava/lang/String;)V", reinterpret_cast<void*>(WebviewControllerJni::OnReceiveValue) },
};
static const char METHOD_LOADURL[] = "loadUrl";

static const char METHOD_LOADDATA[] = "loadData";

static const char METHOD_GETURL[] = "getUrl";

static const char METHOD_ACCESSFORWARD[] = "accessForward";

static const char METHOD_ACCESSBACKWARD[] = "accessBackward";

static const char METHOD_FORWARD[] = "forward";

static const char METHOD_BACKWARD[] = "backward";

static const char METHOD_REFRESH[] = "refresh";

static const char METHOD_EVALUTEJS[] = "evaluateJavascript";

static const char SIGNATURE_LOADURL[] = "(JLjava/lang/String;Ljava/util/HashMap;)V";

static const char SIGNATURE_LOADDATA[] = "(JLjava/util/HashMap;)V";

static const char SIGNATURE_GETURL[] = "(J)Ljava/lang/String;";

static const char SIGNATURE_ACCESSFORWARD[] = "(J)Ljava/lang/String;";

static const char SIGNATURE_ACCESSBACKWARD[] = "(J)Ljava/lang/String;";

static const char SIGNATURE_FORWARD[] = "(J)V";

static const char SIGNATURE_BACKWARD[] = "(J)V";

static const char SIGNATURE_REFRESH[] = "(J)V";

static const char SIGNATURE_EVALUTEJS[] = "(JLjava/lang/String;)V";

struct {
    jmethodID loadUrl;
    jmethodID loadData;
    jmethodID getUrl;
    jmethodID accessForward;
    jmethodID accessBackward;
    jmethodID forward;
    jmethodID backward;
    jmethodID refresh;
    jmethodID evaluateJavascript;
    jobject globalRef;
} g_webWebviewClass;
}

bool WebviewControllerJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(WEB_WEBVIEW_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("WebviewControllerJni JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void WebviewControllerJni::NativeInit(JNIEnv* env, jobject jobj)
{
    LOGI("WebviewControllerJni JNI: NativeInit");
    CHECK_NULL_VOID(env);
    g_webWebviewClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_webWebviewClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);
    g_webWebviewClass.loadUrl = env->GetMethodID(cls, METHOD_LOADURL, SIGNATURE_LOADURL);
    g_webWebviewClass.loadData = env->GetMethodID(cls, METHOD_LOADDATA, SIGNATURE_LOADDATA);
    g_webWebviewClass.getUrl = env->GetMethodID(cls, METHOD_GETURL, SIGNATURE_GETURL);
    g_webWebviewClass.accessForward = env->GetMethodID(cls, METHOD_ACCESSFORWARD, SIGNATURE_ACCESSFORWARD);
    g_webWebviewClass.accessBackward = env->GetMethodID(cls, METHOD_ACCESSBACKWARD, SIGNATURE_ACCESSBACKWARD);
    g_webWebviewClass.forward = env->GetMethodID(cls, METHOD_FORWARD, SIGNATURE_FORWARD);
    g_webWebviewClass.backward = env->GetMethodID(cls, METHOD_BACKWARD, SIGNATURE_BACKWARD);
    g_webWebviewClass.refresh = env->GetMethodID(cls, METHOD_REFRESH, SIGNATURE_REFRESH);
    g_webWebviewClass.evaluateJavascript = env->GetMethodID(cls, METHOD_EVALUTEJS, SIGNATURE_EVALUTEJS);
    env->DeleteLocalRef(cls);
}

void WebviewControllerJni::OnReceiveValue(JNIEnv* env, jobject jobj, jstring jResult)
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
    LOGD("WebviewControllerJni::OnReceiveValue result == %{public}s", result.c_str());
    WebviewController::OnReceiveValue(result);
}

ErrCode WebviewControllerJni::LoadUrl(int id, const std::string& url, 
    const std::map<std::string, std::string>& httpHeaders)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.loadUrl)) {
        return INIT_ERROR;
    }
    jstring jUrl = env->NewStringUTF(url.c_str());
    CHECK_NULL_RETURN(jUrl, INIT_ERROR);
    if (httpHeaders.empty()) {
        env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.loadUrl, id, jUrl, nullptr);
    } else {
        jclass mapClass = env->FindClass("java/util/HashMap");
        CHECK_NULL_RETURN(mapClass, INIT_ERROR);
        jmethodID init = env->GetMethodID(mapClass, "<init>", "()V");
        CHECK_NULL_RETURN(init, INIT_ERROR);
        jmethodID put = env->GetMethodID(mapClass, "put",
            "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
        CHECK_NULL_RETURN(put, INIT_ERROR);
        jobject httpHeaderMap = env->NewObject(mapClass, init);
        CHECK_NULL_RETURN(httpHeaderMap, INIT_ERROR);
        std::map<std::string, std::string>::const_iterator iter = httpHeaders.begin();
        for (; iter != httpHeaders.end(); ++iter) {
            jstring key = env->NewStringUTF(iter->first.c_str());
            jstring value = env->NewStringUTF(iter->second.c_str());
            CHECK_NULL_RETURN(key, INIT_ERROR);
            CHECK_NULL_RETURN(value, INIT_ERROR);
            env->CallObjectMethod(httpHeaderMap, put, key, value);
            env->DeleteLocalRef(key);
            env->DeleteLocalRef(value);
        }
        env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.loadUrl, id, jUrl, httpHeaderMap);
        env->DeleteLocalRef(httpHeaderMap);
    }
    if (env->ExceptionCheck()) {
        LOGE("WebviewControllerJni JNI: call loadUrl has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        env->DeleteLocalRef(jUrl);
        return INIT_ERROR;
    }
    env->DeleteLocalRef(jUrl);
    return NO_ERROR;
}

ErrCode WebviewControllerJni::LoadData(int id, const std::string& data, const std::string& mimeType, const std::string& encoding,
    const std::string& baseUrl, const std::string& historyUrl)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.loadData)) {
        return INIT_ERROR;
    }
    jclass mapClass = env->FindClass("java/util/HashMap");
    CHECK_NULL_RETURN(mapClass, INIT_ERROR);
    jmethodID init = env->GetMethodID(mapClass, "<init>", "()V");
    CHECK_NULL_RETURN(init, INIT_ERROR);
    jmethodID put = env->GetMethodID(mapClass, "put",
        "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    CHECK_NULL_RETURN(put, INIT_ERROR);
    jobject paramContainer = env->NewObject(mapClass, init);
    CHECK_NULL_RETURN(paramContainer, INIT_ERROR);

    jstring JsDataKey = env->NewStringUTF("load_data_data");
    if (JsDataKey && !data.empty()) {
        jstring JsDataValue = env->NewStringUTF(data.c_str());
        if (JsDataValue) {
            env->CallObjectMethod(paramContainer, put, JsDataKey, JsDataValue);
            env->DeleteLocalRef(JsDataValue);
        }
    }
    env->DeleteLocalRef(JsDataKey);
    jstring JsMimeTypeKey = env->NewStringUTF("load_data_mimetype");
    if (JsMimeTypeKey && !mimeType.empty()) {
        jstring JsMimeTypeValue = env->NewStringUTF(mimeType.c_str());
        if (JsMimeTypeValue) {
            env->CallObjectMethod(paramContainer, put, JsMimeTypeKey, JsMimeTypeValue);
            env->DeleteLocalRef(JsMimeTypeValue);
        }
    }
    env->DeleteLocalRef(JsMimeTypeKey);
    jstring JsEncodingKey = env->NewStringUTF("load_data_encoding");
    if (JsEncodingKey && !encoding.empty()) {
        jstring JsEncodingValue = env->NewStringUTF(encoding.c_str());
        if (JsEncodingValue) {
            env->CallObjectMethod(paramContainer, put, JsEncodingKey, JsEncodingValue);
            env->DeleteLocalRef(JsEncodingValue);
        }
    }
    env->DeleteLocalRef(JsEncodingKey);
    jstring JsBaseUrlKey = env->NewStringUTF("load_data_base_url");
    if (JsBaseUrlKey && !baseUrl.empty()) {
        jstring JsBaseUrlValue = env->NewStringUTF(baseUrl.c_str());
        if (JsBaseUrlValue) {
            env->CallObjectMethod(paramContainer, put, JsBaseUrlKey, JsBaseUrlValue);
            env->DeleteLocalRef(JsBaseUrlValue);
        }
    }
    env->DeleteLocalRef(JsBaseUrlKey);
    jstring JsHistoryUrlKey = env->NewStringUTF("load_data_history_url");
    if (JsHistoryUrlKey && !historyUrl.empty()) {
        jstring JsHistoryUrlValue = env->NewStringUTF(historyUrl.c_str());
        if (JsHistoryUrlValue) {
            env->CallObjectMethod(paramContainer, put, JsHistoryUrlKey, JsHistoryUrlValue);
            env->DeleteLocalRef(JsHistoryUrlValue);
        }
    }
    env->DeleteLocalRef(JsHistoryUrlKey);
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.loadData, id, paramContainer);
    env->DeleteLocalRef(paramContainer);

    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INIT_ERROR;
    }
    return NO_ERROR;
}

std::string WebviewControllerJni::GetUrl(int id)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.getUrl)) {
        return "";
    }
    jstring jResult = static_cast<jstring>(
        env->CallObjectMethod(g_webWebviewClass.globalRef, g_webWebviewClass.getUrl, id));
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return "";
    }
    std::string result;
    const char* content = env->GetStringUTFChars(jResult, nullptr);
    if (content != nullptr) {
        result.assign(content);
        env->ReleaseStringUTFChars(jResult, content);
    }
    if (jResult != nullptr) {
        env->DeleteLocalRef(jResult);
    }
    return result;
}

bool WebviewControllerJni::AccessForward(int id)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.accessForward)) {
        return false;
    }
    jstring jResult = static_cast<jstring>(
        env->CallObjectMethod(g_webWebviewClass.globalRef, g_webWebviewClass.accessForward, id));
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    std::string result;
    const char* content = env->GetStringUTFChars(jResult, nullptr);
    if (content != nullptr) {
        result.assign(content);
        env->ReleaseStringUTFChars(jResult, content);
    }
    if (jResult != nullptr) {
        env->DeleteLocalRef(jResult);
    }
    return (result == "true" ? true : false);
}

bool WebviewControllerJni::AccessBackward(int id)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.accessBackward)) {
        return false;
    }
    jstring jResult = static_cast<jstring>(
        env->CallObjectMethod(g_webWebviewClass.globalRef, g_webWebviewClass.accessBackward, id));
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    std::string result;
    const char* content = env->GetStringUTFChars(jResult, nullptr);
    if (content != nullptr) {
        result.assign(content);
        env->ReleaseStringUTFChars(jResult, content);
    }
    if (jResult != nullptr) {
        env->DeleteLocalRef(jResult);
    }
    return (result == "true" ? true : false);
}

void WebviewControllerJni::Forward(int id)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.forward)) {
        return;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.forward, id);
}

void WebviewControllerJni::Backward(int id)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.backward)) {
        return;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.backward, id);
}

void WebviewControllerJni::Refresh(int id)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.refresh)) {
        return;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.refresh, id);
}

void WebviewControllerJni::EvaluateJavaScript(int id, const std::string& script)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.evaluateJavascript)) {
        return;
    }

    jstring JsName = env->NewStringUTF(script.c_str());
    CHECK_NULL_VOID(JsName);
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.evaluateJavascript, id, JsName);
    env->DeleteLocalRef(JsName);
}
}
