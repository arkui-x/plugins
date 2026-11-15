/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "permission_mapper.h"

#include <algorithm>
#include <map>

#include "log.h"

namespace OHOS::Plugin {
namespace {
// 0/-1 align with android.content.pm.PackageManager and OH grant status;
// 2 is this plugin's marker for permissions without a mapping rule.
constexpr int PERMISSION_GRANTED = 0;
constexpr int PERMISSION_DENIED = -1;
constexpr int PERMISSION_INVALID = 2;

// One version segment of an OH permission mapping: effective from minSdkVersion
// (inclusive) until superseded by a segment with a higher minSdkVersion.
struct PermissionRule {
    std::string ohPermission;
    int minSdkVersion;
    std::vector<std::string> androidPermissions; // empty means no permission needed on this segment
};

// How to extend: when a new Android version changes a permission, append one row with
// the new minSdkVersion and leave old rows untouched. Row order does not matter;
// minSdkVersion must be unique within one ohPermission.
const std::vector<PermissionRule> PERMISSION_RULES = {
    {"ohos.permission.CAMERA", 0, {"android.permission.CAMERA"}},
    {"ohos.permission.MICROPHONE", 0, {"android.permission.RECORD_AUDIO"}},
    {"ohos.permission.READ_IMAGEVIDEO", 0, {"android.permission.READ_EXTERNAL_STORAGE"}},
    {"ohos.permission.READ_IMAGEVIDEO", 33,
        {"android.permission.READ_MEDIA_IMAGES", "android.permission.READ_MEDIA_VIDEO"}},
    // Real-device verified on Android 13: requesting WRITE_EXTERNAL_STORAGE is
    // silently denied, without any dialog ("has no effect" per the platform
    // reference). The legacy mapping is kept so the app sees Android's actual
    // grant result as-is.
    {"ohos.permission.WRITE_IMAGEVIDEO", 0, {"android.permission.WRITE_EXTERNAL_STORAGE"}},
    {"ohos.permission.LOCATION", 0, {"android.permission.ACCESS_FINE_LOCATION"}},
    {"ohos.permission.APPROXIMATELY_LOCATION", 0, {"android.permission.ACCESS_COARSE_LOCATION"}},
    // Never AND permissions from different Android versions: the legacy set
    // (BLUETOOTH/BLUETOOTH_ADMIN declared with maxSdkVersion="30" by apps
    // targeting 31+) is not granted on API 31+, while the runtime set does not
    // exist before API 31. The legacy segment keeps location because BLE scan
    // and discovery on Android <12 require it and the bluetooth plugin only
    // checks (never requests) location.
    {"ohos.permission.ACCESS_BLUETOOTH", 0,
        {"android.permission.BLUETOOTH", "android.permission.BLUETOOTH_ADMIN",
            "android.permission.ACCESS_FINE_LOCATION", "android.permission.ACCESS_COARSE_LOCATION"}},
    {"ohos.permission.ACCESS_BLUETOOTH", 31,
        {"android.permission.BLUETOOTH_ADVERTISE", "android.permission.BLUETOOTH_CONNECT",
            "android.permission.BLUETOOTH_SCAN"}},
};

// Picks the rule with the highest minSdkVersion that does not exceed the device version.
const PermissionRule* FindActiveRule(const std::string& ohPermission, int sdkVersion)
{
    const PermissionRule* activeRule = nullptr;
    for (const auto& rule : PERMISSION_RULES) {
        if (rule.ohPermission != ohPermission || rule.minSdkVersion > sdkVersion) {
            continue;
        }
        if (activeRule == nullptr || rule.minSdkVersion > activeRule->minSdkVersion) {
            activeRule = &rule;
        }
    }
    return activeRule;
}
} // namespace

PermissionMapper& PermissionMapper::Instance()
{
    static PermissionMapper instance;
    return instance;
}

PermissionMapper::PermissionMapper() = default;

void PermissionMapper::SetSdkVersion(int sdkVersion)
{
    sdkVersion_ = sdkVersion;
}

std::optional<std::vector<std::string>> PermissionMapper::ToAndroidPermissions(
    const std::string& ohPermission) const
{
    const PermissionRule* rule = FindActiveRule(ohPermission, sdkVersion_);
    if (rule == nullptr) {
        return std::nullopt;
    }
    return rule->androidPermissions;
}

PermissionMapper::RequestContext PermissionMapper::BuildRequestContext(
    const std::vector<std::string>& ohPermissions) const
{
    RequestContext requestContext;
    for (const auto& ohPermission : ohPermissions) {
        bool duplicated = false;
        for (const auto& plannedPermission : requestContext.requestedPermissions) {
            if (plannedPermission.ohPermission == ohPermission) {
                duplicated = true;
                break;
            }
        }
        if (duplicated) {
            continue;
        }
        RequestContext::RequestedPermission requested;
        requested.ohPermission = ohPermission;
        const PermissionRule* rule = FindActiveRule(ohPermission, sdkVersion_);
        requested.isMapped = rule != nullptr;
        if (rule != nullptr) {
            requested.androidPermissions = rule->androidPermissions;
        } else {
            LOGE("PermissionMapper not found permission(%{public}s) in map", ohPermission.c_str());
            requested.androidPermissions = {ohPermission};
        }
        for (const auto& androidPermission : requested.androidPermissions) {
            LOGI("PermissionMapper %{public}s -> %{public}s", ohPermission.c_str(), androidPermission.c_str());
        }
        requestContext.requestedPermissions.push_back(std::move(requested));
    }
    return requestContext;
}

std::vector<std::string> PermissionMapper::RequestContext::AndroidPermissionsToRequest() const
{
    std::vector<std::string> androidPermissions;
    for (const auto& requested : requestedPermissions) {
        for (const auto& androidPermission : requested.androidPermissions) {
            if (std::find(androidPermissions.begin(), androidPermissions.end(), androidPermission) ==
                androidPermissions.end()) {
                androidPermissions.push_back(androidPermission);
            }
        }
    }
    return androidPermissions;
}

PermissionMapper::OhGrantResults PermissionMapper::AggregateGrantResults(const RequestContext& requestContext,
    const std::vector<std::string>& androidPermissions, const std::vector<int>& androidGrantResults) const
{
    if (androidPermissions.size() != androidGrantResults.size()) {
        return {};
    }
    std::map<std::string, int> androidResultMap;
    for (size_t i = 0; i < androidPermissions.size(); i++) {
        androidResultMap[androidPermissions[i]] = androidGrantResults[i];
    }
    OhGrantResults results;
    for (const auto& requested : requestContext.requestedPermissions) {
        int grantResult = PERMISSION_INVALID;
        if (requested.isMapped) {
            grantResult = PERMISSION_GRANTED;
            for (const auto& androidPermission : requested.androidPermissions) {
                auto iter = androidResultMap.find(androidPermission);
                if (iter == androidResultMap.end() || iter->second != PERMISSION_GRANTED) {
                    grantResult = PERMISSION_DENIED;
                    break;
                }
            }
        }
        results.ohPermissions.push_back(requested.ohPermission);
        results.grantResults.push_back(grantResult);
    }
    return results;
}
} // namespace OHOS::Plugin
