/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PLUGIN_WEB_WEBVIEW_WEB_MESSAGE_EXT_H
#define PLUGIN_WEB_WEBVIEW_WEB_MESSAGE_EXT_H

#include "plugins/web/webview/web_message.h"

namespace OHOS::Plugin {
enum class WebMessageType : int {
    NOTSUPPORT = 0,
    STRING,
    NUMBER,
    BOOLEAN,
    ARRAYBUFFER,
    ARRAY,
    ERROR
};

class WebMessageExt {
public:
    explicit WebMessageExt(std::shared_ptr<WebMessage> data) : value_(data) {};
    ~WebMessageExt() = default;

    int GetType()
    {
        if (value_) {
            return ConvertNwebType2JsType(value_->GetType());
        }
        return static_cast<int>(WebMessageType::NOTSUPPORT);
    }

    void SetData(const std::shared_ptr<WebMessage>& value)
    {
        value_ = value;
    }

    std::shared_ptr<WebMessage> GetData() const
    {
        return value_;
    }

    void SetString(std::string value)
    {
        if (value_) {
            value_->SetType(WebValue::Type::STRING);
            value_->SetString(value);
        }
    }

    std::string GetString()
    {
        if (value_) {
            return value_->GetString();
        }
        return "";
    }

    void SetNumber(double value)
    {
        if (value_) {
            value_->SetType(WebValue::Type::DOUBLE);
            value_->SetDouble(value);
        }
    }

    double GetNumber()
    {
        if (value_) {
            return value_->GetDouble();
        }
        return 0;
    }

    void SetBoolean(bool value)
    {
        if (value_) {
            value_->SetType(WebValue::Type::BOOLEAN);
            value_->SetBoolean(value);
        }
    }

    bool GetBoolean()
    {
        if (value_) {
            return value_->GetBoolean();
        }
        return false;
    }

    void SetArrayBuffer(std::vector<uint8_t>& value)
    {
        if (value_) {
            value_->SetType(WebValue::Type::BINARY);
            value_->SetBinary(value);
        }
    }
    std::vector<uint8_t> GetArrayBuffer()
    {
        if (value_) {
            return value_->GetBinary();
        }
        return {};
    }

    WebValue::Type GetArrayType()
    {
        if (value_) {
            return value_->GetType();
        }
        return WebValue::Type::NONE;
    }

    void SetStringArray(std::vector<std::string> value)
    {
        if (value_) {
            value_->SetType(WebValue::Type::STRINGARRAY);
            value_->SetStringArray(value);
        }
    }

    std::vector<std::string> GetStringArray()
    {
        if (value_) {
            return value_->GetStringArray();
        }
        return {};
    }

    void SetDoubleArray(std::vector<double> value)
    {
        if (value_) {
            value_->SetType(WebValue::Type::DOUBLEARRAY);
            value_->SetDoubleArray(value);
        }
    }

    std::vector<double> GetDoubleArray()
    {
        if (value_) {
            return value_->GetDoubleArray();
        }
        return {};
    }

    void SetInt64Array(std::vector<int64_t> value)
    {
        if (value_) {
            value_->SetType(WebValue::Type::INT64ARRAY);
            value_->SetInt64Array(value);
        }
    }

    std::vector<int64_t> GetInt64Array()
    {
        if (value_) {
            return value_->GetInt64Array();
        }
        return {};
    }

    void SetBooleanArray(std::vector<bool> value)
    {
        if (value_) {
            value_->SetType(WebValue::Type::BOOLEANARRAY);
            value_->SetBooleanArray(value);
        }
    }

    std::vector<bool> GetBooleanArray()
    {
        if (value_) {
            return value_->GetBooleanArray();
        }
        return {};
    }

    void SetError(std::string name, std::string message)
    {
        if (value_) {
            value_->SetType(WebValue::Type::ERROR);
            value_->SetErrName(name);
            value_->SetErrMsg(message);
        }
    }

    std::pair<std::string, std::string> GetError()
    {
        if (value_) {
            std::string errorName = value_->GetErrName();
            std::string errorMessage = value_->GetErrMsg();
            return std::make_pair(errorName, errorMessage);;
        }
        return {"", ""};
    }

    void SetType(int type)
    {
        type_ = type;
        WebMessageType jsType = static_cast<WebMessageType>(type);
        WebValue::Type nwebType = WebValue::Type::NONE;
        switch (jsType) {
            case WebMessageType::STRING:
                nwebType = WebValue::Type::STRING;
                break;
            case WebMessageType::NUMBER:
                nwebType = WebValue::Type::DOUBLE;
                break;
            case WebMessageType::BOOLEAN:
                nwebType = WebValue::Type::BOOLEAN;
                break;
            case WebMessageType::ARRAYBUFFER:
                nwebType = WebValue::Type::BINARY;
                break;
            case WebMessageType::ARRAY:
                nwebType = WebValue::Type::STRINGARRAY;
                break;
            case WebMessageType::ERROR:
                nwebType = WebValue::Type::ERROR;
                break;
            default:
                nwebType = WebValue::Type::NONE;
                break;
        }
        if (value_) {
            value_->SetType(nwebType);
        }
    }

    int ConvertNwebType2JsType(WebValue::Type type)
    {
        WebMessageType jsType = WebMessageType::NOTSUPPORT;
        switch (type) {
            case WebValue::Type::STRING:
                jsType = WebMessageType::STRING;
                break;
            case WebValue::Type::DOUBLE:
            case WebValue::Type::INTEGER:
                jsType = WebMessageType::NUMBER;
                break;
            case WebValue::Type::BOOLEAN:
                jsType = WebMessageType::BOOLEAN;
                break;
            case WebValue::Type::STRINGARRAY:
            case WebValue::Type::DOUBLEARRAY:
            case WebValue::Type::INT64ARRAY:
            case WebValue::Type::BOOLEANARRAY:
                jsType = WebMessageType::ARRAY;
                break;
            case WebValue::Type::BINARY:
                jsType = WebMessageType::ARRAYBUFFER;
                break;
            case WebValue::Type::ERROR:
                jsType = WebMessageType::ERROR;
                break;
            default:
                jsType = WebMessageType::NOTSUPPORT;
                break;
        }
        return static_cast<int>(jsType);
    }

private:
    int type_ = 0;
    std::shared_ptr<WebMessage> value_ = nullptr;
};
} // namespace OHOS::Plugin
#endif // PLUGIN_WEB_WEBVIEW_WEB_MESSAGE_H