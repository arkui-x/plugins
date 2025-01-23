/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_GEOLOCATION_PERMISSIONS_ANDROID_H
#define PLUGINS_WEB_WEBVIEW_ANDROID_JAVA_JNI_GEOLOCATION_PERMISSIONS_ANDROID_H

#include "plugins/web/webview/geolocation_permissions.h"

namespace OHOS::Plugin {
class GeolocationPermissionsAndroid final {
public:
    GeolocationPermissionsAndroid() = default;
    ~GeolocationPermissionsAndroid() = default;
    static void AllowGeolocation(const std::string& origin, const bool incognito);
    static void DeleteGeolocation(const std::string& origin, const bool incognito);
    static void DeleteAllGeolocation(const bool incognito);
    static void GetAccessibleGeolocation(const std::string& origin, int32_t asyncCallbackInfoId, const bool incognito);
    static void GetStoredGeolocation(int32_t asyncCallbackInfoId, const bool incognito);
};
} // namespace OHOS::Plugin

#endif