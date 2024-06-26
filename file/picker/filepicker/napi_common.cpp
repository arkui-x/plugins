/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "napi_common.h"

namespace OHOS::Plugin {
bool IsArray(const napi_env& env, const napi_value& value)
{
    bool isArray = false;
    napi_status ret = napi_is_array(env, value, &isArray);
    if (ret != napi_ok) {
        return false;
    }
    return isArray;
}

bool CheckType(const napi_env& env, const napi_value& value, const napi_valuetype& type)
{
    napi_valuetype valuetype = napi_undefined;
    napi_typeof(env, value, &valuetype);
    if (valuetype != type) {
        LOGE("value type dismatch");
        return false;
    }
    return true;
}

bool ParseString(const napi_env& env, const napi_value& value, std::string& result)
{
    if (!CheckType(env, value, napi_string)) {
        return false;
    }
    size_t size;
    if (napi_get_value_string_utf8(env, value, nullptr, 0, &size) != napi_ok) {
        LOGE("cannot get string size");
        return false;
    }
    result.reserve(size + 1);
    result.resize(size);
    if (napi_get_value_string_utf8(env, value, result.data(), size + 1, &size) != napi_ok) {
        LOGE("cannot get value string");
        return false;
    }
    return true;
}

bool ParseStringArray(const napi_env& env, const napi_value& value, std::vector<std::string>& result)
{
    if (!IsArray(env, value)) {
        return false;
    }

    uint32_t length = 0;
    napi_get_array_length(env, value, &length);

    if (length == 0) {
        LOGE("array is empty");
        return true;
    }

    napi_value valueArray;
    for (uint32_t i = 0; i < length; i++) {
        napi_get_element(env, value, i, &valueArray);

        std::string str;
        if (!ParseString(env, valueArray, str)) {
            return false;
        }
        result.emplace_back(str);
    }
    return true;
}
} // namespace OHOS::Plugin