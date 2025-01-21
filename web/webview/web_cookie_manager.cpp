/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "web_cookie_manager.h"

#include "android/java/jni/web_cookie_manager_android.h"
#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "web_cookie_manager.h"
#ifdef ANDROID_PLATFORM
#include "android/java/jni/web_cookie_manager_android.h"
#endif
#ifdef IOS_PLATFORM
#include "ios/web_cookie_manager_ios.h"
#endif

namespace OHOS::Plugin {
thread_local std::vector<std::shared_ptr<AsyncCookieManagerResultCallbackInfo>>
    WebCookieManager::configCookieCallbackInfoContainer_;
thread_local std::vector<std::shared_ptr<AsyncCookieManagerResultCallbackInfo>>
    WebCookieManager::fetchCookieCallbackInfoContainer_;
thread_local std::vector<std::shared_ptr<AsyncCookieManagerResultCallbackInfo>>
    WebCookieManager::clearAllCookiesCallbackInfoContainer_;
thread_local std::vector<std::shared_ptr<AsyncCookieManagerResultCallbackInfo>>
    WebCookieManager::clearSessionCookieCallbackInfoContainer_;

void WebCookieManager::OnFetchReceiveValue(const std::string& result, int32_t asyncCallbackInfoId)
{
    ExcuteAsyncCallbackInfo(result, asyncCallbackInfoId);
}

void WebCookieManager::OnConfigReceiveValue(bool result, int32_t asyncCallbackInfoId)
{
    ExcuteAsyncCallbackInfo(result, asyncCallbackInfoId);
}

void WebCookieManager::OnClearReceiveValue(int32_t asyncCallbackInfoId)
{
    ExcuteAsyncCallbackInfo(asyncCallbackInfoId);
}

void WebCookieManager::InsertCallbackInfo(
    const std::shared_ptr<AsyncCookieManagerResultCallbackInfo>& asyncCallbackInfo, TaskType taskType)
{
    switch (taskType) {
        case TaskType::CONFIG_COOKIE:
            if (asyncCallbackInfo) {
                configCookieCallbackInfoContainer_.push_back(asyncCallbackInfo);
            }
            break;
        case TaskType::FETCH_COOKIE:
            if (asyncCallbackInfo) {
                fetchCookieCallbackInfoContainer_.push_back(asyncCallbackInfo);
            }
            break;
        case TaskType::CLEAR_ALL_COOKIES:
            if (asyncCallbackInfo) {
                clearAllCookiesCallbackInfoContainer_.push_back(asyncCallbackInfo);
            }
            break;
        case TaskType::CLEAR_SESSION_COOKIE:
            if (asyncCallbackInfo) {
                clearSessionCookieCallbackInfoContainer_.push_back(asyncCallbackInfo);
            }
            break;
        case TaskType::NONE:
            break;
    }
}

bool WebCookieManager::EraseCallbackInfo(
    const AsyncCookieManagerResultCallbackInfo* asyncCallbackInfo, TaskType taskType)
{
    switch (taskType) {
        case TaskType::CONFIG_COOKIE:
            if (configCookieCallbackInfoContainer_.empty() || !asyncCallbackInfo) {
                return false;
            }
            for (auto it = configCookieCallbackInfoContainer_.begin(); it != configCookieCallbackInfoContainer_.end();
                it++) {
                if ((*it) && (*it).get() == asyncCallbackInfo) {
                    configCookieCallbackInfoContainer_.erase(it);
                    return true;
                }
            }
            break;
        case TaskType::FETCH_COOKIE:
            if (fetchCookieCallbackInfoContainer_.empty() || !asyncCallbackInfo) {
                return false;
            }
            for (auto it = fetchCookieCallbackInfoContainer_.begin(); it != fetchCookieCallbackInfoContainer_.end();
                it++) {
                if ((*it) && (*it).get() == asyncCallbackInfo) {
                    fetchCookieCallbackInfoContainer_.erase(it);
                    return true;
                }
            }
            break;
        case TaskType::CLEAR_ALL_COOKIES:
            if (clearAllCookiesCallbackInfoContainer_.empty() || !asyncCallbackInfo) {
                return false;
            }
            for (auto it = clearAllCookiesCallbackInfoContainer_.begin();
                it != clearAllCookiesCallbackInfoContainer_.end(); it++) {
                if ((*it) && (*it).get() == asyncCallbackInfo) {
                    clearAllCookiesCallbackInfoContainer_.erase(it);
                    return true;
                }
            }
            break;
        case TaskType::CLEAR_SESSION_COOKIE:
            if (clearSessionCookieCallbackInfoContainer_.empty() || !asyncCallbackInfo) {
                return false;
            }
            for (auto it = clearSessionCookieCallbackInfoContainer_.begin();
                it != clearSessionCookieCallbackInfoContainer_.end(); it++) {
                if ((*it) && (*it).get() == asyncCallbackInfo) {
                    clearSessionCookieCallbackInfoContainer_.erase(it);
                    return true;
                }
            }
            break;
        case TaskType::NONE:
            break;
    }
    return false;
}

bool WebCookieManager::ExcuteAsyncCallbackInfo(const std::string& result, int32_t asyncCallbackInfoId)
{
    LOGD("WebCookieManager fetchCookie asyncCallbackInfoId: %{public}d", asyncCallbackInfoId);
    for (const auto& asyncCallbackInfo : fetchCookieCallbackInfoContainer_) {
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

bool WebCookieManager::ExcuteAsyncCallbackInfo(bool result, int32_t asyncCallbackInfoId)
{
    LOGD("WebCookieManager configCookie asyncCallbackInfoId: %{public}d", asyncCallbackInfoId);
    for (const auto& asyncCallbackInfo : configCookieCallbackInfoContainer_) {
        if (!asyncCallbackInfo) {
            continue;
        }
        if (asyncCallbackInfo->GetUniqueId() == asyncCallbackInfoId) {
            if ((asyncCallbackInfo->env) && (asyncCallbackInfo->asyncWork)) {
                napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
                return true;
            }
        }
    }
    return false;
}

bool WebCookieManager::ExcuteAsyncCallbackInfo(int32_t asyncCallbackInfoId)
{
    LOGD("WebCookieManager ClearAllCookies ClearSessionCookie asyncCallbackInfoId: %{public}d", asyncCallbackInfoId);
    for (const auto& asyncCallbackInfo : clearAllCookiesCallbackInfoContainer_) {
        if (!asyncCallbackInfo) {
            continue;
        }
        if (asyncCallbackInfo->GetUniqueId() == asyncCallbackInfoId) {
            if ((asyncCallbackInfo->env) && (asyncCallbackInfo->asyncWork)) {
                napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
                return true;
            }
        }
    }
    for (const auto& asyncCallbackInfo : clearSessionCookieCallbackInfoContainer_ ) {
        if (!asyncCallbackInfo) {
            continue;
        };
        if (asyncCallbackInfo->GetUniqueId() == asyncCallbackInfoId) {
            if ((asyncCallbackInfo->env) && (asyncCallbackInfo->asyncWork)) {
                napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
                return true;
            };
        };
    };
    return false;
}

void WebCookieManager::ConfigCookie(const std::string& url, const std::string& value, int32_t asyncCallbackInfoId)
{
#ifdef ANDROID_PLATFORM
    WebCookieManagerAndroid::ConfigCookie(url, value, asyncCallbackInfoId);
#endif
#ifdef IOS_PLATFORM
    WebCookieManagerIOS::ConfigCookie(url, value, asyncCallbackInfoId);
#endif
}

void WebCookieManager::FetchCookie(const std::string& url, int32_t asyncCallbackInfoId)
{
#ifdef ANDROID_PLATFORM
    WebCookieManagerAndroid::FetchCookie(url, asyncCallbackInfoId);
#endif
#ifdef IOS_PLATFORM
    WebCookieManagerIOS::FetchCookie(url, asyncCallbackInfoId);
#endif
}

void WebCookieManager::ClearAllCookies(int32_t asyncCallbackInfoId)
{
#ifdef ANDROID_PLATFORM
    WebCookieManagerAndroid::ClearAllCookies(asyncCallbackInfoId);
#endif
#ifdef IOS_PLATFORM
    WebCookieManagerIOS::ClearAllCookies(asyncCallbackInfoId);
#endif
}

bool WebCookieManager::ExistCookie(bool incognito)
{
#ifdef ANDROID_PLATFORM
    return WebCookieManagerAndroid::ExistCookie(incognito);
#endif
#ifdef IOS_PLATFORM
    return WebCookieManagerIOS::ExistCookie(incognito);
#endif
}

void WebCookieManager::ClearSessionCookie(int32_t asyncCallbackInfoId)
{
#ifdef ANDROID_PLATFORM
    WebCookieManagerAndroid::ClearSessionCookie(asyncCallbackInfoId);
#endif
#ifdef IOS_PLATFORM
    WebCookieManagerIOS::ClearSessionCookie(asyncCallbackInfoId);
#endif
}
} // namespace OHOS::Plugin