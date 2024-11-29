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

#include "bridge_module.h"

#include <cstddef>

#include "log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_async_event.h"
#include "napi_utils.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_inner.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_napi.h"
#include "plugins/interfaces/native/plugin_utils.h"

namespace OHOS::Plugin::Bridge {
static constexpr const char* BRIDGE_MODULE_NAME = "bridge";
static constexpr const char* INTERFACE_BRIDGE_TYPE_ENUM = "BridgeType";
static constexpr const int32_t BRIDGE_TYPE_JSON = 0;
static constexpr const int32_t BRIDGE_TYPE_BINARY = 1;
static constexpr const char* BRIDGE_TYPE_ENUM_JSON_NAME = "JSON_TYPE";
static constexpr const char* BRIDGE_TYPE_ENUM_BINARY_NAME = "BINARY_TYPE";
napi_value BridgeModule::InitBridgeModule(napi_env env, napi_value exports)
{
    DefinePluginBridgeObjectClass(env, exports);
    InitBridgeProperties(env, exports);

    return exports;
}

napi_value BridgeModule::CreateBridge(napi_env env, napi_callback_info info)
{
    size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
    napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    if ((argc != PluginUtilsNApi::ARG_NUM_1 && argc != PluginUtilsNApi::ARG_NUM_2) 
        || PluginUtilsNApi::GetValueType(env, argv[PluginUtilsNApi::ARG_NUM_0]) != napi_string) {
        return PluginUtilsNApi::CreateUndefined(env);
    }

    std::string bridgeName = PluginUtilsNApi::GetStringFromValueUtf8(env, argv[PluginUtilsNApi::ARG_NUM_0]);
    if (bridgeName.empty()) {
        return PluginUtilsNApi::CreateUndefined(env);
    }
    napi_ref bridgeNameRef = PluginUtilsNApi::CreateReference(env, argv[PluginUtilsNApi::ARG_NUM_0]);
    napi_value bridgeNameValue = PluginUtilsNApi::GetReference(env, bridgeNameRef);

    napi_value thisVar =
        PluginUtilsNApi::NewInstance(env, info, INTERFACE_PLUGIN_BRIDGE_OBJECT, 1, &bridgeNameValue);
        PluginUtilsNApi::DeleteReference(env, bridgeNameRef);
    if (thisVar == nullptr) {
        return PluginUtilsNApi::CreateUndefined(env);
    }

    CodecType codecType = CodecType::JSON_CODEC;
    if (argc == PluginUtilsNApi::ARG_NUM_2) {
        codecType = static_cast<CodecType>(PluginUtilsNApi::GetCInt32(argv[PluginUtilsNApi::ARG_NUM_1], env));
    }
    LOGI("bridge codec type : %d", static_cast<int32_t>(codecType));

    auto bridge = BridgeWrap::GetInstance().CreateBridge(bridgeName, codecType);
    if (bridge == nullptr) {
        return PluginUtilsNApi::CreateUndefined(env);
    }

    napi_wrap(
        env, thisVar, reinterpret_cast<void*>(bridge),
        [](napi_env env, void* data, void* argv) {
            auto bridge = reinterpret_cast<Bridge*>(data);
            bool isTerminate = bridge->GetTerminate();
            if (!isTerminate) {
                LOGI("Delete bridge object");
                BridgeWrap::GetInstance().DeleteBridge(bridge->GetBridgeName(), bridge->GetInstanceID());
            }
        },
        nullptr, nullptr);
    return thisVar;
}

void BridgeModule::DefinePluginBridgeObjectClass(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_GETTER_SETTER(BridgeObject::PROPERTY_BRIDGE_NAME, BridgeObject::GetBridgeName, nullptr),
        DECLARE_NAPI_FUNCTION(BridgeObject::FUNCTION_CALL_METHOD, BridgeObject::CallMethod),
        DECLARE_NAPI_FUNCTION(BridgeObject::FUNCTION_REGISTER_METHOD, BridgeObject::RegisterMethod),
        DECLARE_NAPI_FUNCTION(BridgeObject::FUNCTION_UNREGISTER_METHOD, BridgeObject::UnRegisterMethod),
        DECLARE_NAPI_FUNCTION(BridgeObject::FUNCTION_SEND_MESSAGE, BridgeObject::SendMessage),
        DECLARE_NAPI_FUNCTION(BridgeObject::FUNCTION_REGISTER_ON_MESSAGE, BridgeObject::SetMessageListener),
        DECLARE_NAPI_FUNCTION(BridgeObject::FUNCTION_CALL_METHOD_CALLBACK, BridgeObject::CallMethodWithCallBack),
    };
    PluginUtilsNApi::DefineClass(env, exports, properties, INTERFACE_PLUGIN_BRIDGE_OBJECT);
}

