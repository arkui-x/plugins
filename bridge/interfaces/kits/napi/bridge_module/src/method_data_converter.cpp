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

#include "method_data_converter.h"

#include <array>

#include "log.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_napi.h"

namespace OHOS::Plugin::Bridge {
static constexpr int32_t MAX_INT_32 = std::numeric_limits<int>::max();
static constexpr double DOUBLE_MIN_VALUE = 0.00001;

CodecableValue MethodDataConverter::ConvertToCodecableValue(napi_env env, napi_value value)
{
    bool isArrayBuffer = PluginUtilsNApi::IsArrayBuffer(env, value);
    if (isArrayBuffer) {
        std::vector<uint8_t> vector;
        PluginUtilsNApi::GetArrayBuffer(env, value, vector);
        return CodecableValue(vector);
    }

    napi_valuetype valueType = PluginUtilsNApi::GetValueType(env, value);
    switch (valueType) {
        case napi_boolean: {
            return CodecableValue(PluginUtilsNApi::GetBool(env, value));
        }
        case napi_number: {
            int32_t int32Value = PluginUtilsNApi::GetCInt32(value, env);
            int64_t int64Value = PluginUtilsNApi::GetCInt64(value, env);
            double doubleValue = PluginUtilsNApi::GetDouble(env, value);

            if (doubleValue - int64Value > DOUBLE_MIN_VALUE) {
                return CodecableValue(doubleValue);
            } else {
                if (int64Value > MAX_INT_32) {
                    return CodecableValue(int64Value);
                } else {
                    return CodecableValue(int32Value);
                }
            }
        }
        case napi_string: {
            return CodecableValue(PluginUtilsNApi::GetStringFromValueUtf8(env, value));
        }
        case napi_object: {
            if (PluginUtilsNApi::IsArray(env, value)) {
                return GainListValue(env, value);
            }
            return GainMapValue(env, value);
        }
        default:
            break;
    }

    return CodecableValue();
}

CodecableValue MethodDataConverter::ConvertToCodecableValue(napi_env env, const size_t& argc, const napi_value* argv)
{
    if (argc == 0 || argv == nullptr) {
        CodecableList list;
        return CodecableValue(list);
    }

    CodecableList list;
    list.reserve(argc);
    for (size_t i = 0; i < argc; i++) {
        list.push_back(ConvertToCodecableValue(env, argv[i]));
    }

    return CodecableValue(list);
}

napi_value MethodDataConverter::ConvertToNapiValue(napi_env env, const CodecableValue& value)
{
    switch (static_cast<CodecableType>(value.index())) {
        case CodecableType::T_BOOL:
            return PluginUtilsNApi::CreateBoolean(env, std::get<bool>(value));
        case CodecableType::T_INT32:
            return PluginUtilsNApi::CreateInt32(env, std::get<int32_t>(value));
        case CodecableType::T_INT64: {
            int64_t temp = std::get<int64_t>(value);
            return PluginUtilsNApi::CreateDouble(env, static_cast<double>(temp));
        }
        case CodecableType::T_DOUBLE:
            return PluginUtilsNApi::CreateDouble(env, std::get<double>(value));
        case CodecableType::T_STRING:
            return PluginUtilsNApi::CreateStringUtf8(env, std::get<std::string>(value));
        case CodecableType::T_LIST_UINT8:
            return PluginUtilsNApi::CreateArrayBuffer(env, std::get<std::vector<uint8_t>>(value));
        case CodecableType::T_LIST_BOOL: {
            return CreateListBoolValue(env, value);
        }
        case CodecableType::T_LIST_INT32: {
            return CreateListInt32Value(env, value);
        }
        case CodecableType::T_LIST_INT64: {
            return CreateListInt64Value(env, value);
        }
        case CodecableType::T_LIST_DOUBLE: {
            return CreateListDoubleValue(env, value);
        }
        case CodecableType::T_LIST_STRING: {
            return CreateListStringValue(env, value);
        }
        case CodecableType::T_MAP: {
            return CreateMapValue(env, value);
        }
        default:
            return PluginUtilsNApi::CreateNull(env);
    }
}

napi_value MethodDataConverter::CreateListBoolValue(napi_env env, const CodecableValue& value)
{
    napi_value result = nullptr;
    result = PluginUtilsNApi::CreateArray(env);
    const auto& vector = std::get<std::vector<bool>>(value);
    int32_t i = 0;
    for (const auto& item : vector) {
        napi_value itemValue = PluginUtilsNApi::CreateBoolean(env, item);
        PluginUtilsNApi::SetSelementToArray(env, result, i, itemValue);
        i++;
    }
    return result;
}

napi_value MethodDataConverter::CreateListInt32Value(napi_env env, const CodecableValue& value)
{
    napi_value result = nullptr;
    result = PluginUtilsNApi::CreateArray(env);
    const auto& vector = std::get<std::vector<int32_t>>(value);
    int32_t i = 0;
    for (const auto& item : vector) {
        napi_value itemValue = PluginUtilsNApi::CreateInt32(env, item);
        PluginUtilsNApi::SetSelementToArray(env, result, i, itemValue);
        i++;
    }
    return result;
}

napi_value MethodDataConverter::CreateListInt64Value(napi_env env, const CodecableValue& value)
{
    napi_value result = nullptr;
    result = PluginUtilsNApi::CreateArray(env);
    const auto& vector = std::get<std::vector<int64_t>>(value);
    int32_t i = 0;
    for (const auto& item : vector) {
        napi_value itemValue = PluginUtilsNApi::CreateDouble(env, static_cast<double>(item));
        PluginUtilsNApi::SetSelementToArray(env, result, i, itemValue);
        i++;
    }
    return result;
}

napi_value MethodDataConverter::CreateListDoubleValue(napi_env env, const CodecableValue& value)
{
    napi_value result = nullptr;
    result = PluginUtilsNApi::CreateArray(env);
    const auto& vector = std::get<std::vector<double>>(value);
    int32_t i = 0;
    for (const auto& item : vector) {
        napi_value itemValue = PluginUtilsNApi::CreateDouble(env, item);
        PluginUtilsNApi::SetSelementToArray(env, result, i, itemValue);
        i++;
    }
    return result;
}

napi_value MethodDataConverter::CreateListStringValue(napi_env env, const CodecableValue& value)
{
    napi_value result = nullptr;
    result = PluginUtilsNApi::CreateArray(env);
    const auto& vector = std::get<std::vector<std::string>>(value);
    int32_t i = 0;
    for (const auto& item : vector) {
        napi_value itemValue = PluginUtilsNApi::CreateStringUtf8(env, item);
        PluginUtilsNApi::SetSelementToArray(env, result, i, itemValue);
        i++;
    }
    return result;
}

napi_value MethodDataConverter::CreateMapValue(napi_env env, const CodecableValue& value)
{
    napi_value result = nullptr;
    result = PluginUtilsNApi::CreateObject(env);
    const auto& map = std::get<CodecableMap>(value);
    for (const auto& pair : map) {
        PluginUtilsNApi::SetNamedProperty(
            env, result, std::get<std::string>(pair.first), ConvertToNapiValue(env, pair.second));
    }
    return result;
}

void MethodDataConverter::ConvertToNapiValues(napi_env env, const CodecableValue& value, size_t& argc, napi_value* argv)
{
    size_t argNum = 0;
    if (!std::holds_alternative<CodecableList>(value)) {
        return;
    }
    CodecableList list = std::get<CodecableList>(value);
    for (const auto& item : list) {
        argv[argNum] = ConvertToNapiValue(env, item);
        argNum++;
        if (argNum >= argc) {
            break;
        }
    }
    argc = argNum;
}

CodecableValue MethodDataConverter::GainListValue(napi_env env, napi_value value)
{
    uint32_t length = 0;
    napi_get_array_length(env, value, &length);
    napi_value firstValue = nullptr;
    napi_get_element(env, value, 0, &firstValue);
    napi_valuetype type = PluginUtilsNApi::GetValueType(env, firstValue);
    switch (type) {
        case napi_boolean: {
            return GainListBoolValue(env, value, length);
        }
        case napi_number: {
            int32_t intValue = PluginUtilsNApi::GetCInt32(firstValue, env);
            double numberValue = PluginUtilsNApi::GetDouble(env, firstValue);
            bool isDouble = numberValue - intValue > DOUBLE_MIN_VALUE;
            if (isDouble) {
                return GainListDoubleValue(env, value, length);
            } else {
                return GainListInt32Value(env, value, length);
            }
        }
        case napi_string: {
            return GainListStringValue(env, value, length);
        }
        default:
            return CodecableValue();
    }
}

CodecableValue MethodDataConverter::GainListBoolValue(napi_env env, napi_value value, uint32_t length)
{
    std::vector<bool> vector;
    vector.reserve(length);
    for (uint32_t i = 0; i < length; i++) {
        napi_value elementValue;
        napi_get_element(env, value, i, &elementValue);
        vector.push_back(PluginUtilsNApi::GetBool(env, elementValue));
    }
    return CodecableValue(vector);
}

CodecableValue MethodDataConverter::GainListStringValue(napi_env env, napi_value value, uint32_t length)
{
    std::vector<std::string> vector;
    vector.reserve(length);
    for (uint32_t i = 0; i < length; i++) {
        napi_value elementValue;
        napi_get_element(env, value, i, &elementValue);
        vector.push_back(PluginUtilsNApi::GetStringFromValueUtf8(env, elementValue));
    }
    return CodecableValue(vector);
}

CodecableValue MethodDataConverter::GainListInt32Value(napi_env env, napi_value value, uint32_t length)
{
    std::vector<int32_t> vector;
    vector.reserve(length);
    for (uint32_t i = 0; i < length; i++) {
        napi_value elementValue;
        napi_get_element(env, value, i, &elementValue);
        vector.push_back(PluginUtilsNApi::GetCInt32(elementValue, env));
    }
    return CodecableValue(vector);
}

CodecableValue MethodDataConverter::GainListDoubleValue(napi_env env, napi_value value, uint32_t length)
{
    std::vector<double> vector;
    vector.reserve(length);
    for (uint32_t i = 0; i < length; i++) {
        napi_value elementValue;
        napi_get_element(env, value, i, &elementValue);
        vector.push_back(PluginUtilsNApi::GetDouble(env, elementValue));
    }
    return CodecableValue(vector);
}

CodecableValue MethodDataConverter::GainMapValue(napi_env env, napi_value value)
{
    CodecableMap mapValue;
    std::vector<std::string> props;
    if (PluginUtilsNApi::GetPropertyNames(env, value, props)) {
        for (auto prop : props) {
            CodecableValue key = CodecableValue(prop);
            CodecableValue itemValue =
                ConvertToCodecableValue(env, PluginUtilsNApi::GetNamedProperty(env, value, prop));
            mapValue.emplace(std::move(key), std::move(itemValue));
        }
    }
    return CodecableValue(mapValue);
}
} // namespace OHOS::Plugin::Bridge