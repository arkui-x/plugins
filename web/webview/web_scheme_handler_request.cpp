/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "web_scheme_handler_request.h"

#include <uv.h>

#include "log.h"
#include "napi_parse_utils.h"
#include "napi_web_scheme_handler_request.h"
#include "web_errors.h"

namespace OHOS::Plugin {

struct RequestStopParam {
    napi_env env_;
    napi_ref callbackRef_;
    WebSchemeHandlerRequest* request_;
    const ArkWeb_ResourceRequest* arkWebRequest_;
};

static void RequestStopAfterWorkCb(uv_work_t* work, int status);

std::unordered_map<WebSchemeHandler*, ArkWeb_SchemeHandler*> g_web_scheme_handler_map;
std::unordered_map<ArkWeb_SchemeHandler*, WebSchemeHandler*> g_arkui_web_scheme_handler_map;
std::mutex g_mutexForHandlerMap;

void OnRequestStart(ArkWeb_SchemeHandler* schemeHandler, ArkWeb_ResourceRequest* resourceRequest,
    ArkWeb_ResourceHandler* resourceHandler, bool* intercept)
{
    WebSchemeHandler* handler = WebSchemeHandler::GetWebSchemeHandler(schemeHandler);
    if (handler == nullptr) {
        return;
    }
    handler->RequestStart(resourceRequest, resourceHandler, intercept);
}

void OnRequestStop(ArkWeb_SchemeHandler* schemeHandler, ArkWeb_ResourceRequest* resourceRequest)
{
    WebSchemeHandler* handler = WebSchemeHandler::GetWebSchemeHandler(schemeHandler);
    if (handler == nullptr) {
        return;
    }
    handler->RequestStop(resourceRequest);
}

WebResourceHandler::WebResourceHandler(napi_env env) : env_(env), handler_(nullptr) {}

WebResourceHandler::WebResourceHandler(napi_env env, const ArkWeb_ResourceHandler* handler)
    : env_(env), handler_(const_cast<ArkWeb_ResourceHandler*>(handler))
{}

WebResourceHandler::~WebResourceHandler() {}

int32_t WebResourceHandler::DidReceiveResponse(const ArkWeb_Response* response)
{
    if (isFinished_) {
        return ARKWEB_ERROR_UNKNOWN;
    }

    if (!response || !handler_) {
        return ARKWEB_INVALID_PARAM;
    }

    if (handler_->DidReceiveResponse(response) != NWebError::NO_ERROR) {
        return ARKWEB_INVALID_PARAM;
    }
    return ARKWEB_NET_OK;
}

int32_t WebResourceHandler::DidReceiveResponseBody(const uint8_t* webResourceHandlerbuffer, int64_t bufferLen)
{
    if (isFinished_) {
        LOGI("WebResourceHandler::DidReceiveResponseBody already finished");
        return ARKWEB_ERROR_UNKNOWN;
    }
    if (!handler_) {
        LOGE("WebResourceHandler::DidReceiveResponseBody handler_ is null");
        return ARKWEB_INVALID_PARAM;
    }
    if (handler_->DidReceiveData(webResourceHandlerbuffer, bufferLen) != NWebError::NO_ERROR) {
        return ARKWEB_INVALID_PARAM;
    }
    return ARKWEB_NET_OK;
}

int32_t WebResourceHandler::DidFinish()
{
    if (isFinished_) {
        return ARKWEB_ERROR_UNKNOWN;
    }
    if (!handler_) {
        return ARKWEB_INVALID_PARAM;
    }
    if (handler_->DidFinish() != NWebError::NO_ERROR) {
        return ARKWEB_INVALID_PARAM;
    }
    isFinished_ = true;
    return ARKWEB_NET_OK;
}

int32_t WebResourceHandler::DidFailWithError(
    ArkWeb_NetError errorCode, const std::string errorDescription, bool completeIfNoResponse)
{
    if (isFinished_) {
        return ArkWeb_ErrorCode::ARKWEB_ERROR_UNKNOWN;
    }
    if (handler_->DidFailWithError(errorCode, errorDescription, completeIfNoResponse) != NWebError::NO_ERROR) {
        return ARKWEB_INVALID_PARAM;
    }
    isFinished_ = true;
    return ARKWEB_NET_OK;
}

void WebResourceHandler::DestroyArkWebResourceHandler()
{
    if (handler_) {
        handler_->DestroyArkWebResourceHandler();
        handler_ = nullptr;
    }
}

WebSchemeHandlerRequest::WebSchemeHandlerRequest(napi_env env) : env_(env) {}

WebSchemeHandlerRequest::WebSchemeHandlerRequest(napi_env env, const ArkWeb_ResourceRequest* request)
{
    env_ = env;
    if (request) {
        url_ = request->url_;
        method_ = request->method_;
        referrer_ = request->referrer_;
        isRedirect_ = request->isRedirect_;
        isMainFrame_ = request->isMainFrame_;
        hasGesture_ = request->hasGesture_;
        headerList_ = request->headerList_;
        requestResourceType_ = request->requestResourceType_;
        frameUrl_ = request->frameUrl_;
    }
}

WebSchemeHandlerRequest::~WebSchemeHandlerRequest() {}

std::string WebSchemeHandlerRequest::GetUrl() const
{
    return url_;
}

std::string WebSchemeHandlerRequest::GetMethod() const
{
    return method_;
}
std::string WebSchemeHandlerRequest::GetReferrer() const
{
    return referrer_;
}

bool WebSchemeHandlerRequest::GetIsRedirect() const
{
    return isRedirect_;
}

bool WebSchemeHandlerRequest::GetIsMainFrame() const
{
    return isMainFrame_;
}

bool WebSchemeHandlerRequest::GetHasGesture() const
{
    return hasGesture_;
}

WebHeaderList WebSchemeHandlerRequest::GetHeaderList() const
{
    return headerList_;
}

int32_t WebSchemeHandlerRequest::GetRequestResourceType() const
{
    return requestResourceType_;
}

std::string WebSchemeHandlerRequest::GetFrameUrl() const
{
    return frameUrl_;
}

void WebSchemeHandlerRequest::SetUrl(const std::string& url)
{
    url_ = url;
}

void WebSchemeHandlerRequest::SetMethod(const std::string& method)
{
    method_ = method;
}

void WebSchemeHandlerRequest::SetReferrer(const std::string& referrer)
{
    referrer_ = referrer;
}

void WebSchemeHandlerRequest::SetIsRedirect(bool isRedirect)
{
    isRedirect_ = isRedirect;
}

void WebSchemeHandlerRequest::SetIsMainFrame(bool isMainFrame)
{
    isMainFrame_ = isMainFrame;
}

void WebSchemeHandlerRequest::SetHasGesture(bool hasGesture)
{
    hasGesture_ = hasGesture;
}

void WebSchemeHandlerRequest::SetHeaderList(const WebHeaderList& headerList)
{
    headerList_ = headerList;
}

void WebSchemeHandlerRequest::SetRequestResourceType(int32_t requestResourceType)
{
    requestResourceType_ = requestResourceType;
}

void WebSchemeHandlerRequest::SetFrameUrl(const std::string& frameUrl)
{
    frameUrl_ = frameUrl;
}

WebSchemeHandlerResponse::WebSchemeHandlerResponse(napi_env env) : env_(env)
{
    if (response_ == nullptr) {
        response_ = new ArkWeb_Response();
    }
}

WebSchemeHandlerResponse::WebSchemeHandlerResponse(napi_env env, ArkWeb_Response* response)
    : env_(env), response_(response)
{}

WebSchemeHandlerResponse::~WebSchemeHandlerResponse()
{
    if (response_ != nullptr) {
        delete response_;
        response_ = nullptr;
    }
}

std::string WebSchemeHandlerResponse::GetUrl() const
{
    if (!response_) {
        return "";
    }
    return response_->url_;
}

int32_t WebSchemeHandlerResponse::SetUrl(const std::string url)
{
    if (!response_ || url.empty()) {
        return ARKWEB_INVALID_PARAM;
    }

    response_->url_ = url;
    return ARKWEB_NET_OK;
}

int32_t WebSchemeHandlerResponse::GetStatus() const
{
    if (!response_) {
        return ARKWEB_INVALID_PARAM;
    }
    return response_->status_;
}

int32_t WebSchemeHandlerResponse::SetStatus(int32_t status)
{
    if (!response_) {
        return ARKWEB_INVALID_PARAM;
    }
    response_->status_ = status;
    return ARKWEB_NET_OK;
}

std::string WebSchemeHandlerResponse::GetStatusText() const
{
    if (!response_) {
        return "";
    }
    return response_->statusText_;
}

int32_t WebSchemeHandlerResponse::SetStatusText(const std::string statusText)
{
    if (!response_ || statusText.empty()) {
        return ARKWEB_INVALID_PARAM;
    }
    response_->statusText_ = statusText;
    return ARKWEB_NET_OK;
}

std::string WebSchemeHandlerResponse::GetMimeType() const
{
    if (!response_) {
        return "";
    }
    return response_->mimeType_;
}

int32_t WebSchemeHandlerResponse::SetMimeType(const std::string mimeType)
{
    if (!response_ || mimeType.empty()) {
        return ARKWEB_INVALID_PARAM;
    }
    response_->mimeType_ = mimeType;
    return ARKWEB_NET_OK;
}

std::string WebSchemeHandlerResponse::GetEncoding() const
{
    if (!response_) {
        return "";
    }
    return response_->encoding_;
}

int32_t WebSchemeHandlerResponse::SetEncoding(const std::string encoding)
{
    if (!response_ || encoding.empty()) {
        return ARKWEB_INVALID_PARAM;
    }
    response_->encoding_ = encoding;
    return ARKWEB_NET_OK;
}

std::string WebSchemeHandlerResponse::GetHeaderByName(const std::string name)
{
    if (!response_ || name.empty()) {
        return "";
    }
    auto it = response_->headers_.find(name);
    if (it != response_->headers_.end()) {
        return it->second;
    }
    return "";
}

int32_t WebSchemeHandlerResponse::SetHeaderByName(const std::string name, const std::string value, bool overwrite)
{
    if (!response_ || name.empty() || value.empty()) {
        return ARKWEB_INVALID_PARAM;
    }
    auto it = response_->headers_.find(name);
    if (overwrite) {
        response_->headers_[name] = value;
    } else {
        if (it == response_->headers_.end()) {
            response_->headers_[name] = value;
        }
    }
    return ARKWEB_NET_OK;
}

int32_t WebSchemeHandlerResponse::GetErrorCode() const
{
    if (!response_) {
        return ARKWEB_ERR_FAILED;
    }
    return response_->errorCode_;
}

int32_t WebSchemeHandlerResponse::SetErrorCode(int32_t code, const std::string errorDescription)
{
    if (!response_) {
        return ARKWEB_INVALID_PARAM;
    }
    response_->errorCode_ = code;
    response_->errorDescription_ = errorDescription;
    return ARKWEB_NET_OK;
}

WebSchemeHandler::WebSchemeHandler(napi_env env) : env_(env)
{
#ifdef IOS_PLATFORM
    ArkWeb_SchemeHandler* handler = AceWebSchemeHandler::CreateArkHandler();
    AceWebSchemeHandler::setOnRequestStart(handler, OnRequestStart);
    AceWebSchemeHandler::setOnRequestStop(handler, OnRequestStop);
#endif
#ifdef ANDROID_PLATFORM
    auto handler = WebSchemeHandlerJni::CreateArkSchemeHandler();
    WebSchemeHandlerJni::SetOnRequestStart(handler, OnRequestStart);
    WebSchemeHandlerJni::SetOnRequestStop(handler, OnRequestStop);
#endif
    if (!handler) {
        return;
    }
    std::lock_guard<std::mutex> auto_lock(g_mutexForHandlerMap);
    g_web_scheme_handler_map.insert(std::make_pair(this, handler));
    g_arkui_web_scheme_handler_map.insert(std::make_pair(handler, this));
}

WebSchemeHandler::~WebSchemeHandler()
{
    ArkWeb_SchemeHandler* handler = const_cast<ArkWeb_SchemeHandler*>(GetArkWebSchemeHandler(this));
    if (handler) {
        std::lock_guard<std::mutex> auto_lock(g_mutexForHandlerMap);
        g_web_scheme_handler_map.erase(this);
        g_arkui_web_scheme_handler_map.erase(handler);
#ifdef IOS_PLATFORM
        AceWebSchemeHandler::destroySchemeHandler(handler);
#endif
#ifdef ANDROID_PLATFORM
        WebSchemeHandlerJni::DeleteArkSchemeHandler(handler);
#endif
    }
}

ArkWeb_SchemeHandler* WebSchemeHandler::GetArkWebSchemeHandler(WebSchemeHandler* handler)
{
    if (!handler) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(g_mutexForHandlerMap);
    auto it = g_web_scheme_handler_map.find(handler);
    if (it != g_web_scheme_handler_map.end()) {
        return it->second;
    }
    return nullptr;
}

WebSchemeHandler* WebSchemeHandler::GetWebSchemeHandler(ArkWeb_SchemeHandler* arkHandler)
{
    if (!arkHandler) {
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(g_mutexForHandlerMap);
    auto it = g_arkui_web_scheme_handler_map.find(arkHandler);
    if (it != g_arkui_web_scheme_handler_map.end()) {
        return it->second;
    }
    return nullptr;
}

void WebSchemeHandler::ClearWebSchemeHandler()
{
    std::lock_guard<std::mutex> lock(g_mutexForHandlerMap);
    g_web_scheme_handler_map.clear();
    g_arkui_web_scheme_handler_map.clear();
}

void WebSchemeHandler::PutRequestStart(napi_env, napi_value callback)
{
    napi_status status = napi_create_reference(env_, callback, 1, &request_start_callback_);
    if (status != napi_status::napi_ok) {
        LOGE("PutRequestStart create reference failed.");
    }
}

void WebSchemeHandler::PutRequestStop(napi_env, napi_value callback)
{
    napi_status status = napi_create_reference(env_, callback, 1, &request_stop_callback_);
    if (status != napi_status::napi_ok) {
        LOGE("PutRequestStop create reference failed.");
    }
}

void WebSchemeHandler::RequestStart(
    ArkWeb_ResourceRequest* request, ArkWeb_ResourceHandler* arkWeb_ResourceHandler, bool* intercept)
{
    napi_handle_scope scope;
    napi_status status_scope = napi_open_handle_scope(env_, &scope);
    if (status_scope != napi_ok) {
        LOGE("scheme handler RequestStart scope is nullptr");
        return;
    }
    size_t paramCount = 2;
    napi_value callbackFunc = nullptr;
    napi_status status;
    if (!request_start_callback_) {
        LOGE("scheme handler onRequestStart nil env");
        return;
    }
    status = napi_get_reference_value(env_, request_start_callback_, &callbackFunc);
    if (status != napi_ok || callbackFunc == nullptr) {
        LOGE("scheme handler get onRequestStart func failed.");
        return;
    }

    napi_value requestValue[2] = { 0 };
    napi_create_object(env_, &requestValue[0]);
    napi_create_object(env_, &requestValue[1]);
    WebSchemeHandlerRequest* schemeHandlerRequest = new (std::nothrow) WebSchemeHandlerRequest(env_, request);
    if (schemeHandlerRequest == nullptr) {
        LOGE("RequestStart, new schemeHandlerRequest failed");
        return;
    }
    std::shared_ptr<WebResourceHandler> resourceHandler =
        std::make_shared<WebResourceHandler>(env_, arkWeb_ResourceHandler);
    if (resourceHandler == nullptr) {
        LOGE("RequestStart, new resourceHandler failed");
        delete schemeHandlerRequest;
        return;
    }
    napi_wrap(
        env_, requestValue[0], schemeHandlerRequest,
        [](napi_env /* env */, void* data, void* /* hint */) {
            WebSchemeHandlerRequest* request = (WebSchemeHandlerRequest*)data;
            delete request;
        },
        nullptr, nullptr);
    NapiWebSchemeHandlerRequest::DefineProperties(env_, &requestValue[0]);
    napi_wrap(
        env_, requestValue[1], resourceHandler.get(),
        [](napi_env /* env */, void* data, void* /* hint */) {}, nullptr, nullptr);
    NapiWebResourceHandler::DefineProperties(env_, &requestValue[1]);
    napi_value result = nullptr;
    status = napi_call_function(env_, nullptr, callbackFunc, paramCount, requestValue, &result);
    if (status != napi_status::napi_ok) {
        LOGE("scheme handler call onRequestStart failed.");
    }
    if (!NapiParseUtils::ParseBoolean(env_, result, *intercept)) {
        LOGE("scheme handler onRequestStart intercept parse failed");
        *intercept = false;
    }
    if (!*intercept) {
        resourceHandler->SetFinishFlag();
    }
}

void WebSchemeHandler::RequestStop(ArkWeb_ResourceRequest* resourceRequest)
{
    uv_loop_s* loop = nullptr;
    uv_work_t* work = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }
    work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        return;
    }
    RequestStopParam* param = new (std::nothrow) RequestStopParam();
    if (param == nullptr) {
        delete work;
        return;
    }
    param->env_ = env_;
    param->callbackRef_ = request_stop_callback_;
    param->request_ = new (std::nothrow) WebSchemeHandlerRequest(param->env_, resourceRequest);
    if (param->request_ == nullptr) {
        delete work;
        delete param;
        return;
    }
    param->arkWebRequest_ = resourceRequest;
    work->data = reinterpret_cast<void*>(param);
    int ret = uv_queue_work_with_qos(loop, work, [](uv_work_t* work) {}, RequestStopAfterWorkCb, uv_qos_user_initiated);
    if (ret != 0) {
        if (param != nullptr) {
            delete param;
            param = nullptr;
        }
        if (work != nullptr) {
            delete work;
            work = nullptr;
        }
    }
}

