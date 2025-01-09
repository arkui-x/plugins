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

#ifndef PLUGINS_BLUETOOTH_COMMON_MOCK_MOCK_SYSTEM_ABILITY_MANAGER_H
#define PLUGINS_BLUETOOTH_COMMON_MOCK_MOCK_SYSTEM_ABILITY_MANAGER_H

#include "iremote_object.h"
#include "isystem_ability_load_callback.h"
#include "refbase.h"
namespace OHOS {

class MockSystemAbilityManager : public RefBase {
public:
    MockSystemAbilityManager() = default;
    ~MockSystemAbilityManager() = default;

    sptr<IRemoteObject> GetSystemAbility(int serviceId);
    int32_t LoadSystemAbility(int32_t systemAbilityId, const sptr<ISystemAbilityLoadCallback>& callback);
};

} // namespace OHOS
#endif // PLUGINS_BLUETOOTH_COMMON_MOCK_MOCK_SYSTEM_ABILITY_MANAGER_H