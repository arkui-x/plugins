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

#include "web_message_port_android.h"
#include "webview_controller_jni.h"

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
WebMessagePortAndroid::WebMessagePortAndroid(int32_t webId, const std::string& portHandle) :
    WebMessagePort(webId, portHandle) {}

WebMessagePortAndroid::WebMessagePortAndroid(int32_t webId, const std::string& portHandle, bool isExtentionType) :
    WebMessagePort(webId, portHandle, isExtentionType) {}

void WebMessagePortAndroid::ClosePort()
{
    WebviewControllerJni::CloseWebMessagePort(GetWebId(), GetPortHandle());
    SetPortHandle("");
}

ErrCode WebMessagePortAndroid::PostMessageEvent(const std::string& webMessage)
{
    return WebviewControllerJni::PostMessageEvent(GetWebId(), GetPortHandle(), webMessage);
}

ErrCode WebMessagePortAndroid::PostMessageEventExt(WebMessageExt* webMessage)
{
    return WebviewControllerJni::PostMessageEventExt(GetWebId(), GetPortHandle(), webMessage);
}

ErrCode WebMessagePortAndroid::OnMessageEvent()
{
    return WebviewControllerJni::OnWebMessagePortEvent(GetWebId(), GetPortHandle());
}

ErrCode WebMessagePortAndroid::OnMessageEventExt()
{
    return WebviewControllerJni::OnWebMessagePortEventExt(GetWebId(), GetPortHandle());
}
} // namespace OHOS::Plugin
