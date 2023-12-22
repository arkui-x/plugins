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

#include "js_notify.h"

#include <uv.h>
#include "log.h"
#include "napi_utils.h"

namespace OHOS::Plugin::Request {
struct CallbackHolder {
    std::shared_ptr<JsNotify> self {nullptr};
    std::string params;
    napi_env env;
};

JsNotify::JsNotify(napi_env env, napi_value cb, Version version, Action action) : env_(env), version_(version), action_(action)
{
    napi_ref ref = nullptr;
    napi_status ret = NapiUtils::GetFunctionRef(env, cb, ref);
    if (ret != napi_ok) {
        REQUEST_HILOGE("GetFunctionRef fail %{public}d", ret);
        return;
    }
    callback_ = std::make_shared<JsRefHolder>(env, ref);
}

JsNotify::~JsNotify()
{
}

void JsNotify::OnCallback(const std::string &params)
{
    REQUEST_HILOGI("on callback in");
    uv_loop_s *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        REQUEST_HILOGE("Failed to get uv event loop");
        return;
    }
    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        REQUEST_HILOGE("Failed to create uv work");
        return;
    }

    CallbackHolder *holder = new (std::nothrow) CallbackHolder();
    if (holder == nullptr) {
        REQUEST_HILOGE("holder is null");
        delete work;
        return;
    }
    holder->self = shared_from_this();
    holder->params = params;
    holder->env = env_;

    work->data = reinterpret_cast<void *>(holder);
    int ret = uv_queue_work(
        loop, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int statusInt) {
            auto holder = static_cast<CallbackHolder *>(work->data);
            if (holder != nullptr && holder->self != nullptr) {
                napi_value undefined = 0;
                napi_get_undefined(holder->env, &undefined);
                napi_value callbackFunc = nullptr;
                napi_get_reference_value(holder->env, holder->self->callback_->Get(), &callbackFunc);
                holder->self->HandleCallback(holder->env, callbackFunc, holder->params);
                delete holder;
            }
            delete work;
        });
    if (ret != 0) {
        delete work;
    }
}

napi_ref JsNotify::GetRef() const
{
    if (callback_ != nullptr) {
        return callback_->Get();
    }
    return nullptr;
}
} // namespace OHOS::Plugin::Request