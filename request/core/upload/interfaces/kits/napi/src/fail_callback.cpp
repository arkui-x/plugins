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

#include "fail_callback.h"
#include "upload_task.h"
#include "upload_task_napi.h"

using namespace OHOS::Plugin::Request::UploadNapi;

namespace OHOS::Plugin::Request::Upload {
FailCallback::FailCallback(ICallbackAbleJudger *judger, napi_env env, napi_value callback)
    :judger_(judger),
    env_(env)
{
    napi_create_reference(env, callback, 1, &callback_);
    napi_get_uv_event_loop(env, &loop_);
}

FailCallback::~FailCallback()
{
    if (callback_ != nullptr) {
        napi_delete_reference(env_, callback_);
    }
}

napi_ref FailCallback::GetCallback()
{
    return callback_;
}

void FailCallback::Fail(const std::vector<TaskState> &taskStates)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "FailCallback::Fail in");
    FailWorker *failWorker = new (std::nothrow)FailWorker(judger_, this, taskStates);
    if (failWorker == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to create FailWorker");
        return;
    }
    uv_work_t *work = new (std::nothrow)uv_work_t();
    if (work == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to create uv work");
        delete failWorker;
        return;
    }
    work->data = failWorker;
    int ret = uv_queue_work(loop_, work,
        [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Fail. uv_queue_work start");
            std::shared_ptr<FailWorker> failWorker(reinterpret_cast<FailWorker *>(work->data),
                [work](FailWorker *data) {
                    delete data;
                    delete work;
            });
            if (!failWorker || !failWorker->callback) {
                return;
            }
            if (!failWorker->judger->JudgeFail(failWorker->callback)) {
                UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Notify. uv_queue_work callback removed!!");
                return;
            }
            napi_value callback = nullptr;
            napi_value args[1];
            napi_value global = nullptr;
            napi_value result;
            napi_status callStatus = napi_generic_failure;
            args[0] = UploadNapi::JSUtil::Convert2JSValue(failWorker->callback->env_, failWorker->taskStates);
            napi_get_reference_value(failWorker->callback->env_,
                                     failWorker->callback->callback_, &callback);
            napi_get_global(failWorker->callback->env_, &global);
            callStatus = napi_call_function(failWorker->callback->env_, global, callback, 1, args, &result);
            if (callStatus != napi_ok) {
                UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI,
                    "Fail callback failed callStatus:%{public}d", callStatus);
            }
        });
    if (ret != 0) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "Fail. uv_queue_work Failed");
        delete failWorker;
        delete work;
    }
}
} // namespace OHOS::Plugin::Request::Upload