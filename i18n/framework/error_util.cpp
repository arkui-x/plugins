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
    {I18N_NO_PERMISSION,
        "Permission verification failed. The application does not have the permission required to call the API."},
    {I18N_NOT_SYSTEM_APP,
        "Permission verification failed. A non-system application calls a system API."},
    {I18N_NOT_VALID, "Invalid parameter"},
    {I18N_NOT_FOUND, "Parameter error"},
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

void ErrorUtil::NapiThrow(napi_env env, int32_t errCode, const std::string& valueName,
    const std::string& valueContent, bool throwError)
{
    if (!throwError) {
        return;
    }
    napi_value code = nullptr;
    napi_create_string_latin1(env, std::to_string(errCode).c_str(), NAPI_AUTO_LENGTH, &code);

    napi_value message = nullptr;
    auto iter = ErrorCodeToMsg.find(errCode);
    std::string errMsg = iter != ErrorCodeToMsg.end() ? iter->second : "";
    std::string allErrMsg;

    if (errCode == I18N_NO_PERMISSION || errCode == I18N_NOT_SYSTEM_APP) {
        allErrMsg = errMsg;
    } else if (errCode == I18N_NOT_VALID) {
        allErrMsg = errMsg + ", the " + valueName + " must be " + valueContent + ".";
    } else if (valueContent.length() == 0) {
        allErrMsg = errMsg + ", the " + valueName + " cannot be empty.";
    } else {
        allErrMsg = errMsg + ", the type of " + valueName + " must be " + valueContent + ".";
    }

    napi_create_string_latin1(env, allErrMsg.c_str(), NAPI_AUTO_LENGTH, &message);

    napi_value error = nullptr;
    napi_create_error(env, code, message, &error);
    napi_throw(env, error);
}
} // namespace I18n
} // namespace Global
} // namespace OHOS