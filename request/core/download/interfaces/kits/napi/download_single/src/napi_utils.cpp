/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#include "napi_utils.h"

#include <cstring>
#include <initializer_list>
#include <memory>

#include "download_manager.h"
#include "log.h"
#include "securec.h"

namespace OHOS::Plugin::Request::Download::NapiUtils {
static constexpr const int MAX_STRING_LENGTH = 65536;

napi_valuetype GetValueType(napi_env env, napi_value value)
{
    if (value == nullptr) {
        return napi_undefined;
    }

    napi_valuetype valueType = napi_undefined;
    NAPI_CALL_BASE(env, napi_typeof(env, value, &valueType), napi_undefined);
    return valueType;
}

/* named property */
bool HasNamedProperty(napi_env env, napi_value object, const std::string &propertyName)
{
    bool hasProperty = false;
    NAPI_CALL_BASE(env, napi_has_named_property(env, object, propertyName.c_str(), &hasProperty), false);
    return hasProperty;
}

napi_value GetNamedProperty(napi_env env, napi_value object, const std::string &propertyName)
{
    napi_value value = nullptr;
    bool hasProperty = false;
    NAPI_CALL(env, napi_has_named_property(env, object, propertyName.c_str(), &hasProperty));
    if (!hasProperty) {
        return value;
    }
    NAPI_CALL(env, napi_get_named_property(env, object, propertyName.c_str(), &value));
    return value;
}

std::vector<std::string> GetPropertyNames(napi_env env, napi_value object)
{
    std::vector<std::string> ret;
    napi_value names = nullptr;
    NAPI_CALL_BASE(env, napi_get_property_names(env, object, &names), ret);
    uint32_t length = 0;
    NAPI_CALL_BASE(env, napi_get_array_length(env, names, &length), ret);
    for (uint32_t index = 0; index < length; ++index) {
        napi_value name = nullptr;
        if (napi_get_element(env, names, index, &name) != napi_ok) {
            continue;
        }
        if (GetValueType(env, name) != napi_string) {
            continue;
        }
        ret.emplace_back(GetStringFromValueUtf8(env, name));
    }
    return ret;
}

/* UINT32 */
napi_value CreateUint32(napi_env env, uint32_t code)
{
    napi_value value = nullptr;
    if (napi_create_uint32(env, code, &value) != napi_ok) {
        return nullptr;
    }
    return value;
}

uint32_t GetUint32FromValue(napi_env env, napi_value value)
{
    uint32_t ret = 0;
    NAPI_CALL_BASE(env, napi_get_value_uint32(env, value, &ret), 0);
    return ret;
}

uint32_t GetUint32Property(napi_env env, napi_value object, const std::string &propertyName)
{
    if (!HasNamedProperty(env, object, propertyName)) {
        return 0;
    }
    napi_value value = GetNamedProperty(env, object, propertyName);
    return GetUint32FromValue(env, value);
}

void SetUint32Property(napi_env env, napi_value object, const std::string &name, uint32_t value)
{
    napi_value jsValue = CreateUint32(env, value);
    if (GetValueType(env, jsValue) != napi_number) {
        return;
    }

    napi_set_named_property(env, object, name.c_str(), jsValue);
}

/* INT32 */
napi_value CreateInt32(napi_env env, int32_t code)
{
    napi_value value = nullptr;
    if (napi_create_int32(env, code, &value) != napi_ok) {
        return nullptr;
    }
    return value;
}

int32_t GetInt32FromValue(napi_env env, napi_value value)
{
    int32_t ret = 0;
    NAPI_CALL_BASE(env, napi_get_value_int32(env, value, &ret), 0);
    return ret;
}

/* String UTF8 */
napi_value CreateStringUtf8(napi_env env, const std::string &str)
{
    napi_value value = nullptr;
    if (napi_create_string_utf8(env, str.c_str(), strlen(str.c_str()), &value) != napi_ok) {
        return nullptr;
    }
    return value;
}

std::string GetStringFromValueUtf8(napi_env env, napi_value value)
{
    std::string result;
    std::vector<char> str(MAX_STRING_LENGTH + 1, '\0');
    size_t length = 0;
    NAPI_CALL_BASE(env, napi_get_value_string_utf8(env, value, &str[0], MAX_STRING_LENGTH, &length), result);
    if (length > 0) {
        return result.append(&str[0], length);
    }
    return result;
}

std::string GetStringPropertyUtf8(napi_env env, napi_value object, const std::string &propertyName)
{
    if (!HasNamedProperty(env, object, propertyName)) {
        return "";
    }
    napi_value value = GetNamedProperty(env, object, propertyName);
    return GetStringFromValueUtf8(env, value);
}

void SetStringPropertyUtf8(napi_env env, napi_value object, const std::string &name, const std::string &value)
{
    napi_value jsValue = CreateStringUtf8(env, value);
    if (GetValueType(env, jsValue) != napi_string) {
        return;
    }
    napi_set_named_property(env, object, name.c_str(), jsValue);
}

/* object */
napi_value CreateObject(napi_env env)
{
    napi_value object = nullptr;
    NAPI_CALL(env, napi_create_object(env, &object));
    return object;
}

/* undefined */
napi_value GetUndefined(napi_env env)
{
    napi_value undefined = nullptr;
    NAPI_CALL(env, napi_get_undefined(env, &undefined));
    return undefined;
}

/* function */
napi_value CallFunction(napi_env env, napi_value recv, napi_value func, size_t argc, const napi_value *argv)
{
    napi_value res = nullptr;
    NAPI_CALL(env, napi_call_function(env, recv, func, argc, argv, &res));
    return res;
}

/* boolean */
bool GetBooleanProperty(napi_env env, napi_value object, const std::string &propertyName)
{
    if (!HasNamedProperty(env, object, propertyName)) {
        return false;
    }
    napi_value value = GetNamedProperty(env, object, propertyName);
    bool ret = false;
    NAPI_CALL_BASE(env, napi_get_value_bool(env, value, &ret), false);
    return ret;
}

std::string ToLower(const std::string &s)
{
    std::string res = s;
    std::transform(res.begin(), res.end(), res.begin(), tolower);
    return res;
}

int32_t GetParameterNumber(napi_env env, napi_callback_info info, napi_value *argv, napi_value *this_arg)
{
    size_t argc = NapiUtils::MAX_ARGC;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, argv, this_arg, &data);
    if (status != napi_ok) {
        return -1;
    }
    return static_cast<int32_t>(argc);
}

