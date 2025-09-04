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

#ifndef PLUGINS_NETMANAGER_CONNECTION_I_NET_CONN_CLIENT_H
#define PLUGINS_NETMANAGER_CONNECTION_I_NET_CONN_CLIENT_H

#include "i_net_conn_callback.h"
#include "net_specifier.h"
#include "net_conn_callback_observer.h"
#include "http_proxy.h"
#include "net_supplier_callback_base.h"
#include "net_supplier_info.h"
#include "net_conn_constants.h"

namespace OHOS::NetManagerStandard {
class NetConnClient {
public:
    NetConnClient() = default;
    ~NetConnClient() = default;

    int32_t RegisterNetConnCallback(const sptr<NetManagerStandard::INetConnCallback> &callback);
    int32_t RegisterNetConnCallback(const sptr<NetManagerStandard::NetSpecifier> &netSpecifier,
        const sptr<NetManagerStandard::INetConnCallback> &callback, const uint32_t &timeoutMS);
    int32_t UnregisterNetConnCallback(const sptr<NetManagerStandard::INetConnCallback> &callback);
    int32_t FactoryResetNetwork()
        { return 0; }
    int32_t SystemReady()
        { return 0; }
    int32_t RegisterNetSupplier(NetManagerStandard::NetBearType bearerType, const std::string &ident,
        const std::set<NetManagerStandard::NetCap> &netCaps, uint32_t &supplierId)
        { return 0; }
    int32_t UnregisterNetSupplier(uint32_t supplierId)
        { return 0; }
    int32_t RegisterNetSupplierCallback(uint32_t supplierId,
        const sptr<NetManagerStandard::NetSupplierCallbackBase> &callback)
        { return 0; }
    int32_t UpdateNetSupplierInfo(uint32_t supplierId,
        const sptr<NetManagerStandard::NetSupplierInfo> &netSupplierInfo)
        { return 0; }
    int32_t UpdateNetLinkInfo(uint32_t supplierId, const sptr<NetManagerStandard::NetLinkInfo> &netLinkInfo)
        { return 0; }
    int32_t GetDefaultNet(NetManagerStandard::NetHandle &netHandle)
        { return 0; }
    int32_t HasDefaultNet(bool &flag);
    int32_t GetAllNets(std::list<sptr<NetManagerStandard::NetHandle>> &netList)
        { return 0; }
    int32_t GetConnectionProperties(const NetManagerStandard::NetHandle &netHandle,
        NetManagerStandard::NetLinkInfo &info)
        { return 0; }
    int32_t GetNetCapabilities(const NetManagerStandard::NetHandle &netHandle,
        NetManagerStandard::NetAllCapabilities &netAllCap)
        { return 0; }
    int32_t GetAddressesByName(const std::string &host, int32_t netId,
        std::vector<NetManagerStandard::INetAddr> &addrList)
        { return 0; }
    int32_t GetAddressByName(const std::string &host, int32_t netId, NetManagerStandard::INetAddr &addr)
        { return 0; }
    int32_t BindSocket(int32_t socket_fd, int32_t netId)
        { return 0; }
    int32_t NetDetection(const NetManagerStandard::NetHandle &netHandle)
        { return 0; }
    int32_t SetAirplaneMode(bool state)
        { return 0; }
    int32_t IsDefaultNetMetered(bool &isMetered)
        { return 0; }
    int32_t SetGlobalHttpProxy(const NetManagerStandard::HttpProxy &httpProxy)
        { return 0; }
    int32_t GetGlobalHttpProxy(NetManagerStandard::HttpProxy &httpProxy)
        { return 0; }
    int32_t GetDefaultHttpProxy(HttpProxy &httpProxy)
        { return 0; }
    int32_t SetAppNet(int32_t netId)
        { return 0; }
    int32_t GetAppNet(int32_t &netId)
        { return 0; }
    int32_t GetNetIdByIdentifier(const std::string &ident, std::list<int32_t> &netIdList)
        { return 0; }
    int32_t SetProxyMode(const int mode)
        { return 0; }

    int32_t GetProxyMode(OHOS::NetManagerStandard::ProxyModeType &mode)
        { return 0; }
};
} // namespace OHOS::NetManagerStandard
#endif // PLUGINS_NETMANAGER_CONNECTION_I_NET_CONN_CLIENT_H
