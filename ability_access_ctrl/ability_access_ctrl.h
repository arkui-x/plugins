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

#ifndef PLUGINS_ABILITY_ACCESS_CONTROL_PLUGIN_H
#define PLUGINS_ABILITY_ACCESS_CONTROL_PLUGIN_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include "plugin_utils.h"

namespace OHOS::Plugin {
typedef enum TypePermissionState {
    PERMISSION_DENIED = -1,
    PERMISSION_GRANTED = 0,
} PermissionState;

using RequestPermissionCallback =
    std::function<void(void*, const std::vector<std::string>& permissions, const std::vector<int> grantResults)>;
class AbilityAccessCtrl {
public:
    AbilityAccessCtrl() = default;
    virtual ~AbilityAccessCtrl() = default;

    static std::unique_ptr<AbilityAccessCtrl> Create();
    virtual bool CheckPermission(const std::string& permission) = 0;
    virtual void RequestPermissions(
        const std::vector<std::string>& permissions, RequestPermissionCallback callback, void* data) = 0;
};
} // namespace OHOS::Plugin

#endif // PLUGINS_ABILITY_ACCESS_CONTROL_PLUGIN_H
