/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "datashare_js_utils.h"

#include "datashare_predicates_proxy.h"
#include "napi/native_common.h"

namespace OHOS {
namespace DataShare {
std::string DataShareJSUtils::Convert2String(napi_env env, napi_value jsStr, const size_t max)
{
    size_t str_buffer_size = max;
    napi_get_value_string_utf8(env, jsStr, nullptr, 0, &str_buffer_size);
    char* buf = new (std::nothrow) char[str_buffer_size + 1];
    if (buf == nullptr) {
        return "";
    }
    size_t len = 0;
    napi_get_value_string_utf8(env, jsStr, buf, str_buffer_size + 1, &len);
    buf[len] = 0;
    std::string value(buf);
    delete[] buf;
    return value;
}

std::vector<std::string> DataShareJSUtils::Convert2StrVector(napi_env env, napi_value value, const size_t strMax)
{
    NAPI_ASSERT_BASE(env, strMax > 0, "failed on strMax > 0", std::vector<std::string>());
    uint32_t arrLen = 0;
    napi_get_array_length(env, value, &arrLen);
    if (arrLen == 0) {
        return {};
    }
    std::vector<std::string> result;
    for (size_t i = 0; i < arrLen; ++i) {
        napi_value element;
        if (napi_get_element(env, value, i, &element) != napi_ok) {
            return {};
        }
        result.push_back(ConvertAny2String(env, element));
    }
    return result;
}

std::vector<uint8_t> DataShareJSUtils::Convert2U8Vector(napi_env env, napi_value input_array)
{
    bool isTypedArray = false;
    bool isArrayBuffer = false;
    napi_is_typedarray(env, input_array, &isTypedArray);
    if (!isTypedArray) {
        napi_is_arraybuffer(env, input_array, &isArrayBuffer);
        if (!isArrayBuffer) {
            return {};
        }
    }
    size_t length = 0;
    void* data = nullptr;
    if (isTypedArray) {
        napi_typedarray_type type;
        napi_value input_buffer = nullptr;
        size_t byte_offset = 0;
        napi_get_typedarray_info(env, input_array, &type, &length, &data, &input_buffer, &byte_offset);
        if (type != napi_uint8_array || data == nullptr) {
            return {};
        }
    } else {
        napi_get_arraybuffer_info(env, input_array, &data, &length);
        if (data == nullptr || length <= 0) {
            return {};
        }
    }
    return std::vector<uint8_t>((uint8_t*)data, ((uint8_t*)data) + length);
}

std::string DataShareJSUtils::ConvertAny2String(napi_env env, napi_value jsValue)
{
    napi_valuetype valueType = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, jsValue, &valueType), "napi_typeof failed");
    if (valueType == napi_string) {
        return DataShareJSUtils::Convert2String(env, jsValue, DataShareJSUtils::DEFAULT_BUF_SIZE);
    } else if (valueType == napi_number) {
        double valueNumber;
        napi_get_value_double(env, jsValue, &valueNumber);
        return std::to_string(valueNumber);
    } else if (valueType == napi_boolean) {
        bool valueBool = false;
        napi_get_value_bool(env, jsValue, &valueBool);
        return std::to_string(valueBool);
    } else if (valueType == napi_null) {
        return "null";
    } else if (valueType == napi_object) {
        std::vector<uint8_t> bytes = DataShareJSUtils::Convert2U8Vector(env, jsValue);
        std::string ret(bytes.begin(), bytes.end());
        return ret;
    }

    return "invalid type";
}

napi_value DataShareJSUtils::Convert2JSValue(napi_env env, const std::monostate& value)
{
    return nullptr;
}

napi_value DataShareJSUtils::Convert2JSValue(napi_env env, const std::vector<std::string>& value)
{
    napi_value jsValue;
    napi_status status = napi_create_array_with_length(env, value.size(), &jsValue);
    if (status != napi_ok) {
        return nullptr;
    }

    for (size_t i = 0; i < value.size(); ++i) {
        napi_set_element(env, jsValue, i, Convert2JSValue(env, value[i]));
    }
    return jsValue;
}

napi_value DataShareJSUtils::Convert2JSValue(napi_env env, const std::string& value)
{
    napi_value jsValue;
    napi_status status = napi_create_string_utf8(env, value.c_str(), value.size(), &jsValue);
    if (status != napi_ok) {
        return nullptr;
    }
    return jsValue;
}

napi_value DataShareJSUtils::Convert2JSValue(napi_env env, int32_t value)
{
    napi_value jsValue;
    napi_status status = napi_create_int32(env, value, &jsValue);
    if (status != napi_ok) {
        return nullptr;
    }
    return jsValue;
}

napi_value DataShareJSUtils::Convert2JSValue(napi_env env, int64_t value)
{
    napi_value jsValue;
    napi_status status = napi_create_int64(env, value, &jsValue);
    if (status != napi_ok) {
        return nullptr;
    }
    return jsValue;
}

napi_value DataShareJSUtils::Convert2JSValue(napi_env env, uint32_t value)
{
    napi_value jsValue;
    napi_status status = napi_create_uint32(env, value, &jsValue);
    if (status != napi_ok) {
        return nullptr;
    }
    return jsValue;
}

napi_value DataShareJSUtils::Convert2JSValue(napi_env env, double value)
{
    napi_value jsValue;
    napi_status status = napi_create_double(env, value, &jsValue);
    if (status != napi_ok) {
        return nullptr;
    }
    return jsValue;
}

napi_value DataShareJSUtils::Convert2JSValue(napi_env env, bool value)
{
    napi_value jsValue;
    napi_status status = napi_get_boolean(env, value, &jsValue);
    if (status != napi_ok) {
        return nullptr;
    }
    return jsValue;
}

napi_value DataShareJSUtils::Convert2JSValue(napi_env env, const std::map<std::string, int>& value)
{
    napi_value jsValue;
    napi_status status = napi_create_array_with_length(env, value.size(), &jsValue);
    if (status != napi_ok) {
        return nullptr;
    }

    int index = 0;
    for (const auto& [device, result] : value) {
        napi_value jsElement;
        status = napi_create_array_with_length(env, SYNC_RESULT_ELEMNT_NUM, &jsElement);
        if (status != napi_ok) {
            return nullptr;
        }
        napi_set_element(env, jsElement, 0, Convert2JSValue(env, device));
        napi_set_element(env, jsElement, 1, Convert2JSValue(env, result));
        napi_set_element(env, jsValue, index++, jsElement);
    }

    return jsValue;
}

int32_t DataShareJSUtils::Convert2Value(napi_env env, napi_value input, std::string& str)
{
    size_t strBufferSize = DEFAULT_BUF_SIZE;
    napi_get_value_string_utf8(env, input, nullptr, 0, &strBufferSize);
    char* buf = new (std::nothrow) char[strBufferSize + 1];
    if (buf == nullptr) {
        return napi_invalid_arg;
    }
    size_t len = 0;
    napi_get_value_string_utf8(env, input, buf, strBufferSize + 1, &len);
    buf[len] = 0;
    str = std::string(buf);
    delete[] buf;
    return napi_ok;
}

} // namespace DataShare
} // namespace OHOS