void BridgeModule::InitBridgeProperties(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_PROPERTY(INTERFACE_BRIDGE_TYPE_ENUM, InitCodecType(env)),
        DECLARE_NAPI_FUNCTION(FUNCTION_CREATE_PLUGIN_BRIDGE, CreateBridge),
    };
    PluginUtilsNApi::DefineProperties(env, exports, properties);
}

napi_value BridgeModule::InitCodecType(napi_env env)
{
    napi_value object = PluginUtilsNApi::CreateObject(env);

    PluginUtilsNApi::SetEnumItem(env, object, BRIDGE_TYPE_ENUM_JSON_NAME, BRIDGE_TYPE_JSON);
    PluginUtilsNApi::SetEnumItem(env, object, BRIDGE_TYPE_ENUM_BINARY_NAME, BRIDGE_TYPE_BINARY);

    return object;
}

Bridge* BridgeModule::GetBridge(napi_env env, napi_value thisVal)
{
    Bridge* bridge = nullptr;
    napi_unwrap(env, thisVal, reinterpret_cast<void**>(&bridge));
    return bridge;
}

void BridgeModule::CallMethodInner(napi_env env, napi_value thisVal, std::shared_ptr<MethodData> methodData)
{
    ErrorCode code { ErrorCode::BRIDGE_ERROR_NO };
    Bridge* bridge = GetBridge(env, thisVal);
    if (bridge != nullptr) {
        methodData->SetBridgeName(bridge->GetBridgeName());
        code = bridge->CallMethod(methodData->GetMethodName(), methodData);
        LOGI("CallMethodInner:success, BridgeName is %{public}s, MethodName is %{public}s,",
        bridge->GetBridgeName().c_str(), methodData->GetMethodName().c_str());
    } else {
        code = ErrorCode::BRIDGE_INVALID;
        LOGE("CallMethodInner:Failed to obtain the Bridge object.");
    }

    if (code != ErrorCode::BRIDGE_ERROR_NO) {
        MethodResult methodResult;
        methodResult.SetErrorCode(static_cast<int>(code));
        methodResult.CreateDefaultJsonString();
        methodData->SendMethodResult(methodResult.GetResult(), true);
        if (bridge) {
            bridge->RemoveJSMethodData(methodData->GetMethodName());
        }
    }
}

void BridgeModule::RegisterMethodInner(
    napi_env env, napi_value thisVal, std::shared_ptr<MethodData> methodData, std::shared_ptr<MethodData> callback)
{
    ErrorCode code { ErrorCode::BRIDGE_ERROR_NO };
    Bridge* bridge = GetBridge(env, thisVal);
    if (bridge != nullptr) {
        methodData->SetBridgeName(bridge->GetBridgeName());
        code = bridge->RegisterMethod(methodData->GetMethodName(), methodData);
    } else {
        code = ErrorCode::BRIDGE_INVALID;
        LOGE("RegisterMethodInner:Failed to obtain the Bridge object.");
    }
    callback->SetMethodName(methodData->GetMethodName());
    callback->SendAsyncCallback(static_cast<int>(code), PluginUtilsNApi::CreateUndefined(env));
}

void BridgeModule::UnRegisterMethodInner(napi_env env, napi_value thisVal, std::shared_ptr<MethodData> callback)
{
    ErrorCode code { ErrorCode::BRIDGE_ERROR_NO };
    Bridge* bridge = GetBridge(env, thisVal);
    if (bridge != nullptr) {
        callback->SetBridgeName(bridge->GetBridgeName());
        code = bridge->UnRegisterMethod(callback->GetMethodName());
    } else {
        code = ErrorCode::BRIDGE_INVALID;
        LOGE("UnRegisterMethodInner:Failed to obtain the Bridge object.");
    }
    callback->SendAsyncCallback(static_cast<int>(code), PluginUtilsNApi::CreateUndefined(env));
}

