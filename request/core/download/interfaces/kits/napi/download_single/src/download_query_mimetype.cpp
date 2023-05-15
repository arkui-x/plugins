/*
 * Copyright (C) 2021-2023 Huawei Device Co., Ltd.
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

#include "download_query_mimetype.h"

#include "download_manager.h"
#include "log.h"
#include "napi_utils.h"

namespace OHOS::Plugin::Request::Download {
napi_value DownloadQueryMimeType::QueryMimeType(napi_env env, napi_callback_info info)
{
    DOWNLOAD_HILOGD("Enter ---->");
    return Exec(env, info);
}

napi_value DownloadQueryMimeType::GetTaskMimeType(napi_env env, napi_callback_info info)
{
    DOWNLOAD_HILOGD("Enter ---->");
    ExceptionError err;
    if (!NapiUtils::CheckParameterCorrect(env, info, FUNCTION_GET_TASK_MIME_TYPE, err)) {
        DOWNLOAD_HILOGE("%{public}s", err.errInfo.c_str());
        NapiUtils::ThrowError(env, err.code, err.errInfo);
        return nullptr;
    }
    return Exec(env, info);
}

napi_value DownloadQueryMimeType::Exec(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<QueryMimeContext>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (argc != 0) {
            std::string errInfo = "should 0 parameter!";
            DOWNLOAD_HILOGE("%{public}s", errInfo.c_str());
            NapiUtils::ThrowError(env, EXCEPTION_PARAMETER_CHECK, errInfo);
            return napi_invalid_arg;
        }
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        *result = NapiUtils::CreateStringUtf8(env, context->result);
        return context->status;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        DownloadManager::GetInstance().GetTaskMimeType(context->task_->GetId(), context->result);
        if (!context->result.empty()) {
            context->status = napi_ok;
        }
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), "", 0);
    return asyncCall.Call(env, exec);
}
} // namespace OHOS::Plugin::Request::Download
