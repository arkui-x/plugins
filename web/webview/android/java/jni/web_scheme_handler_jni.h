/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEB_SCHEME_HANDLER_JNI_H
#define PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEB_SCHEME_HANDLER_JNI_H

#include <jni.h>
#include <map>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "arkweb_net_error_list.h"
#include "web_errors.h"

namespace OHOS::Plugin {
struct ArkWeb_SchemeHandler;
struct ArkWeb_Response;
class ArkWeb_ResourceHandler;
typedef void (*ArkWeb_OnRequestStart)(struct ArkWeb_SchemeHandler* schemeHandler,
    struct ArkWeb_ResourceRequest* resourceRequest, ArkWeb_ResourceHandler* resourceHandler, bool* intercept);

typedef void (*ArkWeb_OnRequestStop)(
    struct ArkWeb_SchemeHandler* schemeHandler, struct ArkWeb_ResourceRequest* resourceRequest);
using WebHeaderList = std::vector<std::pair<std::string, std::string>>;

class WebSchemeHandlerJni final {
public:
    WebSchemeHandlerJni() = delete;
    ~WebSchemeHandlerJni() = delete;
    static bool Register(void* env);
    static void NativeInit(JNIEnv* env, jobject jobj);
    static bool OnSchemeHandlerRequestStart(JNIEnv* env, jclass jcls, jstring scheme, jobject schemeRequestHandler);
    static void OnSchemeHandlerRequestStop(JNIEnv* env, jclass jcls, jstring scheme, jobject schemeRequestHandler);
    static ArkWeb_SchemeHandler* CreateArkSchemeHandler();
    static void DeleteArkSchemeHandler(ArkWeb_SchemeHandler* handler);
    static void SetOnRequestStart(ArkWeb_SchemeHandler* handler, ArkWeb_OnRequestStart callback);
    static void SetOnRequestStop(ArkWeb_SchemeHandler* handler, ArkWeb_OnRequestStop callback);
    static void InsertSchemeRequestHandler(std::string& scheme, ArkWeb_SchemeHandler* handler);
    static void ClearSchemeRequestHandler();
    static bool GetIsIntercept();
    static void SetIsIntercept(bool intercept);

private:
    static ArkWeb_ResourceRequest* CreateArkResourceRequest(JNIEnv* env, jobject jRequest);
    static void DestroyArkResourceRequest(ArkWeb_ResourceRequest* request);
    static std::map<std::string, std::string> GetRequestHeader(JNIEnv* env, jobject javaObject);
    static std::string GetUTFString(JNIEnv* env, jstring jstr);
    static bool GetBooleanFromJava(JNIEnv* env, jclass clazz, jobject jobj, ArkWeb_ResourceRequest*& request,
        const char* methodName, bool& outBool);
    static bool GetStringFromJava(JNIEnv* env, jclass clazz, jobject jobj, ArkWeb_ResourceRequest*& request,
        const char* methodName, std::string& outStr);
    static jobject GetHeaderIterator(JNIEnv* env, jobject jHeadersMap);
    static void IterateAndFillHeaders(JNIEnv* env, jobject iterator, const jmethodID* methodIds,
    std::map<std::string, std::string>& headers);
    static bool GetIteratorAndEntryMethodIds(
        JNIEnv* env, jclass iteratorClass, jclass entryClass, jmethodID* methodIds);

public:
    static ArkWeb_SchemeHandler* handler_;
    static ArkWeb_ResourceRequest* resourceRequest_;
private:
    static std::map<std::string, ArkWeb_SchemeHandler*> g_schemeRequestHandlerMap_;
    static bool isIntercept_;
};

struct ArkWeb_ResourceRequest {
public:
    ArkWeb_ResourceRequest() = default;
    ~ArkWeb_ResourceRequest()
    {
        url_ = "";
        method_ = "";
        referrer_ = "";
        isRedirect_ = false;
        isMainFrame_ = false;
        hasGesture_ = false;
        headerList_.clear();
        requestResourceType_ = -1;
        frameUrl_ = "";
    };

public:
    std::string url_ = "";
    std::string method_ = "";
    std::string referrer_ = "";
    bool isRedirect_ = false;
    bool isMainFrame_ = false;
    bool hasGesture_ = false;
    WebHeaderList headerList_;
    int32_t requestResourceType_ = -1;
    std::string frameUrl_ = "";
};

class ArkWeb_ResourceHandler {
public:
    ArkWeb_ResourceHandler(JNIEnv* env, jobject schemeRequestHandler)
        : env_(env), schemeRequestHandler_(schemeRequestHandler)
    {}
    ~ArkWeb_ResourceHandler();

    ErrCode DidReceiveResponse(const ArkWeb_Response* response);
    ErrCode DidReceiveData(const uint8_t* buffer, int64_t buflen);
    ErrCode DidFinish();
    ErrCode DidFailWithError(ArkWeb_NetError errorCode, const std::string& errorDescription, bool completeIfNoResponse);
    void DestroyArkWebResourceHandler();
    void SetFinishFlag()
    {
        isFinished_ = true;
    }

private:
    jobject ConvertMapToJavaMap(JNIEnv* env, const std::unordered_map<std::string, std::string>& headersMap);
    ErrCode SetResponseUrl(const std::string& url);
    ErrCode SetResponseErrcode(ArkWeb_NetError errorCode, const std::string& errorDescription);

private:
    bool isFinished_ = false;
    JNIEnv* env_ = nullptr;
    jobject schemeRequestHandler_ = nullptr;
    ArkWeb_ResourceHandler* handler_ = nullptr;
};

struct ArkWeb_Response {
public:
    ArkWeb_Response() = default;
    ~ArkWeb_Response()
    {
        url_ = "";
        status_ = 0;
        statusText_ = "";
        mimeType_ = "";
        encoding_ = "";
        data_ = "";
        headers_.clear();
        errorCode_ = 0;
        errorDescription_ = "";
    }

    std::string url_ = "";
    int32_t status_ = 0;
    std::string statusText_ = "";
    std::string mimeType_ = "";
    std::string encoding_ = "";
    std::string data_ = "";
    std::unordered_map<std::string, std::string> headers_;
    int32_t errorCode_ = 0;
    std::string errorDescription_ = "";
};

struct ArkWeb_SchemeHandler {
    ArkWeb_OnRequestStart on_request_start;
    ArkWeb_OnRequestStop on_request_stop;
};
} // namespace OHOS::Plugin
#endif // PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEB_SCHEME_HANDLER_JNI_H
