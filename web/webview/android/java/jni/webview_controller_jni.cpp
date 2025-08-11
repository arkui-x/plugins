/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "plugins/web/webview/napi_web_message_ext.h"

#define MAX_DEEPTH 10

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
namespace {
const char WEB_WEBVIEW_CLASS_NAME[] = "ohos/ace/adapter/capability/web/AceWebPluginBase";
static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(WebviewControllerJni::NativeInit) },
    { "nativeInitStatic", "()V", reinterpret_cast<void*>(WebviewControllerJni::NativeInitStatic) },
    { "onReceiveValue", "(Ljava/lang/String;J)V", reinterpret_cast<void*>(WebviewControllerJni::OnReceiveValue) },
    { "onReceiveRunJavaScriptExtValue", "(Ljava/lang/String;J)V",
        reinterpret_cast<void*>(WebviewControllerJni::OnReceiveRunJavaScriptExtValue) },
    { "onMessage", "(JLjava/lang/String;Ljava/lang/String;)V",
        reinterpret_cast<void*>(WebviewControllerJni::OnMessage) },
    { "onMessageEventExt", "(JLjava/lang/String;Ljava/lang/String;)V",
        reinterpret_cast<void*>(WebviewControllerJni::OnMessageEventExt) },
    { "onReceiveJavascriptExecuteCall",
        "(Ljava/lang/String;Ljava/lang/String;[Ljava/lang/Object;)Ljava/lang/Object;",
        reinterpret_cast<void*>(WebviewControllerJni::OnReceiveJavascriptExecuteCall) },
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

static const char METHOD_EVALUTEJS_EXT[] = "evaluateJavascriptExt";

static const char METHOD_ACCESS_STEP[] = "accessStep";

static const char METHOD_SCROLL_TO[] = "scrollTo";

static const char METHOD_SCROLL_BY[] = "scrollBy";

static const char METHOD_ZOOM[] = "zoom";

static const char METHOD_ZOOM_IN[] = "zoomIn";

static const char METHOD_ZOOM_OUT[] = "zoomOut";

static const char METHOD_GET_ZOOM_ACCESS[] = "getZoomAccess";

static const char METHOD_STOP[] = "stop";

static const char METHOD_GET_ORIGINAL_URL[] = "getOriginalUrl";

static const char METHOD_PAGEUP[] = "pageUp";

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

static const char METHOD_POSTMESSAGEEVENTEXT[] = "postMessageEventExt";

static const char METHOD_ONWEBMESSAGEPORTEVENT[] = "onWebMessagePortEvent";

static const char METHOD_SET_WEB_DEBUGGING_ACCESS[] = "setWebDebuggingAccess";

static const char METHOD_PAGEDOWN[] = "pageDown";

static const char METHOD_POSTURL[] = "postUrl";

static const char METHOD_START_DOWNLOAD[] = "startDownload";

static const char METHOD_ONWEBMESSAGEPORTEVENTEXT[] = "onWebMessagePortEventExt";

static const char METHOD_REGISTERJAVASCRIPTPROXY[] = "registerJavaScriptProxy";

static const char METHOD_DELETEJAVASCRIPTREGISTER[] = "deleteJavaScriptRegister";

static const char SIGNATURE_LOADURL[] = "(JLjava/lang/String;Ljava/util/HashMap;)V";

static const char SIGNATURE_LOADDATA[] = "(JLjava/util/HashMap;)V";

static const char SIGNATURE_GETURL[] = "(J)Ljava/lang/String;";

static const char SIGNATURE_ACCESSFORWARD[] = "(J)Ljava/lang/String;";

static const char SIGNATURE_ACCESSBACKWARD[] = "(J)Ljava/lang/String;";

static const char SIGNATURE_FORWARD[] = "(J)V";

static const char SIGNATURE_BACKWARD[] = "(J)V";

static const char SIGNATURE_REFRESH[] = "(J)V";

static const char SIGNATURE_EVALUTEJS[] = "(JLjava/lang/String;J)V";

static const char SIGNATURE_EVALUTEJS_EXT[] = "(JLjava/lang/String;J)V";

static const char SIGNATURE_GETBACKFORWARDENTRIES[] = "(J)Landroid/webkit/WebBackForwardList;";

static const char SIGNATURE_REMOVECACHE[] = "(JZ)V";

static const char SIGNATURE_BACKORFORWARD[] = "(JI)V";

static const char SIGNATURE_GETTITLE[] = "(J)Ljava/lang/String;";

static const char SIGNATURE_GETPAGEHEIGHT[] = "(J)I";

static const char SIGNATURE_CREATEWEBMESSAGEPORTS[] = "(J)[Ljava/lang/String;";

static const char SIGNATURE_POSTWEBMESSAGE[] = "(JLjava/lang/String;[Ljava/lang/String;Ljava/lang/String;)V";

static const char SIGNATURE_CLOSEWEBMESSAGEPORT[] = "(JLjava/lang/String;)V";

static const char SIGNATURE_POSTMESSAGEEVENT[] = "(JLjava/lang/String;Ljava/lang/String;)I";

