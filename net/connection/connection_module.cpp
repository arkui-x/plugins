/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "connection_module.h"

#include "bindsocket_context.h"
#include "connection_async_work.h"
#include "connection_exec.h"
#include "constant.h"
#include "getaddressbyname_context.h"
#include "getappnet_context.h"
#include "getdefaultnet_context.h"
#include "gethttpproxy_context.h"
#include "module_template.h"
#include "napi_constant.h"
#include "net_all_capabilities.h"
#include "netconnection.h"
#include "netmanager_base_log.h"
#include "none_params_context.h"
#include "parse_nethandle_context.h"
#include "register_context.h"
#include "setappnet_context.h"
#include "setglobalhttpproxy_context.h"
#ifdef ANDROID_PLATFORM
#include "net_conn_client_jni.h"
#include "plugin_utils.h"
#endif

static constexpr const char *CONNECTION_MODULE_NAME = "net.connection";
static thread_local uint64_t g_moduleId;

#define DECLARE_NET_CAP(cap) \
    DECLARE_NAPI_STATIC_PROPERTY(#cap, NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetCap::cap)))

#define DECLARE_NET_BEAR_TYPE(type) \
    DECLARE_NAPI_STATIC_PROPERTY(#type, NapiUtils::CreateUint32(env, static_cast<uint32_t>(NetBearType::type)))

namespace OHOS::NetManagerStandard {
template<typename T>
static bool ParseTypesArray(napi_env env, napi_value obj, std::set<T> &typeArray)
{
    if (!NapiUtils::IsArray(env, obj)) {
        return false;
    }
    uint32_t arrayLenght =
        NapiUtils::GetArrayLength(env, obj) > MAX_ARRAY_LENGTH ? MAX_ARRAY_LENGTH : NapiUtils::GetArrayLength(env, obj);
    for (uint32_t i = 0; i < arrayLenght; ++i) {
        napi_value val = NapiUtils::GetArrayElement(env, obj, i);
        if (NapiUtils::GetValueType(env, val) == napi_number) {
            typeArray.insert(static_cast<T>(NapiUtils::GetUint32FromValue(env, val)));
        } else {
            NETMANAGER_BASE_LOGE("Invalid parameter type of array element!");
            return false;
        }
    }
    return true;
}

static bool ParseCapabilities(napi_env env, napi_value obj, NetAllCapabilities &capabilities)
{
    if (NapiUtils::GetValueType(env, obj) != napi_object) {
        return false;
    }

    capabilities.linkUpBandwidthKbps_ = NapiUtils::GetUint32Property(env, obj, KEY_LINK_UP_BAND_WIDTH_KPS);
    capabilities.linkDownBandwidthKbps_ = NapiUtils::GetUint32Property(env, obj, KEY_LINK_DOWN_BAND_WIDTH_KPS);

    napi_value networkCap = NapiUtils::GetNamedProperty(env, obj, KEY_NETWORK_CAP);
    (void)ParseTypesArray<NetCap>(env, networkCap, capabilities.netCaps_);

    napi_value bearerTypes = NapiUtils::GetNamedProperty(env, obj, KEY_BEARER_TYPE);
    if (!ParseTypesArray<NetBearType>(env, bearerTypes, capabilities.bearerTypes_)) {
        return false;
    }

    return true;
}

static bool ParseNetSpecifier(napi_env env, napi_value obj, NetSpecifier &specifier)
{
    napi_value capabilitiesObj = NapiUtils::GetNamedProperty(env, obj, KEY_NET_CAPABILITIES);
    if (!ParseCapabilities(env, capabilitiesObj, specifier.netCapabilities_)) {
        return false;
    }
    specifier.ident_ = NapiUtils::GetStringPropertyUtf8(env, obj, KEY_BEARER_PRIVATE_IDENTIFIER);
    return true;
}

static void *ParseNetConnectionParams(
    napi_env env, size_t argc, napi_value *argv, std::shared_ptr<EventManager> &manager)
{
    std::unique_ptr<NetConnection, decltype(&NetConnection::DeleteNetConnection)> netConnection(
        NetConnection::MakeNetConnection(manager), NetConnection::DeleteNetConnection);
    netConnection->moduleId_ = g_moduleId;
    if (argc == ARG_NUM_0) {
        NETMANAGER_BASE_LOGI("ParseNetConnectionParams no params");
        return netConnection.release();
    }

    if (argc == ARG_NUM_1 && NapiUtils::GetValueType(env, argv[ARG_INDEX_0]) == napi_object) {
        if (!ParseNetSpecifier(env, argv[ARG_INDEX_0], netConnection->netSpecifier_)) {
            NETMANAGER_BASE_LOGE("ParseNetSpecifier failed");
            return nullptr;
        }
        netConnection->hasNetSpecifier_ = true;
        return netConnection.release();
    }

    if (argc == ARG_NUM_2 && NapiUtils::GetValueType(env, argv[ARG_INDEX_0]) == napi_object &&
        NapiUtils::GetValueType(env, argv[ARG_INDEX_1]) == napi_number) {
        if (!ParseNetSpecifier(env, argv[ARG_INDEX_0], netConnection->netSpecifier_)) {
            NETMANAGER_BASE_LOGE("ParseNetSpecifier failed, do not use params");
            return nullptr;
        }
        netConnection->hasNetSpecifier_ = true;
        netConnection->hasTimeout_ = true;
        netConnection->timeout_ = NapiUtils::GetUint32FromValue(env, argv[ARG_INDEX_1]);
        return netConnection.release();
    }

    NETMANAGER_BASE_LOGE("constructor params invalid, should be none or specifier or specifier+timeout_");
    return nullptr;
}

napi_value ConnectionModule::InitConnectionModule(napi_env env, napi_value exports)
{
    g_moduleId = NapiUtils::CreateUvHandlerQueue(env);
    std::initializer_list<napi_property_descriptor> functions = {
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_DEFAULT_NET, GetDefaultNet),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_DEFAULT_NET_SYNC, GetDefaultNetSync),
        DECLARE_NAPI_FUNCTION(FUNCTION_CREATE_NET_CONNECTION, CreateNetConnection),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_ADDRESSES_BY_NAME, GetAddressesByName),
        DECLARE_NAPI_FUNCTION(FUNCTION_HAS_DEFAULT_NET, HasDefaultNet),
        DECLARE_NAPI_FUNCTION(FUNCTION_IS_DEFAULT_NET_METERED, IsDefaultNetMetered),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_NET_CAPABILITIES, GetNetCapabilities),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_CONNECTION_PROPERTIES, GetConnectionProperties),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_ALL_NETS, GetAllNets),
        DECLARE_NAPI_FUNCTION(FUNCTION_ENABLE_AIRPLANE_MODE, EnableAirplaneMode),
        DECLARE_NAPI_FUNCTION(FUNCTION_DISABLE_AIRPLANE_MODE, DisableAirplaneMode),
        DECLARE_NAPI_FUNCTION(FUNCTION_REPORT_NET_CONNECTED, ReportNetConnected),
        DECLARE_NAPI_FUNCTION(FUNCTION_REPORT_NET_DISCONNECTED, ReportNetDisconnected),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_GLOBAL_HTTP_PROXY, GetGlobalHttpProxy),
        DECLARE_NAPI_FUNCTION(FUNCTION_SET_GLOBAL_HTTP_PROXY, SetGlobalHttpProxy),
        DECLARE_NAPI_FUNCTION(FUNCTION_GET_APP_NET, GetAppNet),
        DECLARE_NAPI_FUNCTION(FUNCTION_SET_APP_NET, SetAppNet),
    };
    NapiUtils::DefineProperties(env, exports, functions);

    std::initializer_list<napi_property_descriptor> netConnectionFunctions = {
        DECLARE_NAPI_FUNCTION(NetConnectionInterface::FUNCTION_ON, NetConnectionInterface::On),
        DECLARE_NAPI_FUNCTION(NetConnectionInterface::FUNCTION_REGISTER, NetConnectionInterface::Register),
        DECLARE_NAPI_FUNCTION(NetConnectionInterface::FUNCTION_UNREGISTER, NetConnectionInterface::Unregister),
    };
    ModuleTemplate::DefineClass(env, exports, netConnectionFunctions, INTERFACE_NET_CONNECTION);

    InitProperties(env, exports);
    NapiUtils::SetEnvValid(env);
    napi_add_env_cleanup_hook(env, NapiUtils::HookForEnvCleanup, env);
    return exports;
}

