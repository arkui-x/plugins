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

#ifndef PLUGINS_BLUETOOTH_COMMON_MOCK_SYSTEM_ABILITY_ON_DEMAND_EVENT_H
#define PLUGINS_BLUETOOTH_COMMON_MOCK_SYSTEM_ABILITY_ON_DEMAND_EVENT_H

#include <string>
#include <vector>

#include "message_parcel.h"

namespace OHOS {
enum class OnDemandPolicyType {
    START_POLICY = 0,
    STOP_POLICY = 1,
};

enum class OnDemandEventId {
    DEVICE_ONLINE = 1,
    SETTING_SWITCH = 2,
    PARAM = 3,
    COMMON_EVENT = 4,
    TIMED_EVENT = 5,
};

struct SystemAbilityOnDemandCondition {
    OnDemandEventId eventId;
    std::string name;
    std::string value;
};

struct SystemAbilityOnDemandEvent {
    OnDemandEventId eventId;
    std::string name;
    std::string value;
    bool persistence = false;
    std::vector<SystemAbilityOnDemandCondition> conditions;
    bool enableOnce = false;
};

class OnDemandEventToParcel {
public:
    static bool WriteOnDemandEventsToParcel(
        const std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents, MessageParcel& data);
    static bool WriteOnDemandEventToParcel(const SystemAbilityOnDemandEvent& event, MessageParcel& data);
    static bool WriteOnDemandConditionToParcel(const SystemAbilityOnDemandCondition& condition, MessageParcel& data);
    static bool ReadOnDemandEventsFromParcel(
        std::vector<SystemAbilityOnDemandEvent>& abilityOnDemandEvents, MessageParcel& reply);
    static bool ReadOnDemandEventFromParcel(SystemAbilityOnDemandEvent& event, MessageParcel& reply);
    static bool ReadOnDemandConditionFromParcel(SystemAbilityOnDemandCondition& condition, MessageParcel& reply);
};
} // namespace OHOS
#endif // PLUGINS_BLUETOOTH_COMMON_MOCK_SYSTEM_ABILITY_ON_DEMAND_EVENT_H