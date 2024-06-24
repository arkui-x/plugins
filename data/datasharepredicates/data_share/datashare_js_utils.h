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

#ifndef DATASHARE_JSUTILS_H
#define DATASHARE_JSUTILS_H

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "napi/native_api.h"

#define NAPI_ASSERT_ERRCODE(env, assertion, error)                                                              \
    do {                                                                                                        \
        if (!(assertion)) {                                                                                     \
            napi_throw_error((env), std::to_string((error)->GetCode()).c_str(), (error)->GetMessage().c_str()); \
            return;                                                                                             \
        }                                                                                                       \
    } while (0)

#define NAPI_ASSERT_CALL_ERRCODE(env, assertion, call, retVal) \
    do {                                                       \
        if (!(assertion)) {                                    \
            call;                                              \
            return retVal;                                     \
        }                                                      \
    } while (0)

#define NAPI_ASSERT_CALL_ERRCODE_SYNC(env, assertion, call, error, retVal)                                      \
    do {                                                                                                        \
        if (!(assertion)) {                                                                                     \
            call;                                                                                               \
            napi_throw_error((env), std::to_string((error)->GetCode()).c_str(), (error)->GetMessage().c_str()); \
            return retVal;                                                                                      \
        }                                                                                                       \
    } while (0)

namespace OHOS {
namespace DataShare {
class DataShareJSUtils final {
public:
    static constexpr int32_t DEFAULT_BUF_SIZE = 1024;
    static constexpr int32_t ASYNC_RST_SIZE = 2;
    static constexpr int32_t SYNC_RESULT_ELEMNT_NUM = 2;

    template<typename T>
    static int32_t Convert2Value(napi_env env, napi_value input, T& output);

    template<typename T>
    static int32_t Convert2JSValue(napi_env env, napi_value input, T& output);

    template<typename T>
    static napi_value Convert2JSValue(napi_env env, const std::vector<T>& value);

    template<typename T>
    static int32_t Convert2Value(napi_env env, napi_value input, std::vector<T>& output);

    template<typename T>
    static int32_t Convert2Value(napi_env env, napi_value input, std::map<std::string, T>& output);

    template<typename T>
    static int32_t Convert2Value(napi_env env, napi_value input, const char* propertyName, T& output);

    static int32_t Convert2Value(napi_env env, napi_value input, std::string& str);
    static std::string Convert2String(napi_env env, napi_value jsStr, size_t max = DEFAULT_BUF_SIZE);
    static std::vector<std::string> Convert2StrVector(napi_env env, napi_value value, size_t strMax);
    static std::vector<uint8_t> Convert2U8Vector(napi_env env, napi_value jsValue);
    static std::string ConvertAny2String(napi_env env, const napi_value jsValue);
    static std::string UnwrapStringFromJS(napi_env env, napi_value param, const std::string& defaultValue = "");

    static napi_value Convert2JSValue(napi_env env, const std::monostate& value = {});
    static napi_value Convert2JSValue(napi_env env, const std::vector<std::string>& value);
    static napi_value Convert2JSValue(napi_env env, const std::string& value);
    static napi_value Convert2JSValue(napi_env env, const std::vector<uint8_t>& value, bool isTypedArray = true);
    static napi_value Convert2JSValue(napi_env env, int32_t value);
    static napi_value Convert2JSValue(napi_env env, int64_t value);
    static napi_value Convert2JSValue(napi_env env, uint32_t value);
    static napi_value Convert2JSValue(napi_env env, double value);
    static napi_value Convert2JSValue(napi_env env, bool value);
    static napi_value Convert2JSValue(napi_env env, const std::map<std::string, int>& value);
    template<class... Types>
    static napi_value Convert2JSValue(napi_env env, const std::variant<Types...>& value);

private:
    template<typename _VTp>
    static napi_value ReadVariant(napi_env env, uint32_t step, uint32_t index, const _VTp& output)
    {
        (void)step;
        (void)index;
        (void)output;
        return Convert2JSValue(env);
    }

    template<typename _VTp, typename _First, typename... _Rest>
    static napi_value ReadVariant(napi_env env, uint32_t step, uint32_t index, const _VTp& value)
    {
        if (step == index) {
            auto* realValue = std::get_if<_First>(&value);
            if (realValue == nullptr) {
                return nullptr;
            }
            return Convert2JSValue(env, *realValue);
        }
        return ReadVariant<_VTp, _Rest...>(env, step + 1, index, value);
    }
};

template<class... Types>
napi_value DataShareJSUtils::Convert2JSValue(napi_env env, const std::variant<Types...>& value)
{
    return ReadVariant<decltype(value), Types...>(env, 0, value.index(), value);
}

template<typename T>
int32_t DataShareJSUtils::Convert2Value(napi_env env, napi_value input, const char* propertyName, T& output)
{
    napi_value jsObject = nullptr;
    if (napi_get_named_property(env, input, propertyName, &jsObject) != napi_ok) {
        return napi_invalid_arg;
    }
    if (Convert2Value(env, jsObject, output) != napi_ok) {
        return napi_invalid_arg;
    }
    return napi_ok;
}

template<typename T>
napi_value DataShareJSUtils::Convert2JSValue(napi_env env, const std::vector<T>& value)
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

template<typename T>
int32_t DataShareJSUtils::Convert2Value(napi_env env, napi_value input, std::vector<T>& output)
{
    bool isArray = false;
    napi_is_array(env, input, &isArray);
    if (!isArray) {
        return napi_invalid_arg;
    }

    uint32_t arrLen = 0;
    napi_get_array_length(env, input, &arrLen);
    if (arrLen == 0) {
        return napi_invalid_arg;
    }

    for (size_t i = 0; i < arrLen; ++i) {
        napi_value element;
        napi_get_element(env, input, i, &element);
        T item;
        auto status = Convert2Value(env, element, item);
        if (status != napi_ok) {
            return napi_invalid_arg;
        }
        output.push_back(std::move(item));
    }
    return napi_ok;
}

template<typename T>
int32_t DataShareJSUtils::Convert2Value(napi_env env, napi_value input, std::map<std::string, T>& output)
{
    napi_value map = nullptr;
    uint32_t count = 0;
    napi_status status = napi_get_property_names(env, input, &map);
    if (status != napi_ok) {
        return napi_invalid_arg;
    }
    status = napi_get_array_length(env, map, &count);
    if (status != napi_ok || count == 0) {
        return napi_invalid_arg;
    }
    napi_value jsKey = nullptr;
    napi_value jsVal = nullptr;
    for (uint32_t index = 0; index < count; index++) {
        status = napi_get_element(env, map, index, &jsKey);
        if (status != napi_ok) {
            return napi_invalid_arg;
        }
        std::string key = Convert2String(env, jsKey);
        status = napi_get_property(env, input, jsKey, &jsVal);
        if (status != napi_ok || jsVal == nullptr) {
            return napi_invalid_arg;
        }
        T val;
        int32_t ret = Convert2Value(env, jsVal, val);
        if (ret != napi_ok) {
            return napi_invalid_arg;
        }
        output.insert(std::pair<std::string, T>(key, val));
    }
    return napi_ok;
}
} // namespace DataShare
} // namespace OHOS

#endif // DATASHARE_JSUTILS_H