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

#include "connection_exec.h"

#include <cstddef>
#include <cstdint>
#include <string>

#include "net_handle_interface.h"
#include "constant.h"
#include "errorcode_convertor.h"
#include "connection/net_conn_client.h"
#include "net_manager_constants.h"
#include "netconnection.h"
#include "netmanager_base_common_utils.h"
#include "netmanager_base_log.h"
#include "napi_utils.h"
#include "securec.h"

namespace OHOS::NetManagerStandard {
namespace {
constexpr int32_t NO_PERMISSION_CODE = 1;
constexpr int32_t RESOURCE_UNAVALIEBLE_CODE = 11;
constexpr int32_t NET_UNREACHABLE_CODE = 101;
} // namespace

napi_value ConnectionExec::CreateNetHandle(napi_env env, NetHandle *handle)
{
    napi_value netHandle = NapiUtils::CreateObject(env);
    if (NapiUtils::GetValueType(env, netHandle) != napi_object) {
        return NapiUtils::GetUndefined(env);
    }

    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_FUNCTION(NetHandleInterface::FUNCTION_GET_ADDRESSES_BY_NAME,
                              NetHandleInterface::GetAddressesByName),
        DECLARE_NAPI_FUNCTION(NetHandleInterface::FUNCTION_GET_ADDRESS_BY_NAME,
                              NetHandleInterface::GetAddressByName),
        DECLARE_NAPI_FUNCTION(NetHandleInterface::FUNCTION_BIND_SOCKET,
                              NetHandleInterface::BindSocket),
    };
    NapiUtils::DefineProperties(env, netHandle, properties);
    NapiUtils::SetUint32Property(env, netHandle, NetHandleInterface::PROPERTY_NET_ID, handle->GetNetId());
    return netHandle;
}

napi_value ConnectionExec::CreateNetCapabilities(napi_env env, NetAllCapabilities *capabilities)
{
    napi_value netCapabilities = NapiUtils::CreateObject(env);
    if (NapiUtils::GetValueType(env, netCapabilities) != napi_object) {
        return NapiUtils::GetUndefined(env);
    }

    NapiUtils::SetUint32Property(env, netCapabilities, KEY_LINK_UP_BAND_WIDTH_KPS,
                                 capabilities->linkUpBandwidthKbps_);
    NapiUtils::SetUint32Property(env, netCapabilities, KEY_LINK_DOWN_BAND_WIDTH_KPS,
                                 capabilities->linkDownBandwidthKbps_);
    if (!capabilities->netCaps_.empty() && capabilities->netCaps_.size() <= MAX_ARRAY_LENGTH) {
        napi_value networkCap =
            NapiUtils::CreateArray(env, std::min(capabilities->netCaps_.size(), MAX_ARRAY_LENGTH));
        auto it = capabilities->netCaps_.begin();
        for (uint32_t index = 0; index < MAX_ARRAY_LENGTH && it != capabilities->netCaps_.end(); ++index, ++it) {
            NapiUtils::SetArrayElement(env, networkCap, index, NapiUtils::CreateUint32(env, *it));
        }
        NapiUtils::SetNamedProperty(env, netCapabilities, KEY_NETWORK_CAP, networkCap);
    }
    if (!capabilities->bearerTypes_.empty() && capabilities->bearerTypes_.size() <= MAX_ARRAY_LENGTH) {
        napi_value bearerTypes =
            NapiUtils::CreateArray(env, std::min(capabilities->bearerTypes_.size(), MAX_ARRAY_LENGTH));
        auto it = capabilities->bearerTypes_.begin();
        for (uint32_t index = 0; index < MAX_ARRAY_LENGTH && it != capabilities->bearerTypes_.end(); ++index, ++it) {
            NapiUtils::SetArrayElement(env, bearerTypes, index, NapiUtils::CreateUint32(env, *it));
        }
        NapiUtils::SetNamedProperty(env, netCapabilities, KEY_BEARER_TYPE, bearerTypes);
    }
    return netCapabilities;
}

napi_value ConnectionExec::CreateConnectionProperties(napi_env env, NetLinkInfo *linkInfo)
{
    napi_value connectionProperties = NapiUtils::CreateObject(env);
    if (NapiUtils::GetValueType(env, connectionProperties) != napi_object) {
        return NapiUtils::GetUndefined(env);
    }
    NapiUtils::SetStringPropertyUtf8(env, connectionProperties, KEY_INTERFACE_NAME, linkInfo->ifaceName_);
    NapiUtils::SetStringPropertyUtf8(env, connectionProperties, KEY_DOMAINS, linkInfo->domain_);
    NapiUtils::SetUint32Property(env, connectionProperties, KEY_MTU, linkInfo->mtu_);
    FillLinkAddress(env, connectionProperties, linkInfo);
    FillRouoteList(env, connectionProperties, linkInfo);
    FillDns(env, connectionProperties, linkInfo);
    return connectionProperties;
}