static const char SIGNATURE_POSTMESSAGEEVENTEXT[] = "(JLjava/lang/String;Ljava/lang/String;)I";

static const char SIGNATURE_ONWEBMESSAGEPORTEVENT[] = "(JLjava/lang/String;)I";

static const char SIGNATURE_START_DOWNLOAD[] = "(JLjava/lang/String;)V";

static const char SIGNATURE_ONWEBMESSAGEPORTEVENTEXT[] = "(JLjava/lang/String;)I";

static const char SIGNATURE_ACCESS_STEP[] = "(JI)Ljava/lang/String;";

static const char SIGNATURE_SCROLL_TO[] = "(JII)V";

static const char SIGNATURE_SCROLL_BY[] = "(JII)V";

static const char SIGNATURE_ZOOM[] = "(JF)V";

static const char SIGNATURE_ZOOM_IN[] = "(J)V";

static const char SIGNATURE_GET_ZOOM_ACCESS[] = "()Z";

static const char SIGNATURE_ZOOM_OUT[] = "(J)V";

static const char SIGNATURE_STOP[] = "(J)V";

static const char SIGNATURE_GET_ORIGINAL_URL[] = "(J)Ljava/lang/String;";

static const char SIGNATURE_PAGEUP[] = "(JZ)V";

static const char SIGNATURE_CLEAR_HISTORY[] = "(J)V";

static const char SIGNATURE_SET_CUSTOM_USER_AGENT[] = "(JLjava/lang/String;)V";

static const char SIGNATURE_GET_CUSTOM_USER_AGENT[] = "(J)Ljava/lang/String;";

static const char SIGNATURE_SET_WEB_DEBUGGING_ACCESS[] = "(Z)V";

static const char SIGNATURE_PAGEDOWN[] = "(JZ)V";

static const char SIGNATURE_POSTURL[] = "(JLjava/lang/String;[B)V";

static const char SIGNATURE_REGISTERJAVASCRIPTPROXY[] =
    "(JLjava/lang/String;[Ljava/lang/String;[Ljava/lang/String;Ljava/lang/String;)V";

static const char SIGNATURE_DELETEJAVASCRIPTREGISTER[] = "(JLjava/lang/String;)V";

bool _webDebuggingAccessInit = false;

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
    jmethodID evaluateJavascriptExt;
    jmethodID accessStep;
    jmethodID scrollTo;
    jmethodID scrollBy;
    jmethodID zoom;
    jmethodID zoomIn;
    jmethodID zoomOut;
    jmethodID getZoomAccess;
    jmethodID stop;
    jmethodID getOriginalUrl;
    jmethodID pageUp;
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
    jmethodID postMessageEventExt;
    jmethodID onWebMessagePortEvent;
    jmethodID setWebDebuggingAccess;
    jmethodID pageDown;
    jmethodID postUrl;
    jmethodID startDownload;
    jmethodID onWebMessagePortEventExt;
    jmethodID registerJavaScriptProxy;
    jmethodID deleteJavaScriptRegister;
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
    g_webWebviewClass.evaluateJavascriptExt = env->GetMethodID(cls, METHOD_EVALUTEJS_EXT, SIGNATURE_EVALUTEJS_EXT);
    g_webWebviewClass.accessStep = env->GetMethodID(cls, METHOD_ACCESS_STEP, SIGNATURE_ACCESS_STEP);
    g_webWebviewClass.scrollTo = env->GetMethodID(cls, METHOD_SCROLL_TO, SIGNATURE_SCROLL_TO);
    g_webWebviewClass.scrollBy = env->GetMethodID(cls, METHOD_SCROLL_BY, SIGNATURE_SCROLL_BY);
    g_webWebviewClass.zoom = env->GetMethodID(cls, METHOD_ZOOM, SIGNATURE_ZOOM);
    g_webWebviewClass.zoomIn = env->GetMethodID(cls, METHOD_ZOOM_IN, SIGNATURE_ZOOM_IN);
    g_webWebviewClass.zoomOut = env->GetMethodID(cls, METHOD_ZOOM_OUT, SIGNATURE_ZOOM_OUT);
    g_webWebviewClass.getZoomAccess = env->GetMethodID(cls, METHOD_GET_ZOOM_ACCESS, SIGNATURE_GET_ZOOM_ACCESS);
    g_webWebviewClass.stop = env->GetMethodID(cls, METHOD_STOP, SIGNATURE_STOP);
    g_webWebviewClass.getOriginalUrl = env->GetMethodID(cls, METHOD_GET_ORIGINAL_URL, SIGNATURE_GET_ORIGINAL_URL);
    g_webWebviewClass.pageUp = env->GetMethodID(cls, METHOD_PAGEUP, SIGNATURE_PAGEUP);
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
    g_webWebviewClass.postMessageEventExt = env->GetMethodID(cls, METHOD_POSTMESSAGEEVENTEXT, SIGNATURE_POSTMESSAGEEVENTEXT);
    g_webWebviewClass.onWebMessagePortEvent = env->GetMethodID(cls, METHOD_ONWEBMESSAGEPORTEVENT, SIGNATURE_ONWEBMESSAGEPORTEVENT);
    g_webWebviewClass.setWebDebuggingAccess = env->GetMethodID(cls, METHOD_SET_WEB_DEBUGGING_ACCESS, SIGNATURE_SET_WEB_DEBUGGING_ACCESS);
    g_webWebviewClass.pageDown = env->GetMethodID(cls, METHOD_PAGEDOWN, SIGNATURE_PAGEDOWN);
    g_webWebviewClass.postUrl = env->GetMethodID(cls, METHOD_POSTURL, SIGNATURE_POSTURL);
    g_webWebviewClass.registerJavaScriptProxy =
        env->GetMethodID(cls, METHOD_REGISTERJAVASCRIPTPROXY, SIGNATURE_REGISTERJAVASCRIPTPROXY);
    g_webWebviewClass.deleteJavaScriptRegister =
        env->GetMethodID(cls, METHOD_DELETEJAVASCRIPTREGISTER, SIGNATURE_DELETEJAVASCRIPTREGISTER);
    env->DeleteLocalRef(cls);
}

