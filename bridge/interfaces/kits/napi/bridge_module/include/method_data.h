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

#ifndef PLUGINS_BRIDGE_METHOD_DATA_H
#define PLUGINS_BRIDGE_METHOD_DATA_H

#include <memory>
#include <string>

#include "method_result.h"
#include "napi/native_api.h"
#include "napi_async_event.h"

namespace OHOS::Plugin::Bridge {
class MethodData {
public:
    explicit MethodData(napi_env env) : env_(env) {};
    ~MethodData();

    static int64_t GetSystemTime(void);
    void SetBridgeName(const std::string& bridgeName);
    const std::string& GetBridgeName(void) const;
    void SetMethodName(const std::string& methodName);
    const std::string& GetMethodName(void) const;
    void SetMethodParamName(const std::string& paramName);
    const std::string& GetMethodParamName(void) const;
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
    bool IsCallback(void) const;
    bool SendMethodResult(const std::string& data, bool removeMethod);
    bool SendMessageResponse(const std::string& data, bool removeMethod);
    void PlatformCallMethod(const std::string& parameter);
    void PlatformSendMessge(const std::string& data);
    void SendAsyncCallback(int errorCode, napi_value okArg);

private:
    NAPIAsyncEvent* asyncEvent_ = nullptr;
    std::string bridgeName_;
    std::string methodName_;
    std::string parameter_;
    napi_env env_ = nullptr;
    bool isMessageEvent_ = false;
    int64_t startTime_ = 0;

    bool CreateEvent(napi_value arg, bool needListenEvent);
    void InitEventSuccessForMethod(void);
    void InitEventErrorForMethod(void);
    void InitEventSuccessForMessage(void);
    void InitEventErrorForMessage(void);
};
} // namespace OHOS::Plugin::Bridge
#endif