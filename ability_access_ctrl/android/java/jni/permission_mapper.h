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

#ifndef PLUGINS_ABILITY_ACCESS_CTRL_ANDROID_JAVA_JNI_PERMISSION_MAPPER_H
#define PLUGINS_ABILITY_ACCESS_CTRL_ANDROID_JAVA_JNI_PERMISSION_MAPPER_H

#include <map>
#include <string>
#include <vector>

namespace OHOS::Plugin {
class PermissionMapper {
public:
    static PermissionMapper& Instance();

    struct SingleResult {
        std::string androidPermission;
        bool found = false;
    };

    struct MultiResult {
        std::vector<std::string> androidPermissions;
        bool found = false;
    };

    struct AggregateResult {
        std::vector<std::string> ohPermissions;
        std::vector<int> grantResults;
    };

    SingleResult ToAndroidSingle(const std::string& ohPerm) const;
    MultiResult ToAndroidMulti(const std::string& ohPerm) const;
    std::string ToOhPermission(const std::string& androidPerm) const;
    AggregateResult AggregateToOh(
        const std::vector<std::string>& androidPerms, const std::vector<int>& androidResults) const;

private:
    PermissionMapper();
    PermissionMapper(const PermissionMapper&) = delete;
    PermissionMapper& operator=(const PermissionMapper&) = delete;
    PermissionMapper(PermissionMapper&&) = delete;
    PermissionMapper& operator=(PermissionMapper&&) = delete;

    struct ReverseResult {
        std::string ohPermission;
        bool found = false;
    };
    ReverseResult ReverseLookup(const std::string& androidPerm) const;

    std::map<std::string, std::string> singleMap_;
    std::map<std::string, std::vector<std::string>> multipleMap_;
};
} // namespace OHOS::Plugin
#endif // PLUGINS_ABILITY_ACCESS_CTRL_ANDROID_JAVA_JNI_PERMISSION_MAPPER_H
