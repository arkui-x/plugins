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

#ifndef PLUGINS_ABILITY_ACCESS_CTRL_ANDROID_JNI_PERMISSION_MAPPER_H
#define PLUGINS_ABILITY_ACCESS_CTRL_ANDROID_JNI_PERMISSION_MAPPER_H

#include <atomic>
#include <optional>
#include <string>
#include <vector>

namespace OHOS::Plugin {
// Version-aware permission mapping between OH and Android.
// One OH permission can map to different Android permissions on different Android
// versions (e.g. READ_EXTERNAL_STORAGE is deprecated since API 33); the active rule
// is selected by the effective SDK version. See PERMISSION_RULES in
// permission_mapper.cpp for how to extend the table.
class PermissionMapper {
public:
    static PermissionMapper& Instance();

    // Injected by the JNI layer at plugin construction: the lower of the device
    // SDK version and the app targetSdkVersion, since permission deprecations take
    // effect by targetSdkVersion while new permissions exist by device version.
    // 0 (unset) makes every permission fall back to its lowest version segment.
    void SetSdkVersion(int sdkVersion);

    // Carried from request to grant-result aggregation: every requested OH permission
    // keeps its mapped Android permissions, so results can be aggregated back by the
    // context itself instead of a reverse lookup, which is ambiguous when several OH
    // permissions share the same Android permission.
    struct RequestContext {
        struct RequestedPermission {
            std::string ohPermission;
            std::vector<std::string> androidPermissions; // all required, AND semantics
            bool isMapped = false; // false when no rule matches: requested as-is, result reported invalid (2)
        };
        std::vector<RequestedPermission> requestedPermissions; // deduplicated, keeps request order

        // Flattened and deduplicated, so Android permissions shared by several
        // OH permissions are requested only once.
        std::vector<std::string> AndroidPermissionsToRequest() const;
    };

    // 0 = granted, -1 = denied, 2 = invalid (no mapping rule for the permission).
    struct OhGrantResults {
        std::vector<std::string> ohPermissions;
        std::vector<int> grantResults;
    };

    std::optional<std::vector<std::string>> ToAndroidPermissions(const std::string& ohPermission) const;

    RequestContext BuildRequestContext(const std::vector<std::string>& ohPermissions) const;

    // An OH permission is granted (0) only when all its mapped Android permissions are
    // granted, otherwise denied (-1); unmapped permissions are reported invalid (2).
    // Returns empty on input size mismatch.
    OhGrantResults AggregateGrantResults(const RequestContext& requestContext,
        const std::vector<std::string>& androidPermissions,
        const std::vector<int>& androidGrantResults) const;

private:
    PermissionMapper();
    PermissionMapper(const PermissionMapper&) = delete;
    PermissionMapper& operator=(const PermissionMapper&) = delete;
    PermissionMapper(PermissionMapper&&) = delete;
    PermissionMapper& operator=(PermissionMapper&&) = delete;

    std::atomic<int> sdkVersion_ {0};
};
} // namespace OHOS::Plugin
#endif // PLUGINS_ABILITY_ACCESS_CTRL_ANDROID_JNI_PERMISSION_MAPPER_H
