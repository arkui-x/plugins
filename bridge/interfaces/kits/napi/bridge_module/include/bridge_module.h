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

#ifndef PLUGINS_BRIDGE_BRIDGE_MODULE_H
#define PLUGINS_BRIDGE_BRIDGE_MODULE_H

#include "napi/native_api.h"
#include "bridge_wrap.h"

namespace OHOS::Plugin::Bridge {
class BridgeModule {
public:
    class BridgeObject {
    public:
        static constexpr const char* PROPERTY_BRIDGE_NAME = "bridgeName";
        static constexpr const char* FUNCTION_CALL_METHOD = "callMethod";
        static constexpr const char* FUNCTION_REGISTER_METHOD = "registerMethod";
        static constexpr const char* FUNCTION_UNREGISTER_METHOD = "unRegisterMethod";
        static constexpr const char* FUNCTION_SEND_MESSAGE = "sendMessage";
        static constexpr const char* FUNCTION_REGISTER_ON_MESSAGE = "setMessageListener";
        static constexpr const char* FUNCTION_CALL_METHOD_CALLBACK = "callMethodWithCallback";

        static napi_value GetBridgeName(napi_env env, napi_callback_info info);
        static napi_value CallMethod(napi_env env, napi_callback_info info);
        static napi_value RegisterMethod(napi_env env, napi_callback_info info);
        static napi_value UnRegisterMethod(napi_env env, napi_callback_info info);
        static napi_value SendMessage(napi_env env, napi_callback_info info);
        static napi_value SetMessageListener(napi_env env, napi_callback_info info);
        static napi_value CallMethodWithCallBack(napi_env env, napi_callback_info info);
    };

    static constexpr const char* FUNCTION_CREATE_PLUGIN_BRIDGE = "createBridge";
    static constexpr const char* INTERFACE_PLUGIN_BRIDGE_OBJECT = "BridgeObject";

    static napi_value InitBridgeModule(napi_env env, napi_value exports);

    static void NotifyJSMethodResult(std::shared_ptr<MethodData> methodData, ErrorCode errorCode);

private:
    static napi_value CreateBridge(napi_env env, napi_callback_info info);
    static void DefinePluginBridgeObjectClass(napi_env env, napi_value exports);
    static void InitBridgeProperties(napi_env env, napi_value exports);
    static Bridge* GetBridge(napi_env env, napi_value thisVal);
    static void CallMethodInner(napi_env env, napi_value thisVal, std::shared_ptr<MethodData> methodData);
    static void RegisterMethodInner(napi_env env, napi_value thisVal, std::shared_ptr<MethodData> methodData,
        std::shared_ptr<MethodData> callback);
    static void UnRegisterMethodInner(napi_env env, napi_value thisVal, std::shared_ptr<MethodData> callback);
    static void SendMessageInner(napi_env env, napi_value thisVal, std::shared_ptr<MethodData> callback);
    static void SetMessageListenerInner(napi_env env, napi_value thisVal,
        std::shared_ptr<MethodData> onMessageCallback);
    static napi_value InitCodecType(napi_env env);
    static napi_value CallMethodWithCallBackInner(napi_env env, napi_callback_info info);
};
} // namespace OHOS::Plugin::Bridge
#endif