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

#include "method_data.h"

#include "bridge_binary_codec.h"
#include "bridge_json_codec.h"
#include "bridge_manager.h"
#include "error_code.h"
#include "log.h"
#include "method_data_converter.h"
#include "napi/native_api.h"
#include "napi_utils.h"

#include "plugins/bridge/interfaces/kits/napi/bridge_module/include/method_id.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_inner.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_napi.h"
#include "plugins/interfaces/native/plugin_utils.h"

namespace OHOS::Plugin::Bridge {
static constexpr const char* JS_REGISTER_METHOD_NAME = "name";
static constexpr const char* JS_REGISTER_METHOD_FUNCTION = "method";

MethodData::MethodData(napi_env env, const CodecType& type) : env_(env), codecType_(type)
{
}

std::shared_ptr<MethodData> MethodData::CreateMethodData(napi_env env, const CodecType& type)
{
    return std::make_shared<MethodData>(env, type);
}

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
    jsonParameter_ = paramName;
}

const std::string& MethodData::GetMethodParamName(void) const
{
    return jsonParameter_;
}

void MethodData::SetMethodParamNameBinary(std::unique_ptr<std::vector<uint8_t>> paramName)
{
    binaryParameter_ = std::move(paramName);
}

const std::vector<uint8_t>& MethodData::GetMethodParamNameBinary(void) const
{
    return *binaryParameter_;
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
    if (PluginUtilsNApi::GetValueType(env_, arg) == napi_string) {
        methodName_ = PluginUtilsNApi::GetStringFromValueUtf8(env_, arg);
        return !methodName_.empty();
    }
    return false;
}

bool MethodData::GetMessageData(napi_value arg)
{
    if (codecType_ == CodecType::JSON_CODEC) {
        NapiRawValue rawValue { .env = env_, .value = arg };
        auto encoded = BridgeJsonCodec::GetInstance().Encode(rawValue);
        SetMethodParamName(encoded->value);
    } else if (codecType_ == CodecType::BINARY_CODEC) {
        const auto& codecableValue = MethodDataConverter::ConvertToCodecableValue(env_, arg);
        auto encoded = BridgeBinaryCodec::GetInstance().Encode(codecableValue);
        if (!encoded || encoded->size() == 0) {
            LOGE("GetMessageData: binary codec failed.");
            return false;
        }
        SetMethodParamNameBinary(std::move(encoded));
    }
    return true;
}

napi_value MethodData::GetMessageResponse(const std::string& data)
{
    DecodeValue decodeValue { .env = env_, .value = data };
    auto decoded = BridgeJsonCodec::GetInstance().Decode(decodeValue);
    return decoded->value;
}

