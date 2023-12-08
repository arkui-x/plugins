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

#ifndef PLUGINS_WEB_WEBVIEW_WEB_COOKIE_MANAGER_H
#define PLUGINS_WEB_WEBVIEW_WEB_COOKIE_MANAGER_H

#include <map>
#include <memory>
#include <vector>

#include "napi_parse_utils.h"
#include "web_errors.h"
#include "web_cookie_manager_callback.h"

namespace OHOS::Plugin {

class WebCookieManager {
public:
    WebCookieManager() = default;
    virtual ~WebCookieManager() = default;
    static void ConfigCookie(const std::string& url, const std::string& value);
    static void FetchCookie(const std::string& url);
    static void ClearAllCookies();

    static void OnFetchReceiveValue(const std::string& result);
    static void OnConfigReceiveValue(bool result);
    static void OnClearReceiveValue();
    static void InsertCallbackInfo(
        const std::shared_ptr<AsyncCookieManagerResultCallbackInfo>& asyncCallbackInfo, TaskType taskType);
    static bool EraseCallbackInfo(
        const AsyncCookieManagerResultCallbackInfo* asyncCallbackInfo, TaskType taskType);

private:
    static bool ExcuteAsyncCallbackInfo(const std::string& result);
    static bool ExcuteAsyncCallbackInfo(bool result);
    static bool ExcuteAsyncCallbackInfo();
    static void IncreaseIndex(TaskType taskType);
    static void DecreaseIndex(TaskType taskType);
    static thread_local std::vector<std::shared_ptr<AsyncCookieManagerResultCallbackInfo>>
        configCookieCallbackInfoContainer_;
    static thread_local std::vector<std::shared_ptr<AsyncCookieManagerResultCallbackInfo>>
        fetchCookieCallbackInfoContainer_;
    static thread_local std::vector<std::shared_ptr<AsyncCookieManagerResultCallbackInfo>>
        clearAllCookiesCallbackInfoContainer_;
    static thread_local int32_t configCookieIndex_;
    static thread_local int32_t fetchCookieIndex_;
    static thread_local int32_t clearAllCookiesIndex_;
};
} // namespace OHOS::Plugin

#endif // PLUGINS_WEB_WEBVIEW_WEB_COOKIE_MANAGER_H
