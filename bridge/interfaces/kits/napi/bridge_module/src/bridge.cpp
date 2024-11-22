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

#include "bridge.h"

#include <cstddef>

#include "bridge_manager.h"
#include "bridge_receiver.h"
#include "log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_utils.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_inner.h"

using namespace OHOS::Ace::Platform;
namespace OHOS::Plugin::Bridge {
Bridge::Bridge(const std::string& bridgeName, int32_t instanceId, const CodecType& type)
    : bridgeName_(bridgeName), instanceId_(instanceId), codecType_(type)
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

bool Bridge::BridgeNameExists(const std::string& bridgeName, const int32_t instanceId)
{
    return BridgeManager::JSBridgeExists(instanceId, bridgeName);
}

const std::string& Bridge::GetBridgeName(void)
{
    return bridgeName_;
}

int32_t Bridge::GetInstanceID(void)
{
    return instanceId_;
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
    receiver->instanceId_ = instanceId_;
    receiver->bridgeName_ = bridgeName;

    if (codecType_ == CodecType::JSON_CODEC) {
        receiver->callMethodCallback_ = std::bind(
            &Bridge::OnPlatformCallMethod, this, std::placeholders::_1, std::placeholders::_2);
        receiver->methodResultCallback_ = std::bind(
            &Bridge::OnPlatformMethodResult, this, std::placeholders::_1, std::placeholders::_2);
        receiver->sendMessageCallback_ =
            std::bind(&Bridge::OnPlatformSendMessage, this, std::placeholders::_1);
    } else if (codecType_ == CodecType::BINARY_CODEC) {
        receiver->callMethodBinaryCallback_ = std::bind(
            &Bridge::OnPlatformCallMethodBinary, this, std::placeholders::_1, std::placeholders::_2);
        receiver->sendMessageBinaryCallback_ =
            std::bind(&Bridge::OnPlatformSendMessageBinary, this, std::placeholders::_1);
        receiver->methodResultBinaryCallback_ = std::bind(
            &Bridge::OnPlatformMethodResultBinary, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
    }
    // only send message response callback by json string results.
    receiver->sendMessageResponseCallback_ =
        std::bind(&Bridge::OnPlatformSendMessageResponse, this, std::placeholders::_1);

    receiver->sendWillTerminateResponseCallback_ =
        std::bind(&Bridge::OnPlatformSendWillTerminate, this, std::placeholders::_1);

    if (BridgeManager::JSRegisterBridge(instanceId_, receiver)) {
        return ErrorCode::BRIDGE_ERROR_NO;
    }
    LOGE("RegisterBridge: The bridge registration failure.");
    return ErrorCode::BRIDGE_CREATE_ERROR;
}

void Bridge::UnRegisterBridge(const std::string& bridgeName)
{
    if (!bridgeName.empty()) {
        available_ = false;
        BridgeManager::JSUnRegisterBridge(instanceId_, bridgeName);
        instanceId_ = -1;
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
        auto task = [instanceId = this->instanceId_, bridgeName = this->bridgeName_, methodName,
                        parameter = methodData->GetMethodParamName()]() {
            BridgeManager::JSCallMethod(instanceId, bridgeName, methodName, parameter);
        };
        PluginUtilsInner::RunTaskOnPlatform(task);
    } else if (codecType_ == CodecType::BINARY_CODEC) {
        const auto& data = methodData->GetMethodParamNameBinary();
        auto task = [instanceId = this->instanceId_, bridgeName = this->bridgeName_, methodName, &data]() {
            BridgeManager::JSCallMethodBinary(instanceId, bridgeName, methodName, data);
        };
        PluginUtilsInner::RunTaskOnPlatform(task);
    }
    LOGI("JSCallMethod task on platform, instanceId is %{public}d bridgeName is %{public}s,",
        this->instanceId_, this->bridgeName_.c_str());

    return ErrorCode::BRIDGE_ERROR_NO;
}

ErrorCode Bridge::SendMethodResult(const std::string& methodName, const std::string& result)
{
    if (!GetAvailable()) {
        LOGE("SendMethodResult: The bridge is unavailable.");
        return ErrorCode::BRIDGE_INVALID;
    }

    auto task = [instanceId = this->instanceId_, bridgeName = this->bridgeName_, methodName, result]() {
        BridgeManager::JSSendMethodResult(instanceId, bridgeName, methodName, result);
    };
    PluginUtilsInner::RunTaskOnPlatform(task);
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
    auto task = [instanceId = this->instanceId_, bridgeName = this->bridgeName_, data]() {
        BridgeManager::JSSendMessage(instanceId, bridgeName, data);
    };
    PluginUtilsInner::RunTaskOnPlatform(task);

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
    auto task = [instanceId = this->instanceId_, bridgeName = this->bridgeName_, &data]() {
        BridgeManager::JSSendMessageBinary(instanceId, bridgeName, data);
    };
    PluginUtilsInner::RunTaskOnPlatform(task);

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

    auto task = [instanceId = this->instanceId_, bridgeName = this->bridgeName_, data]() {
        BridgeManager::JSSendMessageResponse(instanceId, bridgeName, data);
    };
    PluginUtilsInner::RunTaskOnPlatform(task);
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
        auto task = [instanceId = this->instanceId_, bridgeName = this->bridgeName_, methodName]() {
            BridgeManager::JSCancelMethod(instanceId, bridgeName, methodName);
        };
        PluginUtilsInner::RunTaskOnPlatform(task);
        return ErrorCode::BRIDGE_ERROR_NO;
    }
    return ErrorCode::BRIDGE_METHOD_NOT_EXISTS;
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

        auto task = [instanceId = this->instanceId_, methodName, bridgeName = this->bridgeName_] {
            MethodResult result;
            result.SetErrorCodeInfo(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
            result.SetMethodName(methodName);
            result.CreateMethodResultForError();
            BridgeManager::JSSendMethodResult(instanceId, bridgeName, methodName, result.GetResult());
        };
        PluginUtilsInner::RunTaskOnPlatform(task);
        return;
    }

    jsMethodData->PlatformCallMethod(parameter);
}

void Bridge::OnPlatformCallMethodBinary(const std::string& methodName, std::unique_ptr<BufferMapping> data)
{
    std::lock_guard<std::mutex> lock(platformMethodDataListLock_);
    std::shared_ptr<MethodData> jsMethodData = FindPlatformMethodData(methodName);
    if (jsMethodData == nullptr) {
        LOGE("OnPlatformCallMethodBinary: The jsMethodData is null.");
        auto task = [instanceId = this->instanceId_, methodName, bridgeName = this->bridgeName_] {
            MethodResult result;
            result.SetErrorCodeInfo(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
            result.SetMethodName(methodName);
            result.CreateMethodResultForError();
            BridgeManager::JSSendMethodResultBinary(instanceId, bridgeName,
                methodName, result.GetErrorCode(), result.GetErrorMessage(), nullptr);
        };
        PluginUtilsInner::RunTaskOnPlatform(task);
        return;
    }

    jsMethodData->PlatformCallMethodBinary(std::move(data));
}

void Bridge::OnPlatformMethodResult(const std::string& methodName, const std::string& result)
{
    LOGI("OnPlatformMethodResult methodName=%{public}s, result=%{public}s", methodName.c_str(), result.c_str());
    std::lock_guard<std::mutex> lock(jsMethodDataListLock_);
    std::shared_ptr<MethodData> jsMethodData = FindJSMethodData(methodName);
    if (jsMethodData == nullptr) {
        LOGE("OnPlatformCallMethod: The jsMethodData is null.");
        auto task = [instanceId = this->instanceId_, methodName, bridgeName = this->bridgeName_] {
            MethodResult result;
            result.SetErrorCodeInfo(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
            result.SetMethodName(methodName);
            result.CreateMethodResultForError();
            BridgeManager::JSSendMethodResult(instanceId, bridgeName, methodName, result.GetResult());
        };
        PluginUtilsInner::RunTaskOnPlatform(task);
        return;
    }
    jsMethodData->SendMethodResult(result, true);
    EraseJSMethodData(methodName);
}

void Bridge::OnPlatformMethodResultBinary(const std::string& methodName, int errorCode,
    const std::string& errorMessage, std::unique_ptr<BufferMapping> result)
{
    LOGI("OnPlatformMethodResultBinary methodName=%{public}s", methodName.c_str());
    std::lock_guard<std::mutex> lock(jsMethodDataListLock_);
    std::shared_ptr<MethodData> jsMethodData = FindJSMethodData(methodName);
    if (jsMethodData == nullptr) {
        LOGE("OnPlatformMethodResultBinary: The jsMethodData is null.");
        auto task = [instanceId = this->instanceId_, methodName, bridgeName = this->bridgeName_] {
            MethodResult result;
            result.SetErrorCodeInfo(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
            result.SetMethodName(methodName);
            result.CreateMethodResultForError();
            BridgeManager::JSSendMethodResultBinary(instanceId, bridgeName,
                methodName, result.GetErrorCode(), result.GetErrorMessage(), nullptr);
        };
        PluginUtilsInner::RunTaskOnPlatform(task);
        return;
    }
    jsMethodData->SendMethodResultBinary(errorCode, errorMessage, std::move(result), true);
    EraseJSMethodData(methodName);
}

void Bridge::OnPlatformSendMessage(const std::string& data)
{
    if (messageCallback_) {
        messageCallback_->PlatformSendMessage(data);
    }
}

void Bridge::OnPlatformSendMessageBinary(std::unique_ptr<BufferMapping> data)
{
    if (messageCallback_) {
        messageCallback_->PlatformSendMessageBinary(std::move(data));
    }
}

void Bridge::OnPlatformSendMessageResponse(const std::string& data)
{
    std::lock_guard<std::mutex> lock(jsSendMessageDataListLock_);
    if (jsSendMessageDataList_.size() == 0) {
        LOGE("OnPlatformSendMessageResponse: No callback event was found on the JS side.");
        return;
    }

    std::shared_ptr<MethodData> methodData = jsSendMessageDataList_[0];
    methodData->SendMessageResponse(data, true);
    EraseJSMessageData();
}

void Bridge::OnPlatformSendWillTerminate(bool data)
{
    SetTerminate(data);
}
} // namespace OHOS::Plugin::Bridge