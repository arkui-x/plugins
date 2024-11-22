/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#include "js_ref_holder.h"

#include <uv.h>
#include "napi/native_api.h"
#include "napi/native_common.h"

#include "log.h"
#include "napi_utils.h"

namespace OHOS::Plugin::Request {
struct DeleteRefHolder {
    napi_env env {nullptr};
    napi_ref ref {nullptr};
};

void DestoryDeleteWork(uv_work_t *work)
{
    if (work == nullptr) {
        return;
    }
    if (work->data != nullptr) {
        delete (reinterpret_cast<DeleteRefHolder *>(work->data));
    }
    delete work;
}

void OnDeleteRefWork(uv_work_t *work, int status)
{
    REQUEST_HILOGI("start");
    if (work == nullptr) {
        REQUEST_HILOGE("work is null");
        return;
    }
    DeleteRefHolder *deleteRefHolder = reinterpret_cast<DeleteRefHolder *>(work->data);
    if (deleteRefHolder == nullptr) {
        REQUEST_HILOGE("deleteRefHolder is invalid");
        DestoryDeleteWork(work);
        return;
    }
    napi_status ret = napi_delete_reference(deleteRefHolder->env, deleteRefHolder->ref);
    if (ret != napi_ok) {
        REQUEST_HILOGE("napi_delete_reference fail %{public}d", ret);
        DestoryDeleteWork(work);
        return;
    }
    DestoryDeleteWork(work);
}

JsRefHolder::JsRefHolder(napi_env env, napi_ref ref) : env_(env), ref_(ref)
{
}

JsRefHolder::~JsRefHolder()
{
    if (!IsValid()) {
        REQUEST_HILOGE("invalid");
        return;
    }
    REQUEST_HILOGI("delete reference");
    uv_loop_s *loop;
    napi_status napiStatus = napi_get_uv_event_loop(env_, &loop);
    if (napiStatus != napi_ok || loop == nullptr) {
        REQUEST_HILOGE("napi_get_uv_event_loop fail");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        REQUEST_HILOGE("work is null");
        return;
    }
    DeleteRefHolder *deleteRefHolder = new (std::nothrow) DeleteRefHolder();
    if (deleteRefHolder == nullptr) {
        REQUEST_HILOGE("deleteRefHolder is null");
        delete work;
        return;
    }
    deleteRefHolder->env = env_;
    deleteRefHolder->ref = ref_;
    work->data = reinterpret_cast<void *>(deleteRefHolder);
    if (uv_queue_work(loop, work, [](uv_work_t *work) {}, OnDeleteRefWork) != 0) {
        REQUEST_HILOGE("uv_queue_work fail");
        DestoryDeleteWork(work);
    }
}

bool JsRefHolder::IsValid() const
{
    return (env_ != nullptr && ref_ != nullptr);
}

napi_ref JsRefHolder::Get()
{
    return ref_;
}

napi_ref JsRefHolder::Get() const
{
    return ref_;
}
} // namespace OHOS::Plugin::Request