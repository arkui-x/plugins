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

#include "header_receive_callback.h"
#include "upload_task.h"
using namespace OHOS::Plugin::Request::UploadNapi;

namespace OHOS::Plugin::Request::Upload {
HeaderReceiveCallback::HeaderReceiveCallback(napi_env env, napi_value callback)
    : env_(env)
{
    napi_create_reference(env, callback, 1, &callback_);
    napi_get_uv_event_loop(env, &loop_);
}

HeaderReceiveCallback::~HeaderReceiveCallback()
{
    napi_delete_reference(env_, callback_);
}

napi_ref HeaderReceiveCallback::GetCallback()
{
    return callback_;
}

std::string HeaderReceiveCallback::GetHeader()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return header_;
}

napi_env HeaderReceiveCallback::GetEnv()
{
    return env_;
}

void UvOnHeaderReceive(uv_work_t *work, int status)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "HeaderReceive. uv_queue_work start");
    std::shared_ptr<HeaderReceiveWorker> headerReceiveWorker(reinterpret_cast<HeaderReceiveWorker *>(work->data),
        [work](HeaderReceiveWorker *data) {
        delete data;
        delete work;
    });
    if (headerReceiveWorker == nullptr || headerReceiveWorker->observer == nullptr) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "headerReceiveWorker->observer == nullptr");
        return;
    }
    napi_value jsHeader = nullptr;
    napi_value callback = nullptr;
    napi_value args[1];
    napi_value global = nullptr;
    napi_value result;
    napi_status callStatus = napi_generic_failure;

    jsHeader = UploadNapi::JSUtil::Convert2JSString(headerReceiveWorker->observer->GetEnv(),
        headerReceiveWorker->observer->GetHeader());
    args[0] = { jsHeader };

    napi_get_reference_value(headerReceiveWorker->observer->GetEnv(),
        headerReceiveWorker->observer->GetCallback(), &callback);
    napi_get_global(headerReceiveWorker->observer->GetEnv(), &global);
    callStatus = napi_call_function(headerReceiveWorker->observer->GetEnv(), global, callback, 1, args, &result);
    if (callStatus != napi_ok) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI,
            "HeaderReceive callback failed callStatus:%{public}d callback:%{public}p", callStatus, callback);
    }
}

void HeaderReceiveCallback::HeaderReceive(const std::string &header)
{
    UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "HeaderReceive. header : %{public}s", header.c_str());
    HeaderReceiveWorker *headerReceiveWorker = new (std::nothrow)HeaderReceiveWorker();
    if (headerReceiveWorker == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to create headerReceiveWorker");
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        headerReceiveWorker->observer = shared_from_this();
        header_ = header;
    }

    uv_work_t *work = new (std::nothrow)uv_work_t();
    if (work == nullptr) {
        UPLOAD_HILOGD(UPLOAD_MODULE_JS_NAPI, "Failed to create uv work");
        delete headerReceiveWorker;
        return;
    }
    work->data = headerReceiveWorker;
    int ret = uv_queue_work(loop_, work, [](uv_work_t *work) {}, UvOnHeaderReceive);
    if (ret != 0) {
        UPLOAD_HILOGE(UPLOAD_MODULE_JS_NAPI, "HeaderReceive. uv_queue_work Failed");
        delete headerReceiveWorker;
        delete work;
    }
}
} // namespace OHOS::Plugin::Request::Upload