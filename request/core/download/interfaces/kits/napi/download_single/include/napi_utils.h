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

#ifndef PLUGINS_REQUEST_DOWNLOAD_NAPI_UTILS_H
#define PLUGINS_REQUEST_DOWNLOAD_NAPI_UTILS_H

#include <string>
#include <vector>
#include <map>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "constant.h"

namespace OHOS::Plugin::Request::Download::NapiUtils {
static constexpr int32_t MAX_ARGC = 6;
static constexpr int32_t NO_ARG = 0;
static constexpr int32_t ONE_ARG = 1;
static constexpr int32_t TWO_ARG = 2;
static constexpr int32_t THE_ARG = 3;

static constexpr int32_t FIRST_ARGV = 0;
static constexpr int32_t SECOND_ARGV = 1;
static constexpr int32_t THIRD_ARGV = 2;

static constexpr int32_t MAX_NUMBER_BYTES = 8;
static constexpr int32_t MAX_LEN = 4096;

static const std::map<ExceptionErrorCode, std::string> ErrorCodeToMsg {
    {EXCEPTION_PERMISSION, EXCEPTION_PERMISSION_INFO },
    {EXCEPTION_PARAMETER_CHECK, EXCEPTION_PARAMETER_CHECK_INFO },
    {EXCEPTION_UNSUPPORTED, EXCEPTION_UNSUPPORTED_INFO },
    {EXCEPTION_FILE_IO, EXCEPTION_FILE_IO_INFO },
    {EXCEPTION_FILE_PATH, EXCEPTION_FILE_PATH_INFO },
    {EXCEPTION_SERVICE_ERROR, EXCEPTION_SERVICE_ERROR_INFO },
    {EXCEPTION_OTHER, EXCEPTION_OTHER_INFO },
};

napi_valuetype GetValueType(napi_env env, napi_value value);

/* named property */
bool HasNamedProperty(napi_env env, napi_value object, const std::string &propertyName);

napi_value GetNamedProperty(napi_env env, napi_value object, const std::string &propertyName);

std::vector<std::string> GetPropertyNames(napi_env env, napi_value object);

/* UINT32 */
napi_value CreateUint32(napi_env env, uint32_t code);

uint32_t GetUint32FromValue(napi_env env, napi_value value);

uint32_t GetUint32Property(napi_env env, napi_value object, const std::string &propertyName);

void SetUint32Property(napi_env env, napi_value object, const std::string &name, uint32_t value);

/* INT32 */
napi_value CreateInt32(napi_env env, int32_t code);

int32_t GetInt32FromValue(napi_env env, napi_value value);

/* INT64 */
napi_value CreateInt64(napi_env env, int64_t code);

void SetInt64Property(napi_env env, napi_value object, const std::string &name, int64_t value);

/* String UTF8 */
napi_value CreateStringUtf8(napi_env env, const std::string &str);

std::string GetStringFromValueUtf8(napi_env env, napi_value value);

std::string GetStringPropertyUtf8(napi_env env, napi_value object, const std::string &propertyName);

void SetStringPropertyUtf8(napi_env env, napi_value object, const std::string &name, const std::string &value);

/* object */
napi_value CreateObject(napi_env env);

/* undefined */
napi_value GetUndefined(napi_env env);

/* function */
napi_value CallFunction(napi_env env, napi_value recv, napi_value func, size_t argc, const napi_value *argv);

/* boolean */
bool GetBooleanProperty(napi_env env, napi_value object, const std::string &propertyName);

std::string ToLower(const std::string &s);

int32_t GetParameterNumber(napi_env env, napi_callback_info info, napi_value *argv, napi_value *this_arg);

void ThrowError(napi_env env, const ExceptionErrorCode &code, const std::string &msg);

napi_value CreateBusinessError(napi_env env, const ExceptionErrorCode &errorCode, const std::string &msg);

bool CheckParameterCorrect(napi_env env, napi_callback_info info, const std::string &type, ExceptionError &err);
} // namespace OHOS::Plugin::Request::Download::NapiUtils

#endif /* PLUGINS_REQUEST_DOWNLOAD_NAPI_UTILS_H */
