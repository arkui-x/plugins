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
static std::map<std::string, std::vector<std::string>> g_permissionMultipleMap;
static void InitPermissionMap()
{
    // add permission map
    g_permissionMap["ohos.permission.CAMERA"] = "android.permission.CAMERA";
    g_permissionMap["ohos.permission.MICROPHONE"] = "android.permission.RECORD_AUDIO";
    g_permissionMap["ohos.permission.READ_IMAGEVIDEO"] = "android.permission.READ_EXTERNAL_STORAGE";
    g_permissionMap["ohos.permission.WRITE_IMAGEVIDEO"] = "android.permission.WRITE_EXTERNAL_STORAGE";
    g_permissionMap["ohos.permission.APPROXIMATELY_LOCATION"] = "android.permission.ACCESS_COARSE_LOCATION";
    g_permissionMap["ohos.permission.LOCATION"] = "android.permission.ACCESS_FINE_LOCATION";

    // add bluetooth permission
    std::vector<std::string> bluetoothPermission;
    bluetoothPermission.emplace_back("android.permission.BLUETOOTH");
    bluetoothPermission.emplace_back("android.permission.BLUETOOTH_ADMIN");
    bluetoothPermission.emplace_back("android.permission.BLUETOOTH_ADVERTISE");
    bluetoothPermission.emplace_back("android.permission.BLUETOOTH_CONNECT");
    bluetoothPermission.emplace_back("android.permission.BLUETOOTH_SCAN");
    bluetoothPermission.emplace_back("android.permission.ACCESS_FINE_LOCATION");
    bluetoothPermission.emplace_back("android.permission.ACCESS_COARSE_LOCATION");
    g_permissionMultipleMap.insert(std::make_pair("ohos.permission.ACCESS_BLUETOOTH", bluetoothPermission));
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

static bool OhPermissionToJava(const std::string& inPerm, std::vector<std::string>& outPerm)
{
    // outPerm = inPerm;
    auto it = g_permissionMultipleMap.find(inPerm);
    if (it != g_permissionMultipleMap.end()) {
        if (it->second.size() > 0) {
            outPerm = it->second;
            return true;
        }
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

static bool JavaMultiplePermissionToOh(const std::string& inPerm, std::string& outPerm)
{
    for (auto it = g_permissionMultipleMap.begin(); it != g_permissionMultipleMap.end(); it++) {
        for (auto vIter = it->second.begin(); vIter != it->second.end(); vIter++) {
            if (*vIter == inPerm) {
                outPerm = it->first;
                return true;
            }
        }
    }
    return false;
}

static void QueryJavaPermissionToOh(
    const std::vector<std::string>& perms, std::vector<std::string>& permList, std::vector<int>& grantResult)
{
    // 2: invalid operation, something is wrong or the app is not permmited to use the permission.
    const int permissionResult = 2;
    std::map<std::string, int> resultMap;
    for (size_t i = 0; i < perms.size(); i++) {
        std::string ohPerm;
        if (JavaPermissionToOh(perms[i], ohPerm)) {
            resultMap.insert(std::make_pair(ohPerm, grantResult[i]));
        } else if (JavaMultiplePermissionToOh(perms[i], ohPerm)) {
            auto iter = resultMap.find(ohPerm);
            if (iter != resultMap.end()) {
                iter->second = (iter->second == grantResult[i]) ? iter->second : permissionResult;
                continue;
            }
            resultMap.insert(std::make_pair(ohPerm, grantResult[i]));
        } else {
            grantResult[i] = permissionResult;
            resultMap.insert(std::make_pair(ohPerm, grantResult[i]));
        }
    }
    permList.clear();
    grantResult.clear();
    for (auto iter = resultMap.begin(); iter != resultMap.end(); iter++) {
        permList.emplace_back(iter->first);
        grantResult.emplace_back(iter->second);
    }
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
    std::vector<std::string> vJavaPerm;
    for (size_t i = 0; i < permissions.size(); i++) {
        std::string javaPerm;
        vJavaPerm.clear();
        if (OhPermissionToJava(permissions[i], javaPerm)) {
            LOGE(
                "AbilityAccessCtrlImpl transfer permisson %{public}s -> %{public}s", permissions[i].c_str(), javaPerm.c_str());
        } else if(OhPermissionToJava(permissions[i], vJavaPerm)) {
            for (auto it = vJavaPerm.begin(); it != vJavaPerm.end(); ++it) {
                javaStrings.emplace_back(*it);
            }
            continue;
        } else {
            LOGE("AbilityAccessCtrlImpl not found permisson(%{public}s) in map", permissions[i].c_str());
        }
        javaStrings.emplace_back(javaPerm);
    }
    PluginUtilsInner::RunTaskOnPlatform([callback, data]() {
        auto task = [callback, data](const std::vector<std::string> perms, const std::vector<int> result) {
            std::vector<std::string> permList;
            std::vector<int> grantResult = result;
            QueryJavaPermissionToOh(perms, permList, grantResult);
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
