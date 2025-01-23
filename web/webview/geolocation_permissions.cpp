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

#include "geolocation_permissions.h"

#include "inner_api/plugin_utils_inner.h"
#include "geolocation_permissions.h"
#ifdef ANDROID_PLATFORM
#include "android/java/jni/geolocation_permissions_android.h"
#endif
#ifdef IOS_PLATFORM
#endif


namespace OHOS::Plugin {
thread_local std::vector<std::shared_ptr<GeolocationPermissionsResultCallbackInfo>>
    GeolocationPermissions::getAccessibleGeolocationCallbackInfoContainer_;

thread_local std::vector<std::shared_ptr<GeolocationPermissionsResultCallbackInfo>>
    GeolocationPermissions::getStoredGeolocationCallbackInfoContainer_;

void GeolocationPermissions::AllowGeolocation(const std::string &origin, const bool incognito)
{
#ifdef ANDROID_PLATFORM
    GeolocationPermissionsAndroid::AllowGeolocation(origin, incognito);
#endif
}
void GeolocationPermissions::DeleteGeolocation(const std::string &origin, const bool incognito)
{
#ifdef ANDROID_PLATFORM
    GeolocationPermissionsAndroid::DeleteGeolocation(origin, incognito);
#endif
}
void GeolocationPermissions::DeleteAllGeolocation(const bool incognito)
{
#ifdef ANDROID_PLATFORM
    GeolocationPermissionsAndroid::DeleteAllGeolocation(incognito);
#endif
}

void GeolocationPermissions::GetAccessibleGeolocation(const std::string &origin, int32_t asyncCallbackInfoId,
    const bool incognito)
{
#ifdef ANDROID_PLATFORM
    GeolocationPermissionsAndroid::GetAccessibleGeolocation(origin, asyncCallbackInfoId,
    incognito);
#endif
}

void GeolocationPermissions::GetStoredGeolocation(int32_t asyncCallbackInfoId, const bool incognito)
{
#ifdef ANDROID_PLATFORM
    GeolocationPermissionsAndroid::GetStoredGeolocation(asyncCallbackInfoId, incognito);
#endif
}

void GeolocationPermissions::OnGetAccessibleReceiveValue(bool result, int32_t asyncCallbackInfoId)
{
    ExcuteAsyncCallbackInfo(result, asyncCallbackInfoId);
}

void GeolocationPermissions::OnGetAccessibleReceiveError(int32_t errCode, int32_t asyncCallbackInfoId)
{
    ExcuteAsyncCallbackInfo(errCode, asyncCallbackInfoId);
}

void GeolocationPermissions::OnGetStoredReceiveValue(const std::vector<std::string> &result,
    int32_t asyncCallbackInfoId)
{
    ExcuteAsyncCallbackInfo(result, asyncCallbackInfoId);
}

bool GeolocationPermissions::ExcuteAsyncCallbackInfo(bool result, int32_t asyncCallbackInfoId)
{
    for (const auto &asyncCallbackInfo : getAccessibleGeolocationCallbackInfoContainer_) {
        if (!asyncCallbackInfo) {
            continue;
        }
        if (asyncCallbackInfo->GetUniqueId() == asyncCallbackInfoId) {
            if ((asyncCallbackInfo->env) && (asyncCallbackInfo->asyncWork)) {
                asyncCallbackInfo->result = result;
                napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
                return true;
            }
        }
    }
    return false;
}

bool GeolocationPermissions::ExcuteAsyncCallbackInfo(int32_t errCode, int32_t asyncCallbackInfoId)
{
    for (const auto &asyncCallbackInfo : getAccessibleGeolocationCallbackInfoContainer_) {
        if (!asyncCallbackInfo) {
            continue;
        }
        if (asyncCallbackInfo->GetUniqueId() == asyncCallbackInfoId) {
            if ((asyncCallbackInfo->env) && (asyncCallbackInfo->asyncWork)) {
                asyncCallbackInfo->errCode = errCode;
                napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
                return true;
            }
        }
    }
    return false;
}

bool GeolocationPermissions::ExcuteAsyncCallbackInfo(const std::vector<std::string> &result,
    int32_t asyncCallbackInfoId)
{
    for (const auto &asyncCallbackInfo : getStoredGeolocationCallbackInfoContainer_) {
        if (!asyncCallbackInfo) {
            continue;
        }
        if (asyncCallbackInfo->GetUniqueId() == asyncCallbackInfoId) {
            if ((asyncCallbackInfo->env) && (asyncCallbackInfo->asyncWork)) {
                asyncCallbackInfo->originsArray = result;
                napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
                return true;
            }
        }
    }
    return false;
}

void GeolocationPermissions::InsertCallbackInfo(
    const std::shared_ptr<GeolocationPermissionsResultCallbackInfo> &asyncCallbackInfo,
    GeolocationPermissionsTaskType geolocationTaskType)
{
    switch (geolocationTaskType) {
        case GeolocationPermissionsTaskType::GET_ACCESSIBLE_GEOLOCATION:
            if (asyncCallbackInfo) {
                getAccessibleGeolocationCallbackInfoContainer_.push_back(asyncCallbackInfo);
            }
            break;
        case GeolocationPermissionsTaskType::GET_STORED_GEOLOCATION:
            if (asyncCallbackInfo) {
                getStoredGeolocationCallbackInfoContainer_.push_back(asyncCallbackInfo);
            }
            break;
        case GeolocationPermissionsTaskType::NONE:
            break;
    }
}

bool GeolocationPermissions::EraseCallbackInfo(const GeolocationPermissionsResultCallbackInfo *asyncCallbackInfo,
    GeolocationPermissionsTaskType geolocationTaskType)
{
    switch (geolocationTaskType) {
        case GeolocationPermissionsTaskType::GET_ACCESSIBLE_GEOLOCATION:
            if (getAccessibleGeolocationCallbackInfoContainer_.empty() || !asyncCallbackInfo) {
                return false;
            }
            for (auto it = getAccessibleGeolocationCallbackInfoContainer_.begin();
                it != getAccessibleGeolocationCallbackInfoContainer_.end(); it++) {
                if ((*it) && (*it).get() == asyncCallbackInfo) {
                    getAccessibleGeolocationCallbackInfoContainer_.erase(it);
                    return true;
                }
            }
            break;
        case GeolocationPermissionsTaskType::GET_STORED_GEOLOCATION:
            if (getStoredGeolocationCallbackInfoContainer_.empty() || !asyncCallbackInfo) {
                return false;
            }
            for (auto it = getStoredGeolocationCallbackInfoContainer_.begin();
                it != getStoredGeolocationCallbackInfoContainer_.end(); it++) {
                if ((*it) && (*it).get() == asyncCallbackInfo) {
                    getStoredGeolocationCallbackInfoContainer_.erase(it);
                    return true;
                }
            }
            break;
        case GeolocationPermissionsTaskType::NONE:
            break;
    }
    return false;
}
} // namespace OHOS::Plugin