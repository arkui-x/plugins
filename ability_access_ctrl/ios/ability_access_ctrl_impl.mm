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

#include "plugins/ability_access_ctrl/ios/ability_access_ctrl_impl.h"

#include <memory>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"

namespace OHOS::Plugin {
static const std::string CAMERA_PERMISSION = "ohos.permission.CAMERA";
static const std::string MICROPHONE_PERMISSION = "ohos.permission.MICROPHONE";
static const std::string PHOTO_PERMISSION = "ohos.permission.READ_IMAGEVIDEO";

std::unique_ptr<AbilityAccessCtrl> AbilityAccessCtrl::Create()
{
    return std::make_unique<AbilityAccessCtrlImpl>();
}

bool AbilityAccessCtrlImpl::CheckPermission(const std::string& permission)
{
    if (permission == CAMERA_PERMISSION) {
        bool res = [[abilityAccessCtrlIOS shareinstance] CheckCameraPermission];
        LOGI(" %{public}s check result %{public}d.", permission.c_str(), res);
        return res;
    }
    if (permission == MICROPHONE_PERMISSION) {
        bool res =  [[abilityAccessCtrlIOS shareinstance] CheckMicrophonePermission];
        LOGI(" %{public}s check result %{public}d.", permission.c_str(), res);
        return res;
    }
    if (permission == PHOTO_PERMISSION) {
        bool res =  [[abilityAccessCtrlIOS shareinstance] CheckPhotoPermission];
        LOGI(" %{public}s check result %{public}d.", permission.c_str(), res);
        return res;
    }
    return false;
}

static void IosCallback(CallbackInfo* info, bool isLast, int result)
{
    LOGI("IosCallback: permissions, grantResults %{public}d. %{public}d", info->index, result);
    if (info == nullptr) {
        LOGE("info is null.");
        return;
    }

    info->grantResults.emplace_back(result);
    if (isLast) {
        std::vector<std::string> permissions;
        std::vector<int> grantResults;
        for (size_t i = 0; i <= info->index; i++) {
            LOGI("permission, grantResult %{public}s %{public}d.",
                info->permissionList[i].c_str(), info->grantResults[i]);
            permissions.emplace_back(info->permissionList[i]);
            grantResults.emplace_back(info->grantResults[i]);
        }
        RequestPermissionCallback callback = info->napiCb;
        auto data = info->data;
        callback(data, permissions, grantResults);
        delete info;
    }
}

void AbilityAccessCtrlImpl::RequestPermission(
    const std::string& permission, CallbackInfo* cbInfo, bool isLast)
{
    LOGI("AbilityAccessCtrlImpl %{public}s %{public}d", permission.c_str(), cbInfo->index);

    FuncType func = IosCallback;
    if (permission == CAMERA_PERMISSION) {
        [[abilityAccessCtrlIOS shareinstance] RequestCameraPermission:func :cbInfo :isLast];
        return;
    }
    if (permission == MICROPHONE_PERMISSION) {
        [[abilityAccessCtrlIOS shareinstance] RequestMicrophonePermission:func :cbInfo :isLast];
        return;
    }
    if (permission == PHOTO_PERMISSION) {
        [[abilityAccessCtrlIOS shareinstance] RequestPhotoPermission:func :cbInfo :isLast];
        return;
    }
    IosCallback(cbInfo, isLast, GrantResultType::INVALID_OPER);
    return;
}

void AbilityAccessCtrlImpl::RequestPermissions(
    const std::vector<std::string>& permissions, RequestPermissionCallback callback, void* data)
{
    LOGI("AbilityAccessCtrlImpl Request called");
    CallbackInfo *info = new (std::nothrow) CallbackInfo;
    if (info == nullptr) {
        LOGE("Info malloc failed.");
        return;
    }
    info->data = data;
    info->index = 0;
    info->napiCb = callback;
    info->permissionList.assign(permissions.begin(), permissions.end());

    for (info->index = 0; info->index < permissions.size() - 1; info->index++) {
        RequestPermission(permissions[info->index], info, false);
    }
    LOGI("index Request %{public}d", info->index);
    RequestPermission(permissions[info->index], info, true);
}
} // namespace OHOS::Plugin