bool MethodData::GetParamsByRecord(size_t argc, napi_value* arg)
{
    if (codecType_ == CodecType::JSON_CODEC) {
        NapiRawValue rawValue { .env = env_, .argc = static_cast<int>(argc), .argValue = arg };
        auto encoded = BridgeJsonCodec::GetInstance().Encode(rawValue);
        SetMethodParamName(encoded->value);
        return true;
    } else if (codecType_ == CodecType::BINARY_CODEC) {
        const auto& codecableValue = MethodDataConverter::ConvertToCodecableValue(env_, argc, arg);
        auto encoded = BridgeBinaryCodec::GetInstance().Encode(codecableValue);
        if (!encoded || encoded->size() == 0) {
            LOGE("GetParamsByRecord: binary codec failed.");
            return false;
        }
        SetMethodParamNameBinary(std::move(encoded));
        return true;
    }

    LOGE("GetParamsByRecord: codec type is error.");
    return false;
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
    if (PluginUtilsNApi::GetValueType(env_, arg) != napi_function) {
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

    auto event = [this, codecType = codecType_](napi_env env, const std::string& bridgeName,
                     const std::string& methodName, napi_value resultValue) {
        MethodResult result;
        result.SetErrorCode(0);
        result.SetMethodName(methodName);
        if (codecType == CodecType::JSON_CODEC) {
            result.ParseJSMethodResult(env, resultValue);
            auto task = [platformResult = result.GetResult(), bridgeName, methodName]() {
                Ace::Platform::BridgeManager::JSSendMethodResult(bridgeName, methodName, platformResult);
            };
            if (!taskExecutor_) {
                LOGE("InitEventSuccessForMethod(JSON): taskExecutor_ is null.");
            } else {
                taskExecutor_->RunTaskOnBridgeThread(task);
            }
        } else if (codecType == CodecType::BINARY_CODEC) {
            result.ParseJSMethodResultBinary(env, resultValue);
            auto task = [resultHold = result.GetResultBinary(), bridgeName, methodName]() {
                std::unique_ptr<std::vector<uint8_t>> resultData { resultHold };
                Ace::Platform::BridgeManager::JSSendMethodResultBinary(
                    bridgeName, methodName, 0, "", std::move(resultData));
            };
            if (!taskExecutor_) {
                LOGE("InitEventSuccessForMethod(Binary): taskExecutor_ is null.");
            } else {
                taskExecutor_->RunTaskOnBridgeThread(task);
            }
        }
    };
    asyncEvent_->SetAsyncEventSuccess(event);
}

void MethodData::InitEventErrorForMethod(void)
{
    if (asyncEvent_ == nullptr) {
        LOGE("InitEventErrorForMethod: asyncEvent_ is null.");
        return;
    }

    auto event = [this, codecType = codecType_](
                     napi_env env, const std::string& bridgeName, const std::string& methodName, int errorCode) {
        MethodResult result;
        result.SetErrorCode(errorCode);
        result.SetMethodName(methodName);
        if (codecType == CodecType::JSON_CODEC) {
            result.ParseJSMethodResult(env, nullptr);
            auto task = [platformResult = result.GetResult(), bridgeName, methodName]() {
                Ace::Platform::BridgeManager::JSSendMethodResult(bridgeName, methodName, platformResult);
            };
            if (!taskExecutor_) {
                LOGE("InitEventErrorForMethod(JSON): taskExecutor_ is null.");
            } else {
                taskExecutor_->RunTaskOnBridgeThread(task);
            }
        } else if (codecType == CodecType::BINARY_CODEC) {
            auto task = [errorCode = result.GetErrorCode(), errorMessage = result.GetErrorMessage(),
                            bridgeName, methodName]() {
                Ace::Platform::BridgeManager::JSSendMethodResultBinary(
                    bridgeName, methodName, errorCode, errorMessage, nullptr);
            };
            if (!taskExecutor_) {
                LOGE("InitEventErrorForMethod(Binary): taskExecutor_ is null.");
            } else {
                taskExecutor_->RunTaskOnBridgeThread(task);
            }
        }
    };
    asyncEvent_->SetAsyncEventError(event);
}

void MethodData::InitEventSuccessForMessage(void)
{
    if (asyncEvent_ == nullptr) {
        LOGE("InitEventSuccessForMessage: asyncEvent_ is null.");
        return;
    }

    auto event = [this](napi_env env, const std::string& bridgeName, const std::string& methodName,
                     napi_value resultValue) {
        NapiRawValue rawValue { .env = env, .value = resultValue };
        auto encoded = BridgeJsonCodec::GetInstance().Encode(rawValue);
        auto task = [data = encoded->value, bridgeName]() {
            Ace::Platform::BridgeManager::JSSendMessageResponse(bridgeName, data);
        };
        if (!taskExecutor_) {
            LOGE("InitEventSuccessForMessage: taskExecutor_ is null.");
        } else {
            taskExecutor_->RunTaskOnBridgeThread(task);
        }
    };
    asyncEvent_->SetAsyncEventSuccess(event);
}

void MethodData::InitEventErrorForMessage(void)
{
    if (asyncEvent_ == nullptr) {
        LOGE("InitEventErrorForMessage: asyncEvent_ is null.");
        return;
    }

    auto event = [this](napi_env env, const std::string& bridgeName, const std::string& methodName, int errorCode) {
        auto data = BridgeJsonCodec::ParseNullParams("{}");
        auto task = [data, bridgeName]() {
            Ace::Platform::BridgeManager::JSSendMessageResponse(bridgeName, data);
        };
        if (!taskExecutor_) {
            LOGE("InitEventErrorForMessage: taskExecutor_ is null.");
        } else {
            taskExecutor_->RunTaskOnBridgeThread(task);
        }
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
    if (!PluginUtilsNApi::HasNamedProperty(env_, object, JS_REGISTER_METHOD_NAME) ||
        !PluginUtilsNApi::HasNamedProperty(env_, object, JS_REGISTER_METHOD_FUNCTION)) {
        LOGE("GetJSRegisterMethodObject: Parameter error.");
        return false;
    }

    napi_value jsMethodName = PluginUtilsNApi::GetNamedProperty(env_, object, JS_REGISTER_METHOD_NAME);
    napi_value jsMethodFun = PluginUtilsNApi::GetNamedProperty(env_, object, JS_REGISTER_METHOD_FUNCTION);
    if (jsMethodName == nullptr || jsMethodFun == nullptr) {
        LOGE("GetJSRegisterMethodObject: Analytic parameter error.");
        return false;
    }

    methodName_ = PluginUtilsNApi::GetStringFromValueUtf8(env_, jsMethodName);
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

bool MethodData::GetJSRegisterMethodObjectCallBack(const std::string& arg, napi_value object)
{
    methodName_ = arg;
    if (methodName_.empty()) {
        LOGE("Bridge GetJSRegisterMethodObjectCallBack: methodName_ is empty.");
        return false;
    }
    if (!CreateEvent(object, true)) {
        LOGE("Bridge GetJSRegisterMethodObjectCallBack: Failed to create an event.");
        return false;
    }

    bool ret = asyncEvent_->CreateCallback(object);
    if (!ret) {
        LOGE("Bridge GetJSRegisterMethodObjectCallBack: Failed to create the JS callback.");
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
    std::string methodName = MethodID::FetchMethodName(methodName_);
    return asyncEvent_->CreateAsyncWork(
        methodName, [](napi_env env, void* data) {}, jsCallback);
}

bool MethodData::SendMethodResultBinary(int errorCode, const std::string& errorMessage,
    std::unique_ptr<Ace::Platform::BufferMapping> data, bool removeMethod)
{
    if (asyncEvent_ == nullptr) {
        LOGE("SendMethodResultBinary: asyncEvent_ is null.");
        return false;
    }

    MethodResult resultValue;
    resultValue.ParsePlatformMethodResultBinary(env_, errorCode, errorMessage, std::move(data));
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
    std::string methodName = MethodID::FetchMethodName(methodName_);
    return asyncEvent_->CreateAsyncWork(
        methodName, [](napi_env env, void* data) {}, jsCallback);
}

bool MethodData::SendMessageResponse(const std::string& data, bool removeMethod)
{
    if (asyncEvent_ == nullptr) {
        LOGE("SendMessageResponse: asyncEvent_ is null.");
        return false;
    }

    DecodeValue decodeValue { .env = env_, .value = data };
    auto decoded = BridgeJsonCodec::GetInstance().Decode(decodeValue);
    MethodResult resultValue;
    resultValue.SetOkResult(decoded->value);
    resultValue.SetErrorCode(decoded->errorCode);
    resultValue.CreateErrorObject(env_);
    asyncEvent_->SetErrorCode(decoded->errorCode);
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

    return asyncEvent_->CreateAsyncWork(
        methodName_, [](napi_env env, void* data) {}, jsCallback);
}

void MethodData::PlatformCallMethod(const std::string& parameter)
{
    if (asyncEvent_ == nullptr) {
        LOGE("PlatformCallMethod: asyncEvent_ is null.");
        MethodResult result;
        result.SetErrorCode(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
        result.SetMethodName(methodName_);
        result.ParseJSMethodResult(env_, nullptr);

        auto task = [platformResult = result.GetResult(), bridgeName = bridgeName_, methodName = methodName_]() {
            Ace::Platform::BridgeManager::JSSendMethodResult(bridgeName, methodName, platformResult);
        };
        if (!taskExecutor_) {
            LOGE("PlatformCallMethod(error path): taskExecutor_ is null.");
        } else {
            taskExecutor_->RunTaskOnBridgeThread(task);
        }
        return;
    }

    if (asyncEvent_ == nullptr) {
        LOGE("PlatformCallMethod(Posted Task): asyncEvent_ became null.");
        return;
    }
    asyncEvent_->SetMethodParameter(parameter);
    asyncEvent_->SetBridgeName(bridgeName_);
    AsyncWorkComplete jsCallback = [](napi_env env, napi_status status, void* data) {
        NAPIAsyncEvent* event = static_cast<NAPIAsyncEvent*>(data);
        event->AsyncWorkCallMethod();
    };
    bool ret = asyncEvent_->CreateAsyncWork(methodName_, [](napi_env env, void* data) {}, jsCallback);
    if (!ret) {
        LOGE("PlatformCallMethod: CreateAsyncWork failed for method %{public}s", methodName_.c_str());
    }
}

void MethodData::PlatformCallMethodBinary(std::unique_ptr<Ace::Platform::BufferMapping> data)
{
    if (asyncEvent_ == nullptr) {
        LOGE("PlatformCallMethod: asyncEvent_ is null.");
        MethodResult result;
        result.SetErrorCode(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
        result.SetMethodName(methodName_);

        auto task = [errorCode = result.GetErrorCode(), errorMessage = result.GetErrorMessage(),
                        bridgeName = bridgeName_, methodName = methodName_]() {
            Ace::Platform::BridgeManager::JSSendMethodResultBinary(
                bridgeName, methodName, errorCode, errorMessage, nullptr);
        };
        if (!taskExecutor_) {
            LOGE("PlatformCallMethodBinary failed taskExecutor_ is null.");
        } else {
            taskExecutor_->RunTaskOnBridgeThread(task);
        }
        return;
    }

    asyncEvent_->SetBridgeName(bridgeName_);

    size_t bufferSize = data->GetSize();
    uint8_t* buffer = data->Release();
    asyncEvent_->SetMethodParameter(buffer, bufferSize);

    AsyncWorkComplete jsCallback = [](napi_env env, napi_status status, void* data) {
        size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
        napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };

        NAPIAsyncEvent* event = static_cast<NAPIAsyncEvent*>(data);
        auto dataBuffer = event->GetMethodParameter();

        Ace::Platform::BufferMapping mapping(std::get<uint8_t*>(dataBuffer), std::get<size_t>(dataBuffer));
        auto decoded = BridgeBinaryCodec::GetInstance().DecodeBuffer(mapping.GetMapping(), mapping.GetSize());
        MethodDataConverter::ConvertToNapiValues(env, *decoded, argc, argv);

        event->AsyncWorkCallMethod(argc, argv);
    };

    asyncEvent_->CreateAsyncWork(
        methodName_, [](napi_env env, void* data) {}, jsCallback);
}

void MethodData::PlatformSendMessage(const std::string& data)
{
    if (asyncEvent_ == nullptr) {
        LOGE("PlatformSendMessage: asyncEvent_ is null.");
        return;
    }

    DecodeValue decodeValue { .env = env_, .value = data };
    auto decoded = BridgeJsonCodec::GetInstance().Decode(decodeValue);

    MethodResult resultValue;
    resultValue.SetErrorCode(decoded->errorCode);
    resultValue.CreateErrorObject(env_);
    asyncEvent_->SetErrorCode(resultValue.GetErrorCode());
    asyncEvent_->SetRefErrorData(resultValue.GetErrorResult());
    asyncEvent_->SetRefData(decoded->value);
    asyncEvent_->SetBridgeName(bridgeName_);

    asyncEvent_->AsyncWorkMessage();
}

void MethodData::PlatformSendMessageBinary(std::unique_ptr<Ace::Platform::BufferMapping> data)
{
    if (asyncEvent_ == nullptr) {
        LOGE("PlatformSendMessageBinary: asyncEvent_ is null.");
        return;
    }

    auto decoded = BridgeBinaryCodec::GetInstance().DecodeBuffer(data->GetMapping(), data->GetSize());
    napi_value binaryResult = MethodDataConverter::ConvertToNapiValue(env_, *decoded);
    asyncEvent_->SetRefData(binaryResult);
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

    bool ret = asyncEvent_->CreateAsyncWork(
        methodName_, [](napi_env env, void* data) {}, jsCallback);
    if (!ret) {
        LOGE("SendAsyncCallback: Failed to call the JS callback.");
        ReleaseEvent();
    }
}

void MethodData::UpdateMethodName(void)
{
    if (!methodName_.empty()) {
        methodName_ = MethodID::MakeMethodNameID(methodName_);
    }
}

MethodResult MethodData::PlatformCallMethodSync(const std::string& parameter)
{
    LOGD("MethodData::PlatformCallMethodSync called for method '%{public}s'", methodName_.c_str());
    MethodResult result;
    result.SetMethodName(methodName_);

    if (asyncEvent_ == nullptr || !asyncEvent_->IsCallback()) {
        LOGE("PlatformCallMethodSync: No callback registered for method '%{public}s'", methodName_.c_str());
        result.SetErrorCode(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
        result.CreateMethodResultForError();
        return result;
    }

    napi_value methodResultValue = asyncEvent_->AsyncWorkCallMethodSync(parameter);
    if (methodResultValue != nullptr) {
        result.ParseJSMethodResult(env_, methodResultValue);
        LOGD("PlatformCallMethodSync: Successfully called method '%{public}s'", methodName_.c_str());
    } else {
        LOGE("PlatformCallMethodSync: JavaScript method returned null for method '%{public}s'", methodName_.c_str());
        result.SetErrorCode(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
        result.CreateMethodResultForError();
    }

    return result;
}

MethodResult MethodData::PlatformCallMethodSyncBinary(std::unique_ptr<Ace::Platform::BufferMapping> parameter)
{
    LOGD("MethodData::PlatformCallMethodSyncBinary called for method '%{public}s'", methodName_.c_str());
    MethodResult result;
    result.SetMethodName(methodName_);

    if (codecType_ != CodecType::BINARY_CODEC) {
        LOGE("PlatformCallMethodSyncBinary: codec type not binary");
        result.SetErrorCode(static_cast<int>(ErrorCode::BRIDGE_CODEC_TYPE_MISMATCH));
        result.CreateMethodResultForError();
        return result;
    }

    if (asyncEvent_ == nullptr || !asyncEvent_->IsCallback()) {
        LOGE("PlatformCallMethodSyncBinary: No callback registered for method '%{public}s'", methodName_.c_str());
        result.SetErrorCode(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
        result.CreateMethodResultForError();
        return result;
    }

    uint8_t* dataPtr = nullptr;
    size_t dataSize = 0;
    if (parameter) {
        dataPtr = const_cast<uint8_t*>(parameter->GetMapping());
        dataSize = parameter->GetSize();
    }
    napi_value methodResultValue = asyncEvent_->AsyncWorkCallMethodSyncBinary(dataPtr, dataSize);
    if (methodResultValue != nullptr) {
        result.ParseJSMethodResultBinary(env_, methodResultValue);
    } else {
        LOGE("PlatformCallMethodSyncBinary: JS returned null");
        result.SetErrorCode(static_cast<int>(ErrorCode::BRIDGE_METHOD_UNIMPL));
        result.CreateMethodResultForError();
    }
    return result;
}
} // namespace OHOS::Plugin::Bridge