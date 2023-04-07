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
#include "plugins/interfaces/native/inner_utils/plugin_inner_napi_utils.h"
#include "plugins/interfaces/native/plugin_c_utils.h"
#include "plugins/interfaces/native/plugin_utils.h"

namespace OHOS::Plugin::Bridge {
static constexpr const char* BRIDGE_MODULE_NAME = "bridge";

napi_value BridgeModule::InitBridgeModule(napi_env env, napi_value exports)
{
    DefinePluginBridgeObjectClass(env, exports);
    InitBridgeProperties(env, exports);

    return exports;
}

napi_value BridgeModule::CreateBridge(napi_env env, napi_callback_info info)
{
    size_t argc = PluginInnerNApiUtils::MAX_ARG_NUM;
    napi_value argv[PluginInnerNApiUtils::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr));

    if (argc != PluginInnerNApiUtils::ARG_NUM_1 ||
        PluginInnerNApiUtils::GetValueType(env, argv[PluginInnerNApiUtils::ARG_NUM_0]) != napi_string) {
        return PluginInnerNApiUtils::CreateUndefined(env);
    }

    std::string bridgeName = PluginInnerNApiUtils::GetStringFromValueUtf8(env, argv[PluginInnerNApiUtils::ARG_NUM_0]);
    if (bridgeName.empty()) {
        return PluginInnerNApiUtils::CreateUndefined(env);
    }
    napi_ref bridgeNameRef = PluginInnerNApiUtils::CreateReference(env, argv[PluginInnerNApiUtils::ARG_NUM_0]);
    napi_value bridgeNameValue = PluginInnerNApiUtils::GetReference(env, bridgeNameRef);

    napi_value thisVar =
        PluginInnerNApiUtils::NewInstance(env, info, INTERFACE_PLUGIN_BRIDGE_OBJECT, 1, &bridgeNameValue);
    if (thisVar == nullptr) {
        return PluginInnerNApiUtils::CreateUndefined(env);
    }

    auto bridge = BridgeWrap::CreateBridge(bridgeName);
    if (bridge == nullptr) {
        return PluginInnerNApiUtils::CreateUndefined(env);
    }
    napi_wrap(
        env, thisVar, reinterpret_cast<void*>(bridge),
        [](napi_env env, void* data, void* argv) {
            auto bridge = reinterpret_cast<Bridge*>(data);
            BridgeWrap::DeleteBridge(bridge->GetBridgeName());
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
    };
    PluginInnerNApiUtils::DefineClass(env, exports, properties, INTERFACE_PLUGIN_BRIDGE_OBJECT);
}

void BridgeModule::InitBridgeProperties(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_FUNCTION(FUNCTION_CREATE_PLUGIN_BRIDGE, CreateBridge),
    };
    PluginInnerNApiUtils::DefineProperties(env, exports, properties);
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
    callback->SendAsyncCallback(static_cast<int>(code), PluginInnerNApiUtils::CreateUndefined(env));
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
    callback->SendAsyncCallback(static_cast<int>(code), PluginInnerNApiUtils::CreateUndefined(env));
}

void BridgeModule::SendMessageInner(napi_env env, napi_value thisVal, std::shared_ptr<MethodData> callback)
{
    ErrorCode code { ErrorCode::BRIDGE_ERROR_NO };
    Bridge* bridge = GetBridge(env, thisVal);
    if (bridge != nullptr) {
        callback->SetBridgeName(bridge->GetBridgeName());
        code = bridge->SendMessage(callback->GetMethodParamName(), callback);
    } else {
        code = ErrorCode::BRIDGE_INVALID;
        LOGE("SendMessageInner:Failed to obtain the Bridge object.");
    }

    if (code != ErrorCode::BRIDGE_ERROR_NO) {
        callback->SendAsyncCallback(static_cast<int>(code), PluginInnerNApiUtils::CreateUndefined(env));
        if (bridge) {
            bridge->RemoveMessageData();
        }
    }
}