void ConnectionModule::InitProperties(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> netCaps = {
        DECLARE_NET_CAP(NET_CAPABILITY_MMS),
        DECLARE_NET_CAP(NET_CAPABILITY_NOT_METERED),
        DECLARE_NET_CAP(NET_CAPABILITY_INTERNET),
        DECLARE_NET_CAP(NET_CAPABILITY_NOT_VPN),
        DECLARE_NET_CAP(NET_CAPABILITY_VALIDATED),
        DECLARE_NET_CAP(NET_CAPABILITY_PORTAL),
        DECLARE_NET_CAP(NET_CAPABILITY_INTERNAL_DEFAULT),
    };
    napi_value caps = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, caps, netCaps);
    NapiUtils::SetNamedProperty(env, exports, INTERFACE_NET_CAP, caps);

    std::initializer_list<napi_property_descriptor> netBearTypes = {
        DECLARE_NET_BEAR_TYPE(BEARER_CELLULAR),
        DECLARE_NET_BEAR_TYPE(BEARER_WIFI),
        DECLARE_NET_BEAR_TYPE(BEARER_BLUETOOTH),
        DECLARE_NET_BEAR_TYPE(BEARER_ETHERNET),
        DECLARE_NET_BEAR_TYPE(BEARER_VPN),
        DECLARE_NET_BEAR_TYPE(BEARER_WIFI_AWARE),
        DECLARE_NET_BEAR_TYPE(BEARER_DEFAULT),
    };
    napi_value types = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, types, netBearTypes);
    NapiUtils::SetNamedProperty(env, exports, INTERFACE_NET_BEAR_TYPE, types);
}

