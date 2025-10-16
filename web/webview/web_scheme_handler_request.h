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

#ifndef PLUGINS_WEB_WEBVIEW_WEB_SCHEME_HANDLER_REQUEST_H
#define PLUGINS_WEB_WEBVIEW_WEB_SCHEME_HANDLER_REQUEST_H

#include <memory>
#include <mutex>
#include <node_api.h>
#include <unordered_map>

#include "arkweb_error_code.h"
#include "arkweb_net_error_list.h"
#include "napi_parse_utils.h"
#ifdef IOS_PLATFORM
#include "AceWebSchemeHandler.h"
#endif
#ifdef ANDROID_PLATFORM
#include "android/java/jni/web_scheme_handler_jni.h"
#endif

namespace OHOS::Plugin {
using WebHeaderList = std::vector<std::pair<std::string, std::string>>;
class WebSchemeHandlerRequest {
public:
    explicit WebSchemeHandlerRequest(napi_env env);
    WebSchemeHandlerRequest(napi_env env, const ArkWeb_ResourceRequest* request);
    ~WebSchemeHandlerRequest();

public:
    std::string GetUrl() const;
    std::string GetMethod() const;
    std::string GetReferrer() const;
    bool GetIsRedirect() const;
    bool GetIsMainFrame() const;
    bool GetHasGesture() const;
    WebHeaderList GetHeaderList() const;
    int32_t GetRequestResourceType() const;
    std::string GetFrameUrl() const;
    void SetUrl(const std::string& url);
    void SetMethod(const std::string& method);
    void SetReferrer(const std::string& referrer);
    void SetIsRedirect(bool isRedirect);
    void SetIsMainFrame(bool isMainFrame);
    void SetHasGesture(bool hasGesture);
    void SetHeaderList(const WebHeaderList& headerList);
    void SetRequestResourceType(int32_t requestResourceType);
    void SetFrameUrl(const std::string& frameUrl);

private:
    std::string url_ = "";
    std::string method_ = "";
    std::string referrer_ = "";
    bool isRedirect_ = false;
    bool isMainFrame_ = false;
    bool hasGesture_ = false;
    WebHeaderList headerList_;
    int32_t requestResourceType_ = -1;
    std::string frameUrl_ = "";
    napi_env env_;
};

class WebSchemeHandlerResponse {
public:
    explicit WebSchemeHandlerResponse(napi_env env);
    WebSchemeHandlerResponse(napi_env env, ArkWeb_Response* response);
    ~WebSchemeHandlerResponse();

    std::string GetUrl() const;
    int32_t SetUrl(const std::string url);
    int32_t GetStatus() const;
    int32_t SetStatus(int32_t status);
    std::string GetStatusText() const;
    int32_t SetStatusText(const std::string statusText);
    std::string GetMimeType() const;
    int32_t SetMimeType(const std::string mimeType);
    std::string GetEncoding() const;
    int32_t SetEncoding(const std::string encoding);
    std::string GetHeaderByName(const std::string name);
    int32_t SetHeaderByName(const std::string name, const std::string value, bool overwrite);
    int32_t GetErrorCode() const;
    int32_t SetErrorCode(int32_t code, const std::string errorDescription);
    ArkWeb_Response* GetArkWebResponse()
    {
        return response_;
    }

private:
    napi_env env_;
    ArkWeb_Response* response_ = nullptr;
};

class WebResourceHandler {
public:
    explicit WebResourceHandler(napi_env env);
    WebResourceHandler(napi_env env, const ArkWeb_ResourceHandler* handler);
    ~WebResourceHandler();
    int32_t DidReceiveResponse(const ArkWeb_Response* response);
    int32_t DidReceiveResponseBody(const uint8_t* webResourceHandlerbuffer, int64_t bufferLen);
    int32_t DidFinish();
    int32_t DidFailWithError(ArkWeb_NetError errorCode, const std::string errorDescription, bool completeIfNoResponse);
    void DestroyArkWebResourceHandler();
    void SetFinishFlag()
    {
        isFinished_ = true;
    }

private:
    bool isFinished_ = false;
    ArkWeb_ResourceHandler* handler_ = nullptr;
    napi_env env_;
};

class WebSchemeHandler {
public:
    explicit WebSchemeHandler(napi_env env);
    ~WebSchemeHandler();
    napi_ref delegate_ = nullptr;
    static ArkWeb_SchemeHandler* GetArkWebSchemeHandler(WebSchemeHandler* handler);
    static WebSchemeHandler* GetWebSchemeHandler(ArkWeb_SchemeHandler* handler);
    static void ClearWebSchemeHandler();
    void RequestStart(ArkWeb_ResourceRequest* request, ArkWeb_ResourceHandler* ArkWeb_ResourceHandler, bool* intercept);
    void RequestStop(ArkWeb_ResourceRequest* resourceRequest);

    void PutRequestStart(napi_env, napi_value callback);
    void PutRequestStop(napi_env, napi_value callback);

private:
    napi_env env_;
    napi_ref request_start_callback_ = nullptr;
    napi_ref request_stop_callback_ = nullptr;
};
} // namespace OHOS::Plugin
#endif // PLUGINS_WEB_WEBVIEW_WEB_SCHEME_HANDLER_REQUEST_H