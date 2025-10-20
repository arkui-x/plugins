/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "log.h"

#include "napi_utils.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_napi.h"
#include "buffer_mapping.h"
#include "bridge_binary_codec.h"
#include "method_data_converter.h"

namespace OHOS::Plugin::Bridge {
NAPIAsyncEvent::NAPIAsyncEvent(napi_env env) : env_(env) {}

NAPIAsyncEvent::~NAPIAsyncEvent()
{
    DeleteRefData();
    DeleteRefErrorData();
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

void NAPIAsyncEvent::DeleteRefData(void)
{
    if (refData_ != nullptr) {
        PluginUtilsNApi::DeleteReference(env_, refData_);
        refData_ = nullptr;
    }
}

void NAPIAsyncEvent::SetRefData(napi_value data)
{
    DeleteRefData();
    refData_ = PluginUtilsNApi::CreateReference(env_, data);
}

napi_value NAPIAsyncEvent::GetRefData(void)
{
    return PluginUtilsNApi::GetReference(env_, refData_);
}

void NAPIAsyncEvent::DeleteRefErrorData(void)
{
    if (refErrorData_ != nullptr) {
        PluginUtilsNApi::DeleteReference(env_, refErrorData_);
        refErrorData_ = nullptr;
    }
}

void NAPIAsyncEvent::SetRefErrorData(napi_value data)
{
    DeleteRefErrorData();
    refErrorData_ = PluginUtilsNApi::CreateReference(env_, data);
}

napi_value NAPIAsyncEvent::GetRefErrorData(void)
{
    return PluginUtilsNApi::GetReference(env_, refErrorData_);
}

void NAPIAsyncEvent::SetMethodParameter(const std::string& jsonStr)
{
    std::lock_guard<std::mutex> lock(queueLock_);
    methodParameters_.emplace(jsonStr);
}

void NAPIAsyncEvent::SetMethodParameter(uint8_t* data, size_t size)
{
    std::tuple<uint8_t*, size_t> methodBuffer;
    std::get<uint8_t*>(methodBuffer) = data;
    std::get<size_t>(methodBuffer) = size; 
    taskQueue_.push_back(methodBuffer);
}

std::tuple<uint8_t*, size_t> NAPIAsyncEvent::GetMethodParameter(void)
{
    std::tuple<uint8_t*, size_t> nextTask = taskQueue_.front();
    taskQueue_.pop_front();

    return nextTask;
}

bool NAPIAsyncEvent::CreateCallback(napi_value callback)
{
    if (callback_ != nullptr) {
        PluginUtilsNApi::DeleteReference(env_, callback_);
        callback_ = nullptr;
    }

    if (PluginUtilsNApi::GetValueType(env_, callback) != napi_function) {
        return false;
    }

    callback_ = PluginUtilsNApi::CreateReference(env_, callback);
    return callback_ != nullptr;
}

void NAPIAsyncEvent::DeleteCallback(void)
{
    if (callback_ != nullptr) {
        PluginUtilsNApi::DeleteReference(env_, callback_);
        callback_ = nullptr;
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
    napi_value workName = PluginUtilsNApi::CreateStringUtf8(env_, asyncWorkName);
    asyncWork_ = PluginUtilsNApi::CreateAsyncWork(env_, workName, executor, callback, this);
    return asyncWork_ != nullptr;
}

void NAPIAsyncEvent::DeleteAsyncWork(void)
{
    if (asyncWork_ != nullptr) {
        PluginUtilsNApi::DeleteAsyncWork(env_, asyncWork_);
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
    size_t argc = PluginUtilsNApi::ARG_NUM_2;
    napi_value argv[PluginUtilsNApi::ARG_NUM_2] = { GetRefErrorData(), GetRefData() };
    if (callback_) {
        napi_value callback = PluginUtilsNApi::GetReference(env_, callback_);
        PluginUtilsNApi::CallFunction(env_, PluginUtilsNApi::CreateUndefined(env_), callback, argc, argv);
    } else {
        if (deferred_ == nullptr) {
            return;
        }

        if (errorCode_ == 0) {
            napi_resolve_deferred(env_, deferred_, argv[PluginUtilsNApi::ARG_NUM_1]);
        } else {
            napi_reject_deferred(env_, deferred_, argv[PluginUtilsNApi::ARG_NUM_0]);
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
    std::string methodParameter = "";
    {
        std::lock_guard<std::mutex> lock(queueLock_);
        methodParameter = methodParameters_.front();
        methodParameters_.pop();
    }
    if (methodParameter.empty()) {
        methodResultValue = PluginUtilsNApi::CallFunction(
            env_, PluginUtilsNApi::CreateUndefined(env_), callback, 0, nullptr);
        TriggerEventSuccess(methodResultValue);
    } else {
        size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
        napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
        bool ret = NAPIUtils::JsonStringToNapiValues(env_, methodParameter, argc, argv);
        if (ret) {
            methodResultValue = PluginUtilsNApi::CallFunction(
                env_, PluginUtilsNApi::CreateUndefined(env_), callback, argc, argv);
            TriggerEventSuccess(methodResultValue);
        } else {
            TriggerEventError(ErrorCode::BRIDGE_METHOD_PARAM_ERROR);
        }
    }
}

void NAPIAsyncEvent::AsyncWorkCallMethod(size_t argc, const napi_value* argv)
{
    SetErrorCode(0);
    if (callback_ == nullptr) {
        TriggerEventError(ErrorCode::BRIDGE_METHOD_UNIMPL);
        return;
    }

    if (argc > 0 && argv == nullptr) {
        TriggerEventError(ErrorCode::BRIDGE_METHOD_PARAM_ERROR);
        return;
    }

    napi_value callback = nullptr;
    napi_value methodResultValue = nullptr;
    napi_get_reference_value(env_, callback_, &callback);
    if (argc == 0) {
        methodResultValue = PluginUtilsNApi::CallFunction(
            env_, PluginUtilsNApi::CreateUndefined(env_), callback, 0, nullptr);
        TriggerEventSuccess(methodResultValue);
    } else {
        methodResultValue = PluginUtilsNApi::CallFunction(
            env_, PluginUtilsNApi::CreateUndefined(env_), callback, argc, argv);
        TriggerEventSuccess(methodResultValue);
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
    result = PluginUtilsNApi::CallFunction(env_, PluginUtilsNApi::CreateUndefined(env_), callback, 1, &data);
    TriggerEventSuccess(result);
    PluginUtilsNApi::DetachArrayBufferFromTypedArray(env_, data);
    DeleteRefData();
    DeleteRefErrorData();
}

napi_value NAPIAsyncEvent::AsyncWorkCallMethodSync(const std::string& parameter)
{
    SetErrorCode(0);
    if (callback_ == nullptr) {
        LOGE("AsyncWorkCallMethodSync: No callback registered");
        return nullptr;
    }

    napi_value callback = nullptr;
    napi_get_reference_value(env_, callback_, &callback);

    if (callback == nullptr) {
        LOGE("AsyncWorkCallMethodSync: Failed to get callback reference");
        return nullptr;
    }

    napi_value methodResultValue = nullptr;
    if (parameter.empty()) {
        methodResultValue = PluginUtilsNApi::CallFunction(
            env_, PluginUtilsNApi::CreateUndefined(env_), callback, 0, nullptr);
    } else {
        size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
        napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
        bool ret = NAPIUtils::JsonStringToNapiValues(env_, parameter, argc, argv);
        if (!ret) {
            LOGE("AsyncWorkCallMethodSync: Failed to parse parameters");
            return nullptr;
        }
        methodResultValue = PluginUtilsNApi::CallFunction(
            env_, PluginUtilsNApi::CreateUndefined(env_), callback, argc, argv);
    }

    LOGD("AsyncWorkCallMethodSync: Successfully called method");
    return methodResultValue;
}

napi_value NAPIAsyncEvent::AsyncWorkCallMethodSyncBinary(uint8_t* data, size_t size)
{
    LOGD("NAPIAsyncEvent::AsyncWorkCallMethodSyncBinary called, size=%{public}zu", size);
    SetErrorCode(0);
    if (callback_ == nullptr) {
        LOGE("AsyncWorkCallMethodSyncBinary: No callback registered");
        return nullptr;
    }

    napi_value callback = nullptr;
    napi_get_reference_value(env_, callback_, &callback);
    if (callback == nullptr) {
        LOGE("AsyncWorkCallMethodSyncBinary: Failed to get callback reference");
        return nullptr;
    }

    napi_value methodResultValue = nullptr;
    if (data == nullptr || size == 0) {
        methodResultValue = PluginUtilsNApi::CallFunction(
            env_, PluginUtilsNApi::CreateUndefined(env_), callback, 0, nullptr);
        return methodResultValue;
    }

    auto decoded = BridgeBinaryCodec::GetInstance().DecodeBuffer(data, size);
    if (!decoded) {
        LOGE("AsyncWorkCallMethodSyncBinary: Decode buffer failed");
        return nullptr;
    }
    size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
    napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
    MethodDataConverter::ConvertToNapiValues(env_, *decoded, argc, argv);
    methodResultValue = PluginUtilsNApi::CallFunction(
        env_, PluginUtilsNApi::CreateUndefined(env_), callback, argc, argv);
    return methodResultValue;
}
} // namespace OHOS::Plugin::Bridge
