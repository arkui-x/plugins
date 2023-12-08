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
#include "webview_async_work_callback.h"

namespace OHOS::Plugin {
struct WebHistoryItem final {
    WebHistoryItem() = default;
    WebHistoryItem(const WebHistoryItem& other);
    ~WebHistoryItem() = default;
    std::string historyUrl = "";
    std::string historyRawUrl = "";
    std::string title = "";
};

class WebHistoryList {
public:
    WebHistoryList() = default;
    WebHistoryList(const WebHistoryList& other);
    virtual ~WebHistoryList();

    void SetCurrentIndex(int32_t index)
    {
        if (index >= 0 && index != currentIndex_) {
            currentIndex_ = index;
        }
    }

    int32_t GetCurrentIndex() const
    {
        return currentIndex_;
    }

    void SetListSize(int32_t size)
    {
        if (size >= 0 && size != size_) {
            size_ = size;
        }
    }

    int32_t GetListSize() const
    {
        return size_;
    }

    bool InsertHistoryItem(const std::shared_ptr<WebHistoryItem>& value);
    std::shared_ptr<WebHistoryItem> GetItemAtIndex(int32_t index);
private:
    int32_t currentIndex_;
    int32_t size_;
    std::vector<std::shared_ptr<WebHistoryItem>> webHistoryItemContainer_;
};

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
    virtual void EvaluateJavaScript(const std::string& script, int32_t asyncCallbackInfoId) = 0;
    virtual ErrCode ScrollTo(float x, float y) = 0;
    virtual ErrCode ScrollBy(float deltaX, float deltaY) = 0;
    virtual ErrCode Zoom(float factor) = 0;
    virtual ErrCode Stop() = 0;
    virtual ErrCode SetCustomUserAgent(const std::string& userAgent) = 0;
    virtual std::string GetCustomUserAgent() = 0;
    virtual bool AccessStep(int32_t step) = 0;
    virtual void CreateWebMessagePorts(std::vector<std::string>& ports) = 0;
    virtual void PostWebMessage(std::string& message, std::vector<std::string>& ports, std::string& targetUrl) = 0;
    virtual std::shared_ptr<WebHistoryList> GetBackForwardEntries() = 0;
    virtual void RemoveCache(bool value) = 0;
    virtual void BackOrForward(int32_t step) = 0;
    virtual std::string GetTitle() = 0;
    virtual int32_t GetPageHeight() = 0;
    virtual ErrCode ClearHistory() {
        return NWebError::NO_ERROR;
    }
    void SetWebId(int32_t webId)
    {
        webId_= webId;
    }

    int32_t GetWebId() const
    {
        return webId_;
    }

    static void OnReceiveValue(const std::string& result, int32_t asyncCallbackInfoId);
    static void InsertAsyncCallbackInfo(const std::shared_ptr<AsyncEvaluteJSResultCallbackInfo>& asyncCallbackInfo);
    static bool EraseAsyncCallbackInfo(const AsyncEvaluteJSResultCallbackInfo* asyncCallbackInfo);
protected:
    int32_t webId_ = -1;
private:
    static bool ExcuteAsyncCallbackInfo(const std::string& result, int32_t asyncCallbackInfoId);
    static thread_local std::vector<std::shared_ptr<AsyncEvaluteJSResultCallbackInfo>> asyncCallbackInfoContainer_;
};
} // namespace OHOS::Plugin

#endif // PLUGINS_WEB_WEBVIEW_WEBVIEW_CONTROLLER_H