void WebviewControllerJni::NativeInitStatic(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    g_webWebviewClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_webWebviewClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);
    g_webWebviewClass.setWebDebuggingAccess = env->GetMethodID(cls, METHOD_SET_WEB_DEBUGGING_ACCESS, SIGNATURE_SET_WEB_DEBUGGING_ACCESS);
    g_webWebviewClass.startDownload = env->GetMethodID(cls, METHOD_START_DOWNLOAD, SIGNATURE_START_DOWNLOAD);
    g_webWebviewClass.onWebMessagePortEventExt = env->GetMethodID(cls, METHOD_ONWEBMESSAGEPORTEVENTEXT, SIGNATURE_ONWEBMESSAGEPORTEVENTEXT);
    env->DeleteLocalRef(cls);
    WebviewControllerJni::CallStaticMethod(env);
}

void WebviewControllerJni::CallStaticMethod(JNIEnv* env)
{
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.setWebDebuggingAccess, _webDebuggingAccessInit);
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

void WebviewControllerJni::OnReceiveRunJavaScriptExtValue(JNIEnv* env, jclass jcls, jstring jResult, jint jId)
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
    WebviewController::OnReceiveRunJavaScriptExtValue("STRING", result, nativeId);
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

void WebviewControllerJni::EvaluateJavaScriptExt(int id, const std::string& script, int32_t asyncCallbackInfoId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.evaluateJavascriptExt)) {
        LOGE("WebviewControllerJni::EvaluateJavaScriptExt env error");
        return;
    }

    jstring JsName = env->NewStringUTF(script.c_str());
    CHECK_NULL_VOID(JsName);
    env->CallVoidMethod(
        g_webWebviewClass.globalRef, g_webWebviewClass.evaluateJavascriptExt, id, JsName, asyncCallbackInfoId);
    env->DeleteLocalRef(JsName);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
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

ErrCode WebviewControllerJni::ZoomIn(int id)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, INIT_ERROR);
    jboolean ret = env->CallBooleanMethod(g_webWebviewClass.globalRef, g_webWebviewClass.getZoomAccess, id);
    if (ret != JNI_TRUE) {
        return FUNCTION_NOT_ENABLE;
    }

    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.zoomIn, id);
    if (env->ExceptionCheck()) {
        LOGE("WebviewControllerJni JNI: call ZoomIN has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return INIT_ERROR;
    }
    return NO_ERROR;
}

ErrCode WebviewControllerJni::ZoomOut(int id)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, INIT_ERROR);
    jboolean ret = env->CallBooleanMethod(g_webWebviewClass.globalRef, g_webWebviewClass.getZoomAccess, id);
    if (ret != JNI_TRUE) {
        return FUNCTION_NOT_ENABLE;
    }

    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.zoomOut, id);
    if (env->ExceptionCheck()) {
        LOGE("WebviewControllerJni JNI: call ZoomOUT has exception");
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

std::string WebviewControllerJni::GetOriginalUrl(int id)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.getOriginalUrl)) {
        return "";
    }
    jstring jResult = static_cast<jstring>(
        env->CallObjectMethod(g_webWebviewClass.globalRef, g_webWebviewClass.getOriginalUrl, id));
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

ErrCode WebviewControllerJni::PageUp(int id, bool top)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.pageUp)) {
        return INIT_ERROR;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.pageUp, id, top);
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

ErrCode WebviewControllerJni::PageDown(int id, bool bottom)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.pageDown)) {
         return INIT_ERROR;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.pageDown, id, bottom);
     return NO_ERROR;
}

ErrCode WebviewControllerJni::PostUrl(int id, const std::string& url, const std::vector<uint8_t>& postData)
{
   auto env = ARKUI_X_Plugin_GetJniEnv();
   CHECK_NULL_RETURN(env, INIT_ERROR);
   jstring JsName = env->NewStringUTF(url.c_str());
   jbyteArray jPostData = env->NewByteArray(postData.size());
   if (jPostData == nullptr) {
       return INIT_ERROR;
   }
   env->SetByteArrayRegion(jPostData, 0, postData.size(),reinterpret_cast<const jbyte*>(postData.data()));
   env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.postUrl, id, JsName, jPostData);
   if (env->ExceptionCheck()) {
       env->ExceptionDescribe();
       env->ExceptionClear();
       return INIT_ERROR;
   }
   return NO_ERROR;
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

