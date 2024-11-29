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
#include "iservice_registry.h"
#include "isystem_ability_load_callback.h"
#include "mock_system_ability_manager.h"
namespace OHOS {

SystemAbilityManagerClient& SystemAbilityManagerClient::GetInstance()
{
    static auto instance = new SystemAbilityManagerClient();
    return *instance;
}

sptr<MockSystemAbilityManager> SystemAbilityManagerClient::GetSystemAbilityManager()
{
    std::lock_guard<std::mutex> lock(systemAbilityManagerLock_);

    if (systemAbilityManager_ != nullptr) {
        return systemAbilityManager_;
    }
    systemAbilityManager_ = new MockSystemAbilityManager();
    return systemAbilityManager_;
}

void SystemAbilityManagerClient::DestroySystemAbilityManagerObject()
{
    std::lock_guard<std::mutex> lock(systemAbilityManagerLock_);
    systemAbilityManager_.clear();
}

sptr<IRemoteObject> MockSystemAbilityManager::GetSystemAbility(int serviceId)
{
    return nullptr;
}
int32_t MockSystemAbilityManager::LoadSystemAbility(
    int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback>& callback)
{
    return 0;
}
} // namespace OHOS