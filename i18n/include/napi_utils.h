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

#ifndef PLUGINS_I18N_NAPI_UTILS_H
#define PLUGINS_I18N_NAPI_UTILS_H

#include <string>
#include <vector>

#include "napi/native_api.h"
namespace OHOS {
namespace Global {
namespace I18n {
class NAPIUtils {
public:
    static std::string GetString(napi_env &env, napi_value &value, int32_t &code);
    static bool GetStringArrayFromJsParam(
        napi_env env, napi_value &jsArray, const std::string& valueName, std::vector<std::string> &strArray);
    static napi_value CreateString(napi_env env, const std::string &str);
    static void VerifyType(napi_env env, const std::string& valueName, const std::string& type,
        napi_value argv);
};

} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif // PLUGINS_I18N_NAPI_UTILS_H