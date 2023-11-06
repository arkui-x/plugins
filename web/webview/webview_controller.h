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

#ifndef PLUGINS_WEB_WEBVIEW_WEBVIEW_CONTROLLER_H
#define PLUGINS_WEB_WEBVIEW_WEBVIEW_CONTROLLER_H

#include <memory>
#include <map>
#include <vector>
#include "napi_parse_utils.h"
#include "web_errors.h"
#include "webview_javascript_execute_callback.h"

namespace OHOS::Plugin {
class WebviewController {
public:
    WebviewController() = default;
    virtual ~WebviewController() = default;
    virtual bool IsInit() = 0;
    virtual ErrCode LoadUrl(const std::string& url) = 0;
    virtual ErrCode LoadUrl(const std::string& url,
        const std::map<std::string, std::string>& httpHeaders) = 0;
    virtual ErrCode LoadData(const std::string& data, const std::string& mimeType, const std::string& encoding,
        const std::string& baseUrl, const std::string& historyUrl) = 0;
    virtual std::string GetUrl() = 0;
    virtual bool AccessBackward() = 0;
    virtual bool AccessForward() = 0;
    virtual ErrCode Backward() = 0;
    virtual ErrCode Forward() = 0;
    virtual ErrCode Refresh() = 0;
    virtual void EvaluateJavaScript(const std::string& script) = 0;
    void SetWebId(int32_t webId)
    {
        webId_= webId;
    }

    static void OnReceiveValue(const std::string& result);
    static void InsertAsyncCallbackInfo(const std::shared_ptr<AsyncEvaluteJSResultCallbackInfo>& asyncCallbackInfo);
    static bool EraseAsyncCallbackInfo(const AsyncEvaluteJSResultCallbackInfo* asyncCallbackInfo);
protected:
    int32_t webId_ = -1;
private:
    static bool ExcuteAsyncCallbackInfo(const std::string& result);
    static void IncreaseIndex();
    static void DecreaseIndex();
    static std::vector<std::shared_ptr<AsyncEvaluteJSResultCallbackInfo>> asyncCallbackInfoContainer_;
    static int32_t index_;
    static std::mutex mutex_;
};
} // namespace OHOS::Plugin

#endif // PLUGINS_WEB_WEBVIEW_WEBVIEW_CONTROLLER_H
