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
    auto mapping = PermissionMapper::Instance().ToAndroidPermissions(permission);
    if (!mapping.has_value()) {
        return false;
    }
    for (const auto& androidPermission : mapping.value()) {
        if (!AbilityAccessCtrlJni::CheckPermission(androidPermission)) {
            return false;
        }
    }
    return true;
}

void AbilityAccessCtrlImpl::RequestPermissions(
    const std::vector<std::string>& permissions, RequestPermissionCallback callback, void* data)
{
    LOGI("AbilityAccessCtrlImpl Request called");
    auto requestContext = PermissionMapper::Instance().BuildRequestContext(permissions);
    PluginUtilsInner::RunTaskOnPlatform([callback, data, requestContext]() {
        auto task = [callback, data, requestContext](const std::vector<std::string>& androidPermissions,
                       const std::vector<int>& androidGrantResults) {
            auto aggregated = PermissionMapper::Instance().AggregateGrantResults(
                requestContext, androidPermissions, androidGrantResults);
            callback(data, aggregated.ohPermissions, aggregated.grantResults);
        };
        PluginUtilsInner::JSRegisterGrantResult(task);
        LOGI("AbilityAccessCtrlImpl JSRegisterGrantResult end");
    });
    PluginUtilsInner::RunTaskOnPlatform([requestContext]() {
        std::vector<std::string> androidPermissions = requestContext.AndroidPermissionsToRequest();
        std::string requestedList;
        for (const auto& androidPermission : androidPermissions) {
            if (!requestedList.empty()) {
                requestedList += ",";
            }
            requestedList += androidPermission;
        }
        LOGI("AbilityAccessCtrlImpl request android permissions: %{public}s", requestedList.c_str());
        AbilityAccessCtrlJni::RequestPermissions(androidPermissions);
    });
}
} // namespace OHOS::Plugin
