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

#include "async_call.h"

#include "log.h"
#include "napi_utils.h"

namespace OHOS::Plugin::Request {
AsyncCall::AsyncCall(napi_env env, napi_callback_info info, std::shared_ptr<Context> context,
    const std::string &type, size_t pos) : env_(env)
{
    context_ = new AsyncContext();
    size_t argc = NapiUtils::MAX_ARGC;
    napi_value self = nullptr;
    napi_value argv[NapiUtils::MAX_ARGC] = { nullptr };
    NAPI_CALL_RETURN_VOID(env, napi_get_cb_info(env, info, &argc, argv, &self, nullptr));
    pos = ((pos == ASYNC_DEFAULT_POS) ? (argc - 1) : pos);
    if (pos >= 0 && pos < argc) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[pos], &valueType);
        if (valueType == napi_function) {
            napi_create_reference(env, argv[pos], 1, &context_->callback);
            argc = pos;
        }
    }
    napi_status status = (*context)(env, argc, argv, self);
    if (status == napi_ok) {
        context_->ctx = std::move(context);
        context_->type = type;
        napi_create_reference(env, self, 1, &context_->self);
    }
    REQUEST_HILOGI("input result:%{public}d", static_cast<int32_t>(status));
}

AsyncCall::~AsyncCall()
{
    if (context_ == nullptr) {
        return;
    }

    DeleteContext(env_, context_);
}

napi_value AsyncCall::Call(napi_env env, Context::ExecAction exec)
{
    if (context_ == nullptr) {
        REQUEST_HILOGI("context_ is null");
        return nullptr;
    }
    if ((context_ == nullptr) || (context_->ctx == nullptr)) {
        REQUEST_HILOGI("context_ or context_->ctx is null");
        return nullptr;
    }
    REQUEST_HILOGI("async call exec");
    context_->ctx->exec_ = std::move(exec);
    napi_value promise = nullptr;
    if (context_->callback == nullptr) {
        napi_create_promise(env, &context_->defer, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }
    napi_async_work work = context_->work;
    napi_value resource = nullptr;
    napi_create_string_utf8(env, "AsyncCall", NAPI_AUTO_LENGTH, &resource);
    napi_create_async_work(env, nullptr, resource, AsyncCall::OnExecute, AsyncCall::OnComplete, context_, &work);
    context_->work = work;
    context_ = nullptr;
    napi_queue_async_work(env, work);
    REQUEST_HILOGI("async call exec");
    return promise;
}

napi_value AsyncCall::SyncCall(napi_env env, AsyncCall::Context::ExecAction exec)
{
    if ((context_ == nullptr) || (context_->ctx == nullptr)) {
        REQUEST_HILOGI("context_ or context_->ctx is null");
        return nullptr;
    }
    context_->ctx->exec_ = std::move(exec);
    napi_value promise = nullptr;
    if (context_->callback == nullptr) {
        napi_create_promise(env, &context_->defer, &promise);
    } else {
        napi_get_undefined(env, &promise);
    }
    AsyncCall::OnExecute(env, context_);
    AsyncCall::OnComplete(env, napi_ok, context_);
    return promise;
}

void AsyncCall::OnExecute(napi_env env, void *data)
{
    REQUEST_HILOGI("run the async runnable");
    AsyncContext *context = reinterpret_cast<AsyncContext *>(data);
    context->ctx->Exec();
}

void AsyncCall::OnComplete(napi_env env, napi_status status, void *data)
{
    REQUEST_HILOGI("run the js callback function");
    AsyncContext *context = reinterpret_cast<AsyncContext *>(data);
    napi_value output = nullptr;
    napi_status runStatus = (*context->ctx)(env, &output);
    napi_value result[ARG_BUTT] = { 0 };
    if (status == napi_ok && runStatus == napi_ok) {
        napi_get_null(env, &result[ARG_ERROR]);
        if (output != nullptr) {
            result[ARG_DATA] = output;
        } else {
            napi_get_undefined(env, &result[ARG_DATA]);
        }
    } else {
        result[ARG_ERROR] = NapiUtils::CreateBusinessError(env,
                                static_cast<ExceptionErrorCode>(context->ctx->retCode), "service failed");
        napi_get_undefined(env, &result[ARG_DATA]);
    }
    if (context->defer != nullptr) {
        if (status == napi_ok && runStatus == napi_ok) {
            napi_resolve_deferred(env, context->defer, result[ARG_DATA]);
        } else {
            napi_reject_deferred(env, context->defer, result[ARG_ERROR]);
        }
    } else {
        napi_value callback = nullptr;
        napi_get_reference_value(env, context->callback, &callback);
        napi_value returnValue;
        GetOffCallbackParameter(env, context->type, result);
        NAPI_CALL_RETURN_VOID(env, napi_call_function(env, nullptr, callback, ARG_BUTT, result, &returnValue));
    }
    DeleteContext(env, context);
}

void AsyncCall::DeleteContext(napi_env env, AsyncContext *context)
{
    if (env != nullptr) {
        napi_delete_reference(env, context->callback);
        napi_delete_reference(env, context->self);
        napi_delete_async_work(env, context->work);
    }
    delete context;
}

void AsyncCall::GetOffCallbackParameter(napi_env env, const std::string &type, napi_value (&result)[ARG_BUTT])
{
    REQUEST_HILOGI("type:%{public}s", type.c_str());
    if (type == EVENT_PROGRESS || type == EVENT_FAIL) {
        napi_valuetype valueType = napi_undefined;
        int ret = 0;
        napi_typeof(env, result[ARG_DATA], &valueType);
        if (valueType == napi_boolean) {
            bool status;
            NAPI_CALL_RETURN_VOID(env, napi_get_value_bool(env, result[ARG_DATA], &status));

            if (status == false) {
                ret = -1;
            }
        } else {
            ret = -1;
        }
        REQUEST_HILOGI("ret:%{public}d", ret);
        result[ARG_ERROR] = NapiUtils::Convert2JSValue(env, ret);
        if (type == "progress") {
            result[ARG_DATA] = NapiUtils::Convert2JSValue(env, ret);
        }
    }
}
} // namespace OHOS::Plugin::Request