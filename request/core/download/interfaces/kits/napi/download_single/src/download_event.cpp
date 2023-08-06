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

#include "download_event.h"

#include "download_base_notify.h"
#include "download_manager.h"
#include "i_download_task.h"
#include "log.h"
#include "napi_utils.h"

namespace OHOS::Plugin::Request::Download {
napi_value DownloadEvent::On(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    size_t argc = NapiUtils::MAX_ARGC;
    napi_value argv[NapiUtils::MAX_ARGC] = { nullptr };
    napi_value thisVal = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, &data));
    if (argc != NapiUtils::TWO_ARG) {
        DOWNLOAD_HILOGE("Wrong number of arguments, requires 1");
        return result;
    }

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, argv[NapiUtils::FIRST_ARGV], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "type is not a string");
    char event[NapiUtils::MAX_LEN] = { 0 };
    size_t len = 0;
    napi_get_value_string_utf8(env, argv[NapiUtils::FIRST_ARGV], event, NapiUtils::MAX_LEN, &len);
    std::string type = event;
    DOWNLOAD_HILOGD("type : %{public}s", type.c_str());

    valuetype = napi_undefined;
    napi_typeof(env, argv[NapiUtils::SECOND_ARGV], &valuetype);
    NAPI_ASSERT(env, valuetype == napi_function, "callback is not a function");

    IDownloadTask *task;
    NAPI_CALL(env, napi_unwrap(env, thisVal, reinterpret_cast<void **>(&task)));
    if (task == nullptr || !task->IsSupportType(type)) {
        DOWNLOAD_HILOGD("Event On type : %{public}s not support", type.c_str());
        return result;
    }
    napi_ref callbackRef = nullptr;
    napi_create_reference(env, argv[argc - 1], 1, &callbackRef);

    auto listener = CreateNotify(env, type, callbackRef);
    if (listener == nullptr) {
        DOWNLOAD_HILOGD("DownloadPause create callback object fail");
        return result;
    }
    task->AddListener(type, listener);
    DownloadManager::GetInstance().On(task->GetId(), type, listener);
    return result;
}

napi_value DownloadEvent::Off(napi_env env, napi_callback_info info)
{
    auto context = std::make_shared<EventOffContext>();
    napi_value result = nullptr;
    size_t argc = NapiUtils::MAX_ARGC;
    napi_value argv[NapiUtils::MAX_ARGC] = { nullptr };
    napi_value thisVal = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, &data));
    if (argc != NapiUtils::ONE_ARG && argc != NapiUtils::TWO_ARG) {
        DOWNLOAD_HILOGE("Wrong number of arguments, requires 1 or 2");
        return result;
    }

    napi_valuetype valuetype;
    NAPI_CALL(env, napi_typeof(env, argv[NapiUtils::FIRST_ARGV], &valuetype));
    NAPI_ASSERT(env, valuetype == napi_string, "type is not a string");
    char event[NapiUtils::MAX_LEN] = { 0 };
    size_t len = 0;
    napi_get_value_string_utf8(env, argv[NapiUtils::FIRST_ARGV], event, NapiUtils::MAX_LEN, &len);
    context->type_ = event;
    DOWNLOAD_HILOGD("type : %{public}s", context->type_.c_str());

    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        napi_status status = napi_get_boolean(env, context->result, result);
        DOWNLOAD_HILOGD("context->result = %{public}d, result = %{public}p", context->result, result);
        return status;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        if (context->task_ == nullptr || !context->task_->IsSupportType(context->type_)) {
            DOWNLOAD_HILOGD("Event Off type : %{public}s not support", context->type_.c_str());
            return;
        }
        context->result = DownloadManager::GetInstance().Off(context->task_->GetId(), context->type_);
        if (context->result == true) {
            context->task_->RemoveListener(context->type_);
        }
    };
    context->SetAction(std::move(input), std::move(output));
    AsyncCall asyncCall(env, info, std::dynamic_pointer_cast<AsyncCall::Context>(context),
        context->type_, NapiUtils::SECOND_ARGV);
    return asyncCall.Call(env, exec);
}

uint32_t DownloadEvent::GetParamNumber(const std::string &type)
{
    if (type == EVENT_PROGRESS) {
        return TWO_PARAMETER;
    } else if (type == EVENT_FAIL) {
        return ONE_PARAMETER;
    }
    return NO_PARAMETER;
}

std::shared_ptr<DownloadNotifyInterface> DownloadEvent::CreateNotify(napi_env env,
    const std::string &type, napi_ref callbackRef)
{
    uint32_t paramNumber = GetParamNumber(type);
    return std::make_shared<DownloadBaseNotify>(env, paramNumber, callbackRef);
}
} // namespace OHOS::Plugin::Request::Download
