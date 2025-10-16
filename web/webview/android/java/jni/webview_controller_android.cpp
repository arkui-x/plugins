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

#include "webview_controller_android.h"

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugins/web/webview/android/java/jni/webview_controller_jni.h"

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
WebviewControllerAndroid::~WebviewControllerAndroid()
{
    LOGI("~WebviewControllerAndroid id:{public}d", webId_);
}

ErrCode WebviewControllerAndroid::LoadUrl(const std::string& url)
{
    return WebviewControllerJni::LoadUrl(webId_, url, std::map<std::string, std::string>());
}

ErrCode WebviewControllerAndroid::LoadUrl(
    const std::string& url, const std::map<std::string, std::string>& httpHeaders)
{
    return WebviewControllerJni::LoadUrl(webId_, url, httpHeaders);
}

ErrCode WebviewControllerAndroid::LoadData(const std::string& data, const std::string& mimeType, const std::string& encoding,
    const std::string& baseUrl, const std::string& historyUrl)
{
    return WebviewControllerJni::LoadData(webId_, data, mimeType, encoding, baseUrl, historyUrl);
}

std::string WebviewControllerAndroid::GetUrl()
{
    return WebviewControllerJni::GetUrl(webId_);
}

bool WebviewControllerAndroid::AccessForward()
{
    return WebviewControllerJni::AccessForward(webId_);
}

bool WebviewControllerAndroid::AccessBackward()
{
    return WebviewControllerJni::AccessBackward(webId_);
}

ErrCode WebviewControllerAndroid::Forward()
{
    WebviewControllerJni::Forward(webId_);
    return NO_ERROR;
}

ErrCode WebviewControllerAndroid::Backward()
{
    WebviewControllerJni::Backward(webId_);
    return NO_ERROR;
}

ErrCode WebviewControllerAndroid::Refresh()
{
    WebviewControllerJni::Refresh(webId_);
    return NO_ERROR;
}

ErrCode WebviewControllerAndroid::ScrollTo(float x, float y)
{
    return WebviewControllerJni::ScrollTo(webId_, static_cast<int>(x), static_cast<int>(y));
}

ErrCode WebviewControllerAndroid::ScrollBy(float deltaX, float deltaY)
{
    return WebviewControllerJni::ScrollBy(webId_, static_cast<int>(deltaX), static_cast<int>(deltaY));
}

ErrCode WebviewControllerAndroid::Zoom(float factor)
{
    return WebviewControllerJni::Zoom(webId_, factor);
}

ErrCode WebviewControllerAndroid::ZoomIn()
{
    return WebviewControllerJni::ZoomIn(webId_);
}

ErrCode WebviewControllerAndroid::ZoomOut()
{
    return WebviewControllerJni::ZoomOut(webId_);
}

ErrCode WebviewControllerAndroid::Stop()
{
    return WebviewControllerJni::Stop(webId_);
}

std::string WebviewControllerAndroid::GetOriginalUrl()
{
    return WebviewControllerJni::GetOriginalUrl(webId_);
}

ErrCode WebviewControllerAndroid::PageUp(bool value)
{
    return WebviewControllerJni::PageUp(webId_, value);
}

ErrCode WebviewControllerAndroid::SetCustomUserAgent(const std::string& userAgent)
{
    return WebviewControllerJni::SetCustomUserAgent(webId_, userAgent);
}

std::string WebviewControllerAndroid::GetCustomUserAgent()
{
    return WebviewControllerJni::GetCustomUserAgent(webId_);
}

ErrCode WebviewControllerAndroid::ClearHistory()
{
    return WebviewControllerJni::ClearHistory(webId_);
}

bool WebviewControllerAndroid::AccessStep(int32_t step)
{
    return WebviewControllerJni::AccessStep(webId_, step);
}

bool WebviewControllerAndroid::IsInit()
{
    return webId_ != -1;
}

void WebviewControllerAndroid::EvaluateJavaScript(const std::string& script, int32_t asyncCallbackInfoId)
{
    WebviewControllerJni::EvaluateJavaScript(webId_, script, asyncCallbackInfoId);
}

void WebviewControllerAndroid::EvaluateJavaScriptExt(const std::string& script, int32_t asyncCallbackInfoId)
{
    WebviewControllerJni::EvaluateJavaScriptExt(webId_, script, asyncCallbackInfoId);
}

std::shared_ptr<WebHistoryList> WebviewControllerAndroid::GetBackForwardEntries()
{
    return WebviewControllerJni::GetBackForwardEntries(webId_);
}

void WebviewControllerAndroid::RemoveCache(bool value)
{
    WebviewControllerJni::RemoveCache(webId_, value);
}

void WebviewControllerAndroid::BackOrForward(int32_t step)
{
    WebviewControllerJni::BackOrForward(webId_, step);
}

std::string WebviewControllerAndroid::GetTitle()
{
    return WebviewControllerJni::GetTitle(webId_);
}

int32_t WebviewControllerAndroid::GetPageHeight()
{
    return WebviewControllerJni::GetPageHeight(webId_);
}

void WebviewControllerAndroid::CreateWebMessagePorts(std::vector<std::string>& ports)
{
    WebviewControllerJni::CreateWebMessagePorts(webId_, ports);
}

void WebviewControllerAndroid::PostWebMessage(
    std::string& message, std::vector<std::string>& ports,std::string& targetUrl)
{
    WebviewControllerJni::PostWebMessage(webId_, message, ports, targetUrl);
}

void WebviewControllerAndroid::SetWebDebuggingAccess(bool webDebuggingAccess)
{
    WebviewControllerJni::SetWebDebuggingAccess(webDebuggingAccess);
}

ErrCode WebviewControllerAndroid::PageDown(bool value)
{
    return WebviewControllerJni::PageDown(webId_, value);
}

ErrCode WebviewControllerAndroid::PostUrl(const std::string& url, const std::vector<uint8_t>& postData)
{
    return WebviewControllerJni::PostUrl(webId_, url, postData);
}

void WebviewControllerAndroid::StartDownload(const std::string& url)
{
    WebviewControllerJni::StartDownload(webId_, url);
}

void WebviewControllerAndroid::RegisterJavaScriptProxy(const RegisterJavaScriptProxyParam& param)
{
    WebviewControllerJni::RegisterJavaScriptProxy(
        webId_, param.objName, param.syncMethodList, param.asyncMethodList, param.permission);
}

void WebviewControllerAndroid::DeleteJavaScriptRegister(const std::string& objName)
{
    WebviewControllerJni::DeleteJavaScriptRegister(webId_, objName);
}

bool WebviewControllerAndroid::SetWebSchemeHandler(const char* scheme, WebSchemeHandler* handler)
{
    CHECK_NULL_RETURN(scheme, false);
    CHECK_NULL_RETURN(handler, false);
    std::string schemeStr(scheme);
    auto arkHandler = WebSchemeHandler::GetArkWebSchemeHandler(handler);
    WebSchemeHandlerJni::InsertSchemeRequestHandler(schemeStr, arkHandler);
    return WebviewControllerJni::SetWebSchemeHandler(webId_, schemeStr);
}

bool WebviewControllerAndroid::ClearWebSchemeHandler()
{
    WebSchemeHandlerJni::ClearSchemeRequestHandler();
    WebviewControllerJni::ClearWebSchemeHandler(webId_);
    return true;
}

std::string WebviewControllerAndroid::GetUserAgent()
{
    return WebviewControllerJni::GetUserAgent(webId_);
}
}