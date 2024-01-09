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
#ifndef PLUGINS_REQUEST_ASYNC_CALL_H
#define PLUGINS_REQUEST_ASYNC_CALL_H

#include <functional>
#include <memory>
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "constant.h"

namespace OHOS::Plugin::Request {
class AsyncCall final {
public:
    class Context {
    public:
        using InputAction = std::function<napi_status(napi_env, size_t, napi_value *, napi_value)>;
        using OutputAction = std::function<napi_status(napi_env, napi_value *)>;
        using ExecAction = std::function<void(Context *)>;
        virtual ~Context() {}
        void SetAction(InputAction input, OutputAction output = nullptr)
        {
            input_ = std::move(input);
            output_ = std::move(output);
        }

        void SetAction(OutputAction output)
        {
            SetAction(nullptr, output);
        }

        virtual napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self)
        {
            if (input_ == nullptr) {
                return napi_ok;
            }
            return input_(env, argc, argv, self);
        }

        virtual napi_status operator()(napi_env env, napi_value *result)
        {
            if (output_ == nullptr) {
                *result = nullptr;
                return napi_ok;
            }
            return output_(env, result);
        }

        virtual void Exec()
        {
            if (exec_ == nullptr) {
                return;
            }
            exec_(this);
        };
        int32_t retCode = E_OK;

    protected:
        friend class AsyncCall;
        InputAction input_ = nullptr;
        OutputAction output_ = nullptr;
        ExecAction exec_ = nullptr;
    };

    static constexpr size_t ASYNC_DEFAULT_POS = -1;
    AsyncCall(napi_env env, napi_callback_info info, std::shared_ptr<Context> context,
        const std::string &type = "", size_t pos = ASYNC_DEFAULT_POS);
    ~AsyncCall();
    napi_value Call(napi_env env, Context::ExecAction exec = nullptr);
    napi_value SyncCall(napi_env env, Context::ExecAction exec = nullptr);

private:
    enum { ARG_ERROR, ARG_DATA, ARG_BUTT };
    static void OnExecute(napi_env env, void *data);
    static void OnComplete(napi_env env, napi_status status, void *data);
    struct AsyncContext {
        std::shared_ptr<Context> ctx = nullptr;
        napi_ref callback = nullptr;
        napi_ref self = nullptr;
        napi_deferred defer = nullptr;
        napi_async_work work = nullptr;
        std::string type;
    };
    static void DeleteContext(napi_env env, AsyncContext *context);
    static void GetOffCallbackParameter(napi_env env, const std::string &type, napi_value (&result)[ARG_BUTT]);
    AsyncContext *context_ = nullptr;
    napi_env env_ = nullptr;
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_ASYNC_CALL_H
