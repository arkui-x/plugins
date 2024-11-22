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

#ifndef PLUGINS_BRIDGE_BRIDGE_H
#define PLUGINS_BRIDGE_BRIDGE_H

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "buffer_mapping.h"
#include "error_code.h"
#include "method_data.h"
#include "napi/native_api.h"

namespace OHOS::Plugin::Bridge {
class Bridge {
public:
    Bridge(const std::string& bridgeName, int32_t instanceId, const CodecType& type);
    ~Bridge();

    static bool BridgeNameExists(const std::string& bridgeName, const int32_t instanceId);
    const std::string& GetBridgeName(void);
    int32_t GetInstanceID(void);
    ErrorCode RegisterBridge(const std::string& bridgeName);
    void UnRegisterBridge(const std::string& bridgeName);
    void UnRegisterBridge(void);
    ErrorCode CallMethod(const std::string& methodName, const std::shared_ptr<MethodData>& methodData);
    ErrorCode SendMethodResult(const std::string& methodName, const std::string& result);
    ErrorCode SendMessage(const std::string& data, std::shared_ptr<MethodData>& methodData);
    ErrorCode SendMessageResponse(const std::string& data);
    ErrorCode RegisterMethod(const std::string& methodName, const std::shared_ptr<MethodData>& methodData);
    ErrorCode UnRegisterMethod(const std::string& methodName);
    void SetMessageListener(std::shared_ptr<MethodData>& callback);
    void SetAvailable(bool available);
    bool GetAvailable(void);
    void RemoveJSMethodData(const std::string& methodName);
    void RemoveMessageData(void);
    bool GetTerminate(void);
    void SetTerminate(bool terminate);

    ErrorCode SendMessageBinary(const std::vector<uint8_t>& data, std::shared_ptr<MethodData>& methodData);
    CodecType GetCodecType() { return codecType_; };
  
private:
    std::string bridgeName_;
    int32_t instanceId_;
    CodecType codecType_ = CodecType::JSON_CODEC;
    bool available_ = false;
    bool terminate_ = false;
    napi_env env_ = nullptr;
    std::map<std::string, std::shared_ptr<MethodData>> platformMethodDataList_;
    std::map<std::string, std::shared_ptr<MethodData>> jsMethodDataList_;
    std::vector<std::shared_ptr<MethodData>> jsSendMessageDataList_;
    std::shared_ptr<MethodData> messageCallback_;
    std::mutex platformMethodDataListLock_;
    std::mutex jsMethodDataListLock_;
    std::mutex jsSendMessageDataListLock_;

    std::shared_ptr<MethodData> FindPlatformMethodData(const std::string& methodName);
    std::shared_ptr<MethodData> FindJSMethodData(const std::string& methodName);
    void EraseJSMethodData(const std::string& methodName);
    void EraseJSMessageData(void);
    void OnPlatformCallMethod(const std::string& methodName, const std::string& parameter);
    void OnPlatformMethodResult(const std::string& methodName, const std::string& result);
    void OnPlatformSendMessage(const std::string& data);
    void OnPlatformSendMessageResponse(const std::string& data);
    void OnPlatformSendWillTerminate(bool data);

    void OnPlatformMethodResultBinary(const std::string& methodName, int errorCode,
        const std::string& errorMessage, std::unique_ptr<Ace::Platform::BufferMapping> result);
    void OnPlatformCallMethodBinary(const std::string& methodName, std::unique_ptr<Ace::Platform::BufferMapping> data);
    void OnPlatformSendMessageBinary(std::unique_ptr<Ace::Platform::BufferMapping> data);
};
} // namespace OHOS::Plugin::Bridge
#endif