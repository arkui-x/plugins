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

#ifndef PLUGINS_WEB_WEBVIEW_WEB_MESSAGE_PORT_H
#define PLUGINS_WEB_WEBVIEW_WEB_MESSAGE_PORT_H

#include <map>
#include <uv.h>

#include "web_errors.h"
#include "webview_async_work_callback.h"

namespace OHOS::Plugin {
class WebMessagePort {
public:
    WebMessagePort(int32_t webId, const std::string& portHandle) :
        webId_(webId), portHandle_(portHandle), isExtentionType_(false) {}
    WebMessagePort(int32_t webId, const std::string& portHandle, bool isExtentionType) :
        webId_(webId), portHandle_(portHandle), isExtentionType_(isExtentionType) {}
    virtual ~WebMessagePort() = default;
    int32_t GetWebId() const
    {
        return webId_;
    }

    const std::string& GetPortHandle() const
    {
        return portHandle_;
    }

    void SetPortHandle(const std::string& portHandle)
    {
        portHandle_ = portHandle;
    }

    void SetWebMessageCallback(const std::shared_ptr<NapiJsCallBackParm>& callback)
    {
        if (callback) {
            webMessageCallback_ = callback;
        }
    }

    void SetWebMessageExtCallback(const std::shared_ptr<NapiJsCallBackParmExt>& callback)
    {
        if (callback) {
            webMessageExtCallback_ = callback;
        }
    }

    const std::shared_ptr<NapiJsCallBackParm>& GetWebMessageCallback() const
    {
        return webMessageCallback_;
    }

    const std::shared_ptr<NapiJsCallBackParmExt>& GetWebMessageExtCallback() const
    {
        return webMessageExtCallback_;
    }

    bool IsExtentionType()
    {
        return isExtentionType_;
    }

    virtual void ClosePort() = 0;
    virtual ErrCode PostMessageEvent(const std::string& webMessage) = 0;
    virtual ErrCode PostMessageEventExt(WebMessageExt* webMessage) = 0;
    virtual ErrCode OnMessageEvent() = 0;
    virtual ErrCode OnMessageEventExt() = 0;
    static void InsertPort(const WebMessagePort* webMessagePort);
    static void ErasePort(const WebMessagePort* webMessagePort);
    static void OnMessage(int32_t webId, const std::string& portHandle, const std::string& result);
    static void OnMessageExt(
        int32_t webId, const std::string& portHandle, const std::shared_ptr<WebMessageExt>& webMessage);

private:
    static void UvJsCallbackThreadWorker(uv_work_t* work, int status);
    static void UvJsExtCallbackThreadWorker(uv_work_t* work, int status);
    static void HandleArrayType(NapiJsCallBackParmExt* param, WebMessageExt* webMessageExt);
    int32_t webId_;
    std::string portHandle_;
    std::shared_ptr<NapiJsCallBackParm> webMessageCallback_;
    bool isExtentionType_;
    std::shared_ptr<NapiJsCallBackParmExt> webMessageExtCallback_;
    static thread_local std::map<int32_t, std::map<std::string, const WebMessagePort*>> webMessagePortArray_;
};
} // namespace OHOS::Plugin

#endif // PLUGINS_WEB_WEBVIEW_WEB_MESSAGE_PORT_H
