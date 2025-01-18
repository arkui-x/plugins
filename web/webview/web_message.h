/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef PLUGIN_WEB_WEBVIEW_WEB_MESSAGE_H
#define PLUGIN_WEB_WEBVIEW_WEB_MESSAGE_H

#include <vector>

#include "web_value.h"

namespace OHOS::Plugin {
class WebMessage : public WebValue {
public:
    explicit WebMessage(WebValue::Type type) : WebValue(type) {}

    ~WebMessage() = default;

    void SetBinary(std::vector<uint8_t>& binary_data)
    {
        binary_data_.reserve(binary_data.size());
        binary_data_ = binary_data;
    }

    std::vector<uint8_t> GetBinary()
    {
        return binary_data_;
    }

    std::string GetErrName()
    {
        return err_name_;
    }

    std::string GetErrMsg()
    {
        return err_msg_;
    }

    int64_t GetInt64()
    {
        return value_;
    }

    void SetErrName(const std::string& name)
    {
        err_name_ = name;
    }

    void SetErrMsg(const std::string& msg)
    {
        err_msg_ = msg;
    }

    void SetInt64(int64_t value)
    {
        value_ = value;
    }

    std::string GetString()
    {
        return string_;
    }

    void SetString(const std::string& string)
    {
        string_ = string;
    }

    std::vector<std::string> GetStringArray()
    {
        return string_arr_;
    }

    void SetStringArray(const std::vector<std::string>& string_arr)
    {
        string_arr_ = string_arr;
    }

    std::vector<bool> GetBooleanArray()
    {
        return bool_arr_;
    }

    void SetBooleanArray(const std::vector<bool>& bool_arr)
    {
        bool_arr_ = bool_arr;
    }

    std::vector<double> GetDoubleArray()
    {
        return double_arr_;
    }

    void SetDoubleArray(const std::vector<double>& double_arr)
    {
        double_arr_ = double_arr;
    }

    std::vector<int64_t> GetInt64Array()
    {
        return int64_arr_;
    }

    void SetInt64Array(const std::vector<int64_t>& int64_arr)
    {
        int64_arr_ = int64_arr;
    }

private:
    std::vector<uint8_t> binary_data_;
    std::string err_name_;
    std::string err_msg_;
    int64_t value_ = -1;
    std::string string_;
    std::vector<std::string> string_arr_;
    std::vector<bool> bool_arr_;
    std::vector<double> double_arr_;
    std::vector<int64_t> int64_arr_;
};
} // namespace OHOS::Plugin
#endif // PLUGIN_WEB_WEBVIEW_WEB_MESSAGE_H
