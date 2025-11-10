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

#ifndef PLUGINS_BRIDGE_METHOD_DATA_H
#define PLUGINS_BRIDGE_METHOD_DATA_H

#include <memory>
#include <string>
#include <vector>

#include "bridge_event_handle.h"
#include "buffer_mapping.h"
#include "method_result.h"
#include "napi/native_api.h"
#include "napi_async_event.h"

namespace OHOS::Plugin::Bridge {
enum class CodecType {
    JSON_CODEC,
    BINARY_CODEC
};

class MethodData {
public:
    explicit MethodData(napi_env env, const CodecType& type);
    ~MethodData();

    static std::shared_ptr<MethodData> CreateMethodData(napi_env env, const CodecType& type);

    static int64_t GetSystemTime(void);
    void SetBridgeName(const std::string& bridgeName);
    const std::string& GetBridgeName(void) const;
    void SetMethodName(const std::string& methodName);
    const std::string& GetMethodName(void) const;

    void SetMethodParamName(const std::string& paramName);
    const std::string& GetMethodParamName(void) const;
    void SetMethodParamNameBinary(std::unique_ptr<std::vector<uint8_t>> paramName);
    const std::vector<uint8_t>& GetMethodParamNameBinary(void) const;
    
    void SetIsMessageEvent(bool isMessageEvent);
    bool IsMessageEvent(void) const;
    void SetStartTime(int64_t startTime);
    int64_t GetStartTime(void) const;
    NAPIAsyncEvent* GetAsyncEvent(void) const;
    bool GetName(napi_value arg);

    bool GetMessageData(napi_value arg);
    napi_value GetMessageResponse(const std::string& data);
    bool GetParamsByRecord(size_t argc, napi_value* arg);

    bool GetCallback(napi_value arg, bool needListenEvent = false);
    napi_value GetPromise(bool needListenEvent);
    void ReleaseEvent(void);
    bool GetJSRegisterMethodObject(napi_value object);
    bool GetJSRegisterMethodObjectCallBack(const std::string& arg, napi_value object);
    bool IsCallback(void) const;

    bool SendMethodResult(const std::string& data, bool removeMethod);
    bool SendMessageResponse(const std::string& data, bool removeMethod);
    void PlatformCallMethod(const std::string& parameter);
    void PlatformSendMessage(const std::string& data);
    
    bool SendMethodResultBinary(int errorCode,
        const std::string& errorMessage, std::unique_ptr<Ace::Platform::BufferMapping> data, bool removeMethod);
    void PlatformSendMessageBinary(std::unique_ptr<Ace::Platform::BufferMapping> data);
    void PlatformCallMethodBinary(std::unique_ptr<Ace::Platform::BufferMapping> data);
        
    void SendAsyncCallback(int errorCode, napi_value okArg);
    void UpdateMethodName(void);

    MethodResult PlatformCallMethodSync(const std::string& parameter);
    MethodResult PlatformCallMethodSyncBinary(std::unique_ptr<Ace::Platform::BufferMapping> parameter);

private:
    NAPIAsyncEvent* asyncEvent_ = nullptr;
    std::string bridgeName_;
    std::string methodName_;
    napi_env env_ = nullptr;
    bool isMessageEvent_ = false;
    int64_t startTime_ = 0;
    CodecType codecType_ = CodecType::JSON_CODEC;
    std::shared_ptr<BridgeEventHandle> taskExecutor_ = BridgeEventHandle::GetInstance();

    // for json codec
    std::string jsonParameter_;
    // for binary codec
    std::unique_ptr<std::vector<uint8_t>> binaryParameter_;

    bool CreateEvent(napi_value arg, bool needListenEvent);
    void InitEventSuccessForMethod(void);
    void InitEventErrorForMethod(void);
    void InitEventSuccessForMessage(void);
    void InitEventErrorForMessage(void);
};
} // namespace OHOS::Plugin::Bridge
#endif