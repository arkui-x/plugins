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

#ifndef PLUGINS_WEB_WEBVIEW_IOS_WEBVIEW_CONTROLLER_IOS_H
#define PLUGINS_WEB_WEBVIEW_IOS_WEBVIEW_CONTROLLER_IOS_H

#include "plugins/web/webview/webview_controller.h"

namespace OHOS::Plugin {
class WebviewControllerIOS : public WebviewController {
public:
    WebviewControllerIOS() = default;
    ~WebviewControllerIOS() override;
    bool IsInit() override;
    ErrCode LoadUrl(const std::string& url) override;
    ErrCode LoadUrl(const std::string& url, const std::map<std::string, std::string>& httpHeaders) override;
    ErrCode LoadData(const std::string& data, const std::string& mimeType, const std::string& encoding,
        const std::string& baseUrl, const std::string& historyUrl) override;
    ErrCode Backward() override;
    ErrCode Forward() override;
    ErrCode Refresh() override;
    std::string GetUrl() override;
    bool AccessBackward() override;
    bool AccessForward() override;
    void EvaluateJavaScript(const std::string& script) override;
};
} // namespace OHOS::Plugin
#endif
