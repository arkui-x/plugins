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

#include "webview_controller_ios.h"
#include "AceWebControllerBridge.h"
#include "log.h"
using namespace OHOS::NWebError;

namespace OHOS::Plugin {
WebviewControllerIOS::~WebviewControllerIOS() {}

ErrCode WebviewControllerIOS::LoadUrl(const std::string& url)
{
    loadUrlOC(webId_, url, std::map<std::string, std::string>());
    return NO_ERROR;
}

ErrCode WebviewControllerIOS::LoadUrl(const std::string& url, const std::map<std::string, std::string>& httpHeaders)
{
    loadUrlOC(webId_, url, httpHeaders);
    return NO_ERROR;
}

ErrCode WebviewControllerIOS::LoadData(const std::string& data, const std::string& mimeType,
    const std::string& encoding, const std::string& baseUrl, const std::string& historyUrl)
{
    loadDataOC(webId_, data, mimeType, encoding, baseUrl, historyUrl);
    return NO_ERROR;
}

std::string WebviewControllerIOS::GetUrl()
{
    return getUrlOC(webId_);
}

bool WebviewControllerIOS::AccessBackward()
{
    return accessBackwardOC(webId_);
}

bool WebviewControllerIOS::AccessForward()
{
    return accessForwardOC(webId_);
}

ErrCode WebviewControllerIOS::Backward()
{
    backwardOC(webId_);
    return NO_ERROR;
}

ErrCode WebviewControllerIOS::Forward()
{
    forwardOC(webId_);
    return NO_ERROR;
}

ErrCode WebviewControllerIOS::Refresh()
{
    refreshOC(webId_);
    return NO_ERROR;
}

void WebviewControllerIOS::EvaluateJavaScript(const std::string& script, int32_t asyncCallbackInfoId)
{
    evaluateJavaScriptOC(webId_, script, asyncCallbackInfoId, WebviewController::OnReceiveValue);
}

void WebviewControllerIOS::EvaluateJavaScriptExt(const std::string& script, int32_t asyncCallbackInfoId)
{
    evaluateJavaScriptExtOC(webId_, script, asyncCallbackInfoId, WebviewController::OnReceiveRunJavaScriptExtValue);
}

void WebviewControllerIOS::RemoveCache(bool value)
{
    removeCacheOC(webId_, value);
}

void WebviewControllerIOS::BackOrForward(int32_t step){
    backOrForwardOC(webId_, step);
}

std::string WebviewControllerIOS::GetTitle(){
    return getTitleOC(webId_);
}

int32_t WebviewControllerIOS::GetPageHeight(){
    return getPageHeightOC(webId_);
}

std::shared_ptr<WebHistoryList> WebviewControllerIOS::GetBackForwardEntries()
{
    std::shared_ptr<WebHistoryList> historyList = std::make_shared<WebHistoryList>();
    BackForwardResult backForwardResult = getBackForwardEntriesOC(webId_);
    historyList->SetCurrentIndex(backForwardResult.currentIndex);
    historyList->SetListSize(backForwardResult.backForwardItemList.size());
    for (const auto& item : backForwardResult.backForwardItemList) {
        std::shared_ptr<WebHistoryItem> webHistoryItem = std::make_shared<WebHistoryItem>();
        webHistoryItem->historyUrl = item.URL;
        webHistoryItem->title = item.title;
        webHistoryItem->historyRawUrl = item.initialURL;
        historyList->InsertHistoryItem(webHistoryItem);
    }
    return historyList;
}

void WebviewControllerIOS::CreateWebMessagePorts(std::vector<std::string>& ports)
{
    createWebMessagePortsOC(webId_, ports);
}

void WebviewControllerIOS::PostWebMessage(std::string& message, std::vector<std::string>& ports, std::string& targetUrl)
{
    postWebMessageOC(webId_, message, ports, targetUrl);
}

ErrCode WebviewControllerIOS::ScrollTo(float x, float y)
{
    scrollToOC(webId_, x, y);
    return NO_ERROR;
}

ErrCode WebviewControllerIOS::ScrollBy(float deltaX, float deltaY)
{
    scrollByOC(webId_, deltaX, deltaY);
    return NO_ERROR;
}

ErrCode WebviewControllerIOS::Zoom(float factor)
{
    zoomOC(webId_, factor);
    return NO_ERROR;
}

ErrCode WebviewControllerIOS::ZoomIn()
{
    bool accessStatus = isZoomAccessOC(webId_);
    if(accessStatus) {
        zoomInOC(webId_);
        return NO_ERROR;
    }
    return FUNCTION_NOT_ENABLE;
}

ErrCode WebviewControllerIOS::ZoomOut()
{
    bool accessStatus = isZoomAccessOC(webId_);
    if(accessStatus) {
        zoomOutOC(webId_);
        return NO_ERROR;
    }
    return FUNCTION_NOT_ENABLE;
}

ErrCode WebviewControllerIOS::Stop()
{
    stopOC(webId_);
    return NO_ERROR;
}

std::string WebviewControllerIOS::GetOriginalUrl()
{
    return getOriginalUrlOC(webId_);
}

ErrCode WebviewControllerIOS::PageUp(bool top)
{
    pageUpOC(webId_, top);
    return NO_ERROR;
}

ErrCode WebviewControllerIOS::SetCustomUserAgent(const std::string& userAgent)
{
    setCustomUserAgentOC(webId_, userAgent);
    return NO_ERROR;
}

std::string WebviewControllerIOS::GetCustomUserAgent()
{
    return getCustomUserAgentOC(webId_);
}

bool WebviewControllerIOS::AccessStep(int32_t step)
{
    return accessStepOC(webId_, step);
}

bool WebviewControllerIOS::IsInit()
{
    return webId_ != -1;
}

void WebviewControllerIOS::SetWebDebuggingAccess(bool webDebuggingAccess)
{
    setWebDebuggingAccessOC(webDebuggingAccess);
}

ErrCode WebviewControllerIOS::PageDown(bool bottom)
{
    pageDownOC(webId_, bottom);
    return NO_ERROR;
}

ErrCode WebviewControllerIOS::PostUrl(const std::string& url, const std::vector<uint8_t>& postData)
{
    postUrlOC(webId_, url, postData);
    return NO_ERROR;
}

void WebviewControllerIOS::StartDownload(const std::string& url)
{
    startDownloadOC(webId_, url);
}

void WebviewControllerIOS::RegisterJavaScriptProxy(const RegisterJavaScriptProxyParam& param)
{
    registerJavaScriptProxyOC(webId_, param.objName, param.syncMethodList, param.asyncMethodList,
        WebviewController::OnReceiveJavascriptExecuteCall);
}

void WebviewControllerIOS::DeleteJavaScriptRegister(const std::string& objName)
{
    deleteJavaScriptRegisterOC(webId_, objName);
}
} // namespace OHOS::Plugin
