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
    static void ConfigCookie(const std::string& url, const std::string& value, int32_t asyncCallbackInfoId);
    static void FetchCookie(const std::string& url, int32_t asyncCallbackInfoId);
    static void ClearAllCookies(int32_t asyncCallbackInfoId);
    static bool ExistCookie(bool incognito);
    static void ClearSessionCookie(int32_t asyncCallbackInfoId);

    static void OnFetchReceiveValue(const std::string& result, int32_t asyncCallbackInfoId);
    static void OnConfigReceiveValue(bool result, int32_t asyncCallbackInfoId);
    static void OnClearReceiveValue(int32_t asyncCallbackInfoId);
    static void InsertCallbackInfo(
        const std::shared_ptr<AsyncCookieManagerResultCallbackInfo>& asyncCallbackInfo, TaskType taskType);
    static bool EraseCallbackInfo(
        const AsyncCookieManagerResultCallbackInfo* asyncCallbackInfo, TaskType taskType);

private:
    static bool ExcuteAsyncCallbackInfo(const std::string& result, int32_t asyncCallbackInfoId);
    static bool ExcuteAsyncCallbackInfo(bool result, int32_t asyncCallbackInfoId);
    static bool ExcuteAsyncCallbackInfo(int32_t asyncCallbackInfoId);
    static thread_local std::vector<std::shared_ptr<AsyncCookieManagerResultCallbackInfo>>
        configCookieCallbackInfoContainer_;
    static thread_local std::vector<std::shared_ptr<AsyncCookieManagerResultCallbackInfo>>
        fetchCookieCallbackInfoContainer_;
    static thread_local std::vector<std::shared_ptr<AsyncCookieManagerResultCallbackInfo>>
        clearAllCookiesCallbackInfoContainer_;
    static thread_local std::vector<std::shared_ptr<AsyncCookieManagerResultCallbackInfo>>
        clearSessionCookieCallbackInfoContainer_;
};
} // namespace OHOS::Plugin

#endif // PLUGINS_WEB_WEBVIEW_WEB_COOKIE_MANAGER_H
