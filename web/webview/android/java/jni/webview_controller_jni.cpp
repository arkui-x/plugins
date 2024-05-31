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
#include "plugins/web/webview/web_message_port.h"

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
    { "onReceiveValue", "(Ljava/lang/String;J)V", reinterpret_cast<void*>(WebviewControllerJni::OnReceiveValue) },
    { "onMessage", "(JLjava/lang/String;Ljava/lang/String;)V", reinterpret_cast<void*>(WebviewControllerJni::OnMessage) },
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

static const char METHOD_ACCESS_STEP[] = "accessStep";

static const char METHOD_SCROLL_TO[] = "scrollTo";

static const char METHOD_SCROLL_BY[] = "scrollBy";

static const char METHOD_ZOOM[] = "zoom";

static const char METHOD_STOP[] = "stop";

static const char METHOD_CLEAR_HISTORY[] = "clearHistory";

static const char METHOD_SET_CUSTOM_USER_AGENT[] = "setCustomUserAgent";

static const char METHOD_GET_CUSTOM_USER_AGENT[] = "getCustomUserAgent";

static const char METHOD_GETBACKFORWARDENTRIES[] = "getBackForwardEntries";

static const char METHOD_REMOVECACHE[] = "removeCache";

static const char METHOD_BACKORFORWARD[] = "backOrForward";

static const char METHOD_GETTITLE[] = "getTitle";

static const char METHOD_GETPAGEHEIGHT[] = "getPageHeight";

static const char METHOD_CREATEWEBMESSAGEPORTS[] = "createWebMessagePorts";

static const char METHOD_POSTWEBMESSAGE[] = "postWebMessage";

static const char METHOD_CLOSEWEBMESSAGEPORT[] = "closeWebMessagePort";

static const char METHOD_POSTMESSAGEEVENT[] = "postMessageEvent";

static const char METHOD_ONWEBMESSAGEPORTEVENT[] = "onWebMessagePortEvent";

static const char SIGNATURE_LOADURL[] = "(JLjava/lang/String;Ljava/util/HashMap;)V";

static const char SIGNATURE_LOADDATA[] = "(JLjava/util/HashMap;)V";

static const char SIGNATURE_GETURL[] = "(J)Ljava/lang/String;";

static const char SIGNATURE_ACCESSFORWARD[] = "(J)Ljava/lang/String;";

static const char SIGNATURE_ACCESSBACKWARD[] = "(J)Ljava/lang/String;";

static const char SIGNATURE_FORWARD[] = "(J)V";

static const char SIGNATURE_BACKWARD[] = "(J)V";

static const char SIGNATURE_REFRESH[] = "(J)V";

static const char SIGNATURE_EVALUTEJS[] = "(JLjava/lang/String;J)V";

static const char SIGNATURE_GETBACKFORWARDENTRIES[] = "(J)Landroid/webkit/WebBackForwardList;";

static const char SIGNATURE_REMOVECACHE[] = "(JZ)V";

static const char SIGNATURE_BACKORFORWARD[] = "(JI)V";

static const char SIGNATURE_GETTITLE[] = "(J)Ljava/lang/String;";

static const char SIGNATURE_GETPAGEHEIGHT[] = "(J)I";

static const char SIGNATURE_CREATEWEBMESSAGEPORTS[] = "(J)[Ljava/lang/String;";

static const char SIGNATURE_POSTWEBMESSAGE[] = "(JLjava/lang/String;[Ljava/lang/String;Ljava/lang/String;)V";

static const char SIGNATURE_CLOSEWEBMESSAGEPORT[] = "(JLjava/lang/String;)V";

static const char SIGNATURE_POSTMESSAGEEVENT[] = "(JLjava/lang/String;Ljava/lang/String;)I";

static const char SIGNATURE_ONWEBMESSAGEPORTEVENT[] = "(JLjava/lang/String;)I";

static const char SIGNATURE_ACCESS_STEP[] = "(JI)Ljava/lang/String;";

static const char SIGNATURE_SCROLL_TO[] = "(JII)V";

static const char SIGNATURE_SCROLL_BY[] = "(JII)V";

static const char SIGNATURE_ZOOM[] = "(JF)V";

static const char SIGNATURE_STOP[] = "(J)V";

static const char SIGNATURE_CLEAR_HISTORY[] = "(J)V";

static const char SIGNATURE_SET_CUSTOM_USER_AGENT[] = "(JLjava/lang/String;)V";