void BridgeModule::SendMessageInner(napi_env env, napi_value thisVal, std::shared_ptr<MethodData> callback)
{
    ErrorCode code { ErrorCode::BRIDGE_ERROR_NO };
    Bridge* bridge = GetBridge(env, thisVal);
    if (bridge != nullptr) {
        callback->SetBridgeName(bridge->GetBridgeName());
        if (bridge->GetCodecType() == CodecType::JSON_CODEC) {
            code = bridge->SendMessage(callback->GetMethodParamName(), callback); 
        } else if (bridge->GetCodecType() == CodecType::BINARY_CODEC) {
            const auto& params = callback->GetMethodParamNameBinary();
            code = bridge->SendMessageBinary(params, callback);
        }
    } else {
        code = ErrorCode::BRIDGE_INVALID;
        LOGE("SendMessageInner:Failed to obtain the Bridge object.");
    }

    if (code != ErrorCode::BRIDGE_ERROR_NO) {
        callback->SendAsyncCallback(static_cast<int>(code), PluginUtilsNApi::CreateUndefined(env));
        if (bridge) {
            bridge->RemoveMessageData();
        }
    }
}

void BridgeModule::SetMessageListenerInner(
    napi_env env, napi_value thisVal, std::shared_ptr<MethodData> onMessageCallback)
{
    Bridge* bridge = GetBridge(env, thisVal);
    if (bridge != nullptr) {
        onMessageCallback->SetBridgeName(bridge->GetBridgeName());
        bridge->SetMessageListener(onMessageCallback);
    } else {
        LOGE("SetMessageListenerInner:Failed to obtain the Bridge object.");
    }
}

napi_value BridgeModule::BridgeObject::GetBridgeName(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
    napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));

    Bridge* bridge = GetBridge(env, thisVal);
    if (bridge) {
        return PluginUtilsNApi::CreateStringUtf8(env, bridge->GetBridgeName());
    } else {
        LOGE("GetBridgeName: Failed to obtain the Bridge object.");
    }
    return PluginUtilsNApi::CreateUndefined(env);
}

