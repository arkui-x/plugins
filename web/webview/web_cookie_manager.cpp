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
std::vector<std::shared_ptr<AsyncCookieManagerResultCallbackInfo>> WebCookieManager::configCookieCallbackInfoContainer_;
std::vector<std::shared_ptr<AsyncCookieManagerResultCallbackInfo>> WebCookieManager::fetchCookieCallbackInfoContainer_;
std::vector<std::shared_ptr<AsyncCookieManagerResultCallbackInfo>>
    WebCookieManager::clearAllCookiesCallbackInfoContainer_;
int32_t WebCookieManager::configCookieIndex_ = 0;
int32_t WebCookieManager::fetchCookieIndex_ = 0;
int32_t WebCookieManager::clearAllCookiesIndex_ = 0;
std::mutex WebCookieManager::mutex_;

void WebCookieManager::OnFetchReceiveValue(const std::string& result)
{
    if (ExcuteAsyncCallbackInfo(result)) {
        IncreaseIndex(TaskType::FETCH_COOKIE);
    }
}

void WebCookieManager::OnConfigReceiveValue(bool result)
{
    if (ExcuteAsyncCallbackInfo(result)) {
        IncreaseIndex(TaskType::CONFIG_COOKIE);
    }
}

void WebCookieManager::OnClearReceiveValue()
{
    if (ExcuteAsyncCallbackInfo()) {
        IncreaseIndex(TaskType::CLEAR_ALL_COOKIES);
    }
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
                    DecreaseIndex(TaskType::CONFIG_COOKIE);
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
                    DecreaseIndex(TaskType::FETCH_COOKIE);
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
                    DecreaseIndex(TaskType::CLEAR_ALL_COOKIES);
                    return true;
                }
            }
            break;
        case TaskType::NONE:
            break;
    }
    return false;
}

void WebCookieManager::IncreaseIndex(TaskType taskType)
{
    std::lock_guard<std::mutex> guard(mutex_);
    switch (taskType) {
        case TaskType::CONFIG_COOKIE:
            configCookieIndex_++;
            if (configCookieCallbackInfoContainer_.empty() ||
                configCookieIndex_ >= configCookieCallbackInfoContainer_.size()) {
                configCookieIndex_ = 0;
            }
            break;
        case TaskType::FETCH_COOKIE:
            fetchCookieIndex_++;
            if (fetchCookieCallbackInfoContainer_.empty() ||
                fetchCookieIndex_ >= fetchCookieCallbackInfoContainer_.size()) {
                fetchCookieIndex_ = 0;
            }
            break;
        case TaskType::CLEAR_ALL_COOKIES:
            clearAllCookiesIndex_++;
            if (clearAllCookiesCallbackInfoContainer_.empty() ||
                clearAllCookiesIndex_ >= clearAllCookiesCallbackInfoContainer_.size()) {
                clearAllCookiesIndex_ = 0;
            }
            break;
        case TaskType::NONE:
            break;
    }
}

void WebCookieManager::DecreaseIndex(TaskType taskType)
{
    std::lock_guard<std::mutex> guard(mutex_);
    switch (taskType) {
        case TaskType::CONFIG_COOKIE:
            configCookieIndex_--;
            if (configCookieCallbackInfoContainer_.empty() || configCookieIndex_ < 0) {
                configCookieIndex_ = 0;
            }
            break;
        case TaskType::FETCH_COOKIE:
            fetchCookieIndex_--;
            if (fetchCookieCallbackInfoContainer_.empty() || fetchCookieIndex_ < 0) {
                fetchCookieIndex_ = 0;
            }
            break;
        case TaskType::CLEAR_ALL_COOKIES:
            clearAllCookiesIndex_--;
            if (clearAllCookiesCallbackInfoContainer_.empty() || clearAllCookiesIndex_ < 0) {
                clearAllCookiesIndex_ = 0;
            }
            break;
        case TaskType::NONE:
            break;
    }
}

bool WebCookieManager::ExcuteAsyncCallbackInfo(const std::string& result)
{
    LOGD("fetchCookieIndex_ == %{public}d, fetchCookieCallbackInfoContainer_ size == %{public}d", fetchCookieIndex_,
        fetchCookieCallbackInfoContainer_.size());
    if (fetchCookieCallbackInfoContainer_.empty()) {
        return false;
    }
    auto asyncCallbackInfo = fetchCookieCallbackInfoContainer_.at(fetchCookieIndex_);
    CHECK_NULL_RETURN(asyncCallbackInfo, false);
    asyncCallbackInfo->result = result;
    if ((asyncCallbackInfo->env) && (asyncCallbackInfo->asyncWork)) {
        napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
        return true;
    }
    return false;
}

bool WebCookieManager::ExcuteAsyncCallbackInfo(bool result)
{
    LOGD("configCookieIndex_ == %{public}d, configCookieCallbackInfoContainer_ size == %{public}d", configCookieIndex_,
        configCookieCallbackInfoContainer_.size());
    if (configCookieCallbackInfoContainer_.empty()) {
        return false;
    }
    auto asyncCallbackInfo = configCookieCallbackInfoContainer_.at(configCookieIndex_);
    CHECK_NULL_RETURN(asyncCallbackInfo, false);
    if ((asyncCallbackInfo->env) && (asyncCallbackInfo->asyncWork)) {
        napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
        return true;
    }
    return false;
}

bool WebCookieManager::ExcuteAsyncCallbackInfo()
{
    LOGD("clearAllCookiesIndex_ == %{public}d, clearAllCookiesCallbackInfoContainer_ size == %{public}d",
        clearAllCookiesIndex_, clearAllCookiesCallbackInfoContainer_.size());
    if (clearAllCookiesCallbackInfoContainer_.empty()) {
        return false;
    }
    auto asyncCallbackInfo = clearAllCookiesCallbackInfoContainer_.at(clearAllCookiesIndex_);
    CHECK_NULL_RETURN(asyncCallbackInfo, false);
    if ((asyncCallbackInfo->env) && (asyncCallbackInfo->asyncWork)) {
        napi_queue_async_work(asyncCallbackInfo->env, asyncCallbackInfo->asyncWork);
        return true;
    }
    return false;
}

void WebCookieManager::ConfigCookie(const std::string& url, const std::string& value)
{
#ifdef ANDROID_PLATFORM
    WebCookieManagerAndroid::ConfigCookie(url, value);
#endif
#ifdef IOS_PLATFORM
    WebCookieManagerIOS::ConfigCookie(url, value);
#endif
}

void WebCookieManager::FetchCookie(const std::string& url)
{
#ifdef ANDROID_PLATFORM
    WebCookieManagerAndroid::FetchCookie(url);
#endif
#ifdef IOS_PLATFORM
    WebCookieManagerIOS::FetchCookie(url);
#endif
}

void WebCookieManager::ClearAllCookies()
{
#ifdef ANDROID_PLATFORM
    WebCookieManagerAndroid::ClearAllCookies();
#endif
#ifdef IOS_PLATFORM
    WebCookieManagerIOS::ClearAllCookies();
#endif
}
} // namespace OHOS::Plugin