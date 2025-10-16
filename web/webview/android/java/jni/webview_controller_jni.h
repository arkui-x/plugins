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

#ifndef PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEBVIEW_CONTROLLER_JNI_H
#define PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEBVIEW_CONTROLLER_JNI_H

#include <jni.h>

#include "plugins/web/webview/android/java/jni/webview_controller_android.h"
#include "plugins/web/webview/web_message_ext.h"

namespace OHOS::Plugin {
class WebviewControllerJni final {
public:
    WebviewControllerJni() = delete;
    ~WebviewControllerJni() = delete;
    static bool Register(void* env);
    static void NativeInit(JNIEnv* env, jobject jobj);
    static void NativeInitStatic(JNIEnv* env, jobject jobj);
    static void CallStaticMethod(JNIEnv* env);
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
    static ErrCode Stop(int id);
    static std::string GetOriginalUrl(int id);
    static ErrCode PageUp(int id, bool top);
    static ErrCode ClearHistory(int id);
    static bool AccessStep(int id, int32_t step);
    static ErrCode ScrollTo(int id, int x, int y);
    static ErrCode ScrollBy(int id, int x, int y);
    static ErrCode Zoom(int id, float factor);
    static ErrCode ZoomIn(int id);
    static ErrCode ZoomOut(int id);
    static std::string GetCustomUserAgent(int id);
    static ErrCode SetCustomUserAgent(int id, const std::string& userAgent);
    static void EvaluateJavaScript(int id, const std::string& script, int32_t asyncCallbackInfoId);
    static void EvaluateJavaScriptExt(int id, const std::string& script, int32_t asyncCallbackInfoId);
    static void OnReceiveValue(JNIEnv* env, jclass jcls, jstring jResult, jint jId);
    static void OnReceiveRunJavaScriptExtValue(JNIEnv* env, jclass jcls, jstring jResult, jint jId);
    static std::shared_ptr<WebHistoryList> GetBackForwardEntries(int id);
    static void RemoveCache(int id, bool value);
    static void BackOrForward(int id, int32_t step);
    static std::string GetTitle(int id);
    static int32_t GetPageHeight(int id);
    static void CreateWebMessagePorts(int id, std::vector<std::string>& ports);
    static void PostWebMessage(int id, std::string& message, std::vector<std::string>& ports, std::string& targetUrl);
    static void StartDownload(int id, const std::string& url);
    static void CloseWebMessagePort(int id, const std::string& portHandle);
    static ErrCode PostMessageEvent(int id, const std::string& portHandle, const std::string& webMessage);
    static ErrCode PostMessageEventExt(int id, const std::string& portHandle, WebMessageExt* webMessageExt);
    static ErrCode OnWebMessagePortEvent(int id, const std::string& portHandle);
    static ErrCode OnWebMessagePortEventExt(int id, const std::string& portHandle);
    static void OnMessage(JNIEnv* env, jclass jcls, jint jWebId, jstring jPortHandle, jstring jResult);
    static void SetWebDebuggingAccess(bool webDebuggingAccess);
    static ErrCode PageDown(int id, bool bottom);
    static ErrCode PostUrl(int id, const std::string& url, const std::vector<uint8_t>& postData);
    static void OnMessageEventExt(JNIEnv* env, jclass jcls, jint jWebId, jstring jPortHandle, jstring jResult);
    static void RegisterJavaScriptProxy(int id, const std::string& objName,
        const std::vector<std::string>& syncMethodList, const std::vector<std::string>& asyncMethodList,
        const std::string& permission);
    static jobject OnReceiveJavascriptExecuteCall(
        JNIEnv* env, jclass jcls, jstring objName, jstring methodName, jobjectArray argsList);
    static void DeleteJavaScriptRegister(int id, const std::string& objName);
    static std::shared_ptr<Ace::WebJSValue> ProcessJavaObject(JNIEnv* env, jobject obj, int currentDepth = 0);
    static std::shared_ptr<Ace::WebJSValue> ProcessJavaList(JNIEnv* env, jobject obj, int currentDepth);
    static void ProcessMapEntries(JNIEnv* env, jobject iter, std::shared_ptr<Ace::WebJSValue> result, int currentDepth);
    static std::shared_ptr<Ace::WebJSValue> ProcessJavaMap(JNIEnv* env, jobject obj, int currentDepth);
    static std::shared_ptr<Ace::WebJSValue> ProcessJavaString(JNIEnv* env, jobject obj);
    static std::shared_ptr<Ace::WebJSValue> ProcessJavaInteger(JNIEnv* env, jobject obj);
    static std::shared_ptr<Ace::WebJSValue> ProcessJavaBoolean(JNIEnv* env, jobject obj);
    static std::shared_ptr<Ace::WebJSValue> ProcessJavaDouble(JNIEnv* env, jobject obj);
    static jobject ConvertWebToJava(JNIEnv* env, std::shared_ptr<Ace::WebJSValue>& ret);
    static jobject ConvertWebBooleanToJava(JNIEnv* env, std::shared_ptr<Ace::WebJSValue>& ret);
    static jobject ConvertWebIntegerToJava(JNIEnv* env, std::shared_ptr<Ace::WebJSValue>& ret);
    static jobject ConvertWebDoubleToJava(JNIEnv* env, std::shared_ptr<Ace::WebJSValue>& ret);
    static jobject ConvertWebStringToJava(JNIEnv* env, std::shared_ptr<Ace::WebJSValue>& ret);
    static jobject ConvertWebListToJava(JNIEnv* env, std::shared_ptr<Ace::WebJSValue>& ret);
    static jobject ConvertWebDictionaryToJava(JNIEnv* env, std::shared_ptr<Ace::WebJSValue>& ret);
    static bool SetWebSchemeHandler(int id, const std::string& scheme);
    static void ClearWebSchemeHandler(int id);
    static std::string GetUserAgent(int id);
};
}

#endif