static const char SIGNATURE_GET_CUSTOM_USER_AGENT[] = "(J)Ljava/lang/String;";

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
    jmethodID accessStep;
    jmethodID scrollTo;
    jmethodID scrollBy;
    jmethodID zoom;
    jmethodID stop;
    jmethodID clearHistory;
    jmethodID setCustomUserAgent;
    jmethodID getCustomUserAgent;
    jmethodID getBackForwardEntries;
    jmethodID removeCache;
    jmethodID backOrForward;
    jmethodID getTitle;
    jmethodID getPageHeight;
    jmethodID createWebMessagePorts;
    jmethodID postWebMessage;
    jmethodID closeWebMessagePort;
    jmethodID postMessageEvent;
    jmethodID onWebMessagePortEvent;
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
    g_webWebviewClass.accessStep = env->GetMethodID(cls, METHOD_ACCESS_STEP, SIGNATURE_ACCESS_STEP);
    g_webWebviewClass.scrollTo = env->GetMethodID(cls, METHOD_SCROLL_TO, SIGNATURE_SCROLL_TO);
    g_webWebviewClass.scrollBy = env->GetMethodID(cls, METHOD_SCROLL_BY, SIGNATURE_SCROLL_BY);
    g_webWebviewClass.zoom = env->GetMethodID(cls, METHOD_ZOOM, SIGNATURE_ZOOM);
    g_webWebviewClass.stop = env->GetMethodID(cls, METHOD_STOP, SIGNATURE_STOP);
    g_webWebviewClass.clearHistory = env->GetMethodID(cls, METHOD_CLEAR_HISTORY, SIGNATURE_CLEAR_HISTORY);
    g_webWebviewClass.setCustomUserAgent = env->GetMethodID(cls, METHOD_SET_CUSTOM_USER_AGENT, SIGNATURE_SET_CUSTOM_USER_AGENT);
    g_webWebviewClass.getCustomUserAgent = env->GetMethodID(cls, METHOD_GET_CUSTOM_USER_AGENT, SIGNATURE_GET_CUSTOM_USER_AGENT);
    g_webWebviewClass.getBackForwardEntries = env->GetMethodID(cls, METHOD_GETBACKFORWARDENTRIES, SIGNATURE_GETBACKFORWARDENTRIES);
    g_webWebviewClass.removeCache = env->GetMethodID(cls, METHOD_REMOVECACHE, SIGNATURE_REMOVECACHE);
    g_webWebviewClass.backOrForward = env->GetMethodID(cls, METHOD_BACKORFORWARD, SIGNATURE_BACKORFORWARD);
    g_webWebviewClass.getTitle = env->GetMethodID(cls, METHOD_GETTITLE, SIGNATURE_GETTITLE);
    g_webWebviewClass.getPageHeight = env->GetMethodID(cls, METHOD_GETPAGEHEIGHT, SIGNATURE_GETPAGEHEIGHT);
    g_webWebviewClass.createWebMessagePorts = env->GetMethodID(cls, METHOD_CREATEWEBMESSAGEPORTS, SIGNATURE_CREATEWEBMESSAGEPORTS);
    g_webWebviewClass.postWebMessage = env->GetMethodID(cls, METHOD_POSTWEBMESSAGE, SIGNATURE_POSTWEBMESSAGE);
    g_webWebviewClass.closeWebMessagePort = env->GetMethodID(cls, METHOD_CLOSEWEBMESSAGEPORT, SIGNATURE_CLOSEWEBMESSAGEPORT);
    g_webWebviewClass.postMessageEvent = env->GetMethodID(cls, METHOD_POSTMESSAGEEVENT, SIGNATURE_POSTMESSAGEEVENT);
    g_webWebviewClass.onWebMessagePortEvent = env->GetMethodID(cls, METHOD_ONWEBMESSAGEPORTEVENT, SIGNATURE_ONWEBMESSAGEPORTEVENT);
    env->DeleteLocalRef(cls);
}

void WebviewControllerJni::OnReceiveValue(JNIEnv* env, jclass jcls, jstring jResult, jint jId)
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
    WebviewController::OnReceiveValue(result, nativeId);
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

void WebviewControllerJni::EvaluateJavaScript(int id, const std::string& script, int32_t asyncCallbackInfoId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.evaluateJavascript)) {
        return;
    }

    jstring JsName = env->NewStringUTF(script.c_str());
    CHECK_NULL_VOID(JsName);
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.evaluateJavascript, id, JsName, asyncCallbackInfoId);
    env->DeleteLocalRef(JsName);
}

