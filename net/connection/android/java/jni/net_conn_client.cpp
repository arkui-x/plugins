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

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "net_manager_constants.h"
#include "plugins/net/connection/android/java/jni/net_conn_client_jni.h"

namespace OHOS::NetManagerStandard {
int32_t NetConnClient::RegisterNetConnCallback(const sptr<NetManagerStandard::INetConnCallback> &callback)
{
    return Plugin::NetConnClientJni::RegisterDefaultNetConnCallback(callback);
}

int32_t NetConnClient::RegisterNetConnCallback(const sptr<NetManagerStandard::NetSpecifier> &netSpecifier,
    const sptr<NetManagerStandard::INetConnCallback> &callback, const uint32_t &timeoutMS)
{
    return Plugin::NetConnClientJni::RegisterNetConnCallback(netSpecifier, callback, timeoutMS);
}

int32_t NetConnClient::UnregisterNetConnCallback(const sptr<NetManagerStandard::INetConnCallback> &callback)
{
    return Plugin::NetConnClientJni::UnregisterNetConnCallback(callback);
}

int32_t NetConnClient::HasDefaultNet(bool &flag)
{
    return Plugin::NetConnClientJni::HasDefaultNet(flag);
}
} // namespace OHOS::NetManagerStandard