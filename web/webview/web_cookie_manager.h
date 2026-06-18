/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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
#include <string>
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
    static ErrCode ConfigCookie(const std::string& url, const std::string& value, bool incognito,
        bool includeHttpOnly, int32_t asyncCallbackInfoId);
    static ErrCode ConfigCookieSync(
        const std::string& url, const std::string& value, bool incognito, bool includeHttpOnly);
    static void FetchCookie(const std::string& url, int32_t asyncCallbackInfoId);
    static ErrCode FetchCookieSync(const std::string& url, bool incognito, std::string& result);
    static void ClearAllCookies(int32_t asyncCallbackInfoId);
    static ErrCode ClearAllCookiesSync(bool incognito);
    static bool ExistCookie(bool incognito);
    static void ClearSessionCookie(int32_t asyncCallbackInfoId);
    static void ClearSessionCookieSync();
    static ErrCode SaveCookie(int32_t asyncCallbackInfoId);
    static ErrCode PutAcceptCookieEnabled(bool accept);
    static ErrCode PutAcceptThirdPartyCookieEnabled(bool accept);

    static void OnFetchReceiveValue(const std::string& result, int32_t asyncCallbackInfoId);
    static void OnFetchReceiveError(ErrCode errorCode, int32_t asyncCallbackInfoId);
    static void OnConfigReceiveValue(bool result, int32_t asyncCallbackInfoId);
    static void OnConfigReceiveError(ErrCode errorCode, int32_t asyncCallbackInfoId);
    static void OnClearReceiveValue(int32_t asyncCallbackInfoId);
    static void OnSaveReceiveValue(int32_t asyncCallbackInfoId);
    static void OnCookieManagerReceiveError(ErrCode errorCode, int32_t asyncCallbackInfoId);
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
    static thread_local std::vector<std::shared_ptr<AsyncCookieManagerResultCallbackInfo>>
        saveCookieCallbackInfoContainer_;
};
} // namespace OHOS::Plugin

#endif // PLUGINS_WEB_WEBVIEW_WEB_COOKIE_MANAGER_H