void WebviewControllerJni::StartDownload(int id, const std::string& url)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.startDownload)) {
        LOGE("env is null or method is null");
        return;
    }

    if (url.empty()) {
        LOGE("url is invalid");
        return;
    }

    jstring jStringUrl = env->NewStringUTF(url.c_str());
    if (!jStringUrl) {
        return;
    }
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.startDownload, id, jStringUrl);
    env->DeleteLocalRef(jStringUrl);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
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

ErrCode WebviewControllerJni::PostMessageEventExt(int id, const std::string& portHandle, WebMessageExt* webMessageExt)
{
    CHECK_NULL_RETURN(webMessageExt, CAN_NOT_POST_MESSAGE);
    if (portHandle.empty()) {
        return CAN_NOT_POST_MESSAGE;
    }

    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.postMessageEventExt)) {
        return CAN_NOT_POST_MESSAGE;
    }
    jstring jPortHandle = env->NewStringUTF(portHandle.c_str());
    CHECK_NULL_RETURN(jPortHandle, CAN_NOT_POST_MESSAGE);

    std::string resultString = "";
    if (webMessageExt->GetType() == static_cast<int32_t>(WebMessageType::STRING)) {
        resultString = webMessageExt->GetString();
    } else {
        return CAN_NOT_POST_MESSAGE;
    }

    if (resultString == "") {
        return CAN_NOT_POST_MESSAGE;
    }

    jstring jWebMessage = env->NewStringUTF(resultString.c_str());
    if (!jWebMessage) {
        env->DeleteLocalRef(jPortHandle);
        return CAN_NOT_POST_MESSAGE;
    }

    auto result = static_cast<ErrCode>(env->CallIntMethod(
        g_webWebviewClass.globalRef, g_webWebviewClass.postMessageEventExt, id, jPortHandle, jWebMessage));
    env->DeleteLocalRef(jPortHandle);
    env->DeleteLocalRef(jWebMessage);
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

ErrCode WebviewControllerJni::OnWebMessagePortEventExt(int id, const std::string& portHandle)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_webWebviewClass.globalRef) || !(g_webWebviewClass.onWebMessagePortEventExt)) {
        return CAN_NOT_REGISTER_MESSAGE_EVENT;
    }

    jstring JPortHandle = env->NewStringUTF(portHandle.c_str());
    CHECK_NULL_RETURN(JPortHandle, CAN_NOT_REGISTER_MESSAGE_EVENT);
    auto result = static_cast<ErrCode>(env->CallIntMethod(
        g_webWebviewClass.globalRef, g_webWebviewClass.onWebMessagePortEventExt, id, JPortHandle));
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

void WebviewControllerJni::SetWebDebuggingAccess(bool webDebuggingAccess)
{

    _webDebuggingAccessInit = webDebuggingAccess;
}

void WebviewControllerJni::OnMessageEventExt(
    JNIEnv* env, jclass jcls, jint jWebId, jstring jPortHandle, jstring jResult)
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
    env->DeleteLocalRef(jPortHandle);

    content = env->GetStringUTFChars(jResult, nullptr);
    if (content != nullptr) {
        result.assign(content);
        env->ReleaseStringUTFChars(jResult, content);
    }
    env->DeleteLocalRef(jResult);

    int32_t webId = static_cast<int32_t>(jWebId);
    auto webMessage = std::make_shared<WebMessage>(WebValue::Type::STRING);
    auto messageResult = std::make_shared<WebMessageExt>(webMessage);
    messageResult->SetString(result);
    WebMessagePort::OnMessageExt(webId, portHandle, messageResult);
}

void WebviewControllerJni::RegisterJavaScriptProxy(int id, const std::string& objName,
    const std::vector<std::string>& syncMethodList, const std::vector<std::string>& asyncMethodList,
    const std::string& permission)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_webWebviewClass.globalRef);
    CHECK_NULL_VOID(g_webWebviewClass.registerJavaScriptProxy);

    if (syncMethodList.empty() && asyncMethodList.empty()) {
        LOGE("Register JavaScript Proxy MethodList is nullptr");
        return;
    }
    jclass jStringCls = env->FindClass("java/lang/String");
    CHECK_NULL_VOID(jStringCls);
    jobjectArray objectArray = env->NewObjectArray(syncMethodList.size(), jStringCls, nullptr);
    for (size_t index = 0; index < syncMethodList.size(); ++index) {
        jstring jstr = env->NewStringUTF(syncMethodList[index].c_str());
        if (!jstr) {
            return;
        }
        env->SetObjectArrayElement(objectArray, index, jstr);
        env->DeleteLocalRef(jstr);
    }
    jobjectArray asyncObjectArray = env->NewObjectArray(asyncMethodList.size(), jStringCls, nullptr);
    for (size_t index = 0; index < asyncMethodList.size(); ++index) {
        jstring jstr = env->NewStringUTF(asyncMethodList[index].c_str());
        if (!jstr) {
            return;
        }
        env->SetObjectArrayElement(asyncObjectArray, index, jstr);
        env->DeleteLocalRef(jstr);
    }
    jstring jsObjName = env->NewStringUTF(objName.c_str());
    jstring jsPermission = env->NewStringUTF(permission.c_str());
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.registerJavaScriptProxy, id, jsObjName,
        objectArray, asyncObjectArray, jsPermission);
    env->DeleteLocalRef(objectArray);
    env->DeleteLocalRef(asyncObjectArray);
    env->DeleteLocalRef(jsObjName);
    env->DeleteLocalRef(jsPermission);
    env->DeleteLocalRef(jStringCls);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

