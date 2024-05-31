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

#ifndef PLUGINS_NET_CONNECTION_IOS_NET_CONN_CLIENT_ADAPTER_H
#define PLUGINS_NET_CONNECTION_IOS_NET_CONN_CLIENT_ADAPTER_H

#include <map>
#include <vector>
#include "net_conn_callback_observer.h"
#include "net_all_capabilities.h"
#import "net_status_listen.h"

namespace OHOS::Plugin {
class NetConnClientAdapter final {
public:
    NetConnClientAdapter() = default;
    ~NetConnClientAdapter() = default;

    int32_t RegisterNetConnCallback(const sptr<NetManagerStandard::INetConnCallback> &callback);
    int32_t RegisterNetConnCallback(const sptr<NetManagerStandard::NetSpecifier> &netSpecifier,
        const sptr<NetManagerStandard::INetConnCallback> &callback, const uint32_t &timeoutMS);
    int32_t UnregisterNetConnCallback(const sptr<NetManagerStandard::INetConnCallback> &callback);
    int32_t HasDefaultNet(bool &flag);
    NetworkStatus GetNetworkStatus();

    static void NotifyNetworkStatusChanged(NetStatusListen *listen);
private:
    static bool CheckMatch(const sptr<NetManagerStandard::INetConnCallback> &callback,
        const NetManagerStandard::NetBearType &type);
    static NetManagerStandard::NetBearType GetBearType(const NetworkStatus &status);
    static void NotifyNetworkStatusChangedWitchCallback(NetStatusListen *listen,
        const sptr<NetManagerStandard::INetConnCallback> &callback);

    static std::map<sptr<NetManagerStandard::INetConnCallback>, sptr<NetManagerStandard::NetSpecifier>> filter_;
    static std::vector<sptr<NetManagerStandard::INetConnCallback>> observer_;
    static std::map<sptr<NetManagerStandard::INetConnCallback>, bool> unavailable_;
    static NetworkStatus networkStatus_;
};
} // namespace OHOS::Plugin
#endif // PLUGINS_NET_CONNECTION_IOS_NET_CONN_CLIENT_ADAPTER_H