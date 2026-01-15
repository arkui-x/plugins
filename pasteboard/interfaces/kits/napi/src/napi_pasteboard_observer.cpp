/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

#include <sys/syscall.h>
#include <thread>

#include "pasteboard_hilog.h"
#include "pasteboard_observer_napi.h"
#include "unistd.h"

using namespace OHOS::MiscServices;

namespace OHOS {
namespace MiscServicesNapi {
PasteboardObserverImpl::PasteboardObserverImpl(napi_threadsafe_function callback, napi_env env)
    : callback_(callback), env_(env)
{}

PasteboardObserverImpl::~PasteboardObserverImpl()
{
    napi_status status = napi_release_threadsafe_function(callback_, napi_tsfn_release);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "object release and destroyed, status=%{public}d", status);
}

void PasteboardObserverImpl::OnPasteboardChanged()
{
    pid_t threadId = syscall(SYS_gettid);
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_JS_NAPI, "enter");
    auto self = shared_from_this();
    auto task = [self, threadId]() {
        PASTEBOARD_HILOGI(PASTEBOARD_MODULE_JS_NAPI, "napi_send_event start, originTid=%{public}d", threadId);
        napi_status status = napi_acquire_threadsafe_function(self->callback_);
        PASTEBOARD_CHECK_AND_RETURN_LOGE(
            status == napi_ok, PASTEBOARD_MODULE_JS_NAPI, "acquire callback failed, status=%{public}d", status);
        status = napi_call_threadsafe_function(self->callback_, nullptr, napi_tsfn_blocking);
        PASTEBOARD_CHECK_AND_RETURN_LOGE(
            status == napi_ok, PASTEBOARD_MODULE_JS_NAPI, "call callback failed, status=%{public}d", status);
        status = napi_release_threadsafe_function(self->callback_, napi_tsfn_release);
        PASTEBOARD_CHECK_AND_RETURN_LOGE(
            status == napi_ok, PASTEBOARD_MODULE_JS_NAPI, "release callback failed, status=%{public}d", status);
    };
    auto ret = napi_send_event(env_, task, napi_eprio_high);
    PASTEBOARD_CHECK_AND_RETURN_LOGE(
        ret == napi_ok, PASTEBOARD_MODULE_JS_NAPI, "napi_send_event failed, result=%{public}d", ret);
}

PasteboardObserverInstance::PasteboardObserverInstance(napi_threadsafe_function callback, napi_env env)
{
    impl_ = std::make_shared<PasteboardObserverImpl>(callback, env);
}

void PasteboardObserverInstance::OnPasteboardChanged()
{
    if (impl_ == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "impl_ is nullptr");
        return;
    }
    impl_->OnPasteboardChanged();
}

PasteboardNapiScope::PasteboardNapiScope(napi_env env)
{
    env_ = env;
    napi_status status = napi_open_handle_scope(env_, &scope_);
    if (status != napi_ok || scope_ == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_JS_NAPI, "open handle scope failed, status = %{public}d", status);
        scope_ = nullptr;
    }
}

PasteboardNapiScope::~PasteboardNapiScope()
{
    if (scope_ != nullptr && env_ != nullptr) {
        napi_close_handle_scope(env_, scope_);
    }
    scope_ = nullptr;
    env_ = nullptr;
}
} // namespace MiscServicesNapi
} // namespace OHOS