jobject WebviewControllerJni::OnReceiveJavascriptExecuteCall(
    JNIEnv* env, jclass jcls, jstring className, jstring methodName, jobjectArray argsList)
{
    CHECK_NULL_RETURN(env, nullptr);
    CHECK_NULL_RETURN(className, nullptr);
    CHECK_NULL_RETURN(methodName, nullptr);
    CHECK_NULL_RETURN(argsList, nullptr);
    auto getUTFString = [env](jstring jstr) -> std::string {
        const char* pString = env->GetStringUTFChars(jstr, nullptr);
        CHECK_NULL_RETURN(pString, "");
        std::string str(pString);
        env->ReleaseStringUTFChars(jstr, pString);
        return str;
    };
    std::string strClassName = getUTFString(className);
    CHECK_NULL_RETURN(!strClassName.empty(), nullptr);
    std::string strMethodName = getUTFString(methodName);
    CHECK_NULL_RETURN(!strMethodName.empty(), nullptr);
    std::vector<std::shared_ptr<Ace::WebJSValue>> argsValue;
    jsize length = env->GetArrayLength(argsList);
    for (jsize i = 0; i < length; i++) {
        jobject element = env->GetObjectArrayElement(argsList, i);
        std::shared_ptr<Ace::WebJSValue> resultValue = WebviewControllerJni::ProcessJavaObject(env, element);
        CHECK_NULL_RETURN(resultValue, nullptr);
        argsValue.emplace_back(resultValue);
        env->DeleteLocalRef(element);
    }
    std::shared_ptr<Ace::WebJSValue> ret =
        WebviewController::OnReceiveJavascriptExecuteCall(strClassName, strMethodName, argsValue);
    CHECK_NULL_RETURN(ret, nullptr);
    return WebviewControllerJni::ConvertWebToJava(env, ret);
}

jobject WebviewControllerJni::ConvertWebToJava(JNIEnv* env, std::shared_ptr<Ace::WebJSValue>& ret)
{
    CHECK_NULL_RETURN(env, nullptr);
    CHECK_NULL_RETURN(ret, nullptr);
    jobject javaValue = nullptr;
    if (ret->IsString()) {
        javaValue = WebviewControllerJni::ConvertWebStringToJava(env, ret);
    } else if (ret->IsINTEGER()) {
        javaValue = WebviewControllerJni::ConvertWebIntegerToJava(env, ret);
    } else if (ret->IsBoolean()) {
        javaValue = WebviewControllerJni::ConvertWebBooleanToJava(env, ret);
    } else if (ret->IsDouble()) {
        javaValue = WebviewControllerJni::ConvertWebDoubleToJava(env, ret);
    } else if (ret->IsList()) {
        javaValue = WebviewControllerJni::ConvertWebListToJava(env, ret);
    } else if (ret->IsDictionary()) {
        javaValue = WebviewControllerJni::ConvertWebDictionaryToJava(env, ret);
    }
    return javaValue;
}

jobject WebviewControllerJni::ConvertWebDictionaryToJava(JNIEnv* env, std::shared_ptr<Ace::WebJSValue>& ret)
{
    CHECK_NULL_RETURN(env, nullptr);
    CHECK_NULL_RETURN(ret, nullptr);
    jclass mapClass = env->FindClass("java/util/HashMap");
    CHECK_NULL_RETURN(mapClass, nullptr);
    jmethodID mapConstructor = env->GetMethodID(mapClass, "<init>", "()V");
    if (mapConstructor == nullptr) {
        env->DeleteLocalRef(mapClass);
        return nullptr;
    }
    jmethodID putMethod = env->GetMethodID(mapClass, "put", "(Ljava/lang/Object;Ljava/lang/Object;)Ljava/lang/Object;");
    jobject javaMap = env->NewObject(mapClass, mapConstructor);
    if (putMethod != nullptr && javaMap != nullptr) {
        std::map<std::string, Ace::WebJSValue> mapValue = ret->GetDictionaryValue();
        for (auto& [key, value]: mapValue) {
            auto ptr = std::make_shared<Ace::WebJSValue>(value);
            if (ptr != nullptr) {
                jobject javaValue = WebviewControllerJni::ConvertWebToJava(env, ptr);
                jstring javaKey = env->NewStringUTF(key.c_str());
                env->CallObjectMethod(javaMap, putMethod, javaKey, javaValue);
                env->DeleteLocalRef(javaValue);
                env->DeleteLocalRef(javaKey);
            }
        }
    }
    env->DeleteLocalRef(mapClass);
    return javaMap;
}

