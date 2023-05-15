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

#include <cinttypes>

#include "download_query.h"
#include "download_manager.h"
#include "log.h"
#include "napi_utils.h"

namespace OHOS::Plugin::Request::Download {
napi_value DownloadQuery::Query(napi_env env, napi_callback_info info)
{
    DOWNLOAD_HILOGD("Enter ---->");
    return Exec(env, info);
}

napi_value DownloadQuery::GetTaskInfo(napi_env env, napi_callback_info info)
{
    DOWNLOAD_HILOGD("Enter ---->");
    ExceptionError err;
    if (!NapiUtils::CheckParameterCorrect(env, info, FUNCTION_GET_TASK_INFO, err)) {
        DOWNLOAD_HILOGE("%{public}s", err.errInfo.c_str());
        NapiUtils::ThrowError(env, err.code, err.errInfo);
        return nullptr;
    }
    return Exec(env, info);
}

napi_value DownloadQuery::Exec(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<QueryContext>();
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
        // create object with download info
        DOWNLOAD_HILOGD("description: %{public}s", context->info.GetDescription().c_str());
        DOWNLOAD_HILOGD("downloadedBytes: %{public}" PRId64, context->info.GetDownloadedBytes());
        DOWNLOAD_HILOGD("downloadId: %{public}d", context->info.GetDownloadId());
        DOWNLOAD_HILOGD("failedReason: %{public}d", context->info.GetFailedReason());
        DOWNLOAD_HILOGD("fileName: %{public}s", context->info.GetFileName().c_str());
        DOWNLOAD_HILOGD("filePath: %{public}s", context->info.GetFilePath().c_str());
        DOWNLOAD_HILOGD("pausedReason: %{public}d", context->info.GetPausedReason());
        DOWNLOAD_HILOGD("status: %{public}d", context->info.GetStatus());
        DOWNLOAD_HILOGD("targetURI: %{public}s", context->info.GetTargetURI().c_str());
        DOWNLOAD_HILOGD("downloadTitle: %{public}s", context->info.GetDownloadTitle().c_str());
        DOWNLOAD_HILOGD("downloadTotalBytes: %{public}" PRId64, context->info.GetDownloadTotalBytes());
        napi_create_object(env, result);

        NapiUtils::SetStringPropertyUtf8(env, *result, "description", context->info.GetDescription().c_str());
        NapiUtils::SetUint32Property(env, *result, "downloadedBytes", context->info.GetDownloadedBytes());
        NapiUtils::SetUint32Property(env, *result, "downloadId", context->info.GetDownloadId());
        NapiUtils::SetUint32Property(env, *result, "failedReason", context->info.GetFailedReason());
        NapiUtils::SetStringPropertyUtf8(env, *result, "fileName", context->info.GetFileName().c_str());
        NapiUtils::SetStringPropertyUtf8(env, *result, "filePath", context->info.GetFilePath().c_str());
        NapiUtils::SetUint32Property(env, *result, "pausedReason", context->info.GetPausedReason());
        NapiUtils::SetUint32Property(env, *result, "status", context->info.GetStatus());
        NapiUtils::SetStringPropertyUtf8(env, *result, "targetURI", context->info.GetTargetURI().c_str());
        NapiUtils::SetStringPropertyUtf8(env, *result, "downloadTitle", context->info.GetDownloadTitle().c_str());
        NapiUtils::SetUint32Property(env, *result, "downloadTotalBytes", context->info.GetDownloadTotalBytes());
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        if (DownloadManager::GetInstance().GetTaskInfo(context->task_->GetId(), context->info) == true) {
            context->status = napi_ok;
        }
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context), "", 0);
    return asyncCall.Call(env, exec);
}
} // namespace OHOS::Plugin::Request::Download
