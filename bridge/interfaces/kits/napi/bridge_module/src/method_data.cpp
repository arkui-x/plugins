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

#include "method_data.h"

#include "bridge_manager.h"
#include "error_code.h"
#include "log.h"
#include "napi/native_api.h"
#include "napi_utils.h"
#include "plugins/interfaces/native/inner_utils/plugin_inner_napi_utils.h"
#include "plugins/interfaces/native/plugin_c_utils.h"
#include "plugins/interfaces/native/plugin_utils.h"

namespace OHOS::Plugin::Bridge {
static constexpr const char* JS_REGISTER_METHOD_NAME = "name";
static constexpr const char* JS_REGISTER_METHOD_FUNCTION = "method";
static constexpr const char* MESSAGE_RESPONSE_NAME = "result";
static constexpr const char* MESSAGE_RESPONSE_ERRORCODE = "errorcode";

MethodData::~MethodData()
{
    asyncEvent_ = nullptr;
}

const std::string& MethodData::GetBridgeName(void) const
{
    return bridgeName_;
}
void MethodData::SetBridgeName(const std::string& bridgeName)
{
    bridgeName_ = bridgeName;
}

void MethodData::SetMethodName(const std::string& methodName)
{
    methodName_ = methodName;
}
const std::string& MethodData::GetMethodName(void) const
{
    return methodName_;
}

void MethodData::SetMethodParamName(const std::string& paramName)
{
    parameter_ = paramName;
}

const std::string& MethodData::GetMethodParamName(void) const
{
    return parameter_;
}

void MethodData::SetIsMessageEvent(bool isMessageEvent)
{
    isMessageEvent_ = isMessageEvent;
}

bool MethodData::IsMessageEvent(void) const
{
    return isMessageEvent_;
}

void MethodData::SetStartTime(int64_t startTime)
{
    startTime_ = startTime;
}

int64_t MethodData::GetStartTime(void) const
{
    return startTime_;
}

int64_t MethodData::GetSystemTime(void)
{
    auto curNow = std::chrono::system_clock::now();
    std::chrono::milliseconds milliSec =
        std::chrono::duration_cast<std::chrono::milliseconds>(curNow.time_since_epoch());
    return milliSec.count();
}

NAPIAsyncEvent* MethodData::GetAsyncEvent(void) const
{
    return asyncEvent_;
}

bool MethodData::GetName(napi_value arg)
{
    if (PluginInnerNApiUtils::GetValueType(env_, arg) == napi_string) {
        methodName_ = PluginInnerNApiUtils::GetStringFromValueUtf8(env_, arg);
        return !methodName_.empty();
    }
    return false;
}

bool MethodData::GetMessageData(napi_value arg)
{
    Json json {};
    json[MESSAGE_RESPONSE_NAME] = NAPIUtils::PlatformPremers(env_, arg);
    parameter_ = json.dump();
    return true;
}

napi_value MethodData::GetMessageResponse(const std::string& data)
{
    return NAPIUtils::NAPI_GetPremers(env_, Json::parse(data, nullptr, false));
}

bool MethodData::GetParamsByRecord(size_t argc, napi_value* arg)
{
    bool ret = NAPIUtils::NapiValuesToJsonString(env_, argc, arg, parameter_);
    return ret;
}

bool MethodData::CreateEvent(napi_value arg, bool needListenEvent)
{
    if (asyncEvent_ == nullptr) {
        asyncEvent_ = new (std::nothrow) NAPIAsyncEvent(env_);
    }
    if (asyncEvent_ == nullptr) {
        LOGE("CreateEvent: The asyncEvent_ is null.");
        return false;
    }

    if (needListenEvent) {
        if (isMessageEvent_) {
            InitEventSuccessForMessage();
            InitEventErrorForMessage();
        } else {
            InitEventSuccessForMethod();
            InitEventErrorForMethod();
        }
    }
    return true;
}

bool MethodData::GetCallback(napi_value arg, bool needListenEvent)
{
    if (PluginInnerNApiUtils::GetValueType(env_, arg) != napi_function) {
        LOGE("GetCallback: The napi_value is not napi_function.");
        return false;
    }

    if (!CreateEvent(arg, needListenEvent)) {
        LOGE("GetCallback: Failed to create an event.");
        return false;
    }

    bool ret = asyncEvent_->CreateCallback(arg);
    if (!ret) {
        LOGE("GetCallback: Failed to create the JS callback event.");
        ReleaseEvent();
    }
    return ret;
}

napi_value MethodData::GetPromise(bool needListenEvent)
{
    bool ret = CreateEvent(nullptr, needListenEvent);
    if (!ret) {
        LOGE("GetPromise: Failed to create an event.");
        return nullptr;
    }

    napi_value result = asyncEvent_->CreatePromise();
    if (result == nullptr) {
        LOGE("GetPromise: Failed to create the JS promise event.");
        ReleaseEvent();
    }
    return result;
}

bool MethodData::IsCallback(void) const
{
    if (asyncEvent_) {
        return asyncEvent_->IsCallback();
    }
    return false;
}

void MethodData::InitEventSuccessForMethod(void)
{
    if (asyncEvent_ == nullptr) {
        LOGE("InitEventSuccessForMethod: asyncEvent_ is null.");
        return;
    }

    auto event = [](napi_env env, const std::string& bridgeName,
        const std::string& methodName, napi_value resultValue) {
        MethodResult result;
        result.SetErrorCode(0);
        result.SetMethodName(methodName);
        result.ParseJSMethodResult(env, resultValue);

        auto task = [platfromResult = result.GetResult(), bridgeName, methodName]() {
            Ace::Platform::BridgeManager::JSSendMethodResult(bridgeName, methodName, platfromResult);
        };
        PluginUtils::RunTaskOnPlatform(task);
    };
    asyncEvent_->SetAsyncEventSuccess(event);
}

void MethodData::InitEventErrorForMethod(void)
{
    if (asyncEvent_ == nullptr) {
        LOGE("InitEventErrorForMethod: asyncEvent_ is null.");
        return;
    }

    auto event = [](napi_env env, const std::string& bridgeName,
        const std::string& methodName, int errorCode) {
        MethodResult result;
        result.SetErrorCode(errorCode);
        result.SetMethodName(methodName);
        result.ParseJSMethodResult(env, nullptr);

        auto task = [platfromResult = result.GetResult(), bridgeName, methodName]() {
            Ace::Platform::BridgeManager::JSSendMethodResult(bridgeName, methodName, platfromResult);
        };
        PluginUtils::RunTaskOnPlatform(task);
    };
    asyncEvent_->SetAsyncEventError(event);
}

void MethodData::InitEventSuccessForMessage(void)
{
    if (asyncEvent_ == nullptr) {
        LOGE("InitEventSuccessForMessage: asyncEvent_ is null.");
        return;
    }

    auto event = [](napi_env env, const std::string& bridgeName,
        const std::string& methodName, napi_value resultValue) {
        Json json {};
        json[MESSAGE_RESPONSE_NAME] = NAPIUtils::PlatformPremers(env, resultValue);
        std::string data = json.dump();

        auto task = [data, bridgeName]() {
            Ace::Platform::BridgeManager::JSSendMessageResponse(bridgeName, data);
        };
        PluginUtils::RunTaskOnPlatform(task);
    };
    asyncEvent_->SetAsyncEventSuccess(event);
}

void MethodData::InitEventErrorForMessage(void)
{
    if (asyncEvent_ == nullptr) {
        LOGE("InitEventErrorForMessage: asyncEvent_ is null.");
        return;
    }

    auto event = []
        (napi_env env, const std::string& bridgeName, const std::string& methodName, int errorCode) {
        std::string data = Json::parse("{}", nullptr, false).dump();
        auto task = [data, bridgeName]() {
            Ace::Platform::BridgeManager::JSSendMessageResponse(bridgeName, data);
        };
        PluginUtils::RunTaskOnPlatform(task);
    };
    asyncEvent_->SetAsyncEventError(event);
}

void MethodData::ReleaseEvent(void)
{
    if (asyncEvent_) {
        delete asyncEvent_;
        asyncEvent_ = nullptr;
    }
}

bool MethodData::GetJSRegisterMethodObject(napi_value object)
{
    if (!PluginInnerNApiUtils::HasNamedProperty(env_, object, JS_REGISTER_METHOD_NAME) ||
        !PluginInnerNApiUtils::HasNamedProperty(env_, object, JS_REGISTER_METHOD_FUNCTION)) {
        LOGE("GetJSRegisterMethodObject: Parameter error.");
        return false;
    }

    napi_value jsMethodName = PluginInnerNApiUtils::GetNamedProperty(env_, object, JS_REGISTER_METHOD_NAME);
    napi_value jsMethodFun = PluginInnerNApiUtils::GetNamedProperty(env_, object, JS_REGISTER_METHOD_FUNCTION);
    if (jsMethodName == nullptr || jsMethodFun == nullptr) {
        LOGE("GetJSRegisterMethodObject: Analytic parameter error.");
        return false;
    }

    methodName_ = PluginInnerNApiUtils::GetStringFromValueUtf8(env_, jsMethodName);
    if (methodName_.empty()) {
        LOGE("GetJSRegisterMethodObject: methodName_ is empty.");
        return false;
    }
    if (!CreateEvent(jsMethodFun, true)) {
        LOGE("GetJSRegisterMethodObject: Failed to create an event.");
        return false;
    }

    bool ret = asyncEvent_->CreateCallback(jsMethodFun);
    if (!ret) {
        LOGE("GetJSRegisterMethodObject: Failed to create the JS callback.");
        ReleaseEvent();
    }
    return ret;
}

bool MethodData::SendMethodResult(const std::string& data, bool removeMethod)
{
    if (asyncEvent_ == nullptr) {
        LOGE("SendMethodResult: asyncEvent_ is null.");
        return false;
    }

    MethodResult resultValue;
    resultValue.ParsePlatformMethodResult(env_, data);
    asyncEvent_->SetErrorCode(resultValue.GetErrorCode());
    asyncEvent_->SetRefErrorData(resultValue.GetErrorResult());
    asyncEvent_->SetRefData(resultValue.GetOkResult());
    asyncEvent_->SetBridgeName(bridgeName_);
    AsyncWorkComplete jsCallback = nullptr;
    if (removeMethod) {
        jsCallback = [](napi_env env, napi_status status, void* data) {
            auto deleter = [](NAPIAsyncEvent* event) { delete event; };
            std::unique_ptr<NAPIAsyncEvent, decltype(deleter)> event(static_cast<NAPIAsyncEvent*>(data), deleter);
            event->AsyncWorkCallback();
        };
    } else {
        jsCallback = [](napi_env env, napi_status status, void* data) {
            NAPIAsyncEvent* event = static_cast<NAPIAsyncEvent*>(data);
            event->AsyncWorkCallback();
        };
    }
    return asyncEvent_->CreateAsyncWork(methodName_, [](napi_env env, void* data) {}, jsCallback);
}

bool MethodData::SendMessageResponse(const std::string& data, bool removeMethod)
{
    if (asyncEvent_ == nullptr) {
        LOGE("SendMessageResponse: asyncEvent_ is null.");
        return false;
    }

    MethodResult resultValue;
    resultValue.SetOkResult(NAPIUtils::NAPI_GetPremers(env_,
        Json::parse(data, nullptr, false).at(MESSAGE_RESPONSE_NAME)));
    int errorCode = NAPIUtils::NAPI_GetErrorCodeFromFson(
        Json::parse(data, nullptr, false).at(MESSAGE_RESPONSE_ERRORCODE));
    resultValue.SetErrorCode(errorCode);
    resultValue.CreateErrorObject(env_);
    asyncEvent_->SetErrorCode(errorCode);
    asyncEvent_->SetRefErrorData(resultValue.GetErrorResult());
    asyncEvent_->SetRefData(resultValue.GetOkResult());
    asyncEvent_->SetBridgeName(bridgeName_);
    AsyncWorkComplete jsCallback = nullptr;
    if (removeMethod) {
        jsCallback = [](napi_env env, napi_status status, void* data) {
            auto deleter = [](NAPIAsyncEvent* event) { delete event; };
            std::unique_ptr<NAPIAsyncEvent, decltype(deleter)> event(static_cast<NAPIAsyncEvent*>(data), deleter);
            event->AsyncWorkCallback();
        };
    } else {
        jsCallback = [](napi_env env, napi_status status, void* data) {
            NAPIAsyncEvent* event = static_cast<NAPIAsyncEvent*>(data);
            event->AsyncWorkCallback();
        };
    }

    return asyncEvent_->CreateAsyncWork(methodName_, [](napi_env env, void* data) {}, jsCallback);
}

void MethodData::PlatformCallMethod(const std::string& parameter)
{
    if (asyncEvent_ == nullptr) {
        LOGE("PlatformCallMethod: asyncEvent_ is null.");
        MethodResult result;
        result.SetErrorCode(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
        result.SetMethodName(methodName_);
        result.ParseJSMethodResult(env_, nullptr);

        auto task = [platfromResult = result.GetResult(), bridgeName = this->bridgeName_, methodName = methodName_]() {
            Ace::Platform::BridgeManager::JSSendMethodResult(bridgeName, methodName, platfromResult);
        };
        PluginUtils::RunTaskOnPlatform(task);
        return;
    }

    asyncEvent_->SetMethodParameter(parameter);
    asyncEvent_->SetBridgeName(bridgeName_);
    AsyncWorkComplete jsCallback = [](napi_env env, napi_status status, void* data) {
        NAPIAsyncEvent* event = static_cast<NAPIAsyncEvent*>(data);
        event->AsyncWorkCallMethod();
    };
    asyncEvent_->CreateAsyncWork(methodName_, [](napi_env env, void* data) {}, jsCallback);
}

void MethodData::PlatformSendMessge(const std::string& data)
{
    if (asyncEvent_ == nullptr) {
        LOGE("PlatformSendMessge: asyncEvent_ is null.");
        return;
    }

    int errorCode = NAPIUtils::NAPI_GetErrorCodeFromFson(
        Json::parse(data, nullptr, false).at(MESSAGE_RESPONSE_ERRORCODE));
    napi_value jsResult = NAPIUtils::NAPI_GetPremers(env_,
        Json::parse(data, nullptr, false).at(MESSAGE_RESPONSE_NAME));
    MethodResult resultValue;
    resultValue.SetErrorCode(errorCode);
    resultValue.CreateErrorObject(env_);
    asyncEvent_->SetErrorCode(resultValue.GetErrorCode());
    asyncEvent_->SetRefErrorData(resultValue.GetErrorResult());
    asyncEvent_->SetRefData(jsResult);
    asyncEvent_->SetBridgeName(bridgeName_);
    asyncEvent_->AsyncWorkMessage();
}

void MethodData::SendAsyncCallback(int errorCode, napi_value okArg)
{
    if (asyncEvent_ == nullptr) {
        LOGE("SendAsyncCallback: asyncEvent_ is null.");
        return;
    }

    MethodResult resultValue;
    resultValue.SetErrorCode(errorCode);
    resultValue.CreateErrorObject(env_);
    asyncEvent_->SetErrorCode(resultValue.GetErrorCode());
    asyncEvent_->SetRefErrorData(resultValue.GetErrorResult());
    asyncEvent_->SetRefData(okArg);
    asyncEvent_->SetBridgeName(bridgeName_);

    AsyncWorkComplete jsCallback = nullptr;
    jsCallback = [](napi_env env, napi_status status, void* data) {
        auto deleter = [](NAPIAsyncEvent* event) { delete event; };
        std::unique_ptr<NAPIAsyncEvent, decltype(deleter)> event(static_cast<NAPIAsyncEvent*>(data), deleter);
        event->AsyncWorkCallback();
    };

    bool ret = asyncEvent_->CreateAsyncWork(methodName_, [](napi_env env, void* data) {}, jsCallback);
    if (!ret) {
        LOGE("SendAsyncCallback: Failed to call the JS callback.");
        ReleaseEvent();
    }
}
} // namespace OHOS::Plugin::Bridge