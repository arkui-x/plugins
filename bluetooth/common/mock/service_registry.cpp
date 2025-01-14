/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <unistd.h>

#include "errors.h"
#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_object.h"
#include "iremote_proxy.h"
#include "iservice_registry.h"
#include "message_option.h"
#include "message_parcel.h"
#include "refbase.h"
#include "string_ex.h"

namespace OHOS {

SystemAbilityManagerClient& SystemAbilityManagerClient::GetInstance()
{
    static auto instance = new SystemAbilityManagerClient();
    return *instance;
}

sptr<ISystemAbilityManager> SystemAbilityManagerClient::GetSystemAbilityManager()
{
    std::lock_guard<std::mutex> lock(systemAbilityManagerLock_);
    if (systemAbilityManager_ != nullptr) {
        return systemAbilityManager_;
    }
    sptr<IRemoteObject> registryObject = new (std::nothrow) SystemAbilityManager();
    if (registryObject == nullptr) {
        return nullptr;
    }
    systemAbilityManager_ = iface_cast<ISystemAbilityManager>(registryObject);
    if (systemAbilityManager_ == nullptr) {
    }
    return systemAbilityManager_;
}

void SystemAbilityManagerClient::DestroySystemAbilityManagerObject()
{
    std::lock_guard<std::mutex> lock(systemAbilityManagerLock_);
    systemAbilityManager_.clear();
}
} // namespace OHOS
