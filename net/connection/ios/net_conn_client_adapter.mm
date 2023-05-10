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

#include "plugins/net/connection/ios/net_conn_client_adapter.h"

#include "net_manager_constants.h"
#include "net_status_observer.h"

static NetStatusObserver * netStatusObserver = [NetStatusObserver instanceNetStatusObserver];

namespace OHOS::Plugin {
std::map<sptr<NetManagerStandard::INetConnCallback>, sptr<NetManagerStandard::NetSpecifier>>
    NetConnClientAdapter::filter_;
std::vector<sptr<NetManagerStandard::INetConnCallback>> NetConnClientAdapter::observer_;
std::map<sptr<NetManagerStandard::INetConnCallback>, bool> NetConnClientAdapter::unavailable_;
NetworkStatus NetConnClientAdapter::networkStatus_ = NotReachable;

bool NetConnClientAdapter::CheckMatch(const sptr<NetManagerStandard::INetConnCallback> &callback,
    const NetManagerStandard::NetBearType &type)
{
    if (callback == nullptr) {
        return false;
    }

    auto iter = filter_.find(callback);
    if (iter != filter_.end()) {
        if (iter->second == nullptr) {
            return true;
        }
        for (auto it = iter->second->netCapabilities_.bearerTypes_.begin();
            it != iter->second->netCapabilities_.bearerTypes_.end(); it++) {
            if ((*it) == type) {
                return true;
            }
        }
    }

    return false;
}

NetManagerStandard::NetBearType NetConnClientAdapter::GetBearType(const NetworkStatus &status)
{
    if (status == ReachableViaWiFi) {
        return NetManagerStandard::BEARER_WIFI;
    } else if (status == ReachableViaWWAN) {
        return NetManagerStandard::BEARER_CELLULAR;
    } else {
        return NetManagerStandard::BEARER_DEFAULT;
    }
}

void NetConnClientAdapter::NotifyNetworkStatusChangedWitchCallback(NetStatusListen *listen,
        const sptr<NetManagerStandard::INetConnCallback> &callback)
{
    if (callback == nullptr) {
        return;
    }
    NetworkStatus netStatus = [listen getCurrentReachabilityStatus];
    sptr<NetManagerStandard::NetHandle> netHandle =
        std::make_unique<NetManagerStandard::NetHandle>(1001).release();
    switch (netStatus) {
        case NotReachable: {
            NetManagerStandard::NetBearType type = GetBearType(networkStatus_);
            bool idDefualt = false;
            if (networkStatus_ == NotReachable) {
                idDefualt = true;
            }
            if (CheckMatch(callback, type) || idDefualt) {
                if (unavailable_[callback]) {
                    callback->NetUnavailable();
                    unavailable_[callback] = false;
                } else {
                    callback->NetLost(netHandle);
                }
            }
            break;
        }
        case ReachableViaWiFi:
        case ReachableViaWWAN: {
            networkStatus_ = netStatus;
            NetManagerStandard::NetBearType type = GetBearType(netStatus);
            if (CheckMatch(callback, type)) {
                unavailable_[callback] = false;
                callback->NetAvailable(netHandle);
                sptr<NetManagerStandard::NetAllCapabilities> netAllCap =
                    std::make_unique<NetManagerStandard::NetAllCapabilities>().release();
                netAllCap->bearerTypes_.insert(type);
                callback->NetCapabilitiesChange(netHandle, netAllCap);
            } else {
                if (unavailable_[callback]) {
                    callback->NetUnavailable();
                    unavailable_[callback] = false;
                }
            }
            break;
        }
        default:
            break;
    }
}

void NetConnClientAdapter::NotifyNetworkStatusChanged(NetStatusListen *listen)
{
    for (auto iter = observer_.begin(); iter != observer_.end(); iter++) {
        sptr<NetManagerStandard::INetConnCallback> callback = *iter;
        NetConnClientAdapter::NotifyNetworkStatusChangedWitchCallback(listen, callback);
    }
}

int32_t NetConnClientAdapter::RegisterNetConnCallback(const sptr<NetManagerStandard::INetConnCallback> &callback)
{
    auto iter = filter_.find(callback);
    if (iter == filter_.end()) {
        filter_[callback] = nullptr;
    } 

    auto iter_bool = unavailable_.find(callback);
    if (iter_bool == unavailable_.end()) {
        unavailable_[callback] = true;
    }

    auto iter_observer = std::find(observer_.begin(), observer_.end(), callback);
    if (iter_observer == observer_.end()) {
        observer_.push_back(callback);
    } else {
        return NetManagerStandard::NETMANAGER_SUCCESS;
    }

    NetStatusListen *netStatusListen = [NetStatusListen reachabilityForInternetConnection];
    NetConnClientAdapter::NotifyNetworkStatusChangedWitchCallback(netStatusListen, callback);

    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t NetConnClientAdapter::RegisterNetConnCallback(const sptr<NetManagerStandard::NetSpecifier> &netSpecifier,
    const sptr<NetManagerStandard::INetConnCallback> &callback, const uint32_t &timeoutMS)
{
    auto iter = filter_.find(callback);
    if (iter == filter_.end()) {
        filter_[callback] = netSpecifier;
    }

    auto iter_bool = unavailable_.find(callback);
    if (iter_bool == unavailable_.end()) {
        unavailable_[callback] = true;
    }

    auto iter_observer = std::find(observer_.begin(), observer_.end(), callback);
    if (iter_observer == observer_.end()) {
        observer_.push_back(callback);
    } else {
        return NetManagerStandard::NETMANAGER_SUCCESS;
    }

    NetStatusListen *netStatusListen = [NetStatusListen reachabilityForInternetConnection];
    NetConnClientAdapter::NotifyNetworkStatusChangedWitchCallback(netStatusListen, callback);

    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t NetConnClientAdapter::UnregisterNetConnCallback(const sptr<NetManagerStandard::INetConnCallback> &callback)
{
    auto iter = filter_.find(callback);
    if (iter != filter_.end()) {
        filter_.erase(iter);
    } 

    auto iter_bool = unavailable_.find(callback);
    if (iter_bool != unavailable_.end()) {
        unavailable_.erase(iter_bool);
    }

    auto iter_observer = std::find(observer_.begin(), observer_.end(), callback);
    if (iter_observer != observer_.end()) {
        observer_.erase(iter_observer);
    }

    return NetManagerStandard::NETMANAGER_SUCCESS;
}

NetworkStatus NetConnClientAdapter::GetNetworkStatus()
{
    NetStatusListen *netStatusListen = [NetStatusListen reachabilityForInternetConnection];
    return [netStatusListen getCurrentReachabilityStatus];
}

int32_t NetConnClientAdapter::HasDefaultNet(bool &flag)
{
    if (GetNetworkStatus() == NotReachable) {
        flag = false;
    } else {
        flag = true;
    }
    return NetManagerStandard::NETMANAGER_SUCCESS;
}
} // namespace OHOS::Plugin