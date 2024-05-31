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

#include "business_error.h"

#include <unordered_map>

#include "bundle_errors.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr const char* ERR_MSG_BUSINESS_ERROR = "BusinessError $: ";
constexpr const char* ERR_MSG_PARAM_TYPE_ERROR = "Parameter error. The type of $ must be $.";
constexpr const char* ERR_ZLIB_SRC_FILE_INVALID_MSG = "The Input source file is invalid.";
constexpr const char* ERR_ZLIB_DEST_FILE_INVALID_MSG = "The Input destination file is invalid.";
constexpr const char* ERR_MSG_PARAM_NUMBER_ERROR =
    "BusinessError 401: Parameter error. The number of parameters is incorrect.";
constexpr const char* ERR_MSG_BUNDLE_SERVICE_EXCEPTION = "Bundle manager service is excepted.";
constexpr const char* ERR_ZLIB_SRC_FILE_FORMAT_ERROR_OR_DAMAGED_MSG =
    "The input source file is not ZIP format or damaged.";

static std::unordered_map<int32_t, const char*> ERR_MSG_MAP = {
    { ERROR_PARAM_CHECK_ERROR, ERR_MSG_PARAM_TYPE_ERROR },
    { ERROR_BUNDLE_SERVICE_EXCEPTION, ERR_MSG_BUNDLE_SERVICE_EXCEPTION },
    { ERR_ZLIB_SRC_FILE_INVALID, ERR_ZLIB_SRC_FILE_INVALID_MSG },
    { ERR_ZLIB_DEST_FILE_INVALID, ERR_ZLIB_DEST_FILE_INVALID_MSG },
    { ERR_ZLIB_SRC_FILE_FORMAT_ERROR_OR_DAMAGED, ERR_ZLIB_SRC_FILE_FORMAT_ERROR_OR_DAMAGED_MSG },
};
} // namespace

void BusinessError::ThrowError(napi_env env, int32_t err, const std::string& msg)
{
    napi_value error = CreateError(env, err, msg);
    napi_throw(env, error);
}

void BusinessError::ThrowParameterTypeError(
    napi_env env, int32_t err, const std::string& parameter, const std::string& type)
{
    napi_value error = CreateCommonError(env, err, parameter, type);
    napi_throw(env, error);
}

void BusinessError::ThrowTooFewParametersError(napi_env env, int32_t err)
{
    ThrowError(env, err, ERR_MSG_PARAM_NUMBER_ERROR);
}

napi_value BusinessError::CreateError(napi_env env, int32_t err, const std::string& msg)
{
    napi_value businessError = nullptr;
    napi_value errorCode = nullptr;
    NAPI_CALL(env, napi_create_int32(env, err, &errorCode));
    napi_value errorMessage = nullptr;
    NAPI_CALL(env, napi_create_string_utf8(env, msg.c_str(), NAPI_AUTO_LENGTH, &errorMessage));
    napi_create_error(env, nullptr, errorMessage, &businessError);
    napi_set_named_property(env, businessError, "code", errorCode);
    return businessError;
}

napi_value BusinessError::CreateCommonError(
    napi_env env, int32_t err, const std::string& functionName, const std::string& permissionName)
{
    std::string errMessage = ERR_MSG_BUSINESS_ERROR;
    auto iter = errMessage.find("$");
    if (iter != std::string::npos) {
        errMessage = errMessage.replace(iter, 1, std::to_string(err));
    }
    if (ERR_MSG_MAP.find(err) != ERR_MSG_MAP.end()) {
        errMessage += ERR_MSG_MAP[err];
    }
    iter = errMessage.find("$");
    if (iter != std::string::npos) {
        errMessage = errMessage.replace(iter, 1, functionName);
        iter = errMessage.find("$");
        if (iter != std::string::npos) {
            errMessage = errMessage.replace(iter, 1, permissionName);
        }
    }
    return CreateError(env, err, errMessage);
}
} // namespace AppExecFwk
} // namespace OHOS