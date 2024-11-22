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

#include "napi_utils.h"

#include "error_util.h"
#include "log.h"

namespace OHOS {
namespace Global {
namespace I18n {
static const std::string EMPTY_STRING = "";

std::string NAPIUtils::GetString(napi_env &env, napi_value &value, int32_t &code)
{
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, value, nullptr, 0, &len);
    if (status != napi_ok) {
        LOGE("Get string failed");
        code = 1;
        return EMPTY_STRING;
    }
    std::vector<char> buf(len + 1);
    status = napi_get_value_string_utf8(env, value, buf.data(), len + 1, &len);
    if (status != napi_ok) {
        LOGE("Create string failed");
        code = 1;
        return EMPTY_STRING;
    }
    return buf.data();
}

bool NAPIUtils::GetStringArrayFromJsParam(napi_env env, napi_value &jsArray, const std::string& valueName,
    std::vector<std::string> &strArray)
{
    if (jsArray == nullptr) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, valueName, "", true);
        return false;
    }
    bool isArray = false;
    napi_status status = napi_is_array(env, jsArray, &isArray);
    if (status != napi_ok) {
        return false;
    } else if (!isArray) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, valueName, "an Array", true);
        return false;
    }
    uint32_t arrayLength = 0;
    napi_get_array_length(env, jsArray, &arrayLength);
    napi_value element = nullptr;
    int32_t code = 0;
    for (uint32_t i = 0; i < arrayLength; ++i) {
        napi_get_element(env, jsArray, i, &element);
        std::string str = GetString(env, element, code);
        if (code != 0) {
            napi_throw_type_error(env, nullptr, "GetStringArrayFromJsParam: Failed to obtain the parameter.");
            return false;
        }
        strArray.push_back(str);
    }
    return true;
}

napi_value NAPIUtils::CreateString(napi_env env, const std::string &str)
{
    napi_value result;
    napi_status status = napi_create_string_utf8(env, str.c_str(), NAPI_AUTO_LENGTH, &result);
    if (status != napi_ok) {
        LOGE("create string js variable failed.");
        return nullptr;
    }
    return result;
}

void NAPIUtils::VerifyType(napi_env env, const std::string& valueName, const std::string& type,
    napi_value argv)
{
    napi_valuetype valueType = napi_valuetype::napi_undefined;
    napi_typeof(env, argv, &valueType);
    if (type == "string" && valueType != napi_valuetype::napi_string) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, valueName, type, true);
    } else if (type == "number" && valueType != napi_valuetype::napi_number) {
        ErrorUtil::NapiThrow(env, I18N_NOT_FOUND, valueName, type, true);
    }
}
} // namespace I18n
} // namespace Global
} // namespace OHOS