bool ConnectionExec::ExecGetAddressByName(GetAddressByNameContext *context)
{
    return NetHandleExec::ExecGetAddressesByName(context);
}

napi_value ConnectionExec::GetAddressByNameCallback(GetAddressByNameContext *context)
{
    return NetHandleExec::GetAddressesByNameCallback(context);
}

bool ConnectionExec::ExecGetDefaultNet(GetDefaultNetContext *context)
{
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultNet(context->netHandle_);
    if (ret != NETMANAGER_SUCCESS) {
        NETMANAGER_BASE_LOGE("get default net failed %{public}d", ret);
        context->SetErrorCode(ret);
    }
    return ret == NETMANAGER_SUCCESS;
}

napi_value ConnectionExec::GetDefaultNetCallback(GetDefaultNetContext *context)
{
    return CreateNetHandle(context->GetEnv(), &context->netHandle_);
}

bool ConnectionExec::ExecHasDefaultNet(HasDefaultNetContext *context)
{
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->HasDefaultNet(context->hasDefaultNet_);
    NETMANAGER_BASE_LOGI("ExecHasDefaultNet ret %{public}d", ret);
    if (ret != NETMANAGER_SUCCESS && ret != NET_CONN_ERR_NO_DEFAULT_NET) {
        context->SetErrorCode(ret);
        return false;
    }
    return true;
}

napi_value ConnectionExec::HasDefaultNetCallback(HasDefaultNetContext *context)
{
    return NapiUtils::GetBoolean(context->GetEnv(), context->hasDefaultNet_);
}

bool ConnectionExec::ExecIsDefaultNetMetered(IsDefaultNetMeteredContext *context)
{
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->IsDefaultNetMetered(context->isMetered_);
    if (ret != NETMANAGER_SUCCESS) {
        NETMANAGER_BASE_LOGE("get net metered status failed %{public}d", ret);
        context->SetErrorCode(ret);
    }
    NETMANAGER_BASE_LOGD("exec is default net metered ret %{public}d", ret);
    return ret == NETMANAGER_SUCCESS;
}

napi_value ConnectionExec::IsDefaultNetMeteredCallback(IsDefaultNetMeteredContext *context)
{
    return NapiUtils::GetBoolean(context->GetEnv(), context->isMetered_);
}

bool ConnectionExec::ExecGetNetCapabilities(GetNetCapabilitiesContext *context)
{
    if (!context->IsParseOK()) {
        return false;
    }
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->GetNetCapabilities(context->netHandle_,
                                                                                  context->capabilities_);
    context->SetErrorCode(ret);
    return ret == NETMANAGER_SUCCESS;
}

napi_value ConnectionExec::GetNetCapabilitiesCallback(GetNetCapabilitiesContext *context)
{
    return CreateNetCapabilities(context->GetEnv(), &context->capabilities_);
}

bool ConnectionExec::ExecGetConnectionProperties(GetConnectionPropertiesContext *context)
{
    if (!context->IsParseOK()) {
        return false;
    }
    auto ret = DelayedSingleton<NetConnClient>::GetInstance()->GetConnectionProperties(context->netHandle_,
                                                                                       context->linkInfo_);
    context->SetErrorCode(ret);
    return ret == NETMANAGER_SUCCESS;
}

napi_value ConnectionExec::GetConnectionPropertiesCallback(GetConnectionPropertiesContext *context)
{
    return CreateConnectionProperties(context->GetEnv(), &context->linkInfo_);
}

bool ConnectionExec::ExecGetAllNets(GetAllNetsContext *context)
{
    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->GetAllNets(context->netHandleList_);
    context->SetErrorCode(ret);
    return ret == NETMANAGER_SUCCESS;
}

napi_value ConnectionExec::GetAllNetsCallback(GetAllNetsContext *context)
{
    napi_value array = NapiUtils::CreateArray(context->GetEnv(), context->netHandleList_.size());
    uint32_t index = 0;
    std::for_each(context->netHandleList_.begin(), context->netHandleList_.end(),
                  [array, &index, context](const sptr<NetHandle> &handle) {
                      NapiUtils::SetArrayElement(context->GetEnv(), array, index,
                                                 CreateNetHandle(context->GetEnv(), handle.GetRefPtr()));
                      ++index;
                  });
    return array;
}

