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
static const std::string LOCATION_PERMISSION = "ohos.permission.LOCATION";
static const std::string APPROXIMATELY_LOCATION_PERMISSION = "ohos.permission.APPROXIMATELY_LOCATION";

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
    if (permission == LOCATION_PERMISSION ||
        permission == APPROXIMATELY_LOCATION_PERMISSION) {
        bool res = [[abilityAccessCtrlIOS shareinstance] CheckLocationPermission:permission == LOCATION_PERMISSION];
        LOGI(" %{public}s check result %{public}d.", permission.c_str(), res);
        return res;
    }
    return false;
}

static void RequestPermission(CallbackInfo* info);

static void IosCallback(CallbackInfo* info, bool isLast, int result)
{
    if (info == nullptr) {
        LOGE("info is null.");
        return;
    }
    LOGI("IosCallback: permissions, grantResults %{public}zu. %{public}d", info->index, result);

    info->grantResults.emplace_back(result);
    if (!isLast) {
        ++info->index;
        RequestPermission(info);
        return;
    }

    RequestPermissionCallback callback = info->napiCb;
    auto data = info->data;
    callback(data, info->permissionList, info->grantResults);
    delete info;
}

static void RequestPermission(CallbackInfo* cbInfo)
{
    const std::string& permission = cbInfo->permissionList[cbInfo->index];
    bool isLast = cbInfo->index + 1 == cbInfo->permissionList.size();
    LOGI("AbilityAccessCtrlImpl %{public}s %{public}zu", permission.c_str(), cbInfo->index);

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
    if (permission == LOCATION_PERMISSION ||
        permission == APPROXIMATELY_LOCATION_PERMISSION) {
        [[abilityAccessCtrlIOS shareinstance] RequestLocationPermission:permission == LOCATION_PERMISSION
            callback:func data:cbInfo isLast:isLast];
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

    RequestPermission(info);
}
} // namespace OHOS::Plugin
