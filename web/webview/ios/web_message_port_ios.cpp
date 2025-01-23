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

#include "web_message_port_ios.h"
#include "AceWebControllerBridge.h"
#include "AceWebMessageExtImpl.h"
#include "log.h"

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
WebMessagePortIOS::WebMessagePortIOS(int32_t webId, const std::string& portHandle, bool isExtentionType) :
    WebMessagePort(webId, portHandle, isExtentionType) {}

void OnMessageExtOC(int32_t webId, const std::string& portHandle, 
    const std::shared_ptr<AceWebMessageExtImpl> webMessageExtImpl)
{
    if (webMessageExtImpl == nullptr) {
        LOGE("WebMessagePort OnMessageExt webMessageExtImpl is null.");
        return;
    }
    auto webMessage = std::make_shared<WebMessage>(WebValue::Type::NONE);
    auto webMessageExt = std::make_shared<WebMessageExt>(webMessage);
    if (!webMessage || !webMessageExt) {
        LOGE("WebMessagePort OnMessageExt webMessage or webMessageExt is null.");
        return;
    }
    switch (webMessageExtImpl->GetType()) {
        case AceWebMessageType::BOOLEAN:
            webMessageExt->SetBoolean(webMessageExtImpl->GetBoolean());
            break;
        case AceWebMessageType::INTEGER:
            webMessageExt->SetNumber(webMessageExtImpl->GetInt());
            break;
        case AceWebMessageType::DOUBLE:
            webMessageExt->SetNumber(webMessageExtImpl->GetDouble());
            break;
        case AceWebMessageType::STRING:
            webMessageExt->SetString(webMessageExtImpl->GetString());
            break;
        case AceWebMessageType::ERROR: {
            auto error = webMessageExtImpl->GetError();
            webMessageExt->SetError(error.first, error.second);
            break;
        }
        case AceWebMessageType::STRINGARRAY:
            webMessageExt->SetStringArray(webMessageExtImpl->GetStringArray());
            break;
        case AceWebMessageType::BOOLEANARRAY:
            webMessageExt->SetBooleanArray(webMessageExtImpl->GetBooleanArray());
            break;
        case AceWebMessageType::DOUBLEARRAY:
            webMessageExt->SetDoubleArray(webMessageExtImpl->GetDoubleArray());
            break;
        case AceWebMessageType::INT64ARRAY:
            webMessageExt->SetInt64Array(webMessageExtImpl->GetInt64Array());
            break;
        default:
            break;
    }
    WebMessagePort::OnMessageExt(webId, portHandle, webMessageExt);
}

void WebMessagePortIOS::ClosePort()
{
    SetPortHandle("");
    closePortOC(GetWebId());
}

ErrCode WebMessagePortIOS::PostMessageEvent(const std::string& webMessage)
{
    std::string portHandle = GetPortHandle();
    if (portHandle.empty() || portHandle == "" || !postMessageEventOC(GetWebId(), webMessage)) {
        return CAN_NOT_POST_MESSAGE;
    }
    return NO_ERROR;
}

ErrCode WebMessagePortIOS::PostMessageEventExt(WebMessageExt* webMessageExt)
{
    std::string portHandle = GetPortHandle();
    auto webMessageExtImpl = std::make_shared<AceWebMessageExtImpl>();
    if (!webMessageExtImpl.get() || portHandle.empty() || portHandle == "") {
        return CAN_NOT_POST_MESSAGE;
    }
    int msgType = webMessageExt->GetType();
    switch (msgType) {
        case static_cast<int32_t>(WebMessageType::BOOLEAN): {
            webMessageExtImpl->SetType(AceWebMessageType::BOOLEAN);
            webMessageExtImpl->SetBoolean(webMessageExt->GetBoolean());
            break;
        }
        case static_cast<int32_t>(WebMessageType::NUMBER): {
            webMessageExtImpl->SetType(AceWebMessageType::DOUBLE);
            webMessageExtImpl->SetDouble(webMessageExt->GetNumber());
            break;
        }
        case static_cast<int32_t>(WebMessageType::STRING): {
            webMessageExtImpl->SetType(AceWebMessageType::STRING);
            webMessageExtImpl->SetString(webMessageExt->GetString());
            break;
        }
        case static_cast<int32_t>(WebMessageType::ARRAY): {
            WebValue::Type arrayType = webMessageExt->GetArrayType();
            switch (arrayType) {
                case WebValue::Type::STRINGARRAY: {
                    webMessageExtImpl->SetType(AceWebMessageType::STRINGARRAY);
                    webMessageExtImpl->SetStringArray(webMessageExt->GetStringArray());
                    break;
                }
                case WebValue::Type::BOOLEANARRAY: {
                    webMessageExtImpl->SetType(AceWebMessageType::BOOLEANARRAY);
                    webMessageExtImpl->SetBooleanArray(webMessageExt->GetBooleanArray());
                    break;
                }
                case WebValue::Type::DOUBLEARRAY: {
                    webMessageExtImpl->SetType(AceWebMessageType::DOUBLEARRAY);
                    webMessageExtImpl->SetDoubleArray(webMessageExt->GetDoubleArray());
                    break;
                }
                case WebValue::Type::INT64ARRAY: {
                    webMessageExtImpl->SetType(AceWebMessageType::INT64ARRAY);
                    webMessageExtImpl->SetInt64Array(webMessageExt->GetInt64Array());
                    break;
                }
                default:
                    break;
            }
            break;
        }
        case static_cast<int32_t>(WebMessageType::ARRAYBUFFER): {
            webMessageExtImpl->SetType(AceWebMessageType::BINARY);
            std::vector<uint8_t> value = webMessageExt->GetArrayBuffer();
            webMessageExtImpl->SetArrayBuffer(value);
            break;
        }
        case static_cast<int32_t>(WebMessageType::ERROR): {
            webMessageExtImpl->SetType(AceWebMessageType::ERROR);
            auto error = webMessageExt->GetError();
            webMessageExtImpl->SetError(error.first, error.second);
            break;
        }
        default:
            break;
    }
    return postMessageEventExtOC(GetWebId(), webMessageExtImpl) ? NO_ERROR : CAN_NOT_POST_MESSAGE;
}

ErrCode WebMessagePortIOS::OnMessageEvent()
{
    onMessageEventOC(GetWebId(),GetPortHandle(),WebMessagePort::OnMessage);
    return NO_ERROR;
}

ErrCode WebMessagePortIOS::OnMessageEventExt()
{
    onMessageEventExtOC(GetWebId(), GetPortHandle(), OnMessageExtOC);
    return NO_ERROR;
}
} // namespace OHOS::Plugin