static void RequestStopAfterWorkCb(uv_work_t* work, int status)
{
    RequestStopParam* param = reinterpret_cast<RequestStopParam*>(work->data);
    if (param == nullptr) {
        delete work;
        return;
    }

    napi_handle_scope scope;
    napi_status status_scope = napi_open_handle_scope(param->env_, &scope);
    if (status_scope != napi_ok) {
        LOGE("scheme handler RequestStopAfterWorkCb scope is nullptr");
        delete param->request_;
        delete param;
        delete work;
        return;
    }

    if (param->callbackRef_) {
        napi_value callbackFunc = nullptr;
        napi_status napi_status = napi_get_reference_value(param->env_, param->callbackRef_, &callbackFunc);
        if (napi_status == napi_ok && callbackFunc != nullptr) {
            napi_value requestValue;
            napi_create_object(param->env_, &requestValue);
            napi_wrap(
                param->env_, requestValue, param->request_,
                [](napi_env /* env */, void* data, void* /* hint */) {
                    WebSchemeHandlerRequest* request = (WebSchemeHandlerRequest*)data;
                    delete request;
                },
                nullptr, nullptr);
            NapiWebSchemeHandlerRequest::DefineProperties(param->env_, &requestValue);

            napi_value result = nullptr;
            napi_status = napi_call_function(param->env_, nullptr, callbackFunc, 1, &requestValue, &result);
            if (napi_status != napi_status::napi_ok) {
                LOGE("scheme handler call onRequestStop failed.");
            }
        }
    } else {
        delete param->request_;
    }

    napi_close_handle_scope(param->env_, scope);
    delete param;
    delete work;
}
} // namespace OHOS::Plugin