void ThrowError(napi_env env, const ExceptionErrorCode &code, const std::string &msg)
{
    std::string errorCode = std::to_string(code);
    auto iter = ErrorCodeToMsg.find(code);
    std::string strMsg = (iter != ErrorCodeToMsg.end() ? iter->second : "") + ": " + msg;
    napi_status status = napi_throw_error(env, errorCode.c_str(), strMsg.c_str());
    if (status != napi_ok) {
        DOWNLOAD_HILOGE("Failed to napi_throw_error");
    }
}

napi_value CreateBusinessError(napi_env env, const ExceptionErrorCode &errorCode, const std::string &msg)
{
    napi_value result = nullptr;
    napi_value codeValue = nullptr;
    napi_value message = nullptr;
    auto iter = ErrorCodeToMsg.find(errorCode);
    std::string errCode = std::to_string(errorCode);
    std::string strMsg = (iter != ErrorCodeToMsg.end() ? iter->second : "") + msg;
    NAPI_CALL(env, napi_create_string_utf8(env, strMsg.c_str(), strMsg.length(), &message));
    NAPI_CALL(env, napi_create_string_utf8(env, errCode.c_str(), errCode.length(), &codeValue));
    NAPI_CALL(env, napi_create_error(env, codeValue, message, &result));
    return result;
}

bool CheckParameterCorrect(napi_env env, napi_callback_info info, const std::string &type, ExceptionError &err)
{
    std::string errInfo;
    napi_value argv[NapiUtils::MAX_ARGC] = { nullptr };
    int32_t num = NapiUtils::GetParameterNumber(env, info, argv, nullptr);
    if (num < 0) {
        err.code = EXCEPTION_PARAMETER_CHECK;
        err.errInfo = "function ${" + type + "} Wrong number of arguments";
        return false;
    }
    if (num == NapiUtils::ONE_ARG) {
        if (NapiUtils::GetValueType(env, argv[NapiUtils::FIRST_ARGV]) != napi_function) {
            err.code = EXCEPTION_PARAMETER_CHECK;
            err.errInfo = "function ${" + type + "} the first parameter must be function";
            return false;
        }
    }
    return true;
}
} // namespace OHOS::Plugin::Request::Download::NapiUtils