bool WebviewControllerJni::AccessStep(int id, int32_t step)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.accessStep)) {
        return false;
    }
    jstring jResult = static_cast<jstring>(
        env->CallObjectMethod(g_webWebviewClass.globalRef, g_webWebviewClass.accessStep, id, step));
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
    return result == "true";
}

ErrCode WebviewControllerJni::ScrollTo(int id, int x, int y)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, INIT_ERROR);

    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.scrollTo, id, x, y);
    if (env->ExceptionCheck()) {
        LOGE("WebviewControllerJni JNI: call ScrollTo has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INIT_ERROR;
    }
    return NO_ERROR;
}

ErrCode WebviewControllerJni::ScrollBy(int id, int x, int y)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, INIT_ERROR);

    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.scrollBy, id, x, y);
    if (env->ExceptionCheck()) {
        LOGE("WebviewControllerJni JNI: call ScrollBy has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INIT_ERROR;
    }
    return NO_ERROR;
}

ErrCode WebviewControllerJni::Zoom(int id, float factor)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, INIT_ERROR);

    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.zoom, id, factor);
    if (env->ExceptionCheck()) {
        LOGE("WebviewControllerJni JNI: call Zoom has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INIT_ERROR;
    }
    return NO_ERROR;
}

ErrCode WebviewControllerJni::Stop(int id)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, INIT_ERROR);

    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.stop, id);
    if (env->ExceptionCheck()) {
        LOGE("WebviewControllerJni JNI: call Stop has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INIT_ERROR;
    }
    return NO_ERROR;
}

ErrCode WebviewControllerJni::ClearHistory(int id)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, INIT_ERROR);

    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.clearHistory, id);
    if (env->ExceptionCheck()) {
        LOGE("WebviewControllerJni JNI: call ClearHistory has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INIT_ERROR;
    }
    return NO_ERROR;
}

ErrCode WebviewControllerJni::SetCustomUserAgent(int id, const std::string& userAgent)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, INIT_ERROR);

    jstring jsUserAgent = env->NewStringUTF(userAgent.c_str());
    CHECK_NULL_RETURN(jsUserAgent, INIT_ERROR);
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.setCustomUserAgent, id, jsUserAgent);
    if (env->ExceptionCheck()) {
        LOGE("WebviewControllerJni JNI: call SetCustomUserAgent has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INIT_ERROR;
    }
    return NO_ERROR;
}

std::string WebviewControllerJni::GetCustomUserAgent(int id)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.getCustomUserAgent)) {
        return "";
    }
    jstring jResult = static_cast<jstring>(
        env->CallObjectMethod(g_webWebviewClass.globalRef, g_webWebviewClass.getCustomUserAgent, id));
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

