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

#ifndef PLUGINS_BRIDGE_NAPI_UTILS_H
#define PLUGINS_BRIDGE_NAPI_UTILS_H

#include <string>

#include "error_code.h"
#include "napi/native_api.h"
#include "nlohmann/json.hpp"
#include "uv.h"

namespace OHOS::Plugin::Bridge {
using Json = nlohmann::json;
class NAPIUtils {
public:
    static bool JsonStringToNapiValues(napi_env env, const std::string& str, size_t& argc, napi_value* argv);
    static bool NapiValuesToJsonString(napi_env env, const size_t& argc, const napi_value* argv, std::string& str);
    static napi_value NAPI_GetParams(napi_env env, Json json);
    static Json PlatformParams(napi_env env, napi_value value);
    static int NAPI_GetErrorCodeFromJson(Json json);
    static napi_value CreateErrorMessage(napi_env env, int32_t errorCode);
};
} // namespace OHOS::Plugin::Bridge
#endif // PLUGINS_BRIDGE_NAPI_UTILS_H
