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
#include "web_errors.h"

namespace OHOS::Plugin {
class WebviewController {
public:
    WebviewController() = default;
    virtual ~WebviewController() = default;
    virtual bool IsInit() = 0;
    virtual ErrCode LoadUrl(const std::string& url) = 0;
    virtual ErrCode LoadUrl(const std::string& url,
        const std::map<std::string, std::string>& httpHeaders) = 0;
    void SetWebId(int32_t webId)
    {
        webId_= webId;
    }
protected:
    int32_t webId_ = -1;
};
} // namespace OHOS::Plugin

#endif // PLUGINS_WEB_WEBVIEW_WEBVIEW_CONTROLLER_H