void BridgeModule::SetMessageListenerInner(
    napi_env env, napi_value thisVal, std::shared_ptr<MethodData> onMessagecallback)
{
    Bridge* bridge = GetBridge(env, thisVal);
    if (bridge != nullptr) {
        onMessagecallback->SetBridgeName(bridge->GetBridgeName());
        bridge->SetMessageListener(onMessagecallback);
    } else {
        LOGE("SetMessageListenerInner:Failed to obtain the Bridge object.");
    }
}

napi_value BridgeModule::BridgeObject::GetBridgeName(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    size_t argc = PluginInnerNApiUtils::MAX_ARG_NUM;
    napi_value argv[PluginInnerNApiUtils::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));

    Bridge* bridge = GetBridge(env, thisVal);
    if (bridge) {
        return PluginInnerNApiUtils::CreateStringUtf8(env, bridge->GetBridgeName());
    } else {
        LOGE("GetBridgeName: Failed to obtain the Bridge object.");
    }
    return PluginInnerNApiUtils::CreateUndefined(env);
}

napi_value BridgeModule::BridgeObject::CallMethod(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    size_t argc = PluginInnerNApiUtils::MAX_ARG_NUM;
    napi_value argv[PluginInnerNApiUtils::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));

    if (argc == PluginInnerNApiUtils::ARG_NUM_0 || argc > PluginInnerNApiUtils::MAX_ARG_NUM) {
        LOGE("BridgeObject::CallMethod: Method parameter error.");
        return PluginInnerNApiUtils::CreateUndefined(env);
    }

    std::shared_ptr<MethodData> methodData = std::make_shared<MethodData>(env);
    if (!methodData->GetName(argv[PluginInnerNApiUtils::ARG_NUM_0])) {
        LOGE("BridgeObject::CallMethod: Parsing the method name failed.");
        return PluginInnerNApiUtils::CreateUndefined(env);
    }

    size_t paramCount = (argc > PluginInnerNApiUtils::ARG_NUM_1) ? argc - 1 : 0;
    bool ret = false;
    if (paramCount > 0) {
        ret = methodData->GetParamsByRecord(paramCount, &argv[1]);
    } else {
        ret = methodData->GetParamsByRecord(0, nullptr);
    }

    if (!ret) {
        LOGE("BridgeObject::CallMethod: Parsing the method parameters failed.");
        return PluginInnerNApiUtils::CreateUndefined(env);
    }

    napi_value result = PluginInnerNApiUtils::CreateUndefined(env);
    if (!methodData->IsCallback()) {
        result = methodData->GetPromise(true);
    }

    CallMethodInner(env, thisVal, methodData);
    return result;
}

napi_value BridgeModule::BridgeObject::RegisterMethod(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    size_t argc = PluginInnerNApiUtils::MAX_ARG_NUM;
    napi_value argv[PluginInnerNApiUtils::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));

    if (argc == PluginInnerNApiUtils::ARG_NUM_0 || argc > PluginInnerNApiUtils::ARG_NUM_2) {
        LOGE("BridgeObject::RegisterMethod: Method parameter error.");
        return PluginInnerNApiUtils::CreateUndefined(env);
    }

    std::shared_ptr<MethodData> methodData = std::make_shared<MethodData>(env);
    if (argc > PluginInnerNApiUtils::ARG_NUM_0 &&
        !methodData->GetJSRegisterMethodObject(argv[PluginInnerNApiUtils::ARG_NUM_0])) {
        LOGE("BridgeObject::RegisterMethod: Failed to create the callback event.");
        return PluginInnerNApiUtils::CreateUndefined(env);
    }

    std::shared_ptr<MethodData> callback = std::make_shared<MethodData>(env);
    if (argc > PluginInnerNApiUtils::ARG_NUM_1 &&
        !callback->GetCallback(argv[PluginInnerNApiUtils::ARG_NUM_1], false)) {
        methodData->ReleaseEvent();
        LOGE("BridgeObject::RegisterMethod: Failed to create the callback event.");
        return PluginInnerNApiUtils::CreateUndefined(env);
    }

    napi_value result = PluginInnerNApiUtils::CreateUndefined(env);
    if (!callback->IsCallback()) {
        result = callback->GetPromise(false);
    }

    RegisterMethodInner(env, thisVal, methodData, callback);
    return result;
}

