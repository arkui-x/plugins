/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "error_util.h"

#include <string>
#include <unordered_map>

namespace OHOS {
namespace Global {
namespace I18n {
static const std::unordered_map<int32_t, std::string> ErrorCodeToMsg {
    {I18N_NO_PERMISSION, "The application does not have permission to call this function"},
    {I18N_NOT_VALID, "Param value not valid"},
    {I18N_NOT_FOUND, "Check param failed"},
    {I18N_OPTION_NOT_VALID, "Invalid option name"}
};

void ErrorUtil::NapiThrow(napi_env env, int32_t errCode, bool throwError)
{
    if (!throwError) {
        return;
    }
    napi_value code = nullptr;
    napi_create_string_latin1(env, std::to_string(errCode).c_str(), NAPI_AUTO_LENGTH, &code);

    napi_value message = nullptr;
    auto iter = ErrorCodeToMsg.find(errCode);
    std::string errMsg = iter != ErrorCodeToMsg.end() ? iter->second : "";
    napi_create_string_latin1(env, errMsg.c_str(), NAPI_AUTO_LENGTH, &message);

    napi_value error = nullptr;
    napi_create_error(env, code, message, &error);
    napi_throw(env, error);
}

} // namespace I18n
} // namespace Global
} // namespace OHOS