jobject WebviewControllerJni::ConvertWebListToJava(JNIEnv* env, std::shared_ptr<Ace::WebJSValue>& ret)
{
    CHECK_NULL_RETURN(env, nullptr);
    CHECK_NULL_RETURN(ret, nullptr);
    jclass listClass = env->FindClass("java/util/ArrayList");
    CHECK_NULL_RETURN(listClass, nullptr);
    jmethodID addMethod = env->GetMethodID(listClass, "add", "(Ljava/lang/Object;)Z");
    jmethodID listConstructor = env->GetMethodID(listClass, "<init>", "()V");
    if (listConstructor == nullptr || addMethod == nullptr) {
        env->DeleteLocalRef(listClass);
        return nullptr;
    }
    jobject javaList = env->NewObject(listClass, listConstructor);
    std::vector<Ace::WebJSValue> listValue = ret->GetListValue();
    for (Ace::WebJSValue& item: listValue) {
        auto ptr = std::make_shared<Ace::WebJSValue>(item);
        if (ptr != nullptr) {
            jobject element = WebviewControllerJni::ConvertWebToJava(env, ptr);
            env->CallBooleanMethod(javaList, addMethod, element);
            env->DeleteLocalRef(element);
        }
    }
    env->DeleteLocalRef(listClass);
    return javaList;
}

jobject WebviewControllerJni::ConvertWebBooleanToJava(JNIEnv* env, std::shared_ptr<Ace::WebJSValue>& ret)
{
    CHECK_NULL_RETURN(env, nullptr);
    CHECK_NULL_RETURN(ret, nullptr);
    jclass booleanClass = env->FindClass("java/lang/Boolean");
    CHECK_NULL_RETURN(booleanClass, nullptr);
    jmethodID booleanConstructor = env->GetMethodID(booleanClass, "<init>", "(Z)V");
    if (booleanConstructor == nullptr) {
        env->DeleteLocalRef(booleanClass);
        return nullptr;
    }
    bool value = ret->GetBoolean();
    jobject result = env->NewObject(booleanClass, booleanConstructor, value);
    env->DeleteLocalRef(booleanClass);
    return result;
}

jobject WebviewControllerJni::ConvertWebIntegerToJava(JNIEnv* env, std::shared_ptr<Ace::WebJSValue>& ret)
{
    CHECK_NULL_RETURN(env, nullptr);
    CHECK_NULL_RETURN(ret, nullptr);
    jclass intClass = env->FindClass("java/lang/Integer");
    CHECK_NULL_RETURN(intClass, nullptr);
    jmethodID intConstructor = env->GetMethodID(intClass, "<init>", "(I)V");
    if (intConstructor == nullptr) {
        env->DeleteLocalRef(intClass);
        return nullptr;
    }
    std::int32_t value = ret->GetInt();
    jobject result = env->NewObject(intClass, intConstructor, value);
    env->DeleteLocalRef(intClass);
    return result;
}

jobject WebviewControllerJni::ConvertWebDoubleToJava(JNIEnv* env, std::shared_ptr<Ace::WebJSValue>& ret)
{
    CHECK_NULL_RETURN(env, nullptr);
    CHECK_NULL_RETURN(ret, nullptr);
    jclass doubleClass = env->FindClass("java/lang/Double");
    CHECK_NULL_RETURN(doubleClass, nullptr);
    jmethodID doubleConstructor = env->GetMethodID(doubleClass, "<init>", "(D)V");
    if (doubleConstructor == nullptr) {
        env->DeleteLocalRef(doubleClass);
        return nullptr;
    }
    double value = ret->GetDouble();
    jobject result = env->NewObject(doubleClass, doubleConstructor, value);
    env->DeleteLocalRef(doubleClass);
    return result;
}

jobject WebviewControllerJni::ConvertWebStringToJava(JNIEnv* env, std::shared_ptr<Ace::WebJSValue>& ret)
{
    CHECK_NULL_RETURN(env, nullptr);
    CHECK_NULL_RETURN(ret, nullptr);
    std::string str = ret->GetString();
    return env->NewStringUTF(str.c_str());
}

