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
    g_permissionMap["ohos.permission.READ_IMAGEVIDEO"] = "android.permission.READ_EXTERNAL_STORAGE";
    g_permissionMap["ohos.permission.WRITE_IMAGEVIDEO"] = "android.permission.WRITE_EXTERNAL_STORAGE";
}

static bool OhPermissionToJava(const std::string& inPerm, std::string& outPerm)
{
    outPerm = inPerm;
    auto it = g_permissionMap.find(inPerm);
    if (it != g_permissionMap.end()) {
        outPerm = it->second;
        return true;
    }
    return false;
}

static bool JavaPermissionToOh(const std::string& inPerm, std::string& outPerm)
{
    outPerm = inPerm;
    for (auto iter = g_permissionMap.begin(); iter != g_permissionMap.end(); ++iter) {
        std::string tmp = iter->second;
        if (tmp == inPerm) {
            outPerm = iter->first;
            return true;
        }
    }
    return false;
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
    LOGI("AbilityAccessCtrlImpl CheckPermission %{public}s", permission.c_str());
    std::string javaPermission;
    if (!OhPermissionToJava(permission, javaPermission)) {
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
        std::string javaPerm;
        if (!OhPermissionToJava(permissions[i], javaPerm)) {
            LOGE("AbilityAccessCtrlImpl not found permisson(%{public}s) in map", permissions[i].c_str());
        } else {
            LOGE(
                "AbilityAccessCtrlImpl transfer permisson %{public}s -> %{public}s", permissions[i].c_str(), javaPerm.c_str());
        }
        javaStrings.emplace_back(javaPerm);
    }
    PluginUtilsInner::RunTaskOnPlatform([callback, data]() {
        auto task = [callback, data](const std::vector<std::string> perms, const std::vector<int> result) {
            std::vector<std::string> permList;
            std::vector<int> grantResult = result;
            for (size_t i = 0; i < perms.size(); i++) {
                std::string ohPerm;
                if (!JavaPermissionToOh(perms[i], ohPerm)) {
                    // 2: invalid operation, something is wrong or the app is not permmited to use the permission.
                    grantResult[i] = 2;
                }
                permList.emplace_back(ohPerm);
            }
            callback(data, permList, grantResult);
        };
        PluginUtilsInner::JSRegisterGrantResult(task);
        LOGI("AbilityAccessCtrlImpl JSRegisterGrantResult end");
    });
    PluginUtilsInner::RunTaskOnPlatform([javaStrings]() {
        AbilityAccessCtrlJni::RequestPermissions(javaStrings);
    });
}
} // namespace OHOS::Plugin
