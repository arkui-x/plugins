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

#include "progress_callback.h"
#include "upload_task.h"
#include "upload_task_napi.h"

using namespace OHOS::Plugin::Request::UploadNapi;

namespace OHOS::Plugin::Request::Upload {
ProgressCallback::ProgressCallback(ICallbackAbleJudger *judger, napi_env env, napi_value callback)
    :judger_(judger), env_(env)
{
    napi_create_reference(env, callback, 1, &callback_);
    napi_get_uv_event_loop(env, &loop_);
}

ProgressCallback::~ProgressCallback()
{
    if (callback_ != nullptr) {
        napi_delete_reference(env_, callback_);
    }
}

napi_ref ProgressCallback::GetCallback()
{
    return callback_;
}

void ProgressCallback::Progress(const int64_t uploadedSize, const int64_t totalSize)
{
    ProgressWorker *progressWorker = new (std::nothrow)ProgressWorker(judger_, this, uploadedSize, totalSize);
    if (progressWorker == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to create progressWorker");
        return;
    }
    uv_work_t *work = new (std::nothrow)uv_work_t();
    if (work == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to create uv work");
        delete progressWorker;
        return;
    }
    work->data = progressWorker;
    int ret = uv_queue_work(loop_, work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            std::shared_ptr<ProgressWorker> progressWorker(reinterpret_cast<ProgressWorker *>(work->data),
                [work](ProgressWorker *data) {
                    delete data;
                    delete work;
            });
            if (!progressWorker || !progressWorker->judger || !progressWorker->callback) {
                return;
            }
            if (!progressWorker->judger->JudgeProgress(progressWorker->callback)) {
                UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Progress. uv_queue_work callback removed!!");
                return;
            }
            napi_value callback = nullptr;
            napi_value args[2];
            napi_value global = nullptr;
            napi_value result;
            napi_status calStatus = napi_generic_failure;
            napi_create_int64(progressWorker->callback->env_, progressWorker->uploadedSize, &args[0]);
            napi_create_int64(progressWorker->callback->env_, progressWorker->totalSize, &args[1]);
            napi_get_reference_value(progressWorker->callback->env_,
                                     progressWorker->callback->callback_, &callback);
            napi_get_global(progressWorker->callback->env_, &global);
            calStatus = napi_call_function(progressWorker->callback->env_, global, callback, 2, args, &result);
            if (calStatus != napi_ok) {
                UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI,
                    "Progress callback failed calStatus:%{public}d callback:%{public}p", calStatus, callback);
            }
        });
    if (ret != 0) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "Progress. uv_queue_work Failed");
        delete progressWorker;
        delete work;
    }
}
} // namespace OHOS::Plugin::Request::Upload