std::shared_ptr<WebHistoryList> WebviewControllerJni::GetBackForwardEntries(int id)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.getBackForwardEntries)) {
        return nullptr;
    }

    auto jsResult = static_cast<jobject>(
        env->CallObjectMethod(g_webWebviewClass.globalRef, g_webWebviewClass.getBackForwardEntries, id));
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }

    jclass cls = env->GetObjectClass(jsResult);
    CHECK_NULL_RETURN(cls, nullptr);
    jmethodID getCurrentIndexMethodID = env->GetMethodID(cls, "getCurrentIndex", "()I");
    jmethodID getSizeMethodID = env->GetMethodID(cls, "getSize", "()I");
    jmethodID getItemAtIndexMethodID = env->GetMethodID(cls, "getItemAtIndex", "(I)Landroid/webkit/WebHistoryItem;");
    CHECK_NULL_RETURN(getCurrentIndexMethodID, nullptr);
    CHECK_NULL_RETURN(getSizeMethodID, nullptr);
    CHECK_NULL_RETURN(getItemAtIndexMethodID, nullptr);
    std::shared_ptr<WebHistoryList> historyList = std::make_shared<WebHistoryList>();
    auto currentindex = static_cast<int32_t>(env->CallIntMethod(jsResult, getCurrentIndexMethodID));
    historyList->SetCurrentIndex(currentindex);
    auto size = static_cast<int32_t>(env->CallIntMethod(jsResult, getSizeMethodID));
    historyList->SetListSize(size);
    for (int32_t i = 0; i < size; ++i) {
        auto jHistoryItem = static_cast<jobject>(env->CallObjectMethod(jsResult, getItemAtIndexMethodID, i));
        CHECK_NULL_RETURN(jHistoryItem, nullptr);
        auto historyItemClass = env->GetObjectClass(jHistoryItem);
        CHECK_NULL_RETURN(historyItemClass, nullptr);
        jmethodID getUrlMethodID = env->GetMethodID(historyItemClass, "getUrl", "()Ljava/lang/String;");
        jmethodID getOriginalUrlMethodID = env->GetMethodID(historyItemClass, "getOriginalUrl", "()Ljava/lang/String;");
        jmethodID getTitleMethodID = env->GetMethodID(historyItemClass, "getTitle", "()Ljava/lang/String;");
        CHECK_NULL_RETURN(getUrlMethodID, nullptr);
        CHECK_NULL_RETURN(getOriginalUrlMethodID, nullptr);
        CHECK_NULL_RETURN(getTitleMethodID, nullptr);
        std::shared_ptr<WebHistoryItem> webHistoryItem = std::make_shared<WebHistoryItem>();
        // geturl
        jstring jResult = static_cast<jstring>(env->CallObjectMethod(jHistoryItem, getUrlMethodID));
        const char* url = env->GetStringUTFChars(jResult, nullptr);
        if (url != nullptr) {
            webHistoryItem->historyUrl = std::string(url);
            env->ReleaseStringUTFChars(jResult, url);
        }
        if (jResult != nullptr) {
            env->DeleteLocalRef(jResult);
            jResult = nullptr;
        }
        // getOriginalUrl
        jResult = static_cast<jstring>(env->CallObjectMethod(jHistoryItem, getOriginalUrlMethodID));
        const char* originalUrl = env->GetStringUTFChars(jResult, nullptr);
        if (originalUrl != nullptr) {
            webHistoryItem->historyRawUrl = std::string(originalUrl);
            env->ReleaseStringUTFChars(jResult, originalUrl);
        }
        if (jResult != nullptr) {
            env->DeleteLocalRef(jResult);
            jResult = nullptr;
        }
        // getTitle
        jResult = static_cast<jstring>(env->CallObjectMethod(jHistoryItem, getTitleMethodID));
        const char* title = env->GetStringUTFChars(jResult, nullptr);
        if (title != nullptr) {
            webHistoryItem->title = std::string(title);
            env->ReleaseStringUTFChars(jResult, title);
        }
        if (jResult != nullptr) {
            env->DeleteLocalRef(jResult);
            jResult = nullptr;
        }
        // intsert webHistoryItem
        historyList->InsertHistoryItem(webHistoryItem);
    }
    return historyList;
}

void WebviewControllerJni::RemoveCache(int id, bool value)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.removeCache)) {
        return;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.removeCache, id, value);
}

void WebviewControllerJni::BackOrForward(int id, int32_t step)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.backOrForward)) {
        return;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.backOrForward, id, step);
}

std::string WebviewControllerJni::GetTitle(int id)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.getTitle)) {
        return "";
    }
    jstring jResult = static_cast<jstring>(
        env->CallObjectMethod(g_webWebviewClass.globalRef, g_webWebviewClass.getTitle, id));
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

int32_t WebviewControllerJni::GetPageHeight(int id)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.getPageHeight)) {
        return -1;
    }

    auto height = static_cast<int32_t>(env->CallIntMethod(
        g_webWebviewClass.globalRef, g_webWebviewClass.getPageHeight, id));
    return height;
}

void WebviewControllerJni::CreateWebMessagePorts(int id, std::vector<std::string>& ports)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.createWebMessagePorts)) {
        return;
    }

    jstring jstr = nullptr;
    const char *str = nullptr;
    jobjectArray jReturnObjectArray = (jobjectArray)env->CallObjectMethod(g_webWebviewClass.globalRef,
        g_webWebviewClass.createWebMessagePorts, id);
    jsize arrayLength = env->GetArrayLength(jReturnObjectArray);
    for (int index = 0; index < arrayLength; index++) {
        jstr = static_cast<jstring>(env->GetObjectArrayElement(jReturnObjectArray, index));
        str = env->GetStringUTFChars(jstr, JNI_FALSE);
        ports.push_back(std::string(str));
        if (str != nullptr) {
            env->ReleaseStringUTFChars(jstr, str);
        }
        if (jstr != nullptr) {
            env->DeleteLocalRef(jstr);
        }
        str = nullptr;
    }
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
}