napi_value BridgeModule::BridgeObject::UnRegisterMethod(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    size_t argc = PluginInnerNApiUtils::MAX_ARG_NUM;
    napi_value argv[PluginInnerNApiUtils::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));

    if (argc == PluginInnerNApiUtils::ARG_NUM_0 || argc > PluginInnerNApiUtils::ARG_NUM_2) {
        LOGE("BridgeObject::UnRegisterMethod: Method parameter error.");
        return PluginInnerNApiUtils::CreateUndefined(env);
    }

    std::shared_ptr<MethodData> callback = std::make_shared<MethodData>(env);
    if (argc > PluginInnerNApiUtils::ARG_NUM_0 &&
        !callback->GetName(argv[PluginInnerNApiUtils::ARG_NUM_0])) {
        LOGE("BridgeObject::UnRegisterMethod: Method name error.");
        return PluginInnerNApiUtils::CreateUndefined(env);
    }

    if (argc > PluginInnerNApiUtils::ARG_NUM_1 &&
        !callback->GetCallback(argv[PluginInnerNApiUtils::ARG_NUM_1], false)) {
        LOGE("BridgeObject::UnRegisterMethod: Failed to create the callback event.");
        return PluginInnerNApiUtils::CreateUndefined(env);
    }

    napi_value result = PluginInnerNApiUtils::CreateUndefined(env);
    if (!callback->IsCallback()) {
        result = callback->GetPromise(false);
    }

    UnRegisterMethodInner(env, thisVal, callback);
    return result;
}

napi_value BridgeModule::BridgeObject::SendMessage(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    size_t argc = PluginInnerNApiUtils::MAX_ARG_NUM;
    napi_value argv[PluginInnerNApiUtils::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));

    if (argc == PluginInnerNApiUtils::ARG_NUM_0 || argc > PluginInnerNApiUtils::ARG_NUM_2) {
        LOGE("BridgeObject::SendMessage: Method parameter error.");
        return PluginInnerNApiUtils::CreateUndefined(env);
    }

    std::shared_ptr<MethodData> callback = std::make_shared<MethodData>(env);
    if (argc > PluginInnerNApiUtils::ARG_NUM_0 &&
        !callback->GetMessageData(argv[PluginInnerNApiUtils::ARG_NUM_0])) {
        LOGE("BridgeObject::SendMessage: Failed to create the callback event.");
        return PluginInnerNApiUtils::CreateUndefined(env);
    }

    if (argc == PluginInnerNApiUtils::ARG_NUM_2 &&
        !callback->GetCallback(argv[PluginInnerNApiUtils::ARG_NUM_1], false)) {
        LOGE("BridgeObject::SendMessage: Failed to create the callback event.");
        return PluginInnerNApiUtils::CreateUndefined(env);
    }

    napi_value result = PluginInnerNApiUtils::CreateUndefined(env);
    if (!callback->IsCallback()) {
        result = callback->GetPromise(false);
    }

    SendMessageInner(env, thisVal, callback);
    return result;
}

napi_value BridgeModule::BridgeObject::SetMessageListener(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    size_t argc = PluginInnerNApiUtils::MAX_ARG_NUM;
    napi_value argv[PluginInnerNApiUtils::MAX_ARG_NUM] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVal, nullptr));

    if (argc != PluginInnerNApiUtils::ARG_NUM_1) {
        LOGE("BridgeObject::SetMessageListener: Method parameter error.");
        return PluginInnerNApiUtils::CreateUndefined(env);
    }

    std::shared_ptr<MethodData> onMessageCallback = std::make_shared<MethodData>(env);
    onMessageCallback->SetIsMessageEvent(true);

    if (!onMessageCallback->GetCallback(argv[PluginInnerNApiUtils::ARG_NUM_0], true)) {
        LOGE("BridgeObject::SetMessageListener: Failed to create the listener method.");
        return PluginInnerNApiUtils::CreateUndefined(env);
    }

    SetMessageListenerInner(env, thisVal, onMessageCallback);
    return PluginInnerNApiUtils::CreateUndefined(env);
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
