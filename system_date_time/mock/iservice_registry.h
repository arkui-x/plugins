/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef PLUGIN_TIME_MOCK_BUNDLE_ACTIVE_INCLUDE_H
#define PLUGIN_TIME_MOCK_BUNDLE_ACTIVE_INCLUDE_H

#include <iremote_broker.h>
#include "if_system_ability_manager.h"

namespace OHOS {
class SystemAbilityManagerClient {
public:
    static SystemAbilityManagerClient& GetInstance();

    sptr<ISystemAbilityManager> GetSystemAbilityManager();
    SystemAbilityManagerClient() = default;
    ~SystemAbilityManagerClient() = default;

    sptr<ISystemAbilityManager> systemAbilityManager_;
    std::string action_ {""};
    std::mutex systemAbilityManagerLock_;
};
}  // namespace OHOS
#endif  // PLUGIN_TIME_MOCK_BUNDLE_ACTIVE_INCLUDE_H
