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

#include "web_storage.h"

#include "android/java/jni/web_storage_android.h"
#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "web_storage.h"
#ifdef ANDROID_PLATFORM
#include "android/java/jni/web_storage_android.h"
#endif

namespace OHOS::Plugin {
thread_local std::vector<std::shared_ptr<AsyncWebStorageResultCallbackInfo>>
    WebStorage::getOriginQuotaCallbackInfoContainer_;
thread_local std::vector<std::shared_ptr<AsyncWebStorageResultCallbackInfo>>
    WebStorage::getOriginUsageCallbackInfoContainer_;
thread_local std::vector<std::shared_ptr<AsyncWebStorageResultCallbackInfo>>
    WebStorage::getOriginsCallbackInfoContainer_;

void WebStorage::OnGetOriginQuotaReceiveValue(const int64_t& result, int32_t asyncCallbackInfoId)
{
    ExcuteAsyncCallbackInfo(result, asyncCallbackInfoId);
}

void WebStorage::OnGetOriginUsageReceiveValue(const int32_t& result, int32_t asyncCallbackInfoId)
{
    ExcuteAsyncCallbackInfo(result, asyncCallbackInfoId);
}

void WebStorage::OnGetOriginsReceiveValue(const std::vector<WebStorageStruct>& result, int32_t asyncCallbackInfoId)
{
    ExcuteAsyncCallbackInfo(result, asyncCallbackInfoId);
}

void WebStorage::InsertCallbackInfo(
    const std::shared_ptr<AsyncWebStorageResultCallbackInfo> &asyncCallbackInfo, StorageTaskType storageTaskType)
{
    switch (storageTaskType) {
        case StorageTaskType::GET_ORIGINQUOTA:
            if (asyncCallbackInfo) {
                getOriginQuotaCallbackInfoContainer_.push_back(asyncCallbackInfo);
            }
            break;
        case StorageTaskType::GET_ORIGINUSAGE:
            if (asyncCallbackInfo) {
                getOriginUsageCallbackInfoContainer_.push_back(asyncCallbackInfo);
            }
            break;
        case StorageTaskType::GET_ORIGINS:
            if (asyncCallbackInfo) {
                getOriginsCallbackInfoContainer_.push_back(asyncCallbackInfo);
            }
            break;
        case StorageTaskType::NONE:
            break;
    }
}

bool WebStorage::EraseCallbackInfo(
    const AsyncWebStorageResultCallbackInfo* asyncCallbackInfo, StorageTaskType storageTaskType)
{
    switch (storageTaskType) {
        case StorageTaskType::GET_ORIGINQUOTA:
            if (getOriginQuotaCallbackInfoContainer_.empty() || !asyncCallbackInfo) {
                return false;
            }
            for (auto it = getOriginQuotaCallbackInfoContainer_.begin(); it != getOriginQuotaCallbackInfoContainer_.end();
                it++) {
                if ((*it) && (*it).get() == asyncCallbackInfo) {
                    getOriginQuotaCallbackInfoContainer_.erase(it);
                    return true;
                }
            }
            break;
        case StorageTaskType::GET_ORIGINUSAGE:
            if (getOriginUsageCallbackInfoContainer_.empty() || !asyncCallbackInfo) {
                return false;
            }
            for (auto it = getOriginUsageCallbackInfoContainer_.begin(); it != getOriginUsageCallbackInfoContainer_.end();
                it++) {
                if ((*it) && (*it).get() == asyncCallbackInfo) {
                    getOriginUsageCallbackInfoContainer_.erase(it);
                    return true;
                }
            }
            break;
        case StorageTaskType::GET_ORIGINS:
            if (getOriginsCallbackInfoContainer_.empty() || !asyncCallbackInfo) {
                return false;
            }
            for (auto it = getOriginsCallbackInfoContainer_.begin(); it != getOriginsCallbackInfoContainer_.end();
                it++) {
                if ((*it) && (*it).get() == asyncCallbackInfo) {
                    getOriginsCallbackInfoContainer_.erase(it);
                    return true;
                }
            }
            break;
        
        case StorageTaskType::NONE:
            break;
    }
    return false;
}

bool WebStorage::ExcuteAsyncCallbackInfo(const int64_t& result, int32_t asyncCallbackInfoId)
{
    for (const auto& asyncCallbackInfo : getOriginQuotaCallbackInfoContainer_) {
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

bool WebStorage::ExcuteAsyncCallbackInfo(const int32_t& result, int32_t asyncCallbackInfoId)
{
    for (const auto& asyncCallbackInfo : getOriginUsageCallbackInfoContainer_) {
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

bool WebStorage::ExcuteAsyncCallbackInfo(const std::vector<WebStorageStruct>& result, int32_t asyncCallbackInfoId)
{
    for (const auto& asyncCallbackInfo : getOriginsCallbackInfoContainer_) {
        if (!asyncCallbackInfo) {
            continue;
        }
        if (asyncCallbackInfo->GetUniqueId() == asyncCallbackInfoId) {
            if ((asyncCallbackInfo->env) && (asyncCallbackInfo->asyncWork)) {
                asyncCallbackInfo->resultVector = result;
                napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
                return true;
            }
        }
    }
    return false;
}


void WebStorage::GetOriginQuota(const std::string& origin, int32_t asyncCallbackInfoId)
{
#ifdef ANDROID_PLATFORM
    WebStorageAndroid::GetOriginQuota(origin, asyncCallbackInfoId);
#endif
}

void WebStorage::GetOriginUsage(const std::string& origin, int32_t asyncCallbackInfoId)
{
#ifdef ANDROID_PLATFORM
    WebStorageAndroid::GetOriginUsage(origin, asyncCallbackInfoId);
#endif
}

void WebStorage::GetOrigins(int32_t asyncCallbackInfoId)
{
#ifdef ANDROID_PLATFORM
    WebStorageAndroid::GetOrigins(asyncCallbackInfoId);
#endif
}

void WebStorage::DeleteAllData()
{
#ifdef ANDROID_PLATFORM
    WebStorageAndroid::DeleteAllData();
#endif
}

void WebStorage::DeleteOrigin(const std::string& origin)
{
#ifdef ANDROID_PLATFORM
    WebStorageAndroid::DeleteOrigin(origin);
#endif
}

} // namespace OHOS::Plugin