std::shared_ptr<Ace::WebJSValue> WebviewControllerJni::ProcessJavaObject(JNIEnv* env, jobject obj, int currentDepth)
{
    auto result = std::make_shared<Ace::WebJSValue>();
    CHECK_NULL_RETURN(result, nullptr);
    CHECK_NULL_RETURN(env, result);
    CHECK_NULL_RETURN(obj, result);
    if (currentDepth > MAX_DEEPTH) {
        return result;
    }

    static std::once_flag flag;
    static jclass listClass;
    static jclass mapClass;
    static jclass stringClass;
    static jclass intClass;
    static jclass boolClass;
    static jclass doubleClass;

    std::call_once(flag, [env]() {
        auto createGlobalRef = [env](const char* className) {
            jclass localClass = env->FindClass(className);
            jclass globalClass = static_cast<jclass>(env->NewGlobalRef(localClass));
            env->DeleteLocalRef(localClass);
            return globalClass;
        };

        listClass = createGlobalRef("java/util/List");
        mapClass = createGlobalRef("java/util/Map");
        stringClass = createGlobalRef("java/lang/String");
        intClass = createGlobalRef("java/lang/Integer");
        boolClass = createGlobalRef("java/lang/Boolean");
        doubleClass = createGlobalRef("java/lang/Double");
    });

    if (env->IsInstanceOf(obj, listClass)) {
        result = ProcessJavaList(env, obj, currentDepth + 1);
    } else if (env->IsInstanceOf(obj, mapClass)) {
        result = ProcessJavaMap(env, obj, currentDepth + 1);
    } else if (env->IsInstanceOf(obj, stringClass)) {
        result = ProcessJavaString(env, obj);
    } else if (env->IsInstanceOf(obj, intClass)) {
        result = ProcessJavaInteger(env, obj);
    } else if (env->IsInstanceOf(obj, boolClass)) {
        result = ProcessJavaBoolean(env, obj);
    } else if (env->IsInstanceOf(obj, doubleClass)) {
        result = ProcessJavaDouble(env, obj);
    }
    return result;
}

std::shared_ptr<Ace::WebJSValue> WebviewControllerJni::ProcessJavaList(JNIEnv* env, jobject obj, int currentDepth)
{
    auto result = std::make_shared<Ace::WebJSValue>(Ace::WebJSValue::Type::LIST);
    CHECK_NULL_RETURN(result, nullptr);
    CHECK_NULL_RETURN(env, result);
    CHECK_NULL_RETURN(obj, result);
    if (currentDepth > MAX_DEEPTH) {
        result = std::make_shared<Ace::WebJSValue>(Ace::WebJSValue::Type::NONE);
        return result;
    }
    jclass listClass = env->GetObjectClass(obj);
    CHECK_NULL_RETURN(listClass, result);
    jmethodID sizeMethod = env->GetMethodID(listClass, "size", "()I");
    jmethodID getMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
    if (sizeMethod == nullptr || getMethod == nullptr) {
        env->DeleteLocalRef(listClass);
        return result;
    }
    const jint size = env->CallIntMethod(obj, sizeMethod);
    for (jint i = 0; i < size; ++i) {
        jobject element = env->CallObjectMethod(obj, getMethod, i);
        if (element != nullptr) {
            std::shared_ptr<Ace::WebJSValue> valuePtr =
                WebviewControllerJni::ProcessJavaObject(env, element, currentDepth);
            if (valuePtr != nullptr) {
                result->AddListValue(*valuePtr);
            }
            env->DeleteLocalRef(element);
        } else {
            Ace::WebJSValue nullValue(Ace::WebJSValue::Type::NONE);
            result->AddListValue(nullValue);
        }
    }
    env->DeleteLocalRef(listClass);
    return result;
}

void WebviewControllerJni::ProcessMapEntries(
    JNIEnv* env, jobject iter, std::shared_ptr<Ace::WebJSValue> result, int currentDepth)
{
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(iter);
    CHECK_NULL_VOID(result);
    jclass iteratorClass = env->GetObjectClass(iter);
    CHECK_NULL_VOID(iteratorClass);
    jclass entryClass = env->FindClass("java/util/Map$Entry");
    if (entryClass == nullptr) {
        env->DeleteLocalRef(iteratorClass);
        return;
    }
    jmethodID hasNext = env->GetMethodID(iteratorClass, "hasNext", "()Z");
    jmethodID next = env->GetMethodID(iteratorClass, "next", "()Ljava/lang/Object;");
    jmethodID getKey = env->GetMethodID(entryClass, "getKey", "()Ljava/lang/Object;");
    jmethodID getValue = env->GetMethodID(entryClass, "getValue", "()Ljava/lang/Object;");
    if (hasNext != nullptr && next != nullptr && getKey != nullptr && getValue != nullptr) {
        while (env->CallBooleanMethod(iter, hasNext)) {
            jobject entry = env->CallObjectMethod(iter, next);
            jstring key = static_cast<jstring>(env->CallObjectMethod(entry, getKey));
            jobject value = env->CallObjectMethod(entry, getValue);
            const char* keyStr = env->GetStringUTFChars(key, nullptr);
            std::shared_ptr<Ace::WebJSValue> valuePtr = ProcessJavaObject(env, value, currentDepth);
            if (keyStr != nullptr && valuePtr != nullptr) {
                result->AddDictionaryValue(keyStr, *valuePtr);
            }
            env->ReleaseStringUTFChars(key, keyStr);
            env->DeleteLocalRef(entry);
            env->DeleteLocalRef(key);
            env->DeleteLocalRef(value);
        }
    }
    env->DeleteLocalRef(iteratorClass);
    env->DeleteLocalRef(entryClass);
}

