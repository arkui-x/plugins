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

#ifndef PLUGINS_REQUEST_DOWNLOAD_TASK_NAPI_H
#define PLUGINS_REQUEST_DOWNLOAD_TASK_NAPI_H

#include <string>
#include <vector>

#include "async_call.h"
#include "download_config.h"

namespace OHOS::Plugin::Request::Download {
class DownloadTaskNapi {
public:
    static napi_value JsMain(napi_env env, napi_callback_info info);

private:
    static napi_value GetCtor(napi_env env);
    static napi_value Initialize(napi_env env, napi_callback_info info);
    static bool ParseConfig(napi_env env, napi_value configValue, DownloadConfig &config);
    static bool ParseHeader(napi_env env, napi_value configValue, DownloadConfig &config);
    static bool IsPathValid(const std::string &filePath);

private:
    static __thread napi_ref globalCtor;
};
} // namespace OHOS::Plugin::Request::Download
#endif // PLUGINS_REQUEST_DOWNLOAD_TASK_NAPI_H
