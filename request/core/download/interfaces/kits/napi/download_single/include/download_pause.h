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

#ifndef DOWNLOAD_PAUSE_H
#define DOWNLOAD_PAUSE_H

#include "async_call.h"
#include "i_download_task.h"
#include "napi/native_api.h"
#include "noncopyable.h"

namespace OHOS::Plugin::Request::Download {
class DownloadPause final {
public:
    ACE_DISALLOW_COPY_AND_MOVE(DownloadPause);

    DownloadPause() = default;
    ~DownloadPause() = default;

    // static napi_value Pause(napi_env env, napi_callback_info info);
    static napi_value Suspend(napi_env env, napi_callback_info info);

private:
    struct PauseContext : public AsyncCall::Context {
        IDownloadTask *task_ = nullptr;
        bool result = false;
        napi_status status = napi_generic_failure;
        PauseContext() : Context(nullptr, nullptr) {}
        PauseContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)) {}
        virtual ~PauseContext() {}

        napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
        {
            NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
            NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&task_)), napi_invalid_arg);
            NAPI_ASSERT_BASE(env, task_ != nullptr, "there is no native task", napi_invalid_arg);
            return Context::operator()(env, argc, argv, self);
        }
        napi_status operator()(napi_env env, napi_value *result) override
        {
            if (status != napi_ok) {
                return status;
            }
            return Context::operator()(env, result);
        }
    };
    static napi_value Exec(napi_env env, napi_callback_info info);
};
} // namespace OHOS::Plugin::Request::Download

#endif // DOWNLOAD_PAUSE_H
