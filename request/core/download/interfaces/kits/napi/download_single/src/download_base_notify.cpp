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

#include "download_base_notify.h"

#include <uv.h>

#include "log.h"
#include "napi_utils.h"

namespace OHOS::Plugin::Request::Download {
DownloadBaseNotify::DownloadBaseNotify(napi_env env, uint32_t paramNumber, napi_ref ref)
{
    notifyData_ = std::make_shared<NotifyData>();
    notifyData_->env = env;
    notifyData_->paramNumber = paramNumber;
    notifyData_->ref = ref;
}

DownloadBaseNotify::~DownloadBaseNotify()
{
    if (notifyData_->ref != nullptr) {
        napi_delete_reference(notifyData_->env, notifyData_->ref);
    }
    DOWNLOAD_HILOGD("");
}

void DownloadBaseNotify::OnCallBack(const std::vector<uint32_t> &params)
{
    DOWNLOAD_HILOGD("on callback in");
    NotifyDataPtr *notifyDataPtr = GetNotifyDataPtr();
    if (notifyDataPtr == nullptr) {
        DOWNLOAD_HILOGE("Failed to get ptr");
        return;
    }

    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(notifyData_->env, &loop);
    if (loop == nullptr) {
        DOWNLOAD_HILOGE("Failed to get uv event loop");
        delete notifyDataPtr;
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        DOWNLOAD_HILOGE("Failed to create uv work");
        delete notifyDataPtr;
        return;
    }
    
    notifyData_->params = params;
    notifyDataPtr->notifyData = notifyData_;
    work->data = notifyDataPtr;
    int ret = uv_queue_work(
        loop, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int statusInt) {
            NotifyDataPtr *notifyDataPtr = static_cast<NotifyDataPtr *>(work->data);
            if (notifyDataPtr != nullptr) {
                napi_value undefined = 0;
                if (notifyDataPtr->notifyData != nullptr) {
                    napi_get_undefined(notifyDataPtr->notifyData->env, &undefined);
                    napi_value callbackFunc = nullptr;
                    napi_get_reference_value(notifyDataPtr->notifyData->env,
                        notifyDataPtr->notifyData->ref, &callbackFunc);
                    napi_value callbackResult = nullptr;
                    napi_value callbackValues[Download::TWO_PARAMETER] = { 0 };
                    for (uint32_t i = 0; i < notifyDataPtr->notifyData->paramNumber; i++) {
                        napi_create_uint32(notifyDataPtr->notifyData->env, notifyDataPtr->notifyData->params[i],
                            &callbackValues[i]);
                    }
                    napi_call_function(notifyDataPtr->notifyData->env, nullptr, callbackFunc,
                        notifyDataPtr->notifyData->paramNumber, callbackValues, &callbackResult);
                }
                delete notifyDataPtr;
            }
            delete work;
        });
    if (ret != 0) {
        delete notifyDataPtr;
        delete work;
    }
}

NotifyDataPtr *DownloadBaseNotify::GetNotifyDataPtr()
{
    return new (std::nothrow) NotifyDataPtr;
}
} // namespace OHOS::Plugin::Request::Download