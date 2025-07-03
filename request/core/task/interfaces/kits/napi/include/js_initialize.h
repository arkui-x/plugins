/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_REQUEST_JS_INITIALIZE_H
#define PLUGINS_REQUEST_JS_INITIALIZE_H

#include "async_call.h"
#include "constant.h"
#include "i_task.h"

namespace OHOS::Plugin::Request {
static constexpr uint32_t TOKEN_MAX_BYTES = 2048;
static constexpr uint32_t TOKEN_MIN_BYTES = 8;

class JsInitialize {
public:
    JsInitialize() = default;
    ~JsInitialize() = default;

    static napi_value Initialize(napi_env env, napi_callback_info info, Version version);
    static void CreatProperties(napi_env env, napi_value &self, napi_value config, ITask *task);
private:
    static ExceptionError InitParam(napi_env env, napi_value* argv, Config &config);
    static bool ParseConfig(napi_env env, napi_value jsConfig, Config &config, std::string &errInfo);
    static bool ParseConfigV9(napi_env env, napi_value jsConfig, Config &config, std::string &errInfo);
    static bool ParseUploadConfig(napi_env env, napi_value jsConfig, Config &config, std::string &errInfo);
    static bool ParseDownloadConfig(napi_env env, napi_value jsConfig, Config &config, std::string &errInfo);
    static bool ParseProxy(napi_env env, napi_value jsConfig, std::string &proxy, std::string &errInfo);
    static bool ParseAction(napi_env env, napi_value jsConfig, Action &action);
    static bool ParseUrl(napi_env env, napi_value jsConfig, std::string &url);
    static bool ParseData(napi_env env, napi_value jsConfig, Config &config);
    static bool ParseIndex(napi_env env, napi_value jsConfig, Config &config);
    static bool ParseName(napi_env env, napi_value jsVal, std::string &name);
    static bool ParseTitle(napi_env env, napi_value jsConfig, Config &config);
    static void ParseNetwork(napi_env env, napi_value jsConfig, Network &network);
    static void ParseMethod(napi_env env, napi_value jsConfig, Config &config);
    static void ParseRedirect(napi_env env, napi_value jsConfig, bool &redirect);
    static void ParseRoaming(napi_env env, napi_value jsConfig, Config &config);
    static void ParseRetry(napi_env env, napi_value jsConfig, bool &retry);
    static void ParseSaveas(napi_env env, napi_value jsConfig, Config &config);
    static bool ParseToken(napi_env env, napi_value jsConfig, Config &config);
    static bool ParseDescription(napi_env env, napi_value jsConfig, std::string &description);
    static int64_t ParseEnds(napi_env env, napi_value jsConfig);
    static int64_t ParseBegins(napi_env env, napi_value jsConfig);
    static uint32_t ParsePriority(napi_env env, napi_value jsConfig);
    static Mode ParseMode(napi_env env, napi_value jsConfig);
    static std::map<std::string, std::string> ParseMap(napi_env env, napi_value jsConfig,
        const std::string &propertyName);

    static bool GetFormItems(napi_env env, napi_value jsVal, std::vector<FormItem> &forms,
        std::vector<FileSpec> &files);
    static bool Convert2FormItems(napi_env env, napi_value jsValue, std::vector<FormItem> &forms,
        std::vector<FileSpec> &files);
    static bool Convert2FileSpecs(napi_env env, napi_value jsValue, const std::string &name,
        std::vector<FileSpec> &files);
    static bool Convert2FileSpec(napi_env env, napi_value jsValue, const std::string &name, FileSpec &file);
    static ExceptionError CheckFilePath(Config &config);
    static ExceptionError CheckUploadBodyFiles(Config &config, const std::string &filePath);
    static ExceptionError GetFD(const std::string &path, const Config &config, int32_t &fd);
    static void InterceptData(const std::string &str, const std::string &in, std::string &out);
    static void CheckPartialDownload(Config &config);
    static void CheckFileUri(const Config &config, FileSpec &file);
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_JS_INITIALIZE_H