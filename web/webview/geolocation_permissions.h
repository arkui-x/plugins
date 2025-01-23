/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef PLUGINS_WEB_WEBVIEW_GEOLOCATION_PERMISSIONS_H
#define PLUGINS_WEB_WEBVIEW_GEOLOCATION_PERMISSIONS_H

#include <vector>
#include <string>

#include "napi_parse_utils.h"
#include "web_errors.h"
#include "geolocation_permissions_callback.h"

namespace OHOS::Plugin {
class GeolocationPermissions {
public:
    GeolocationPermissions() = default;
    virtual ~GeolocationPermissions() = default;
    static void AllowGeolocation(const std::string &origin, const bool incognito);
    static void DeleteGeolocation(const std::string &origin, const bool incognito);
    static void DeleteAllGeolocation(const bool incognito);
    static void GetAccessibleGeolocation(const std::string &origin, int32_t asyncCallbackInfoId, const bool incognito);
    static void GetStoredGeolocation(int32_t asyncCallbackInfoId, const bool incognito);

    static void OnGetAccessibleReceiveValue(bool result, int32_t asyncCallbackInfoId);
    static void OnGetAccessibleReceiveError(int32_t errCode, int32_t asyncCallbackInfoId);
    static void OnGetStoredReceiveValue(const std::vector<std::string> &result, int32_t asyncCallbackInfoId);

    static void InsertCallbackInfo(
        const std::shared_ptr<GeolocationPermissionsResultCallbackInfo> &asyncCallbackInfo,
        GeolocationPermissionsTaskType geolocationTaskType);
    static bool EraseCallbackInfo(const GeolocationPermissionsResultCallbackInfo *asyncCallbackInfo,
        GeolocationPermissionsTaskType geolocationTaskType);

private:
    static bool ExcuteAsyncCallbackInfo(bool result, int32_t asyncCallbackInfoId);
    static bool ExcuteAsyncCallbackInfo(int32_t errCode, int32_t asyncCallbackInfoId);
    static bool ExcuteAsyncCallbackInfo(const std::vector<std::string> &result, int32_t asyncCallbackInfoId);

    static thread_local std::vector<std::shared_ptr<GeolocationPermissionsResultCallbackInfo>>
        getAccessibleGeolocationCallbackInfoContainer_;
    static thread_local std::vector<std::shared_ptr<GeolocationPermissionsResultCallbackInfo>>
        getStoredGeolocationCallbackInfoContainer_;
};
} // namespace OHOS::Plugin

#endif // PLUGINS_WEB_WEBVIEW_GEOLOCATION_PERMISSIONS_H