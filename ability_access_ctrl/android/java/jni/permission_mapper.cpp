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

namespace OHOS::Plugin {
PermissionMapper& PermissionMapper::Instance()
{
    static PermissionMapper instance;
    return instance;
}

PermissionMapper::PermissionMapper()
{
    singleMap_["ohos.permission.CAMERA"] = "android.permission.CAMERA";
    singleMap_["ohos.permission.MICROPHONE"] = "android.permission.RECORD_AUDIO";
    singleMap_["ohos.permission.READ_IMAGEVIDEO"] = "android.permission.READ_EXTERNAL_STORAGE";
    singleMap_["ohos.permission.WRITE_IMAGEVIDEO"] = "android.permission.WRITE_EXTERNAL_STORAGE";
    singleMap_["ohos.permission.APPROXIMATELY_LOCATION"] = "android.permission.ACCESS_COARSE_LOCATION";
    singleMap_["ohos.permission.LOCATION"] = "android.permission.ACCESS_FINE_LOCATION";

    multipleMap_["ohos.permission.ACCESS_BLUETOOTH"] = {
        "android.permission.BLUETOOTH",
        "android.permission.BLUETOOTH_ADMIN",
        "android.permission.BLUETOOTH_ADVERTISE",
        "android.permission.BLUETOOTH_CONNECT",
        "android.permission.BLUETOOTH_SCAN",
        "android.permission.ACCESS_FINE_LOCATION",
        "android.permission.ACCESS_COARSE_LOCATION",
    };
}

PermissionMapper::SingleResult PermissionMapper::ToAndroidSingle(const std::string& ohPerm) const
{
    auto it = singleMap_.find(ohPerm);
    if (it != singleMap_.end()) {
        return {it->second, true};
    }
    return {ohPerm, false};
}

PermissionMapper::MultiResult PermissionMapper::ToAndroidMulti(const std::string& ohPerm) const
{
    auto it = singleMap_.find(ohPerm);
    if (it != singleMap_.end()) {
        return {{it->second}, true};
    }
    auto mit = multipleMap_.find(ohPerm);
    if (mit != multipleMap_.end()) {
        return {mit->second, true};
    }
    return {{ohPerm}, false};
}

PermissionMapper::ReverseResult PermissionMapper::ReverseLookup(const std::string& androidPerm) const
{
    for (const auto& [ohPerm, androidPerm_] : singleMap_) {
        if (androidPerm_ == androidPerm) {
            return {ohPerm, true};
        }
    }
    for (const auto& [ohPerm, androidPerms] : multipleMap_) {
        if (std::find(androidPerms.begin(), androidPerms.end(), androidPerm) != androidPerms.end()) {
            return {ohPerm, true};
        }
    }
    return {androidPerm, false};
}

std::string PermissionMapper::ToOhPermission(const std::string& androidPerm) const
{
    return ReverseLookup(androidPerm).ohPermission;
}

PermissionMapper::AggregateResult PermissionMapper::AggregateToOh(
    const std::vector<std::string>& androidPerms, const std::vector<int>& androidResults) const
{
    static constexpr int invalidResult = 2;
    if (androidPerms.size() != androidResults.size()) {
        return {};
    }
    std::map<std::string, int> resultMap;

    for (size_t i = 0; i < androidPerms.size(); i++) {
        auto reverse = ReverseLookup(androidPerms[i]);
        auto iter = resultMap.find(reverse.ohPermission);
        if (iter != resultMap.end()) {
            iter->second = (iter->second == androidResults[i]) ? iter->second : invalidResult;
        } else if (!reverse.found) {
            resultMap[reverse.ohPermission] = invalidResult;
        } else {
            resultMap[reverse.ohPermission] = androidResults[i];
        }
    }

    AggregateResult result;
    for (const auto& [perm, grantResult] : resultMap) {
        result.ohPermissions.emplace_back(perm);
        result.grantResults.emplace_back(grantResult);
    }
    return result;
}
} // namespace OHOS::Plugin