bool ConnectionExec::ExecEnableAirplaneMode(EnableAirplaneModeContext *context)
{
    int32_t res = DelayedSingleton<NetConnClient>::GetInstance()->SetAirplaneMode(true);
    if (res != NETMANAGER_SUCCESS) {
        NETMANAGER_BASE_LOGE("ExecEnableAirplaneMode failed %{public}d", res);
        context->SetErrorCode(res);
    }
    return res == NETMANAGER_SUCCESS;
}

napi_value ConnectionExec::EnableAirplaneModeCallback(EnableAirplaneModeContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecDisableAirplaneMode(DisableAirplaneModeContext *context)
{
    int32_t res = DelayedSingleton<NetConnClient>::GetInstance()->SetAirplaneMode(false);
    if (res != NETMANAGER_SUCCESS) {
        NETMANAGER_BASE_LOGE("ExecDisableAirplaneMode failed %{public}d", res);
        context->SetErrorCode(res);
    }
    return res == NETMANAGER_SUCCESS;
}

napi_value ConnectionExec::DisableAirplaneModeCallback(DisableAirplaneModeContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecReportNetConnected(ReportNetConnectedContext *context)
{
    if (!context->IsParseOK()) {
        return false;
    }
    int32_t res = DelayedSingleton<NetConnClient>::GetInstance()->NetDetection(context->netHandle_);
    if (res != NETMANAGER_SUCCESS) {
        NETMANAGER_BASE_LOGE("ExecReportNetConnected failed %{public}d", res);
        context->SetErrorCode(res);
    }
    return res == NETMANAGER_SUCCESS;
}

napi_value ConnectionExec::ReportNetConnectedCallback(ReportNetConnectedContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecReportNetDisconnected(ReportNetConnectedContext *context)
{
    if (!context->IsParseOK()) {
        return false;
    }
    int32_t res = DelayedSingleton<NetConnClient>::GetInstance()->NetDetection(context->netHandle_);
    if (res != NETMANAGER_SUCCESS) {
        NETMANAGER_BASE_LOGE("ExecReportNetDisconnected failed %{public}d", res);
        context->SetErrorCode(res);
    }
    return res == NETMANAGER_SUCCESS;
}

napi_value ConnectionExec::ReportNetDisconnectedCallback(ReportNetConnectedContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecGetGlobalHttpProxy(GetHttpProxyContext *context)
{
    int32_t errorCode = DelayedSingleton<NetConnClient>::GetInstance()->GetGlobalHttpProxy(context->httpProxy_);
    if (errorCode != NET_CONN_SUCCESS) {
        context->SetErrorCode(errorCode);
        return false;
    }
    return true;
}

bool ConnectionExec::ExecGetDefaultHttpProxy(GetHttpProxyContext *context)
{
    int32_t errorCode = DelayedSingleton<NetConnClient>::GetInstance()->GetDefaultHttpProxy(context->httpProxy_);
    if (errorCode != NET_CONN_SUCCESS) {
        context->SetErrorCode(errorCode);
        return false;
    }
    return true;
}

napi_value ConnectionExec::GetDefaultHttpProxyCallback(GetHttpProxyContext *context)
{
    napi_value host = NapiUtils::CreateStringUtf8(context->GetEnv(), context->httpProxy_.GetHost());
    napi_value port = NapiUtils::CreateInt32(context->GetEnv(), context->httpProxy_.GetPort());
    auto lists = context->httpProxy_.GetExclusionList();
    napi_value exclusionList = NapiUtils::CreateArray(context->GetEnv(), lists.size());
    size_t index = 0;
    for (auto list : lists) {
        napi_value jsList = NapiUtils::CreateStringUtf8(context->GetEnv(), list);
        NapiUtils::SetArrayElement(context->GetEnv(), exclusionList, index++, jsList);
    }
    napi_value httpProxy = NapiUtils::CreateObject(context->GetEnv());
    NapiUtils::SetNamedProperty(context->GetEnv(), httpProxy, "host", host);
    NapiUtils::SetNamedProperty(context->GetEnv(), httpProxy, "port", port);
    NapiUtils::SetNamedProperty(context->GetEnv(), httpProxy, "exclusionList", exclusionList);
    return httpProxy;
}

napi_value ConnectionExec::GetGlobalHttpProxyCallback(GetHttpProxyContext *context)
{
    napi_value host = NapiUtils::CreateStringUtf8(context->GetEnv(), context->httpProxy_.GetHost());
    napi_value port = NapiUtils::CreateInt32(context->GetEnv(), context->httpProxy_.GetPort());
    auto lists = context->httpProxy_.GetExclusionList();
    napi_value exclusionList = NapiUtils::CreateArray(context->GetEnv(), lists.size());
    size_t index = 0;
    for (auto list : lists) {
        napi_value jsList = NapiUtils::CreateStringUtf8(context->GetEnv(), list);
        NapiUtils::SetArrayElement(context->GetEnv(), exclusionList, index++, jsList);
    }
    napi_value httpProxy = NapiUtils::CreateObject(context->GetEnv());
    NapiUtils::SetNamedProperty(context->GetEnv(), httpProxy, "host", host);
    NapiUtils::SetNamedProperty(context->GetEnv(), httpProxy, "port", port);
    NapiUtils::SetNamedProperty(context->GetEnv(), httpProxy, "exclusionList", exclusionList);
    return httpProxy;
}

bool ConnectionExec::ExecSetGlobalHttpProxy(SetGlobalHttpProxyContext *context)
{
    int32_t errorCode = DelayedSingleton<NetConnClient>::GetInstance()->SetGlobalHttpProxy(context->httpProxy_);
    if (errorCode != NET_CONN_SUCCESS) {
        context->SetErrorCode(errorCode);
        return false;
    }
    return true;
}

napi_value ConnectionExec::SetGlobalHttpProxyCallback(SetGlobalHttpProxyContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecGetAppNet(GetAppNetContext *context)
{
    NETMANAGER_BASE_LOGI("into");
    int32_t netId = 0;
    int32_t errorCode = DelayedSingleton<NetConnClient>::GetInstance()->GetAppNet(netId);
    if (errorCode != NET_CONN_SUCCESS) {
        NETMANAGER_BASE_LOGE("exec getAppNet failed errorCode: %{public}d", errorCode);
        context->SetErrorCode(errorCode);
        return false;
    }
    context->netHandle_.SetNetId(netId);
    return true;
}

napi_value ConnectionExec::GetAppNetCallback(GetAppNetContext *context)
{
    NETMANAGER_BASE_LOGI("into");
    return CreateNetHandle(context->GetEnv(), &context->netHandle_);
}

bool ConnectionExec::ExecSetAppNet(SetAppNetContext *context)
{
    NETMANAGER_BASE_LOGI("into");
    int32_t errorCode = DelayedSingleton<NetConnClient>::GetInstance()->SetAppNet(context->netHandle_.GetNetId());
    if (errorCode != NET_CONN_SUCCESS) {
        NETMANAGER_BASE_LOGE("exec setAppNet failed errorCode: %{public}d", errorCode);
        context->SetErrorCode(errorCode);
        return false;
    }
    return true;
}

napi_value ConnectionExec::SetAppNetCallback(SetAppNetContext *context)
{
    NETMANAGER_BASE_LOGI("into");
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecSetNetExtAttribute(SetNetExtAttributeContext *context)
{
    return false;
}

napi_value ConnectionExec::SetNetExtAttributeCallback(SetNetExtAttributeContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecGetNetExtAttribute(GetNetExtAttributeContext *context)
{
    return false;
}

napi_value ConnectionExec::GetNetExtAttributeCallback(GetNetExtAttributeContext *context)
{
    return NapiUtils::CreateStringUtf8(context->GetEnv(), context->netExtAttribute_);
}

bool ConnectionExec::ExecSetCustomDNSRule(SetCustomDNSRuleContext *context)
{
    if (context == nullptr) {
        NETMANAGER_BASE_LOGE("context is nullptr");
        return false;
    }

    if (!CommonUtils::HasInternetPermission()) {
        context->SetErrorCode(NETMANAGER_ERR_PERMISSION_DENIED);
        return false;
    }

    if (context->host_.empty() || context->ip_.empty()) {
        context->SetErrorCode(NETMANAGER_ERR_PARAMETER_ERROR);
        return false;
    }

    std::vector<std::string> ip = context->ip_;
    for (size_t i = 0; i < ip.size(); i++) {
        if (!CommonUtils::IsValidIPV4(ip[i]) && !CommonUtils::IsValidIPV6(ip[i])) {
            context->SetErrorCode(NETMANAGER_ERR_PARAMETER_ERROR);
            return false;
        }
    }

    if (!context->IsParseOK()) {
        context->SetErrorCode(NETMANAGER_ERR_PARAMETER_ERROR);
        return false;
    }

    std::string host_ips = context->host_ + ",";
    for (size_t i = 0; i < ip.size(); i++) {
        host_ips.append(ip[i]);
        if (i < ip.size() - 1) {
            host_ips.append(",");
        }
    }

    NETMANAGER_BASE_LOGI("set host with ip addr string: %{public}s", host_ips.c_str());
    return true;
}

napi_value ConnectionExec::SetCustomDNSRuleCallback(SetCustomDNSRuleContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecDeleteCustomDNSRule(DeleteCustomDNSRuleContext *context)
{
    if (context == nullptr) {
        NETMANAGER_BASE_LOGE("context is nullptr");
        return false;
    }
    if (!CommonUtils::HasInternetPermission()) {
        context->SetErrorCode(NETMANAGER_ERR_PERMISSION_DENIED);
        return false;
    }

    if (context->host_.empty()) {
        context->SetErrorCode(NETMANAGER_ERR_PARAMETER_ERROR);
        return false;
    }

    if (!context->IsParseOK()) {
        context->SetErrorCode(NETMANAGER_ERR_PARAMETER_ERROR);
        return false;
    }

    NETMANAGER_BASE_LOGI("delete host with ip addr string: %{public}s", context->host_.c_str());
    return true;
}

napi_value ConnectionExec::DeleteCustomDNSRuleCallback(DeleteCustomDNSRuleContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecDeleteCustomDNSRules(DeleteCustomDNSRulesContext *context)
{
    if (context == nullptr) {
        NETMANAGER_BASE_LOGE("context is nullptr");
        return false;
    }
    if (!CommonUtils::HasInternetPermission()) {
        context->SetErrorCode(NETMANAGER_ERR_PERMISSION_DENIED);
        return false;
    }

    if (!context->IsParseOK()) {
        context->SetErrorCode(NETMANAGER_ERR_PARAMETER_ERROR);
        return false;
    }

    return true;
}

napi_value ConnectionExec::DeleteCustomDNSRulesCallback(DeleteCustomDNSRulesContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecSetInterfaceUp(SetInterfaceUpContext *context)
{
    return false;
}

napi_value ConnectionExec::SetInterfaceUpCallback(SetInterfaceUpContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecSetInterfaceIpAddr(SetInterfaceIpAddrContext *context)
{
    return false;
}

napi_value ConnectionExec::SetInterfaceIpAddrCallback(SetInterfaceIpAddrContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecAddNetworkRoute(AddNetworkRouteContext *context)
{
    return false;
}

napi_value ConnectionExec::AddNetworkRouteCallback(AddNetworkRouteContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecGetNetInterfaceConfiguration(GetNetInterfaceConfigurationContext *context)
{
    return false;
}

napi_value ConnectionExec::GetNetInterfaceConfigurationCallback(GetNetInterfaceConfigurationContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecRegisterNetSupplier(RegisterNetSupplierContext *context)
{
    return false;
}

napi_value ConnectionExec::RegisterNetSupplierCallback(RegisterNetSupplierContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecUnregisterNetSupplier(UnregisterNetSupplierContext *context)
{
    return false;
}

napi_value ConnectionExec::UnregisterNetSupplierCallback(UnregisterNetSupplierContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::ExecFactoryResetNetwork(FactoryResetNetworkContext *context)
{
    NETMANAGER_BASE_LOGI("ExecFactoryResetNetwork into");
    int32_t errorCode = DelayedSingleton<NetConnClient>::GetInstance()->FactoryResetNetwork();
    if (errorCode != NET_CONN_SUCCESS) {
        NETMANAGER_BASE_LOGE("exec ResetNetwork failed errorCode: %{public}d", errorCode);
        context->SetErrorCode(errorCode);
        return false;
    }
    return true;
}

napi_value ConnectionExec::FactoryResetNetworkCallback(FactoryResetNetworkContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

int32_t TransErrorCode(int32_t error)
{
    switch (error) {
        case NO_PERMISSION_CODE:
            return NETMANAGER_ERR_PERMISSION_DENIED;
        case RESOURCE_UNAVALIEBLE_CODE:
            return NETMANAGER_ERR_INVALID_PARAMETER;
        case NET_UNREACHABLE_CODE:
            return NETMANAGER_ERR_INTERNAL;
        default:
            return NETMANAGER_ERR_OPERATION_FAILED;
    }
}

bool ConnectionExec::NetHandleExec::ExecGetAddressesByName(GetAddressByNameContext *context)
{
    if (!context->IsParseOK()) {
        return false;
    }
    addrinfo *res = nullptr;
    int status = getaddrinfo(context->host_.c_str(), nullptr, nullptr, &res);
    if (status < 0) {
        NETMANAGER_BASE_LOGE("getaddrinfo errno %{public}d %{public}s", errno, strerror(errno));
        int32_t temp = TransErrorCode(errno);
        context->SetErrorCode(temp);
        return false;
    }

    for (addrinfo *tmp = res; tmp != nullptr; tmp = tmp->ai_next) {
        std::string host;
        if (tmp->ai_family == AF_INET) {
            auto addr = reinterpret_cast<sockaddr_in *>(tmp->ai_addr);
            char ip[MAX_IPV4_STR_LEN] = {0};
            inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
            host = ip;
        } else if (tmp->ai_family == AF_INET6) {
            auto addr = reinterpret_cast<sockaddr_in6 *>(tmp->ai_addr);
            char ip[MAX_IPV6_STR_LEN] = {0};
            inet_ntop(AF_INET6, &addr->sin6_addr, ip, sizeof(ip));
            host = ip;
        }

        NetAddress address;
        SetAddressInfo(host.c_str(), tmp, address);

        context->addresses_.emplace_back(address);
    }
    freeaddrinfo(res);
    return true;
}

napi_value ConnectionExec::NetHandleExec::GetAddressesByNameCallback(GetAddressByNameContext *context)
{
    napi_value addresses = NapiUtils::CreateArray(context->GetEnv(), context->addresses_.size());
    for (uint32_t index = 0; index < context->addresses_.size(); ++index) {
        napi_value obj = MakeNetAddressJsValue(context->GetEnv(), context->addresses_[index]);
        NapiUtils::SetArrayElement(context->GetEnv(), addresses, index, obj);
    }
    return addresses;
}

bool ConnectionExec::NetHandleExec::ExecGetAddressByName(GetAddressByNameContext *context)
{
    if (!context->IsParseOK()) {
        return false;
    }
    addrinfo *res = nullptr;
    int status = getaddrinfo(context->host_.c_str(), nullptr, nullptr, &res);
    if (status < 0) {
        NETMANAGER_BASE_LOGE("getaddrinfo errno %{public}d %{public}s", errno, strerror(errno));
        int32_t temp = TransErrorCode(errno);
        context->SetErrorCode(temp);
        return false;
    }

    if (res != nullptr) {
        std::string host;
        if (res->ai_family == AF_INET) {
            auto addr = reinterpret_cast<sockaddr_in *>(res->ai_addr);
            char ip[MAX_IPV4_STR_LEN] = {0};
            inet_ntop(AF_INET, &addr->sin_addr, ip, sizeof(ip));
            host = ip;
        } else if (res->ai_family == AF_INET6) {
            auto addr = reinterpret_cast<sockaddr_in6 *>(res->ai_addr);
            char ip[MAX_IPV6_STR_LEN] = {0};
            inet_ntop(AF_INET6, &addr->sin6_addr, ip, sizeof(ip));
            host = ip;
        }

        NetAddress address;
        SetAddressInfo(host.c_str(), res, address);

        context->addresses_.emplace_back(address);
    }
    freeaddrinfo(res);
    return true;
}

napi_value ConnectionExec::NetHandleExec::GetAddressByNameCallback(GetAddressByNameContext *context)
{
    if (context->addresses_.empty()) {
        return NapiUtils::GetUndefined(context->GetEnv());
    }
    return MakeNetAddressJsValue(context->GetEnv(), context->addresses_[0]);
}

napi_value ConnectionExec::NetHandleExec::MakeNetAddressJsValue(napi_env env, const NetAddress &address)
{
    napi_value obj = NapiUtils::CreateObject(env);
    if (NapiUtils::GetValueType(env, obj) != napi_object) {
        return NapiUtils::GetUndefined(env);
    }

    NapiUtils::SetStringPropertyUtf8(env, obj, KEY_ADDRESS, address.GetAddress());
    NapiUtils::SetUint32Property(env, obj, KEY_FAMILY, address.GetJsValueFamily());
    NapiUtils::SetUint32Property(env, obj, KEY_PORT, address.GetPort());
    return obj;
}

bool ConnectionExec::NetHandleExec::ExecBindSocket(BindSocketContext *context)
{
    if (!context->IsParseOK()) {
        return false;
    }
    NetHandle handle(context->netId_);
    int32_t res = handle.BindSocket(context->socketFd_);
    if (res != NETMANAGER_SUCCESS) {
        NETMANAGER_BASE_LOGE("ExecBindSocket failed %{public}d", res);
        context->SetErrorCode(res);
    }
    return res == NETMANAGER_SUCCESS;
}

napi_value ConnectionExec::NetHandleExec::BindSocketCallback(BindSocketContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

void ConnectionExec::NetHandleExec::SetAddressInfo(const char *host, addrinfo *info, NetAddress &address)
{
    address.SetAddress(host);
    address.SetFamilyBySaFamily(info->ai_addr->sa_family);
    if (info->ai_addr->sa_family == AF_INET) {
        auto addr4 = reinterpret_cast<sockaddr_in *>(info->ai_addr);
        address.SetPort(addr4->sin_port);
    } else if (info->ai_addr->sa_family == AF_INET6) {
        auto addr6 = reinterpret_cast<sockaddr_in6 *>(info->ai_addr);
        address.SetPort(addr6->sin6_port);
    }
}

bool ConnectionExec::NetConnectionExec::ExecRegister(RegisterContext *context)
{
    auto manager = context->GetManager();
    auto conn = static_cast<NetConnection *>(manager->GetData());
    sptr<INetConnCallback> callback = conn->GetObserver();

    if (conn->hasNetSpecifier_ && conn->hasTimeout_) {
        sptr<NetSpecifier> specifier = new NetSpecifier(conn->netSpecifier_);
        int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(specifier, callback,
                                                                                              conn->timeout_);
        NETMANAGER_BASE_LOGI("Register result hasNetSpecifier_ and hasTimeout_ %{public}d", ret);
        context->SetErrorCode(ret);
        return ret == NETMANAGER_SUCCESS;
    }

    if (conn->hasNetSpecifier_) {
        sptr<NetSpecifier> specifier = new NetSpecifier(conn->netSpecifier_);
        int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(specifier, callback, 0);
        NETMANAGER_BASE_LOGI("Register result hasNetSpecifier_ %{public}d", ret);
        context->SetErrorCode(ret);
        return ret == NETMANAGER_SUCCESS;
    }

    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->RegisterNetConnCallback(callback);
    NETMANAGER_BASE_LOGI("Register result %{public}d", ret);
    context->SetErrorCode(ret);
    return ret == NETMANAGER_SUCCESS;
}

napi_value ConnectionExec::NetConnectionExec::RegisterCallback(RegisterContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::NetConnectionExec::ExecUnregister(UnregisterContext *context)
{
    auto manager = context->GetManager();
    auto conn = static_cast<NetConnection *>(manager->GetData());
    sptr<INetConnCallback> callback = conn->GetObserver();

    int32_t ret = DelayedSingleton<NetConnClient>::GetInstance()->UnregisterNetConnCallback(callback);
    NETMANAGER_BASE_LOGI("Unregister result %{public}d", ret);
    context->SetErrorCode(ret);
    return ret == NETMANAGER_SUCCESS;
}

napi_value ConnectionExec::NetConnectionExec::UnregisterCallback(RegisterContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::NetInterfaceExec::ExecIfaceRegister(IfaceRegisterContext *context)
{
    return NETMANAGER_ERR_INTERNAL;
}

napi_value ConnectionExec::NetInterfaceExec::IfaceRegisterCallback(IfaceRegisterContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool ConnectionExec::NetInterfaceExec::ExecIfaceUnregister(IfaceUnregisterContext *context)
{
    return NETMANAGER_ERR_INTERNAL;
}

napi_value ConnectionExec::NetInterfaceExec::IfaceUnregisterCallback(IfaceUnregisterContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

void ConnectionExec::FillLinkAddress(napi_env env, napi_value connectionProperties, NetLinkInfo *linkInfo)
{
    if (!linkInfo->netAddrList_.empty() && linkInfo->netAddrList_.size() <= MAX_ARRAY_LENGTH) {
        napi_value linkAddresses =
            NapiUtils::CreateArray(env, std::min(linkInfo->netAddrList_.size(), MAX_ARRAY_LENGTH));
        auto it = linkInfo->netAddrList_.begin();
        for (uint32_t index = 0; index < MAX_ARRAY_LENGTH && it != linkInfo->netAddrList_.end(); ++index, ++it) {
            napi_value netAddr = NapiUtils::CreateObject(env);
            NapiUtils::SetStringPropertyUtf8(env, netAddr, KEY_ADDRESS, it->address_);
            NapiUtils::SetUint32Property(env, netAddr, KEY_FAMILY, it->family_);
            NapiUtils::SetUint32Property(env, netAddr, KEY_PORT, it->port_);

            napi_value linkAddr = NapiUtils::CreateObject(env);
            NapiUtils::SetNamedProperty(env, linkAddr, KEY_ADDRESS, netAddr);
            NapiUtils::SetUint32Property(env, linkAddr, KEY_PREFIX_LENGTH, it->prefixlen_);
            NapiUtils::SetArrayElement(env, linkAddresses, index, linkAddr);
        }
        NapiUtils::SetNamedProperty(env, connectionProperties, KEY_LINK_ADDRESSES, linkAddresses);
    }
}

void ConnectionExec::FillRouoteList(napi_env env, napi_value connectionProperties, NetLinkInfo *linkInfo)
{
    if (!linkInfo->routeList_.empty() && linkInfo->routeList_.size() <= MAX_ARRAY_LENGTH) {
        napi_value routes = NapiUtils::CreateArray(env, std::min(linkInfo->routeList_.size(), MAX_ARRAY_LENGTH));
        auto it = linkInfo->routeList_.begin();
        for (uint32_t index = 0; index < MAX_ARRAY_LENGTH && it != linkInfo->routeList_.end(); ++index, ++it) {
            napi_value route = NapiUtils::CreateObject(env);
            NapiUtils::SetStringPropertyUtf8(env, route, KEY_INTERFACE, it->iface_);

            napi_value dest = NapiUtils::CreateObject(env);
            NapiUtils::SetStringPropertyUtf8(env, dest, KEY_ADDRESS, it->destination_.address_);
            NapiUtils::SetUint32Property(env, dest, KEY_PREFIX_LENGTH, it->destination_.prefixlen_);
            NapiUtils::SetNamedProperty(env, route, KEY_DESTINATION, dest);

            napi_value gateway = NapiUtils::CreateObject(env);
            NapiUtils::SetStringPropertyUtf8(env, gateway, KEY_ADDRESS, it->gateway_.address_);
            NapiUtils::SetUint32Property(env, gateway, KEY_PREFIX_LENGTH, it->gateway_.prefixlen_);
            NapiUtils::SetNamedProperty(env, route, KEY_GATE_WAY, gateway);

            NapiUtils::SetBooleanProperty(env, route, KEY_HAS_GET_WAY, it->hasGateway_);
            NapiUtils::SetBooleanProperty(env, route, KEY_IS_DEFAULT_ROUE, it->isDefaultRoute_);

            NapiUtils::SetArrayElement(env, routes, index, route);
        }
        NapiUtils::SetNamedProperty(env, connectionProperties, KEY_ROUTES, routes);
    }
}

void ConnectionExec::FillDns(napi_env env, napi_value connectionProperties, NetLinkInfo *linkInfo)
{
    if (!linkInfo->dnsList_.empty() && linkInfo->dnsList_.size() <= MAX_ARRAY_LENGTH) {
        napi_value dnsList = NapiUtils::CreateArray(env, std::min(linkInfo->dnsList_.size(), MAX_ARRAY_LENGTH));
        auto it = linkInfo->dnsList_.begin();
        for (uint32_t index = 0; index < MAX_ARRAY_LENGTH && it != linkInfo->dnsList_.end(); ++index, ++it) {
            napi_value netAddr = NapiUtils::CreateObject(env);
            NapiUtils::SetStringPropertyUtf8(env, netAddr, KEY_ADDRESS, it->address_);
            NapiUtils::SetUint32Property(env, netAddr, KEY_FAMILY, it->family_);
            NapiUtils::SetUint32Property(env, netAddr, KEY_PORT, it->port_);
            NapiUtils::SetArrayElement(env, dnsList, index, netAddr);
        }
        NapiUtils::SetNamedProperty(env, connectionProperties, KEY_DNSES, dnsList);
    }
}

bool ConnectionExec::ExecGetProxyMode(ProxyModeContext* context)
{
    int32_t errorCode = DelayedSingleton<NetConnClient>::GetInstance()->GetProxyMode(context->mode_);
    if (errorCode != NET_CONN_SUCCESS) {
        NETMANAGER_BASE_LOGE("exec GetProxyMode failed errorCode: %{public}d", errorCode);
        context->SetErrorCode(errorCode);
        return false;
    }
    return true;
}

bool ConnectionExec::ExecSetProxyMode(ProxyModeContext* context)
{
    int32_t errorCode = DelayedSingleton<NetConnClient>::GetInstance()->SetProxyMode(context->mode_);
    if (errorCode != NET_CONN_SUCCESS) {
        NETMANAGER_BASE_LOGE("exec SetProxyMode failed errorCode: %{public}d", errorCode);
        context->SetErrorCode(errorCode);
        return false;
    }
    return true;
}

napi_value ConnectionExec::GetProxyModeCallback(ProxyModeContext* context)
{
    return NapiUtils::CreateInt32(context->GetEnv(), context->mode_);
}

napi_value ConnectionExec::SetProxyModeCallback(ProxyModeContext* context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}
} // namespace OHOS::NetManagerStandard
