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

#ifndef PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEB_MESSAGE_PORT_ANDROID_H
#define PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEB_MESSAGE_PORT_ANDROID_H

#include "plugins/web/webview/web_message_port.h"

namespace OHOS::Plugin {
class WebMessagePortAndroid : public WebMessagePort {
public:
    WebMessagePortAndroid(int32_t webId, const std::string& portHandle);
    WebMessagePortAndroid(int32_t webId, const std::string& portHandle, bool isExtentionType);
    ~WebMessagePortAndroid() = default;
    void ClosePort() override;
    ErrCode PostMessageEvent(const std::string& webMessage) override;
    ErrCode PostMessageEventExt(WebMessageExt* webMessage) override;
    ErrCode OnMessageEvent() override;
    ErrCode OnMessageEventExt() override;
};
} // namespace OHOS::Plugin

#endif // PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_WEB_MESSAGE_PORT_ANDROID_H
