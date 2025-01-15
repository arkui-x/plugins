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

#include "geolocation_permissions_android.h"

#include "plugins/web/webview/android/java/jni/geolocation_permissions_jni.h"

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
void GeolocationPermissionsAndroid::AllowGeolocation(const std::string& origin, const bool incognito)
{
    GeolocationPermissionsJni::AllowGeolocation(origin, incognito);
}
void GeolocationPermissionsAndroid::DeleteGeolocation(const std::string& origin, const bool incognito)
{
    GeolocationPermissionsJni::DeleteGeolocation(origin, incognito);
}
void GeolocationPermissionsAndroid::DeleteAllGeolocation(const bool incognito)
{
    GeolocationPermissionsJni::DeleteAllGeolocation(incognito);
}

void GeolocationPermissionsAndroid::GetAccessibleGeolocation(
    const std::string& origin, int32_t asyncCallbackInfoId, const bool incognito)
{
    GeolocationPermissionsJni::GetAccessibleGeolocation(origin, asyncCallbackInfoId, incognito);
}

void GeolocationPermissionsAndroid::GetStoredGeolocation(int32_t asyncCallbackInfoId, const bool incognito)
{
    GeolocationPermissionsJni::GetStoredGeolocation(asyncCallbackInfoId, incognito);
}
} // namespace OHOS::Plugin