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
Bridge::Bridge(const std::string& bridgeName) : bridgeName_(bridgeName)
{
    avaiable_ = (RegisterBridge(bridgeName) == ErrorCode::BRIDGE_ERROR_NO);
}

Bridge::~Bridge()
{
    if (avaiable_) {
        avaiable_ = false;
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

    receiver->callMethodCallback_ = std::bind(
        &Bridge::OnPlatformCallMethod, this, std::placeholders::_1, std::placeholders::_2);

    receiver->methodResultCallback_ = std::bind(
        &Bridge::OnPlatformMethodResult, this, std::placeholders::_1, std::placeholders::_2);

    receiver->sendMessageCallback_ =
        std::bind(&Bridge::OnPlatformSendMessage, this, std::placeholders::_1);

    receiver->sendMessageResponseCallback_ =
        std::bind(&Bridge::OnPlatformSendMessageResponse, this, std::placeholders::_1);

    if (BridgeManager::JSRegisterBridge(bridgeName, receiver)) {
        return ErrorCode::BRIDGE_ERROR_NO;
    }
    LOGE("RegisterBridge: The bridge registration failure.");
    return ErrorCode::BRIDGE_CREATE_ERROR;
}

void Bridge::UnRegisterBridge(const std::string& bridgeName)
{
    if (!bridgeName.empty()) {
        avaiable_ = false;
        BridgeManager::JSUnRegisterBridge(bridgeName);
    }
}

void Bridge::UnRegisterBridge(void)
{
    UnRegisterBridge(bridgeName_);
}

ErrorCode Bridge::CallMethod(const std::string& methodName, const std::shared_ptr<MethodData>& methodData)
{
    if (!GetAvaiable()) {
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

    auto task = [bridgeName = this->bridgeName_, methodName, parameter = methodData->GetMethodParamName()]() {
        BridgeManager::JSCallMethod(bridgeName, methodName, parameter);
    };
    PluginUtilsInner::RunTaskOnPlatform(task);
    return ErrorCode::BRIDGE_ERROR_NO;
}

ErrorCode Bridge::SendMethodResult(const std::string& methodName, const std::string& result)
{
    if (!GetAvaiable()) {
        LOGE("SendMethodResult: The bridge is unavailable.");
        return ErrorCode::BRIDGE_INVALID;
    }

    auto task = [bridgeName = this->bridgeName_, methodName, result]() {
        BridgeManager::JSSendMethodResult(bridgeName, methodName, result);
    };
    PluginUtilsInner::RunTaskOnPlatform(task);
    return ErrorCode::BRIDGE_ERROR_NO;
}

ErrorCode Bridge::SendMessage(const std::string& data, std::shared_ptr<MethodData>& methodData)
{
    if (!GetAvaiable()) {
        LOGE("SendMessage: The bridge is unavailable.");
        return ErrorCode::BRIDGE_INVALID;
    }

    std::lock_guard<std::mutex> lock(jsSendMessageDataListLock_);
    jsSendMessageDataList_.push_back(methodData);
    auto task = [bridgeName = this->bridgeName_, data]() { BridgeManager::JSSendMessage(bridgeName, data); };
    PluginUtilsInner::RunTaskOnPlatform(task);

    return ErrorCode::BRIDGE_ERROR_NO;
}

ErrorCode Bridge::SendMessageResponse(const std::string& data)
{
    if (!GetAvaiable()) {
        LOGE("SendMessageResponse: The bridge is unavailable.");
        return ErrorCode::BRIDGE_INVALID;
    }

    if (data.empty()) {
        LOGE("SendMessageResponse: The data is empty.");
        return ErrorCode::BRIDGE_DATA_ERROR;
    }

    auto task = [bridgeName = this->bridgeName_, data]() { BridgeManager::JSSendMessageResponse(bridgeName, data); };
    PluginUtilsInner::RunTaskOnPlatform(task);
    return ErrorCode::BRIDGE_ERROR_NO;
}

ErrorCode Bridge::RegisterMethod(const std::string& methodName, const std::shared_ptr<MethodData>& methodData)
{
    if (!GetAvaiable()) {
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
    if (!GetAvaiable()) {
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

void Bridge::SetAvaiable(bool avaiable)
{
    avaiable_ = avaiable;
}

bool Bridge::GetAvaiable(void)
{
    return avaiable_;
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

void Bridge::OnPlatformCallMethod(const std::string& methodName, const std::string& parameter)
{
    std::lock_guard<std::mutex> lock(platformMethodDataListLock_);
    std::shared_ptr<MethodData> jsMethodData = FindPlatformMethodData(methodName);
    if (jsMethodData == nullptr) {
        LOGE("OnPlatformCallMethod: The jsMethodData is null.");

        auto task = [methodName, braidgeName = this->bridgeName_] {
            MethodResult result;
            result.SetErrorCodeInfo(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
            result.SetMethodName(methodName);
            result.CreateMethodResultForError();
            BridgeManager::JSSendMethodResult(braidgeName, methodName, result.GetResult());
        };
        PluginUtilsInner::RunTaskOnPlatform(task);
        return;
    }

    jsMethodData->PlatformCallMethod(parameter);
}

void Bridge::OnPlatformMethodResult(const std::string& methodName, const std::string& result)
{
    LOGI("OnPlatformMethodResult methodName=%{public}s, result=%{public}s", methodName.c_str(), result.c_str());
    std::lock_guard<std::mutex> lock(jsMethodDataListLock_);
    std::shared_ptr<MethodData> jsMethodData = FindJSMethodData(methodName);
    if (jsMethodData == nullptr) {
        LOGE("OnPlatformCallMethod: The jsMethodData is null.");
        auto task = [methodName, braidgeName = this->bridgeName_] {
            MethodResult result;
            result.SetErrorCodeInfo(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
            result.SetMethodName(methodName);
            result.CreateMethodResultForError();
            BridgeManager::JSSendMethodResult(braidgeName, methodName, result.GetResult());
        };
        PluginUtilsInner::RunTaskOnPlatform(task);
        return;
    }
    jsMethodData->SendMethodResult(result, true);
    EraseJSMethodData(methodName);
}

void Bridge::OnPlatformSendMessage(const std::string& data)
{
    if (messageCallback_) {
        messageCallback_->PlatformSendMessage(data);
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
} // namespace OHOS::Plugin::Bridge