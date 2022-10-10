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
#ifndef PLUGINS_REQUEST_UPLOAD_JS_UTIL_H
#define PLUGINS_REQUEST_UPLOAD_JS_UTIL_H

#include <cstdint>
#include <map>
#include <vector>

#include "upload_common.h"
#include "upload_config.h"
#include "upload_hilog_wrapper.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

namespace  OHOS::Plugin::Request::UploadNapi {
class JSUtil {
public:
    static constexpr int32_t MAX_ARGC = 6;
    static constexpr int32_t MAX_NUMBER_BYTES = 8;
    static constexpr int32_t MAX_LEN = 4096;
    static constexpr const char *SEPARATOR = ": ";

    static std::string Convert2String(napi_env env, napi_value jsString);
    static napi_value Convert2JSString(napi_env env, const std::string &cString);
    static napi_value Convert2JSValue(napi_env env, int32_t value);
    static std::vector<std::string> Convert2StrVector(napi_env env, napi_value value);
    static napi_value Convert2JSStringVector(napi_env env, const std::vector<std::string> &cStrings);
    static napi_value Convert2JSUploadResponse(napi_env env, const Upload::UploadResponse &response);
    static void ParseFunction(napi_env env, napi_value &object, const char *name, bool &hasFunction, napi_ref &output);
    static std::vector<std::string> Convert2Header(napi_env env, napi_value value);

    static std::shared_ptr<Upload::UploadConfig> Convert2UploadConfig(napi_env env, napi_value jsConfig);
    static napi_value Convert2JSUploadConfig(napi_env env, const Upload::UploadConfig &config);

    static Upload::File Convert2File(napi_env env, napi_value jsFile);
    static napi_value Convert2JSFile(napi_env env, const Upload::File &file);

    static std::vector<Upload::File> Convert2FileVector(napi_env env, napi_value jsFiles);
    static napi_value Convert2JSFileVector(napi_env env, const std::vector<Upload::File> &files);

    static Upload::RequestData Convert2RequestData(napi_env env, napi_value jsRequestData);
    static napi_value Convert2JSRequestData(napi_env env, const Upload::RequestData &requestData);

    static std::vector<Upload::RequestData> Convert2RequestDataVector(napi_env env, napi_value jsRequestDatas);
    static napi_value Convert2JSRequestDataVector(napi_env env, const std::vector<Upload::RequestData> &requestDatas);

    static napi_value Convert2JSValue(napi_env env, const std::vector<int32_t> &cInts);
    static napi_value Convert2JSValue(napi_env env, const std::vector<Upload::TaskState> &taskStates);
    static bool Equals(napi_env env, napi_value value, napi_ref copy);
};
}
#endif // REQUEST_JS_UTIL_H