void WebviewControllerJni::PostWebMessage(int id, std::string& message, std::vector<std::string>& ports,
    std::string& targetUrl)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.postWebMessage)) {
        return;
    }
    if (ports.empty()) {
        LOGE("result is nullptr");
        return;
    }
    jclass jStringCls = env->FindClass("java/lang/String");
    if (!jStringCls) {
        LOGE("jStringCls is nullptr");
        return;
    }
    jstring jMessage = env->NewStringUTF(message.c_str());
    CHECK_NULL_VOID(jMessage);
    jobjectArray objectArray = env->NewObjectArray(ports.size(), jStringCls, nullptr);
    for (size_t index = 0; index < ports.size(); ++index) {
        jstring jstr = env->NewStringUTF(ports[index].c_str());
        if (!jstr) {
            env->DeleteLocalRef(jMessage);
            return;
        }
        env->SetObjectArrayElement(objectArray, index, jstr);
        env->DeleteLocalRef(jstr);
    }
    jstring jTargetUrl = env->NewStringUTF(targetUrl.c_str());
    if (!jTargetUrl) {
        env->DeleteLocalRef(jMessage);
        return;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.postWebMessage, id, jMessage, objectArray,
        jTargetUrl);
    env->DeleteLocalRef(jMessage);
    env->DeleteLocalRef(jTargetUrl);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }
}

void WebviewControllerJni::CloseWebMessagePort(int id, const std::string& portHandle)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.closeWebMessagePort)) {
        return;
    }
    jstring JPortHandle = env->NewStringUTF(portHandle.c_str());
    CHECK_NULL_VOID(JPortHandle);
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.closeWebMessagePort, id, JPortHandle);
    env->DeleteLocalRef(JPortHandle);
}

ErrCode WebviewControllerJni::PostMessageEvent(int id, const std::string& portHandle,
    const std::string& webMessage)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.postMessageEvent)) {
        return CAN_NOT_POST_MESSAGE;
    }
    jstring JPortHandle = env->NewStringUTF(portHandle.c_str());
    CHECK_NULL_RETURN(JPortHandle, CAN_NOT_POST_MESSAGE);
    jstring JWebMessage = env->NewStringUTF(webMessage.c_str());
    if (!JWebMessage) {
        env->DeleteLocalRef(JPortHandle);
        return CAN_NOT_POST_MESSAGE;
    }
    auto result = static_cast<ErrCode>(env->CallIntMethod(
        g_webWebviewClass.globalRef, g_webWebviewClass.postMessageEvent, id, JPortHandle, JWebMessage));
    env->DeleteLocalRef(JPortHandle);
    env->DeleteLocalRef(JWebMessage);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CAN_NOT_POST_MESSAGE;
    }
    return result;
}

ErrCode WebviewControllerJni::OnWebMessagePortEvent(int id, const std::string& portHandle)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.onWebMessagePortEvent)) {
        return CAN_NOT_REGISTER_MESSAGE_EVENT;
    }

    jstring JPortHandle = env->NewStringUTF(portHandle.c_str());
    CHECK_NULL_RETURN(JPortHandle, CAN_NOT_REGISTER_MESSAGE_EVENT);
    auto result = static_cast<ErrCode>(env->CallIntMethod(
        g_webWebviewClass.globalRef, g_webWebviewClass.onWebMessagePortEvent, id, JPortHandle));
    env->DeleteLocalRef(JPortHandle);

    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
        return CAN_NOT_REGISTER_MESSAGE_EVENT;
    }
    return result;
}

void WebviewControllerJni::OnMessage(JNIEnv* env, jclass jcls, jint jWebId, jstring jPortHandle, jstring jResult)
{
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(jPortHandle);
    CHECK_NULL_VOID(jResult);

    std::string portHandle;
    std::string result;
    const char* content = env->GetStringUTFChars(jPortHandle, nullptr);
    if (content != nullptr) {
        portHandle.assign(content);
        env->ReleaseStringUTFChars(jPortHandle, content);
    }
    if (jPortHandle != nullptr) {
        env->DeleteLocalRef(jPortHandle);
    }

    content = env->GetStringUTFChars(jResult, nullptr);
    if (content != nullptr) {
        result.assign(content);
        env->ReleaseStringUTFChars(jResult, content);
    }
    if (jResult != nullptr) {
        env->DeleteLocalRef(jResult);
    }

    int32_t webId = static_cast<int32_t>(jWebId);
    WebMessagePort::OnMessage(webId, portHandle, result);
}
}