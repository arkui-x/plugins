/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "napi_async_event.h"

#include "napi_utils.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "plugins/interfaces/native/inner_utils/plugin_inner_napi_utils.h"

namespace OHOS::Plugin::Bridge {
NAPIAsyncEvent::NAPIAsyncEvent(napi_env env) : env_(env) {}

NAPIAsyncEvent::~NAPIAsyncEvent()
{
    DeleteCallback();
    DeleteAsyncWork();
    eventSuccess_ = nullptr;
    eventError_ = nullptr;
}

void NAPIAsyncEvent::SetAsyncEventSuccess(OnAsyncEventSuccess eventSuccess)
{
    eventSuccess_ = eventSuccess;
}

void NAPIAsyncEvent::SetAsyncEventError(OnAsyncEventError eventError)
{
    eventError_ = eventError;
}

void NAPIAsyncEvent::SetErrorCode(int errorCode)
{
    errorCode_ = errorCode;
}

napi_env NAPIAsyncEvent::GetEnv(void)
{
    return env_;
}

int NAPIAsyncEvent::GetErrorCode(void)
{
    return errorCode_;
}

const std::string& NAPIAsyncEvent::GetAsyncWorkName(void)
{
    return asyncWorkName_;
}

void NAPIAsyncEvent::SetBridgeName(const std::string& bridgeName)
{
    bridgeName_ = bridgeName;
}

void NAPIAsyncEvent::SetData(napi_value data)
{
    data_ = data;
}

napi_value NAPIAsyncEvent::GetData(void)
{
    return data_;
}

void NAPIAsyncEvent::SetRefData(napi_value data)
{
    refData_ = PluginInnerNApiUtils::CreateReference(env_, data);
}

napi_value NAPIAsyncEvent::GetRefData(void)
{
    return PluginInnerNApiUtils::GetReference(env_, refData_);
}

void NAPIAsyncEvent::SetRefErrorData(napi_value data)
{
    refErrorData_ = PluginInnerNApiUtils::CreateReference(env_, data);
}

napi_value NAPIAsyncEvent::GetRefErrorData(void)
{
    return PluginInnerNApiUtils::GetReference(env_, refErrorData_);
}

void NAPIAsyncEvent::SetMethodParameter(const std::string& jsonStr)
{
    methodParameter_ = jsonStr;
}

bool NAPIAsyncEvent::CreateCallback(napi_value callback)
{
    if (callback_ != nullptr) {
        PluginInnerNApiUtils::DeleteReference(env_, callback_);
        callback_ = nullptr;
    }

    if (PluginInnerNApiUtils::GetValueType(env_, callback) != napi_function) {
        return false;
    }

    callback_ = PluginInnerNApiUtils::CreateReference(env_, callback);
    return callback_ != nullptr;
}

void NAPIAsyncEvent::DeleteCallback(void)
{
    if (callback_ != nullptr) {
        PluginInnerNApiUtils::DeleteReference(env_, callback_);
        callback_ = nullptr;
    }
    if (refData_ != nullptr) {
        PluginInnerNApiUtils::DeleteReference(env_, refData_);
        refData_ = nullptr;
    }
    if (refErrorData_ != nullptr) {
        PluginInnerNApiUtils::DeleteReference(env_, refErrorData_);
        refErrorData_ = nullptr;
    }
}

bool NAPIAsyncEvent::IsCallback(void)
{
    return callback_ != nullptr;
}

bool NAPIAsyncEvent::CreateAsyncWork(const std::string& asyncWorkName,
    AsyncWorkExecutor executor, AsyncWorkComplete callback)
{
    asyncWorkName_ = asyncWorkName;
    napi_value workName = PluginInnerNApiUtils::CreateStringUtf8(env_, asyncWorkName);
    asyncWork_ = PluginInnerNApiUtils::CreateAsyncWork(env_, workName, executor, callback, this);
    return asyncWork_ != nullptr;
}

void NAPIAsyncEvent::DeleteAsyncWork(void)
{
    if (asyncWork_ != nullptr) {
        PluginInnerNApiUtils::DeleteAsyncWork(env_, asyncWork_);
        asyncWork_ = nullptr;
    }
}

napi_value NAPIAsyncEvent::CreatePromise(void)
{
    napi_value result = nullptr;
    NAPI_CALL(env_, napi_create_promise(env_, &deferred_, &result));
    return result;
}

void NAPIAsyncEvent::TriggerEventSuccess(napi_value result)
{
    if (eventSuccess_ == nullptr) {
        return;
    }

    eventSuccess_(env_, bridgeName_, asyncWorkName_, result);
}

void NAPIAsyncEvent::TriggerEventError(ErrorCode code)
{
    if (eventError_ == nullptr) {
        return;
    }

    eventError_(env_, bridgeName_, asyncWorkName_, static_cast<int>(code));
}

void NAPIAsyncEvent::AsyncWorkCallback(void)
{
    size_t argc = PluginInnerNApiUtils::ARG_NUM_2;
    napi_value argv[PluginInnerNApiUtils::ARG_NUM_2] = { GetRefErrorData(), GetRefData() };
    if (callback_) {
        napi_value callback = PluginInnerNApiUtils::GetReference(env_, callback_);
        PluginInnerNApiUtils::CallFunction(env_, PluginInnerNApiUtils::CreateUndefined(env_), callback, argc, argv);
    } else {
        if (deferred_ == nullptr) {
            return;
        }

        if (errorCode_ == 0) {
            napi_resolve_deferred(env_, deferred_, argv[PluginInnerNApiUtils::ARG_NUM_1]);
        } else {
            napi_reject_deferred(env_, deferred_, argv[PluginInnerNApiUtils::ARG_NUM_0]);
        }
    }
}

void NAPIAsyncEvent::AsyncWorkCallMethod(void)
{
    SetErrorCode(0);
    if (callback_ == nullptr) {
        TriggerEventError(ErrorCode::BRIDGE_METHOD_UNIMPL);
        return;
    }

    napi_value callback = nullptr;
    napi_value methodResultValue = nullptr;
    napi_get_reference_value(env_, callback_, &callback);
    if (methodParameter_.empty()) {
        methodResultValue = PluginInnerNApiUtils::CallFunction(
            env_, PluginInnerNApiUtils::CreateUndefined(env_), callback, 0, nullptr);
        TriggerEventSuccess(methodResultValue);
    } else {
        size_t argc = PluginInnerNApiUtils::MAX_ARG_NUM;
        napi_value argv[PluginInnerNApiUtils::MAX_ARG_NUM] = { nullptr };

        bool ret = NAPIUtils::JsonStringToNapiValues(env_, methodParameter_, argc, argv);
        if (ret) {
            methodResultValue = PluginInnerNApiUtils::CallFunction(
                env_, PluginInnerNApiUtils::CreateUndefined(env_), callback, argc, argv);
            TriggerEventSuccess(methodResultValue);
        } else {
            TriggerEventError(ErrorCode::BRIDGE_METHOD_PARAM_ERROR);
        }
    }
}

void NAPIAsyncEvent::AsyncWorkMessage(void)
{
    if (callback_ == nullptr) {
        TriggerEventError(ErrorCode::BRIDGE_METHOD_UNIMPL);
        return;
    }

    napi_value callback = nullptr;
    napi_value result = nullptr;
    napi_get_reference_value(env_, callback_, &callback);
    napi_value data = GetRefData();
    result = PluginInnerNApiUtils::CallFunction(env_, PluginInnerNApiUtils::CreateUndefined(env_), callback, 1, &data);
    TriggerEventSuccess(result);
}
} // namespace OHOS::Plugin::Bridge
