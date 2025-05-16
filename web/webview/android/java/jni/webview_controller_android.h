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

#ifndef PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEBVIEW_CONTROLLER_ANDROID_H
#define PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEBVIEW_CONTROLLER_ANDROID_H

#include "plugins/web/webview/webview_controller.h"

namespace OHOS::Plugin {
class WebviewControllerAndroid : public WebviewController {
public:
    WebviewControllerAndroid() = default;
    ~WebviewControllerAndroid() override;
    bool IsInit() override;
    ErrCode LoadUrl(const std::string& url) override;
    ErrCode LoadUrl(const std::string& url,
        const std::map<std::string, std::string>& httpHeaders) override;
    ErrCode LoadData(const std::string& data, const std::string& mimeType, const std::string& encoding,
        const std::string& baseUrl, const std::string& historyUrl) override;
    std::string GetUrl() override;
    bool AccessForward() override;
    bool AccessBackward() override;
    ErrCode Forward() override;
    ErrCode Backward() override;
    ErrCode Refresh() override;
    ErrCode ScrollTo(float x, float y) override;
    ErrCode ScrollBy(float deltaX, float deltaY) override;
    ErrCode Zoom(float factor) override;
    ErrCode ZoomIn() override;
    ErrCode ZoomOut() override;
    ErrCode Stop() override;
    std::string GetOriginalUrl() override;
    ErrCode PageUp(bool top) override;
    ErrCode SetCustomUserAgent(const std::string& userAgent) override;
    std::string GetCustomUserAgent() override;
    ErrCode ClearHistory() override;
    bool AccessStep(int32_t step) override;
    void EvaluateJavaScript(const std::string& script, int32_t asyncCallbackInfoId) override;
    void EvaluateJavaScriptExt(const std::string& script, int32_t asyncCallbackInfoId) override;
    std::shared_ptr<WebHistoryList> GetBackForwardEntries() override;
    void RemoveCache(bool value) override;
    void BackOrForward(int32_t step) override;
    std::string GetTitle() override;
    int32_t GetPageHeight() override;
    void CreateWebMessagePorts(std::vector<std::string>& ports) override;
    void PostWebMessage(std::string& message, std::vector<std::string>& ports, std::string& targetUrl) override;
    static void SetWebDebuggingAccess(bool webDebuggingAccess);
    ErrCode PageDown(bool value) override;
    ErrCode PostUrl(const std::string& url, const std::vector<uint8_t>& postData) override;
    void StartDownload(const std::string& url) override;
    void RegisterJavaScriptProxy(const RegisterJavaScriptProxyParam& param) override;
    void DeleteJavaScriptRegister(const std::string& objName) override;
};
}
#endif
