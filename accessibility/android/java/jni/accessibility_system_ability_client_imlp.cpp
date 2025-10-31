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

#include "accessibility_system_ability_client_imlp.h"

using namespace OHOS::Ace::Framework;

namespace OHOS::Plugin {
bool AccessibilitySystemAbilityClientImpl::IsEnable()
{
    return AccessibilityClientJni::IsEnable();
}

bool AccessibilitySystemAbilityClientImpl::RegisterAccessibilityStateListener()
{
    return AccessibilityClientJni::RegisterStateListener();
}

void AccessibilitySystemAbilityClientImpl::UnregisterAccessibilityStateListener()
{
    AccessibilityClientJni::UnregisterStateListener();
}

bool AccessibilitySystemAbilityClientImpl::IsTouchExplorationEnable()
{
    return AccessibilityClientJni::IsTouchExplorationEnable();
}

bool AccessibilitySystemAbilityClientImpl::RegisterTouchExplorationListener()
{
    return AccessibilityClientJni::RegisterTouchExplorationListener();
}

void AccessibilitySystemAbilityClientImpl::UnregisterTouchExplorationListener()
{
    AccessibilityClientJni::UnregisterTouchExplorationListener();
}

std::vector<AccessibilityAbilityInfo> AccessibilitySystemAbilityClientImpl::GetAccessibilityExtensionListSync(
    const std::string& abilityType, const std::string& stateType)
{
    return AccessibilityClientJni::GetAccessibilityExtensionListSync(abilityType, stateType);
}
} // namespace OHOS::Plugin