std::shared_ptr<Ace::WebJSValue> WebviewControllerJni::ProcessJavaMap(JNIEnv* env, jobject obj, int currentDepth)
{
    auto result = std::make_shared<Ace::WebJSValue>(Ace::WebJSValue::Type::DICTIONARY);
    CHECK_NULL_RETURN(result, nullptr);
    CHECK_NULL_RETURN(env, result);
    CHECK_NULL_RETURN(obj, result);
    if (currentDepth > MAX_DEEPTH) {
        auto result = std::make_shared<Ace::WebJSValue>(Ace::WebJSValue::Type::NONE);
        return result;
    }
    jclass mapClass = env->GetObjectClass(obj);
    CHECK_NULL_RETURN(mapClass, result);
    jmethodID entrySet = env->GetMethodID(mapClass, "entrySet", "()Ljava/util/Set;");
    jobject set = env->CallObjectMethod(obj, entrySet);
    if (set == nullptr) {
        env->DeleteLocalRef(mapClass);
        return result;
    }
    jclass setClass = env->GetObjectClass(set);
    if (setClass != nullptr) {
        jmethodID iteratorMethod = env->GetMethodID(setClass, "iterator", "()Ljava/util/Iterator;");
        jobject iter = env->CallObjectMethod(set, iteratorMethod);
        ProcessMapEntries(env, iter, result, currentDepth);
        env->DeleteLocalRef(iter);
    }
    env->DeleteLocalRef(mapClass);
    env->DeleteLocalRef(set);
    env->DeleteLocalRef(setClass);
    return result;
}

std::shared_ptr<Ace::WebJSValue> WebviewControllerJni::ProcessJavaString(JNIEnv* env, jobject obj)
{
    auto result = std::make_shared<Ace::WebJSValue>();
    CHECK_NULL_RETURN(result, nullptr);
    CHECK_NULL_RETURN(env, result);
    CHECK_NULL_RETURN(obj, result);
    result->SetType(Ace::WebJSValue::Type::STRING);
    jstring str = static_cast<jstring>(obj);
    const char* utf = env->GetStringUTFChars(str, nullptr);
    if (utf != nullptr) {
        result->SetString(utf);
    }
    env->ReleaseStringUTFChars(str, utf);
    return result;
}

std::shared_ptr<Ace::WebJSValue> WebviewControllerJni::ProcessJavaInteger(JNIEnv* env, jobject obj)
{
    auto result = std::make_shared<Ace::WebJSValue>();
    CHECK_NULL_RETURN(result, nullptr);
    CHECK_NULL_RETURN(env, result);
    CHECK_NULL_RETURN(obj, result);
    result->SetType(Ace::WebJSValue::Type::INTEGER);
    jclass cls = env->GetObjectClass(obj);
    CHECK_NULL_RETURN(cls, result);
    jmethodID method = env->GetMethodID(cls, "intValue", "()I");
    if (method != nullptr) {
        result->SetInt(env->CallIntMethod(obj, method));
    }
    env->DeleteLocalRef(cls);
    return result;
}

std::shared_ptr<Ace::WebJSValue> WebviewControllerJni::ProcessJavaBoolean(JNIEnv* env, jobject obj)
{
    auto result = std::make_shared<Ace::WebJSValue>();
    CHECK_NULL_RETURN(result, nullptr);
    CHECK_NULL_RETURN(env, result);
    CHECK_NULL_RETURN(obj, result);
    result->SetType(Ace::WebJSValue::Type::BOOLEAN);
    jclass cls = env->GetObjectClass(obj);
    CHECK_NULL_RETURN(cls, result);
    jmethodID method = env->GetMethodID(cls, "booleanValue", "()Z");
    if (method != nullptr) {
        result->SetBoolean(env->CallBooleanMethod(obj, method));
    }
    env->DeleteLocalRef(cls);
    return result;
}

std::shared_ptr<Ace::WebJSValue> WebviewControllerJni::ProcessJavaDouble(JNIEnv* env, jobject obj)
{
    auto result = std::make_shared<Ace::WebJSValue>();
    CHECK_NULL_RETURN(result, nullptr);
    CHECK_NULL_RETURN(env, result);
    CHECK_NULL_RETURN(obj, result);
    result->SetType(Ace::WebJSValue::Type::DOUBLE);
    jclass cls = env->GetObjectClass(obj);
    CHECK_NULL_RETURN(cls, result);
    jmethodID method = env->GetMethodID(cls, "doubleValue", "()D");
    if (method != nullptr) {
        result->SetDouble(env->CallDoubleMethod(obj, method));
    }
    env->DeleteLocalRef(cls);
    return result;
}

void WebviewControllerJni::DeleteJavaScriptRegister(int id, const std::string& objName)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_webWebviewClass.globalRef);
    CHECK_NULL_VOID(g_webWebviewClass.deleteJavaScriptRegister);
    jstring jsObjName = env->NewStringUTF(objName.c_str());
    env->CallVoidMethod(g_webWebviewClass.globalRef, g_webWebviewClass.deleteJavaScriptRegister, id, jsObjName);
    env->DeleteLocalRef(jsObjName);
    if (env->ExceptionCheck()) {
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}
}