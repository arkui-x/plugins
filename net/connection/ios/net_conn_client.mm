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

#include "plugins/net/connection/net_conn_client.h"

#include <memory>
#include "plugins/net/connection/ios/net_conn_client_adapter.h"

namespace OHOS::NetManagerStandard {
int32_t NetConnClient::RegisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    std::shared_ptr<Plugin::NetConnClientAdapter> adapter = std::make_shared<Plugin::NetConnClientAdapter>();
    return adapter->RegisterNetConnCallback(callback);
}

int32_t NetConnClient::RegisterNetConnCallback(const sptr<NetSpecifier> &netSpecifier,
    const sptr<INetConnCallback> &callback, const uint32_t &timeoutMS)
{
    std::shared_ptr<Plugin::NetConnClientAdapter> adapter = std::make_shared<Plugin::NetConnClientAdapter>();
    return adapter->RegisterNetConnCallback(netSpecifier, callback, timeoutMS);
}

int32_t NetConnClient::UnregisterNetConnCallback(const sptr<INetConnCallback> &callback)
{
    std::shared_ptr<Plugin::NetConnClientAdapter> adapter = std::make_shared<Plugin::NetConnClientAdapter>();
    return adapter->UnregisterNetConnCallback(callback);
}

int32_t NetConnClient::HasDefaultNet(bool &flag)
{
    std::shared_ptr<Plugin::NetConnClientAdapter> adapter = std::make_shared<Plugin::NetConnClientAdapter>();
    return adapter->HasDefaultNet(flag);
}
} // namespace OHOS::NetManagerStandard