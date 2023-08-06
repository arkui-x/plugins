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
#include <map>
#include "ability_access_ctrl_jni.h"
#include "inner_api/plugin_utils_inner.h"
#include "log.h"

namespace OHOS::Plugin {
static bool isInited = false;
static std::map<std::string, std::string> g_permissionMap;
static void InitPermissionMap()
{
    // add permission map
    g_permissionMap["ohos.permission.CAMERA"] = "android.permission.CAMERA";
    g_permissionMap["ohos.permission.MICROPHONE"] = "android.permission.RECORD_AUDIO";
}

static std::string OhPermissionToJava(const std::string& permission)
{
    auto it = g_permissionMap.find(permission);
    if (it != g_permissionMap.end()) {
        return it->second;
    }
    return "";
}

std::unique_ptr<AbilityAccessCtrl> AbilityAccessCtrl::Create()
{
    if (!isInited) {
        InitPermissionMap();
        isInited = true;
    }
    
    return std::make_unique<AbilityAccessCtrlImpl>();
}

bool AbilityAccessCtrlImpl::CheckPermission(const std::string& permission)
{
    LOGI("AbilityAccessCtrlImpl Check called");
    std::string javaPermission = OhPermissionToJava(permission);
    if (javaPermission.empty()) {
        return false;
    }
    return AbilityAccessCtrlJni::CheckPermission(javaPermission);
}

void AbilityAccessCtrlImpl::RequestPermissions(
    const std::vector<std::string>& permissions, RequestPermissionCallback callback, void* data)
{
    LOGI("AbilityAccessCtrlImpl Request called");
    std::vector<std::string> javaStrings;
    for (size_t i = 0; i < permissions.size(); i++) {
        javaStrings.emplace_back(OhPermissionToJava(permissions[i]));
    }
    PluginUtilsInner::RunTaskOnPlatform([permissions, callback, data]() {
        auto task = [permissions, callback, data](const std::vector<std::string> perms, const std::vector<int> result) {
            std::vector<int> grantResult = result;
            for (size_t i = 0; i < permissions.size(); i++) {
                if (OhPermissionToJava(permissions[i]).empty()) {
                    // 2: invalid operation, something is wrong or the app is not permmited to use the permission.
                    grantResult[i] = 2;
                }
            }
            callback(data, permissions, grantResult);
        };
        PluginUtilsInner::JSRegisterGrantResult(task);
        LOGI("AbilityAccessCtrlImpl JSRegisterGrantResult end");
    });
    PluginUtilsInner::RunTaskOnPlatform([javaStrings]() {
        AbilityAccessCtrlJni::RequestPermissions(javaStrings);
    });
}
} // namespace OHOS::Plugin
