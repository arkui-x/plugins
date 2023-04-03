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

#ifndef PLUGINS_IOS_ABILITYACCESSCTRL_ABILITY_ACCESS_CTRL_IMPL_H
#define PLUGINS_IOS_ABILITYACCESSCTRL_ABILITY_ACCESS_CTRL_IMPL_H

#include <memory>
#include "ability_access_ctrl_ios.h"
#include "plugins/ability_access_ctrl/ability_access_ctrl.h"

namespace OHOS::Plugin {
typedef void (*FuncType)(CallbackInfo* info, bool isLast, int granted);

class AbilityAccessCtrlImpl final : public AbilityAccessCtrl {
public:
    AbilityAccessCtrlImpl() = default;
    ~AbilityAccessCtrlImpl() override = default;

    bool CheckPermission(const std::string& permission) override;
    void RequestPermissions(
        const std::vector<std::string>& permissions, RequestPermissionCallback callback, void* data) override;
    void RequestPermission(const std::string& permission, CallbackInfo *cbInfo, bool isLast);
};
} // namespace OHOS::Plugin
#endif  // PLUGINS_IOS_ABILITYACCESSCTRL_ABILITY_ACCESS_CTRL_IMPL_H