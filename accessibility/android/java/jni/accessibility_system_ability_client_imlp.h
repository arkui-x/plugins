/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef PLUGIN_ACCESSIBILITY_ANDROID_ACCESSIBILITY_SYSTEM_ABILITY_CLIENT_IMPL_H
#define PLUGIN_ACCESSIBILITY_ANDROID_ACCESSIBILITY_SYSTEM_ABILITY_CLIENT_IMPL_H

#include "accessibility_system_ability_client_jni.h"

namespace OHOS::Plugin {
class AccessibilitySystemAbilityClientImpl {
public:
    AccessibilitySystemAbilityClientImpl() = default;
    ~AccessibilitySystemAbilityClientImpl() = default;

    static bool IsEnable();
    static bool RegisterAccessibilityStateListener();
    static void UnregisterAccessibilityStateListener();
    static bool IsTouchExplorationEnable();
    static bool RegisterTouchExplorationListener();
    static void UnregisterTouchExplorationListener();
    static std::vector<OHOS::Ace::Framework::AccessibilityAbilityInfo> GetAccessibilityExtensionListSync(
        const std::string& abilityType, const std::string& stateType);
};
} // namespace OHOS::Plugin
#endif // PLUGIN_ACCESSIBILITY_ANDROID_ACCESSIBILITY_SYSTEM_ABILITY_CLIENT_IMPL_H