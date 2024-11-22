/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#include "napi_error.h"

#include "medialibrary_client_errno.h"
#include "medialibrary_napi_utils.h"

using namespace std;

namespace OHOS {
namespace Media {
void NapiError::SetApiName(const std::string &Name)
{
    apiName = Name;
}

void NapiError::SaveError(int32_t ret)
{
    if (ret < 0) {
        error = MediaLibraryNapiUtils::TransErrorCode(apiName, ret);
    }
}

void NapiError::HandleError(napi_env env, napi_value &errorObj)
{
    // deal with context->error
    MediaLibraryNapiUtils::HandleError(env, error, errorObj, apiName);
}

void NapiError::ThrowError(napi_env env, int32_t err, const std::string &errMsg)
{
    string message = errMsg;
    if (message.empty()) {
        message = "operation not support";
        if (jsErrMap.count(err) > 0) {
            message = jsErrMap.at(err);
        }
    }

    LOGE("ThrowError errCode:%{public}d errMsg:%{public}s", err, message.c_str());
    NAPI_CALL_RETURN_VOID(env, napi_throw_error(env, to_string(err).c_str(), message.c_str()));
}

void NapiError::ThrowError(napi_env env, int32_t err, const char *funcName, int32_t line, const std::string &errMsg)
{
    string message = errMsg;
    if (message.empty()) {
        message = "operation not support";
        if (jsErrMap.count(err) > 0) {
            message = jsErrMap.at(err);
        }
    }

    LOGE("{%{public}s:%d} ThrowError errCode:%{public}d errMsg:%{public}s", funcName, line,
        err, message.c_str());
    NAPI_CALL_RETURN_VOID(env, napi_throw_error(env, to_string(err).c_str(), message.c_str()));
}

} // namespace Media
} // namespace OHOS