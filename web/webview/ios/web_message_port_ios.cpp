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

#include "web_message_port_ios.h"
#include "AceWebControllerBridge.h"
#include "log.h"

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
WebMessagePortIOS::WebMessagePortIOS(int32_t webId, const std::string& portHandle) :
    WebMessagePort(webId, portHandle) {}

void WebMessagePortIOS::ClosePort()
{
    closePortOC(GetWebId());
}

ErrCode WebMessagePortIOS::PostMessageEvent(const std::string& webMessage)
{
    bool result = postMessageEventOC(GetWebId(), webMessage);
    if (!result) {
        return CAN_NOT_POST_MESSAGE;
    }
    return NO_ERROR;
}

ErrCode WebMessagePortIOS::OnMessageEvent()
{
    onMessageEventOC(GetWebId(),GetPortHandle(),WebMessagePort::OnMessage);
    return NO_ERROR;
}
} // namespace OHOS::Plugin