napi_value BridgeModule::BridgeObject::CallMethod(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
    napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));

    if (argc == PluginUtilsNApi::ARG_NUM_0 || argc > PluginUtilsNApi::MAX_ARG_NUM) {
        LOGE("BridgeObject::CallMethod: Method parameter error.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    Bridge* bridge = GetBridge(env, thisVal);
    if (bridge == nullptr) {
        LOGE("BridgeObject::CallMethod: Failed to obtain the Bridge object.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    auto methodData = MethodData::CreateMethodData(env, bridge->GetCodecType());
    if (!methodData->GetName(argv[PluginUtilsNApi::ARG_NUM_0])) {
        LOGE("BridgeObject::CallMethod: Parsing the method name failed.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    size_t paramCount =
        (argc > PluginUtilsNApi::ARG_NUM_1) ? argc - PluginUtilsNApi::ARG_NUM_1 : PluginUtilsNApi::ARG_NUM_0;
    bool ret = false;
    if (paramCount > PluginUtilsNApi::ARG_NUM_0) {
        ret = methodData->GetParamsByRecord(paramCount, &argv[PluginUtilsNApi::ARG_NUM_1]);
    } else {
        ret = methodData->GetParamsByRecord(PluginUtilsNApi::ARG_NUM_0, nullptr);
    }

    if (!ret) {
        LOGE("BridgeObject::CallMethod: Parsing the method parameters failed.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    napi_value result = PluginUtilsNApi::CreateUndefined(env);
    if (!methodData->IsCallback()) {
        result = methodData->GetPromise(true);
    }

    methodData->UpdateMethodName();
    CallMethodInner(env, thisVal, methodData);
    return result;
}

napi_value BridgeModule::BridgeObject::RegisterMethod(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
    napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));

    if (argc == PluginUtilsNApi::ARG_NUM_0 || argc > PluginUtilsNApi::ARG_NUM_2) {
        LOGE("BridgeObject::RegisterMethod: Method parameter error.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    Bridge* bridge = GetBridge(env, thisVal);
    if (bridge == nullptr) {
        LOGE("BridgeObject:::RegisterMethod Failed to obtain the Bridge object.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    auto methodData = MethodData::CreateMethodData(env, bridge->GetCodecType());
    if (argc > PluginUtilsNApi::ARG_NUM_0 &&
        !methodData->GetJSRegisterMethodObject(argv[PluginUtilsNApi::ARG_NUM_0])) {
        LOGE("BridgeObject::RegisterMethod: Failed to create the callback event.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    auto callback = MethodData::CreateMethodData(env, bridge->GetCodecType());
    if (argc > PluginUtilsNApi::ARG_NUM_1 &&
        !callback->GetCallback(argv[PluginUtilsNApi::ARG_NUM_1], false)) {
        methodData->ReleaseEvent();
        LOGE("BridgeObject::RegisterMethod: Failed to create the callback event.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    napi_value result = PluginUtilsNApi::CreateUndefined(env);
    if (!callback->IsCallback()) {
        result = callback->GetPromise(false);
    }

    RegisterMethodInner(env, thisVal, methodData, callback);
    return result;
}

napi_value BridgeModule::BridgeObject::UnRegisterMethod(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
    napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));

    if (argc == PluginUtilsNApi::ARG_NUM_0 || argc > PluginUtilsNApi::ARG_NUM_2) {
        LOGE("BridgeObject::UnRegisterMethod: Method parameter error.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    Bridge* bridge = GetBridge(env, thisVal);
    if (bridge == nullptr) {
        LOGE("BridgeObject::UnRegisterMethod: Failed to obtain the Bridge object.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    auto callback = MethodData::CreateMethodData(env, bridge->GetCodecType());
    if (argc > PluginUtilsNApi::ARG_NUM_0 &&
        !callback->GetName(argv[PluginUtilsNApi::ARG_NUM_0])) {
        LOGE("BridgeObject::UnRegisterMethod: Method name error.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    if (argc > PluginUtilsNApi::ARG_NUM_1 &&
        !callback->GetCallback(argv[PluginUtilsNApi::ARG_NUM_1], false)) {
        LOGE("BridgeObject::UnRegisterMethod: Failed to create the callback event.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    napi_value result = PluginUtilsNApi::CreateUndefined(env);
    if (!callback->IsCallback()) {
        result = callback->GetPromise(false);
    }

    UnRegisterMethodInner(env, thisVal, callback);
    return result;
}

napi_value BridgeModule::BridgeObject::SendMessage(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
    napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));

    if (argc == PluginUtilsNApi::ARG_NUM_0 || argc > PluginUtilsNApi::ARG_NUM_2) {
        LOGE("BridgeObject::SendMessage: Method parameter error.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    Bridge* bridge = GetBridge(env, thisVal);
    if (bridge == nullptr) {
        LOGE("BridgeObject::SendMessage: Failed to obtain the Bridge object.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    auto callback = MethodData::CreateMethodData(env, bridge->GetCodecType());
    if (argc > PluginUtilsNApi::ARG_NUM_0 &&
        !callback->GetMessageData(argv[PluginUtilsNApi::ARG_NUM_0])) {
        LOGE("BridgeObject::SendMessage: Failed to create the callback event.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    if (argc == PluginUtilsNApi::ARG_NUM_2 &&
        !callback->GetCallback(argv[PluginUtilsNApi::ARG_NUM_1], false)) {
        LOGE("BridgeObject::SendMessage: Failed to create the callback event.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    napi_value result = PluginUtilsNApi::CreateUndefined(env);
    if (!callback->IsCallback()) {
        result = callback->GetPromise(false);
    }

    SendMessageInner(env, thisVal, callback);
    return result;
}

napi_value BridgeModule::BridgeObject::SetMessageListener(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
    napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));

    if (argc != PluginUtilsNApi::ARG_NUM_1) {
        LOGE("BridgeObject::SetMessageListener: Method parameter error.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    Bridge* bridge = GetBridge(env, thisVal);
    if (bridge == nullptr) {
        LOGE("BridgeObject::SendMessage: Failed to obtain the Bridge object.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    auto onMessageCallback = MethodData::CreateMethodData(env, bridge->GetCodecType());
    onMessageCallback->SetIsMessageEvent(true);

    if (!onMessageCallback->GetCallback(argv[PluginUtilsNApi::ARG_NUM_0], true)) {
        LOGE("BridgeObject::SetMessageListener: Failed to create the listener method.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    SetMessageListenerInner(env, thisVal, onMessageCallback);
    return PluginUtilsNApi::CreateUndefined(env);
}

napi_value BridgeModule::CallMethodWithCallBackInner(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
    napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));

    if (argc < PluginUtilsNApi::ARG_NUM_2 || argc > PluginUtilsNApi::MAX_ARG_NUM) {
        LOGE("BridgeObject::CallMethodWithCallBackInner: Method parameter error.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    Bridge* bridge = GetBridge(env, thisVal);
    if (bridge == nullptr) {
        LOGE("BridgeObject::CallMethodWithCallBackInner: Failed to obtain the Bridge object.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    auto methodData = MethodData::CreateMethodData(env, bridge->GetCodecType());
    if (!methodData->GetName(argv[PluginUtilsNApi::ARG_NUM_0])) {
        LOGE("BridgeObject::CallMethodWithCallBackInner: Parsing the method name failed.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    size_t paramCount =
        (argc > PluginUtilsNApi::ARG_NUM_2) ? argc - PluginUtilsNApi::ARG_NUM_2 : PluginUtilsNApi::ARG_NUM_0;
    bool ret = false;
    if (paramCount > PluginUtilsNApi::ARG_NUM_0) {
        ret = methodData->GetParamsByRecord(paramCount, &argv[PluginUtilsNApi::ARG_NUM_2]);
    } else {
        ret = methodData->GetParamsByRecord(PluginUtilsNApi::ARG_NUM_0, nullptr);
    }
    if (!ret) {
        LOGE("BridgeObject::CallMethodWithCallBackInner: Parsing the method parameters failed.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    napi_value result = PluginUtilsNApi::CreateUndefined(env);
    if (!methodData->IsCallback()) {
        result = methodData->GetPromise(true);
    }

    methodData->UpdateMethodName();
    CallMethodInner(env, thisVal, methodData);
    return result;
}

napi_value BridgeModule::BridgeObject::CallMethodWithCallBack(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    size_t argc = PluginUtilsNApi::MAX_ARG_NUM;
    napi_value argv[PluginUtilsNApi::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));

    if (argc < PluginUtilsNApi::ARG_NUM_2 || argc > PluginUtilsNApi::MAX_ARG_NUM) {
        LOGE("BridgeObject::CallMethodWithCallBack: Method parameter error.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    Bridge* bridge = GetBridge(env, thisVal);
    if (bridge == nullptr) {
        LOGE("BridgeObject::CallMethodWithCallBack: Failed to obtain the Bridge object.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    auto methodData = MethodData::CreateMethodData(env, bridge->GetCodecType());
    if (!methodData->GetName(argv[PluginUtilsNApi::ARG_NUM_0])) {
        LOGE("BridgeObject::CallMethodWithCallBack: Parsing the method name failed.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    auto methodName = PluginUtilsNApi::GetStringFromValueUtf8(env, argv[PluginUtilsNApi::ARG_NUM_0]);
    if (!methodData->GetJSRegisterMethodObjectCallBack(methodName, argv[PluginUtilsNApi::ARG_NUM_1])) {
        LOGE("BridgeObject::CallMethodWithCallBack: Failed to create the callback event.");
        return PluginUtilsNApi::CreateUndefined(env);
    }

    auto callback = MethodData::CreateMethodData(env, bridge->GetCodecType());
    callback->GetPromise(false);

    RegisterMethodInner(env, thisVal, methodData, callback);
    auto result = CallMethodWithCallBackInner(env, info);
    return result;
}

static napi_module BridgeModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = BridgeModule::InitBridgeModule,
    .nm_modname = BRIDGE_MODULE_NAME,
    .nm_priv = nullptr,
    .reserved = { nullptr },
};

extern "C" __attribute__((constructor)) void RegisterBridgeModule(void)
{
    napi_module_register(&BridgeModule);
}
} // namespace OHOS::Plugin::Bridge
