/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef NAPI_PARSE_UTILS_H
#define NAPI_PARSE_UTILS_H

#include <vector>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "web_message.h"

namespace OHOS::Plugin {
constexpr int INTEGER_ZERO = 0;
constexpr int INTEGER_ONE = 1;
constexpr int INTEGER_TWO = 2;
constexpr int INTEGER_THREE = 3;
constexpr int INTEGER_FOUR = 4;
constexpr int INTEGER_FIVE = 5;
constexpr int MAX_CUSTOM_SCHEME_NAME_LENGTH = 32;
constexpr int MAX_CUSTOM_SCHEME_SIZE = 10;
constexpr int32_t MAX_STRING_LENGTH = 40960;
constexpr int32_t MAX_PWD_LENGTH = 256;
constexpr float ZOOM_FACTOR_LOW_LIMIT_ANDROID = 0.01;
constexpr float ZOOM_FACTOR_HIGH_LIMIT_ANDROID = 100.0;
const std::string ZOOM_FACTOR_NAN = "nan";

class NapiParseUtils {
public:
    static napi_value CreateEnumConstructor(napi_env env, napi_callback_info info);
    static napi_value ToInt32Value(napi_env env, int32_t number);
    static bool ParseInt32(napi_env env, napi_value argv, int32_t& outValue);
    static bool ParseInt64(napi_env env, napi_value argv, int64_t& outValue);
    static bool ParseDouble(napi_env env, napi_value argv, double& outValue);
    static bool ParseString(napi_env env, napi_value argv, std::string& outValue);
    static bool ParseBoolean(napi_env env, napi_value argv, bool& outValue);
    static bool ParseStringArray(napi_env env, napi_value argv, std::vector<std::string>& outValue);
    static bool ParseBooleanArray(napi_env env, napi_value argv, std::vector<bool>& outValue);
    static bool ParseDoubleArray(napi_env env, napi_value argv, std::vector<double>& outValue);
    static bool ParseInt64Array(napi_env env, napi_value argv, std::vector<int64_t>& outValue);
    static bool ParseFloat(napi_env env, napi_value argv, float& outValue);
    static bool ParseSize(napi_env env, napi_value argv, size_t& outValue);
    static bool ParseChar(napi_env env, napi_value argv, char* buffer, size_t bufferSize);
    static bool ConvertWebToNapiValue(napi_env env, std::shared_ptr<WebMessage> src, napi_value& dst);
};
} // namespace OHOS::Plugin
#endif