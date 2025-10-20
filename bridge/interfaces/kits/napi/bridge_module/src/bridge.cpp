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

#include "bridge.h"

#include <cstddef>
#include <cstdint>

#include "bridge_manager.h"
#include "bridge_receiver.h"
#include "log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_utils.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_inner.h"

using namespace OHOS::Ace::Platform;
namespace OHOS::Plugin::Bridge {
Bridge::Bridge(const std::string& bridgeName, const CodecType& type)
    : bridgeName_(bridgeName), codecType_(type)
{
    available_ = (RegisterBridge(bridgeName) == ErrorCode::BRIDGE_ERROR_NO);
}

Bridge::~Bridge()
{
    if (available_) {
        available_ = false;
        UnRegisterBridge(bridgeName_);
    }
}

bool Bridge::BridgeNameExists(const std::string& bridgeName)
{
    return BridgeManager::JSBridgeExists(bridgeName);
}

const std::string& Bridge::GetBridgeName(void)
{
    return bridgeName_;
}

ErrorCode Bridge::RegisterBridge(const std::string& bridgeName)
{
    if (bridgeName.empty()) {
        LOGE("RegisterBridge: The bridgeName is unavailable.");
        return ErrorCode::BRIDGE_NAME_ERROR;
    }

    std::shared_ptr<BridgeReceiver> receiver = std::make_shared<BridgeReceiver>();
    if (!receiver) {
        LOGE("RegisterBridge: Failed to create BridgeReceiver.");
        return ErrorCode::BRIDGE_CREATE_ERROR;
    }
    receiver->bridgeName_ = bridgeName;
    receiver->bridgeType_ = static_cast<int32_t>(codecType_);

    receiver->callMethodCallback_ = std::bind(
        &Bridge::OnPlatformCallMethod, this, std::placeholders::_1, std::placeholders::_2);
    receiver->methodResultCallback_ = std::bind(
        &Bridge::OnPlatformMethodResult, this, std::placeholders::_1, std::placeholders::_2);
    receiver->sendMessageCallback_ =
        std::bind(&Bridge::OnPlatformSendMessage, this, std::placeholders::_1);
    receiver->callMethodSyncCallback_ = std::bind(
        &Bridge::OnPlatformCallMethodSync, this, std::placeholders::_1, std::placeholders::_2);

    receiver->callMethodBinaryCallback_ = std::bind(
        &Bridge::OnPlatformCallMethodBinary, this, std::placeholders::_1, std::placeholders::_2);
    receiver->callMethodSyncBinaryCallback_ = std::bind(
        &Bridge::OnPlatformCallMethodSyncBinary, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
    receiver->sendMessageBinaryCallback_ =
        std::bind(&Bridge::OnPlatformSendMessageBinary, this, std::placeholders::_1);
    receiver->methodResultBinaryCallback_ = std::bind(
        &Bridge::OnPlatformMethodResultBinary, this,
        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

    receiver->sendMessageResponseCallback_ =
        std::bind(&Bridge::OnPlatformSendMessageResponse, this, std::placeholders::_1);

    receiver->sendWillTerminateResponseCallback_ =
        std::bind(&Bridge::OnPlatformSendWillTerminate, this, std::placeholders::_1);

    if (BridgeManager::JSRegisterBridge(receiver)) {
        return ErrorCode::BRIDGE_ERROR_NO;
    }
    LOGE("RegisterBridge: The bridge registration failure.");
    return ErrorCode::BRIDGE_CREATE_ERROR;
}

void Bridge::UnRegisterBridge(const std::string& bridgeName)
{
    if (!bridgeName.empty()) {
        available_ = false;
        BridgeManager::JSUnRegisterBridge(bridgeName);
    }
}

void Bridge::UnRegisterBridge(void)
{
    UnRegisterBridge(bridgeName_);
}

ErrorCode Bridge::CallMethod(const std::string& methodName, const std::shared_ptr<MethodData>& methodData)
{
    if (!GetAvailable()) {
        LOGE("CallMethod: The bridge is unavailable.");
        return ErrorCode::BRIDGE_INVALID;
    }
    if (methodName.empty()) {
        LOGE("SendMethodResult: The methodName is empty.");
        return ErrorCode::BRIDGE_METHOD_NAME_ERROR;
    }

    if (methodData == nullptr) {
        LOGE("SendMethodResult: The methodData is null.");
        return ErrorCode::BRIDGE_METHOD_PARAM_ERROR;
    }

    std::lock_guard<std::mutex> lock(jsMethodDataListLock_);

    methodData->SetStartTime(MethodData::GetSystemTime());
    jsMethodDataList_[methodName] = methodData;

    if (codecType_ == CodecType::JSON_CODEC) {
        auto task = [bridgeName = this->bridgeName_, methodName, parameter = methodData->GetMethodParamName()]() {
            BridgeManager::JSCallMethod(bridgeName, methodName, parameter);
        };
        if (!taskExecutor_) {
            LOGE("CallMethod(JSON): taskExecutor_ is null.");
        } else {
            taskExecutor_->RunTaskOnBridgeThread(task);
        }
    } else if (codecType_ == CodecType::BINARY_CODEC) {
        const auto& data = methodData->GetMethodParamNameBinary();
        auto task = [bridgeName = this->bridgeName_, methodName, &data]() {
            BridgeManager::JSCallMethodBinary(bridgeName, methodName, data);
        };
        if (!taskExecutor_) {
            LOGE("CallMethod(Binary): taskExecutor_ is null.");
        } else {
            taskExecutor_->RunTaskOnBridgeThread(task);
        }
    }

    return ErrorCode::BRIDGE_ERROR_NO;
}

ErrorCode Bridge::CallMethodSync(napi_env env, const std::string& methodName,
    const std::shared_ptr<MethodData>& methodData,
    std::shared_ptr<MethodResult>& methodResult)
{
    if (!GetAvailable()) {
        LOGE("CallMethod: The bridge is unavailable.");
        return ErrorCode::BRIDGE_INVALID;
    }
    if (methodName.empty()) {
        LOGE("SendMethodResult: The methodName is empty.");
        return ErrorCode::BRIDGE_METHOD_NAME_ERROR;
    }

    if (methodData == nullptr) {
        LOGE("SendMethodResult: The methodData is null.");
        return ErrorCode::BRIDGE_METHOD_PARAM_ERROR;
    }

    std::lock_guard<std::mutex> lock(jsMethodDataListLock_);

    if (codecType_ == CodecType::JSON_CODEC) {
        auto result = BridgeManager::JSCallMethodSync(bridgeName_, methodName, methodData->GetMethodParamName());
        methodResult->ParsePlatformMethodResult(env, result);
    } else if (codecType_ == CodecType::BINARY_CODEC) {
        const auto& data = methodData->GetMethodParamNameBinary();
        auto result = BridgeManager::JSCallMethodBinarySync(bridgeName_, methodName, data);
        methodResult->ParsePlatformMethodResultBinary(env, result.errorCode, "", std::move(result.buffer));
    }
    return ErrorCode::BRIDGE_ERROR_NO;
}

ErrorCode SetCallMethodSyncBinaryError(MethodResult &methodResult, ErrorCode code, const char *logMsg)
{
    LOGE("%{public}s", logMsg);
    methodResult.SetErrorCode(static_cast<int>(code));
    methodResult.CreateDefaultJsonString();
    return code;
}

ErrorCode Bridge::CallMethodSyncBinary(
    const std::string& methodName, const std::shared_ptr<MethodData>& methodData, MethodResult& methodResult)
{
    if (!GetAvailable()) {
        return SetCallMethodSyncBinaryError(methodResult, ErrorCode::BRIDGE_INVALID,
            "CallMethodSyncBinary: bridge unavailable");
    }
    if (codecType_ != CodecType::BINARY_CODEC) {
        return SetCallMethodSyncBinaryError(methodResult, ErrorCode::BRIDGE_CODEC_TYPE_MISMATCH,
            "CallMethodSyncBinary: not binary codec");
    }
    if (methodName.empty()) {
        return SetCallMethodSyncBinaryError(methodResult, ErrorCode::BRIDGE_METHOD_NAME_ERROR,
            "CallMethodSyncBinary: method empty");
    }
    if (!methodData) {
        return SetCallMethodSyncBinaryError(methodResult, ErrorCode::BRIDGE_METHOD_PARAM_ERROR,
            "CallMethodSyncBinary: methodData null");
    }
    std::lock_guard<std::mutex> lock(platformMethodDataListLock_);
    auto platformMethodData = FindPlatformMethodData(methodName);
    if (!platformMethodData) {
        return SetCallMethodSyncBinaryError(methodResult, ErrorCode::BRIDGE_METHOD_UNIMPL,
            "CallMethodSyncBinary: platform method not found");
    }

    std::unique_ptr<Ace::Platform::BufferMapping> paramMapping = nullptr;
    const auto& paramVec = methodData->GetMethodParamNameBinary();
    if (!paramVec.empty()) {
        auto copy = Ace::Platform::BufferMapping::Copy(paramVec.data(), paramVec.size());
        paramMapping = std::make_unique<Ace::Platform::BufferMapping>(copy.Release(), paramVec.size());
    }
    methodResult = platformMethodData->PlatformCallMethodSyncBinary(std::move(paramMapping));
    return ErrorCode::BRIDGE_ERROR_NO;
}

ErrorCode Bridge::SendMethodResult(const std::string& methodName, const std::string& result)
{
    if (!GetAvailable()) {
        LOGE("SendMethodResult: The bridge is unavailable.");
        return ErrorCode::BRIDGE_INVALID;
    }

    auto task = [bridgeName = this->bridgeName_, methodName, result]() {
        BridgeManager::JSSendMethodResult(bridgeName, methodName, result);
    };
    if (!taskExecutor_) {
        LOGE("SendMethodResult: taskExecutor_ is null.");
    } else {
        taskExecutor_->RunTaskOnBridgeThread(task);
    }
    return ErrorCode::BRIDGE_ERROR_NO;
}

ErrorCode Bridge::SendMessage(const std::string& data, std::shared_ptr<MethodData>& methodData)
{
    if (!GetAvailable()) {
        LOGE("SendMessage: The bridge is unavailable.");
        return ErrorCode::BRIDGE_INVALID;
    }

    std::lock_guard<std::mutex> lock(jsSendMessageDataListLock_);
    jsSendMessageDataList_.push_back(methodData);
    auto task = [bridgeName = this->bridgeName_, data]() {
        BridgeManager::JSSendMessage(bridgeName, data);
    };
    if (!taskExecutor_) {
        LOGE("SendMessage: taskExecutor_ is null.");
    } else {
        taskExecutor_->RunTaskOnBridgeThread(task);
    }

    return ErrorCode::BRIDGE_ERROR_NO;
}

ErrorCode Bridge::SendMessageBinary(const std::vector<uint8_t>& data, std::shared_ptr<MethodData>& methodData)
{
    if (!GetAvailable()) {
        LOGE("SendMessage: The bridge is unavailable.");
        return ErrorCode::BRIDGE_INVALID;
    }

    std::lock_guard<std::mutex> lock(jsSendMessageDataListLock_);
    jsSendMessageDataList_.push_back(methodData);
    auto task = [bridgeName = this->bridgeName_, &data]() {
        BridgeManager::JSSendMessageBinary(bridgeName, data);
    };
    if (!taskExecutor_) {
        LOGE("SendMessageBinary: taskExecutor_ is null.");
    } else {
        taskExecutor_->RunTaskOnBridgeThread(task);
    }

    return ErrorCode::BRIDGE_ERROR_NO;
}

ErrorCode Bridge::SendMessageResponse(const std::string& data)
{
    if (!GetAvailable()) {
        LOGE("SendMessageResponse: The bridge is unavailable.");
        return ErrorCode::BRIDGE_INVALID;
    }

    if (data.empty()) {
        LOGE("SendMessageResponse: The data is empty.");
        return ErrorCode::BRIDGE_DATA_ERROR;
    }

    auto task = [bridgeName = this->bridgeName_, data]() {
        BridgeManager::JSSendMessageResponse(bridgeName, data);
    };
    if (!taskExecutor_) {
        LOGE("SendMessageResponse: taskExecutor_ is null.");
    } else {
        taskExecutor_->RunTaskOnBridgeThread(task);
    }
    return ErrorCode::BRIDGE_ERROR_NO;
}

ErrorCode Bridge::RegisterMethod(const std::string& methodName, const std::shared_ptr<MethodData>& methodData)
{
    if (!GetAvailable()) {
        LOGE("RegisterMethod: The bridge is unavailable.");
        return ErrorCode::BRIDGE_INVALID;
    }

    if (methodName.empty()) {
        LOGE("RegisterMethod: The method is empty.");
        return ErrorCode::BRIDGE_METHOD_NAME_ERROR;
    }

    if (methodData == nullptr) {
        LOGE("RegisterMethod: The methodData is null.");
        return ErrorCode::BRIDGE_METHOD_PARAM_ERROR;
    }

    std::lock_guard<std::mutex> lock(platformMethodDataListLock_);
    auto platformMethodData = FindPlatformMethodData(methodName);
    if (platformMethodData) {
        LOGE("RegisterMethod: The %{public}s is exists.", methodName.c_str());
        return ErrorCode::BRIDGE_METHOD_EXISTS;
    }

    platformMethodDataList_[methodName] = methodData;
    return ErrorCode::BRIDGE_ERROR_NO;
}

ErrorCode Bridge::UnRegisterMethod(const std::string& methodName)
{
    if (!GetAvailable()) {
        LOGE("UnRegisterMethod: The bridge is unavailable.");
        return ErrorCode::BRIDGE_INVALID;
    }

    if (methodName.empty()) {
        LOGE("UnRegisterMethod: The methodName is null.");
        return ErrorCode::BRIDGE_METHOD_NAME_ERROR;
    }

    std::lock_guard<std::mutex> lock(platformMethodDataListLock_);
    auto iter = platformMethodDataList_.find(methodName);
    if (iter != platformMethodDataList_.end()) {
        platformMethodDataList_.erase(iter);
        auto task = [bridgeName = this->bridgeName_, methodName]() {
            BridgeManager::JSCancelMethod(bridgeName, methodName);
        };
        if (!taskExecutor_) {
            LOGE("UnRegisterMethod: taskExecutor_ is null.");
        } else {
            taskExecutor_->RunTaskOnBridgeThread(task);
        }
        return ErrorCode::BRIDGE_ERROR_NO;
    }
    return ErrorCode::BRIDGE_METHOD_UNIMPL;
}

void Bridge::SetMessageListener(std::shared_ptr<MethodData>& callback)
{
    if (messageCallback_) {
        messageCallback_->ReleaseEvent();
        messageCallback_.reset();
    }
    messageCallback_ = callback;
}

void Bridge::SetAvailable(bool available)
{
    available_ = available;
}

bool Bridge::GetAvailable(void)
{
    return available_;
}

std::shared_ptr<MethodData> Bridge::FindPlatformMethodData(const std::string& methodName)
{
    auto iter = platformMethodDataList_.find(methodName);
    if (iter != platformMethodDataList_.end()) {
        return iter->second;
    }
    return nullptr;
}

std::shared_ptr<MethodData> Bridge::FindJSMethodData(const std::string& methodName)
{
    auto iter = jsMethodDataList_.find(methodName);
    if (iter != jsMethodDataList_.end()) {
        return iter->second;
    }
    return nullptr;
}

void Bridge::EraseJSMethodData(const std::string& methodName)
{
    auto iter = jsMethodDataList_.find(methodName);
    if (iter != jsMethodDataList_.end()) {
        jsMethodDataList_.erase(iter);
    }
}

void Bridge::EraseJSMessageData(void)
{
    if (jsSendMessageDataList_.size() > 0) {
        jsSendMessageDataList_.erase(jsSendMessageDataList_.begin());
    }
}

void Bridge::RemoveJSMethodData(const std::string& methodName)
{
    std::lock_guard<std::mutex> lock(jsMethodDataListLock_);
    EraseJSMethodData(methodName);
}

void Bridge::RemoveMessageData(void)
{
    std::lock_guard<std::mutex> lock(jsSendMessageDataListLock_);
    EraseJSMessageData();
}

void Bridge::SetTerminate(bool terminate)
{
    terminate_ = terminate;
}

bool Bridge::GetTerminate(void)
{
    return terminate_;
}

void Bridge::OnPlatformCallMethod(const std::string& methodName, const std::string& parameter)
{
    std::lock_guard<std::mutex> lock(platformMethodDataListLock_);
    std::shared_ptr<MethodData> jsMethodData = FindPlatformMethodData(methodName);
    if (jsMethodData == nullptr) {
        LOGE("OnPlatformCallMethod: The jsMethodData is null.");

        auto task = [methodName, bridgeName = this->bridgeName_] {
            MethodResult result;
            result.SetErrorCodeInfo(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
            result.SetMethodName(methodName);
            result.CreateMethodResultForError();
            BridgeManager::JSSendMethodResult(bridgeName, methodName, result.GetResult());
        };
        if (!taskExecutor_) {
            LOGE("OnPlatformCallMethod(error path): taskExecutor_ is null.");
        } else {
            taskExecutor_->RunTaskOnBridgeThread(task);
        }
        return;
    }

    auto task = [jsMethodData, parameter]() {
        jsMethodData->PlatformCallMethod(parameter);
    };
    if (!taskExecutor_) {
        LOGE("OnPlatformCallMethod: taskExecutor_ is null.");
    } else {
        taskExecutor_->RunTaskOnMainThread(task);
    }
}

std::string Bridge::OnPlatformCallMethodSync(const std::string& methodName, const std::string& parameter)
{
    std::lock_guard<std::mutex> lock(platformMethodDataListLock_);
    std::shared_ptr<MethodData> jsMethodData = FindPlatformMethodData(methodName);
    if (jsMethodData == nullptr) {
        LOGE("OnPlatformCallMethodSync: The jsMethodData is null.");
        MethodResult result;
        result.SetErrorCodeInfo(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
        result.SetMethodName(methodName);
        result.CreateMethodResultForError();
        return result.GetResult();
    }
    MethodResult methodResult = jsMethodData->PlatformCallMethodSync(parameter);
    return methodResult.GetResult();
}

void Bridge::OnPlatformCallMethodBinary(const std::string& methodName, std::unique_ptr<BufferMapping> data)
{
    std::lock_guard<std::mutex> lock(platformMethodDataListLock_);
    std::shared_ptr<MethodData> jsMethodData = FindPlatformMethodData(methodName);
    if (jsMethodData == nullptr) {
        LOGE("OnPlatformCallMethodBinary: The jsMethodData is null.");
        auto task = [methodName, bridgeName = this->bridgeName_] {
            MethodResult result;
            result.SetErrorCodeInfo(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
            result.SetMethodName(methodName);
            result.CreateMethodResultForError();
            BridgeManager::JSSendMethodResultBinary(bridgeName,
                methodName, result.GetErrorCode(), result.GetErrorMessage(), nullptr);
        };
        if (!taskExecutor_) {
            LOGE("OnPlatformCallMethodBinary(error path): taskExecutor_ is null.");
        } else {
            taskExecutor_->RunTaskOnBridgeThread(task);
        }
        return;
    }
    struct BufferHolder { std::unique_ptr<BufferMapping> ptr; };
    auto holder = std::make_shared<BufferHolder>();
    if (!holder) {
        LOGE("OnPlatformCallMethodBinary: holder is null.");
        return;
    }
    holder->ptr = std::move(data);
    if (!taskExecutor_) {
        LOGE("OnPlatformCallMethodBinary: taskExecutor_ is null.");
        return;
    }
    taskExecutor_->RunTaskOnMainThread([jsMethodData, holder]() mutable {
        jsMethodData->PlatformCallMethodBinary(std::move(holder->ptr));
    });
}

std::unique_ptr<BufferMapping> Bridge::OnPlatformCallMethodSyncBinary(
    const std::string& methodName, std::unique_ptr<BufferMapping> data, int32_t& errorCode)
{
    errorCode = 0;
    std::lock_guard<std::mutex> lock(platformMethodDataListLock_);
    std::shared_ptr<MethodData> jsMethodData = FindPlatformMethodData(methodName);
    if (jsMethodData == nullptr) {
        LOGE("OnPlatformCallMethodSyncBinary: method not implemented! The method is %{public}s", methodName.c_str());
        errorCode = static_cast<int32_t>(ErrorCode::BRIDGE_METHOD_UNIMPL);
        return nullptr;
    }

    MethodResult methodResult = jsMethodData->PlatformCallMethodSyncBinary(std::move(data));
    if (methodResult.GetErrorCode() != 0) {
        LOGE("OnPlatformCallMethodSyncBinary: method execute failed, err=%{public}d", methodResult.GetErrorCode());
        errorCode = methodResult.GetErrorCode();
        return nullptr;
    }
    auto binaryVec = methodResult.GetResultBinary();
    if (binaryVec == nullptr || binaryVec->empty()) {
        LOGI("OnPlatformCallMethodSyncBinary: empty result");
        return nullptr;
    }
    auto copy = BufferMapping::Copy(binaryVec->data(), binaryVec->size());
    return std::make_unique<BufferMapping>(copy.Release(), binaryVec->size());
}

void Bridge::OnPlatformMethodResult(const std::string& methodName, const std::string& result)
{
    std::lock_guard<std::mutex> lock(jsMethodDataListLock_);
    std::shared_ptr<MethodData> jsMethodData = FindJSMethodData(methodName);
    if (jsMethodData == nullptr) {
        LOGE("OnPlatformCallMethod: The jsMethodData is null.");
        auto task = [methodName, bridgeName = this->bridgeName_] {
            MethodResult result;
            result.SetErrorCodeInfo(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
            result.SetMethodName(methodName);
            result.CreateMethodResultForError();
            BridgeManager::JSSendMethodResult(bridgeName, methodName, result.GetResult());
        };
        if (!taskExecutor_) {
            LOGE("OnPlatformMethodResult(error path): taskExecutor_ is null.");
        } else {
            taskExecutor_->RunTaskOnBridgeThread(task);
        }
        return;
    }
    if (!taskExecutor_) {
        LOGE("OnPlatformMethodResult: taskExecutor_ is null.");
        return;
    }
    taskExecutor_->RunTaskOnMainThread([jsMethodData, result, methodName, this]() {
        jsMethodData->SendMethodResult(result, true);
        EraseJSMethodData(methodName);
    });
}

void Bridge::OnPlatformMethodResultBinary(const std::string& methodName, int errorCode,
    const std::string& errorMessage, std::unique_ptr<BufferMapping> result)
{
    std::lock_guard<std::mutex> lock(jsMethodDataListLock_);
    std::shared_ptr<MethodData> jsMethodData = FindJSMethodData(methodName);
    if (jsMethodData == nullptr) {
        LOGE("OnPlatformMethodResultBinary: The jsMethodData is null.");
        auto task = [methodName, bridgeName = this->bridgeName_] {
            MethodResult result;
            result.SetErrorCodeInfo(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
            result.SetMethodName(methodName);
            result.CreateMethodResultForError();
            BridgeManager::JSSendMethodResultBinary(
                bridgeName, methodName, result.GetErrorCode(), result.GetErrorMessage(), nullptr);
        };
        if (!taskExecutor_) {
            LOGE("OnPlatformMethodResultBinary(error path): taskExecutor_ is null.");
        } else {
            taskExecutor_->RunTaskOnBridgeThread(task);
        }
        return;
    }
    struct BufferHolder { std::unique_ptr<BufferMapping> ptr; };
    auto holder = std::make_shared<BufferHolder>();
    if (!holder) {
        LOGE("OnPlatformMethodResultBinary: holder is null.");
        return;
    }
    holder->ptr = std::move(result);
    if (!taskExecutor_) {
        LOGE("OnPlatformMethodResultBinary: taskExecutor_ is null.");
        return;
    }
    taskExecutor_->RunTaskOnMainThread([
        jsMethodData, holder, errorCode, errorMessage, methodName, this]() mutable {
        jsMethodData->SendMethodResultBinary(errorCode, errorMessage, std::move(holder->ptr), true);
        EraseJSMethodData(methodName);
    });
}

void Bridge::OnPlatformSendMessage(const std::string& data)
{
    if (messageCallback_) {
        auto cb = messageCallback_;
        std::string copy = data;
        if (!taskExecutor_) {
            LOGE("OnPlatformSendMessage: taskExecutor_ is null.");
            return;
        }
        taskExecutor_->RunTaskOnMainThread([cb, copy = std::move(copy)]() {
            cb->PlatformSendMessage(copy);
        });
    }
}

void Bridge::OnPlatformSendMessageBinary(std::unique_ptr<BufferMapping> data)
{
    if (!messageCallback_) {
        return;
    }
    auto cb = messageCallback_;
    struct BufferHolder { std::unique_ptr<BufferMapping> ptr; };
    auto holder = std::make_shared<BufferHolder>();
    if (!holder) {
        LOGE("OnPlatformSendMessageBinary: holder is null.");
        return;
    }
    holder->ptr = std::move(data);
    if (!taskExecutor_) {
        LOGE("OnPlatformSendMessageBinary: taskExecutor_ is null.");
        return;
    }
    taskExecutor_->RunTaskOnMainThread([cb, holder]() mutable {
        cb->PlatformSendMessageBinary(std::move(holder->ptr));
    });
}

void Bridge::OnPlatformSendMessageResponse(const std::string& data)
{
    std::shared_ptr<MethodData> methodData = nullptr;
    {
        std::lock_guard<std::mutex> lock(jsSendMessageDataListLock_);
        if (jsSendMessageDataList_.size() == 0) {
            LOGE("OnPlatformSendMessageResponse: No callback event was found on the JS side.");
            return;
        }

        methodData = jsSendMessageDataList_[0];
        EraseJSMessageData();
    }

    if (methodData) {
        auto copy = data;
        if (!taskExecutor_) {
            LOGE("OnPlatformSendMessageResponse: taskExecutor_ is null.");
            return;
        }
        taskExecutor_->RunTaskOnMainThread([methodData, copy = std::move(copy)]() {
            methodData->SendMessageResponse(copy, true);
        });
    }
}

void Bridge::OnPlatformSendWillTerminate(bool data)
{
    SetTerminate(data);
}
} // namespace OHOS::Plugin::Bridge