napi_value ConnectionModule::GetAddressesByName(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetAddressByNameContext>(env, info, FUNCTION_GET_ADDRESSES_BY_NAME, nullptr,
        ConnectionAsyncWork::ExecGetAddressesByName, ConnectionAsyncWork::GetAddressesByNameCallback);
}

napi_value ConnectionModule::HasDefaultNet(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<HasDefaultNetContext>(env, info, FUNCTION_HAS_DEFAULT_NET, nullptr,
        ConnectionAsyncWork::ExecHasDefaultNet, ConnectionAsyncWork::HasDefaultNetCallback);
}

napi_value ConnectionModule::IsDefaultNetMetered(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<IsDefaultNetMeteredContext>(env, info, FUNCTION_IS_DEFAULT_NET_METERED, nullptr,
        ConnectionAsyncWork::ExecIsDefaultNetMetered, ConnectionAsyncWork::IsDefaultNetMeteredCallback);
}

napi_value ConnectionModule::GetNetCapabilities(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetNetCapabilitiesContext>(env, info, FUNCTION_GET_NET_CAPABILITIES, nullptr,
        ConnectionAsyncWork::ExecGetNetCapabilities, ConnectionAsyncWork::GetNetCapabilitiesCallback);
}

napi_value ConnectionModule::GetConnectionProperties(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetConnectionPropertiesContext>(env, info, FUNCTION_GET_CONNECTION_PROPERTIES,
        nullptr, ConnectionAsyncWork::ExecGetConnectionProperties,
        ConnectionAsyncWork::GetConnectionPropertiesCallback);
}

napi_value ConnectionModule::CreateNetConnection(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::NewInstance(
        env, info, INTERFACE_NET_CONNECTION, ParseNetConnectionParams, [](napi_env, void *data, void *) {
            NETMANAGER_BASE_LOGI("finalize netConnection");
            auto sharedManager = static_cast<std::shared_ptr<EventManager> *>(data);
            if (sharedManager == nullptr || *sharedManager == nullptr) {
                return;
            }
            auto manager = *sharedManager;
            auto netConnection = static_cast<NetConnection *>(manager->GetData());
            delete sharedManager;
            NetConnection::DeleteNetConnection(netConnection);
        });
}

napi_value ConnectionModule::GetDefaultNet(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetDefaultNetContext>(env, info, FUNCTION_GET_DEFAULT_NET, nullptr,
        ConnectionAsyncWork::ExecGetDefaultNet, ConnectionAsyncWork::GetDefaultNetCallback);
}

napi_value ConnectionModule::GetDefaultNetSync(napi_env env, napi_callback_info info)
{
    std::shared_ptr<EventManager> manager = nullptr;
    GetDefaultNetContext context(env, manager);
    if (ConnectionExec::ExecGetDefaultNet(&context)) {
        return ConnectionExec::GetDefaultNetCallback(&context);
    }
    return NapiUtils::CreateErrorMessage(env, context.GetErrorCode(), context.GetErrorMessage());
}

napi_value ConnectionModule::GetAllNets(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetAllNetsContext>(env, info, FUNCTION_GET_ALL_NETS, nullptr,
        ConnectionAsyncWork::ExecGetAllNets, ConnectionAsyncWork::GetAllNetsCallback);
}

