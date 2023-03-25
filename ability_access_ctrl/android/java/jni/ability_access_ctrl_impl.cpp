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

#include "ability_access_ctrl_impl.h"
#include "ability_access_ctrl_jni.h"
#include "log.h"
#include "plugin_utils.h"

namespace OHOS::Plugin {
std::unique_ptr<AbilityAccessCtrl> AbilityAccessCtrl::Create()
{
    return std::make_unique<AbilityAccessCtrlImpl>();
}

bool AbilityAccessCtrlImpl::CheckPermission(const std::string& permission)
{
    LOGI("AbilityAccessCtrlImpl Check called");
    return AbilityAccessCtrlJni::CheckPermission(permission);
}

void AbilityAccessCtrlImpl::RequestPermissions(
    const std::vector<std::string>& permissions, RequestPermissionCallback callback, void* data)
{
    LOGI("AbilityAccessCtrlImpl Request called");
    PluginUtils::RunTaskOnPlatform([permissions, callback, data]() {
        auto task = [permissions, callback, data](const std::vector<std::string> perms, const std::vector<int> result) {
            callback(data, permissions, result);
        };
        PluginUtils::JSRegisterGrantResult(task);
        LOGI("AbilityAccessCtrlImpl JSRegisterGrantResult end");
    });
    PluginUtils::RunTaskOnPlatform([permissions]() {
        AbilityAccessCtrlJni::RequestPermissions(permissions);
    });
}
} // namespace OHOS::Plugin
