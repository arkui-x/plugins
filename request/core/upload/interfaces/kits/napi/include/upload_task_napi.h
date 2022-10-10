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

#ifndef PLUGINS_REQUEST_UPLOAD_REQUEST_NAPI_H
#define PLUGINS_REQUEST_UPLOAD_REQUEST_NAPI_H

#include <string>
#include <vector>

#include "async_call.h"
#include "complete_callback.h"
#include "fail_callback.h"
#include "i_callbackable_judger.h"
#include "progress_callback.h"
#include "upload_common.h"
#include "upload_config.h"
#include "upload_task.h"

namespace OHOS::Plugin::Request::UploadNapi {
using namespace OHOS::Plugin::Request::Upload;
class UploadTaskNapi : public ICallbackAbleJudger {
public:
    static napi_value JsUpload(napi_env env, napi_callback_info info);

    static napi_value JsOn(napi_env env, napi_callback_info info);
    static napi_value JsOff(napi_env env, napi_callback_info info);
    static napi_value JsRemove(napi_env env, napi_callback_info info);

    UploadTaskNapi &operator=(std::unique_ptr<Upload::UploadTask> &&uploadTask);
    bool operator==(const std::unique_ptr<Upload::UploadTask> &uploadTask);
    static void OnSystemSuccess(napi_env env, napi_ref ref, Upload::UploadResponse &response);
    static void OnSystemFail(napi_env env, napi_ref ref, std::string &response, int32_t &code);
    static void OnSystemComplete(napi_env env, napi_ref ref);

    bool JudgeFail(const IFailCallback *target);
    bool JudgeComplete(const ICompleteCallback *target);
    bool JudgeProgress(const IProgressCallback *target);
    napi_ref success_;
    napi_ref fail_;
    napi_ref complete_;
    napi_env env_;
private:
    static napi_value GetCtor(napi_env env);
    static napi_value Initialize(napi_env env, napi_callback_info info);

    std::unique_ptr<Upload::UploadTask> napiUploadTask_ = nullptr;
    std::shared_ptr<Upload::UploadConfig> napiUploadConfig_ = nullptr;

    struct RemoveContextInfo : public AsyncCall::Context {
        UploadTaskNapi *proxy = nullptr;
        bool removeStatus = false;
        napi_status status = napi_generic_failure;
        RemoveContextInfo() : Context(nullptr, nullptr) {};
        RemoveContextInfo(InputAction input, OutputAction output) : Context(std::move(input), std::move(output)) {};
        virtual ~RemoveContextInfo() {};

        napi_status operator()(napi_env env, size_t argc, napi_value *argv, napi_value self) override
        {
            NAPI_ASSERT_BASE(env, self != nullptr, "self is nullptr", napi_invalid_arg);
            NAPI_CALL_BASE(env, napi_unwrap(env, self, reinterpret_cast<void **>(&proxy)), napi_invalid_arg);
            NAPI_ASSERT_BASE(env, proxy != nullptr, "there is no native upload task", napi_invalid_arg);
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

    struct SystemFailCallback {
        std::string data;
        int32_t code;
        napi_env env;
        napi_ref ref;
    };

    struct SystemSuccessCallback {
        Upload::UploadResponse response;
        napi_env env;
        napi_ref ref;
    };

    struct SystemCompleteCallback {
        napi_env env;
        napi_ref ref;
    };

    using Exec = std::function<napi_status(napi_env, size_t, napi_value *, napi_value, napi_value *)>;
    static std::map<std::string, Exec> onTypeHandlers_;
    static std::map<std::string, Exec> offTypeHandlers_;

    static napi_status OnProgress(napi_env env, size_t argc, napi_value *argv, napi_value self, napi_value *result);
    static napi_status OnFail(napi_env env, size_t argc, napi_value *argv, napi_value self, napi_value *result);
    static napi_status OnComplete(napi_env env, size_t argc, napi_value *argv, napi_value self, napi_value *result);
    static napi_status OffProgress(napi_env env, size_t argc, napi_value *argv, napi_value self, napi_value *result);
    static napi_status OffFail(napi_env env, size_t argc, napi_value *argv, napi_value self, napi_value *result);
    static napi_status OffComplete(napi_env env, size_t argc, napi_value *argv, napi_value self, napi_value *result);
    static napi_status CheckOffCompleteParam(napi_env env, size_t argc, napi_value *argv, napi_value self);

    std::shared_ptr<Upload::IProgressCallback> onProgress_ = nullptr;
    std::shared_ptr<Upload::IFailCallback> onFail_ = nullptr;
    std::shared_ptr<Upload::ICompleteCallback> onComplete_ = nullptr;
    std::shared_ptr<Upload::IProgressCallback> offProgress_ = nullptr;
    std::shared_ptr<Upload::IFailCallback> offFail_ = nullptr;
    std::shared_ptr<Upload::ICompleteCallback> offComplete_ = nullptr;
};
} // namespace  OHOS::Plugin::Request::UploadNapi
#endif // REQUEST_NAPI_H