napi_value ConnectionModule::EnableAirplaneMode(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<EnableAirplaneModeContext>(env, info, FUNCTION_ENABLE_AIRPLANE_MODE, nullptr,
        ConnectionAsyncWork::ExecEnableAirplaneMode, ConnectionAsyncWork::EnableAirplaneModeCallback);
}

napi_value ConnectionModule::DisableAirplaneMode(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<DisableAirplaneModeContext>(env, info, FUNCTION_DISABLE_AIRPLANE_MODE, nullptr,
        ConnectionAsyncWork::ExecDisableAirplaneMode, ConnectionAsyncWork::DisableAirplaneModeCallback);
}

napi_value ConnectionModule::ReportNetConnected(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<ReportNetConnectedContext>(env, info, FUNCTION_REPORT_NET_CONNECTED, nullptr,
        ConnectionAsyncWork::ExecReportNetConnected, ConnectionAsyncWork::ReportNetConnectedCallback);
}

napi_value ConnectionModule::ReportNetDisconnected(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<ReportNetDisconnectedContext>(env, info, FUNCTION_REPORT_NET_DISCONNECTED, nullptr,
        ConnectionAsyncWork::ExecReportNetDisconnected, ConnectionAsyncWork::ReportNetDisconnectedCallback);
}

napi_value ConnectionModule::GetGlobalHttpProxy(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetHttpProxyContext>(env, info, FUNCTION_GET_GLOBAL_HTTP_PROXY, nullptr,
        ConnectionAsyncWork::ExecGetGlobalHttpProxy, ConnectionAsyncWork::GetGlobalHttpProxyCallback);
}

napi_value ConnectionModule::SetGlobalHttpProxy(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<SetGlobalHttpProxyContext>(env, info, FUNCTION_SET_GLOBAL_HTTP_PROXY, nullptr,
        ConnectionAsyncWork::ExecSetGlobalHttpProxy, ConnectionAsyncWork::SetGlobalHttpProxyCallback);
}

napi_value ConnectionModule::GetAppNet(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<GetAppNetContext>(env, info, FUNCTION_GET_APP_NET, nullptr,
        ConnectionAsyncWork::ExecGetAppNet, ConnectionAsyncWork::GetAppNetCallback);
}

napi_value ConnectionModule::SetAppNet(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<SetAppNetContext>(env, info, FUNCTION_SET_APP_NET, nullptr,
        ConnectionAsyncWork::ExecSetAppNet, ConnectionAsyncWork::SetAppNetCallback);
}

napi_value ConnectionModule::NetConnectionInterface::On(napi_env env, napi_callback_info info)
{
    std::initializer_list<std::string> events = { EVENT_NET_AVAILABLE, EVENT_NET_BLOCK_STATUS_CHANGE,
        EVENT_NET_CAPABILITIES_CHANGE, EVENT_NET_CONNECTION_PROPERTIES_CHANGE, EVENT_NET_LOST, EVENT_NET_UNAVAILABLE };
    return ModuleTemplate::On(env, info, events, false);
}

napi_value ConnectionModule::NetConnectionInterface::Register(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<RegisterContext>(env, info, FUNCTION_REGISTER, nullptr,
        ConnectionAsyncWork::NetConnectionAsyncWork::ExecRegister,
        ConnectionAsyncWork::NetConnectionAsyncWork::RegisterCallback);
}

napi_value ConnectionModule::NetConnectionInterface::Unregister(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::Interface<UnregisterContext>(
        env, info, FUNCTION_UNREGISTER,
        [](napi_env theEnv, napi_value thisVal, UnregisterContext *context) -> bool {
            if (context && context->GetManager()) {
                context->GetManager()->DeleteAllListener();
            }
            return true;
        },
        ConnectionAsyncWork::NetConnectionAsyncWork::ExecUnregister,
        ConnectionAsyncWork::NetConnectionAsyncWork::UnregisterCallback);
}

static napi_module g_connectionModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ConnectionModule::InitConnectionModule,
    .nm_modname = CONNECTION_MODULE_NAME,
    .nm_priv = nullptr,
    .reserved = { nullptr },
};

#ifdef ANDROID_PLATFORM
static void NetConnClientJniRegister()
{
    const char className[] = "ohos.ace.plugin.netconnclientplugin.NetConnClientPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&Plugin::NetConnClientJni::Register, className);
}
#endif

extern "C" __attribute__((constructor)) void RegisterConnectionModule(void)
{
#ifdef ANDROID_PLATFORM
    NetConnClientJniRegister();
#endif
    napi_module_register(&g_connectionModule);
}
} // namespace OHOS::NetManagerStandard
