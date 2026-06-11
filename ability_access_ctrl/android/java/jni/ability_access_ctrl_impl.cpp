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
#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "permission_mapper.h"

namespace OHOS::Plugin {
std::unique_ptr<AbilityAccessCtrl> AbilityAccessCtrl::Create()
{
    return std::make_unique<AbilityAccessCtrlImpl>();
}

bool AbilityAccessCtrlImpl::CheckPermission(const std::string& permission)
{
    LOGI("AbilityAccessCtrlImpl CheckPermission %{public}s", permission.c_str());
    auto mapping = PermissionMapper::Instance().ToAndroidSingle(permission);
    if (!mapping.found) {
        return false;
    }
    return AbilityAccessCtrlJni::CheckPermission(mapping.androidPermission);
}

void AbilityAccessCtrlImpl::RequestPermissions(
    const std::vector<std::string>& permissions, RequestPermissionCallback callback, void* data)
{
    LOGI("AbilityAccessCtrlImpl Request called");
    std::vector<std::string> javaStrings;
    for (const auto& perm : permissions) {
        auto singleMapping = PermissionMapper::Instance().ToAndroidSingle(perm);
        if (singleMapping.found) {
            LOGE("AbilityAccessCtrlImpl transfer permission %{public}s -> %{public}s",
                perm.c_str(), singleMapping.androidPermission.c_str());
            javaStrings.emplace_back(singleMapping.androidPermission);
            continue;
        }
        auto multiMapping = PermissionMapper::Instance().ToAndroidMulti(perm);
        if (multiMapping.found) {
            for (const auto& javaPerm : multiMapping.androidPermissions) {
                javaStrings.emplace_back(javaPerm);
            }
            continue;
        }
        LOGE("AbilityAccessCtrlImpl not found permission(%{public}s) in map", perm.c_str());
        javaStrings.emplace_back(perm);
    }
    PluginUtilsInner::RunTaskOnPlatform([callback, data]() {
        auto task = [callback, data](const std::vector<std::string> perms, const std::vector<int> result) {
            auto aggregated = PermissionMapper::Instance().AggregateToOh(perms, result);
            callback(data, aggregated.ohPermissions, aggregated.grantResults);
        };
        PluginUtilsInner::JSRegisterGrantResult(task);
        LOGI("AbilityAccessCtrlImpl JSRegisterGrantResult end");
    });
    PluginUtilsInner::RunTaskOnPlatform([javaStrings]() {
        AbilityAccessCtrlJni::RequestPermissions(javaStrings);
    });
}
} // namespace OHOS::Plugin
