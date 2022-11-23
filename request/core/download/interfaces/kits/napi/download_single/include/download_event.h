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

#ifndef PLUGINS_REQUEST_DOWNLOAD_EVENT_H
#define PLUGINS_REQUEST_DOWNLOAD_EVENT_H

#include <string>

#include "async_call.h"
#include "download_task.h"
#include "napi/native_api.h"
#include "noncopyable.h"

namespace OHOS::Plugin::Request::Download {
enum EventType {
    NO_ARG_EVENT,
    ONE_ARG_EVENT,
    TWO_ARG_EVENT,
};
class DownloadEvent final {
public:
    ACE_DISALLOW_COPY_AND_MOVE(DownloadEvent);

    DownloadEvent() = default;
    ~DownloadEvent() = default;

    static napi_value On(napi_env env, napi_callback_info info);
    static napi_value Off(napi_env env, napi_callback_info info);
    static uint32_t GetParamNumber(const std::string &type);

private:
	static std::shared_ptr<DownloadNotifyInterface> CreateNotify(napi_env env,
        const std::string &type, napi_ref callbackRef);

private:
    struct EventOffContext : public AsyncCall::Context {
        DownloadTask *task_ = nullptr;
        std::string type_ = "";
        bool result = false;
        napi_status status = napi_generic_failure;
        EventOffContext() : Context(nullptr, nullptr) {};
        EventOffContext(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)) {};
        virtual ~EventOffContext() {};

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
};
} // namespace OHOS::Plugin::Request::Download

#endif // PLUGINS_REQUEST_DOWNLOAD_EVENT_H