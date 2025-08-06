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

#ifndef PLUGINS_REQUEST_NAPI_UTILS_H
#define PLUGINS_REQUEST_NAPI_UTILS_H

#include <string>
#include <vector>
#include <map>

#include "napi/native_api.h"
#include "napi/native_common.h"

#include "constant.h"

namespace OHOS::Plugin::Request::NapiUtils {
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

napi_status Convert2JSValue(napi_env env, bool in, napi_value &out);
napi_status Convert2JSValue(napi_env env, std::string &in, napi_value &out);
napi_status Convert2JSValue(napi_env env, const DownloadInfo &in, napi_value &out);
napi_value Convert2JSValue(napi_env env, bool code);
napi_value Convert2JSValue(napi_env env, int32_t code);
napi_value Convert2JSValue(napi_env env, uint32_t code);
napi_value Convert2JSValue(napi_env env, int64_t code);
napi_value Convert2JSValue(napi_env env, uint64_t code);
napi_value Convert2JSValue(napi_env env, const std::vector<int64_t> &code);
napi_value Convert2JSValue(napi_env env, const std::vector<std::string> &ids);
napi_value Convert2JSValue(napi_env env, const std::map<std::string, std::string> &code);
napi_value Convert2JSValue(napi_env env, const std::string &str);
napi_value Convert2JSValue(napi_env env, const std::vector<TaskState> &taskStates);
napi_value Convert2JSValue(napi_env env, const Progress &progress);
napi_value Convert2JSValue(napi_env env, const Response &response);
napi_value Convert2JSValue(napi_env env, TaskInfo &taskInfo);
napi_value Convert2JSValue(napi_env env, const std::vector<FileSpec> &files, const std::vector<FormItem> &forms);
napi_value Convert2JSHeaders(napi_env env, const Progress &progress);
napi_value Convert2JSHeadersAndBody(napi_env env, const std::map<std::string, std::string> &header,
    const std::vector<uint8_t> &bodyBytes, bool isSeparate);
bool Convert2Boolean(napi_env env, napi_value object, const std::string &propertyName);
uint32_t Convert2Uint32(napi_env env, napi_value value);
uint32_t Convert2Uint32(napi_env env, napi_value object, const std::string &propertyName);
int64_t Convert2Int64(napi_env env, napi_value value);
int64_t Convert2Int64(napi_env env, napi_value object, const std::string &propertyName);
std::string Convert2String(napi_env env, napi_value value);
std::string Convert2String(napi_env env, napi_value object, const std::string &propertyName);

int32_t GetParameterNumber(napi_env env, napi_callback_info info, napi_value *argv, napi_value *this_arg);
void ThrowError(napi_env env, ExceptionErrorCode code, const std::string &msg);
napi_value CreateBusinessError(napi_env env, ExceptionErrorCode errorCode, const std::string &errorMessage);
bool CheckParameterCorrect(napi_env env, napi_callback_info info, const std::string &type, ExceptionError &err);

napi_valuetype GetValueType(napi_env env, napi_value value);
bool HasNamedProperty(napi_env env, napi_value object, const std::string &propertyName);
napi_value GetNamedProperty(napi_env env, napi_value object, const std::string &propertyName);
std::vector<std::string> GetPropertyNames(napi_env env, napi_value object);

void SetUint32Property(napi_env env, napi_value object, const std::string &name, uint32_t value);
void SetInt64Property(napi_env env, napi_value object, const std::string &name, int64_t value);
void SetStringPropertyUtf8(napi_env env, napi_value object, const std::string &name, const std::string &value);

Action GetRequestAction(napi_env env, napi_value configValue);
std::vector<FileSpec> Convert2FileVector(napi_env env, napi_value jsFiles);
bool Convert2File(napi_env env, napi_value jsFile, FileSpec &file);
std::vector<FormItem> Convert2RequestDataVector(napi_env env, napi_value jsRequestDatas);
FormItem Convert2RequestData(napi_env env, napi_value jsRequestData);
std::string Convert2ReasonMsg(Reason code);
uint32_t Convert2Broken(Reason code);
std::string GetSaveas(const std::vector<FileSpec> &files, Action action);
bool IsPathValid(const std::string &filePath);
bool IsTextUTF8(const std::vector<uint8_t> &bytes);
napi_status GetFunctionRef(napi_env env, napi_value value, napi_ref &ref);
bool Equals(napi_env env, napi_value value, napi_ref copy);
} // namespace OHOS::Plugin::Request::NapiUtils

#endif // PLUGINS_REQUEST_NAPI_UTILS_H