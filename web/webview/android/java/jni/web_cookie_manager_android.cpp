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

#include "web_cookie_manager_android.h"

#include "log.h"
#include "plugins/web/webview/android/java/jni/web_cookie_manager_jni.h"

using namespace OHOS::NWebError;

namespace OHOS::Plugin {
void WebCookieManagerAndroid::ConfigCookie(
    const std::string& url, const std::string& value, int32_t asyncCallbackInfoId)

{
    WebCookieManagerJni::ConfigCookie(url, value, asyncCallbackInfoId);
}

void WebCookieManagerAndroid::ConfigCookie(
    const std::string& url, const std::string& value, bool incognito, bool includeHttpOnly, int32_t asyncCallbackInfoId)
{
    WebCookieManagerJni::ConfigCookie(url, value, incognito, includeHttpOnly, asyncCallbackInfoId);
}

ErrCode WebCookieManagerAndroid::ConfigCookieSync(
    const std::string& url, const std::string& value, bool incognito, bool includeHttpOnly)
{
    return WebCookieManagerJni::ConfigCookieSync(url, value, incognito, includeHttpOnly);
}

void WebCookieManagerAndroid::FetchCookie(const std::string& url, int32_t asyncCallbackInfoId)
{
    WebCookieManagerJni::FetchCookie(url, asyncCallbackInfoId);
}

std::string WebCookieManagerAndroid::FetchCookieSync(const std::string& url, bool incognito, ErrCode& errorCode)
{
    return WebCookieManagerJni::FetchCookieSync(url, incognito, errorCode);
}

ErrCode WebCookieManagerAndroid::PutAcceptCookieEnabled(bool accept)
{
    return WebCookieManagerJni::PutAcceptCookieEnabled(accept);
}

ErrCode WebCookieManagerAndroid::PutAcceptThirdPartyCookieEnabled(bool accept)
{
    return WebCookieManagerJni::PutAcceptThirdPartyCookieEnabled(accept);
}

ErrCode WebCookieManagerAndroid::ClearAllCookiesSync(bool incognito)
{
    return WebCookieManagerJni::ClearAllCookiesSync(incognito);
}

void WebCookieManagerAndroid::ClearSessionCookieSync()
{
    WebCookieManagerJni::ClearSessionCookieSync();
}

void WebCookieManagerAndroid::ClearAllCookies(int32_t asyncCallbackInfoId)
{
    WebCookieManagerJni::ClearAllCookies(asyncCallbackInfoId);
}

bool WebCookieManagerAndroid::ExistCookie(bool incognito)
{
    return WebCookieManagerJni::ExistCookie(incognito);
}

void WebCookieManagerAndroid::ClearSessionCookie(int32_t asyncCallbackInfoId)
{
    WebCookieManagerJni::ClearSessionCookie(asyncCallbackInfoId);
}

void WebCookieManagerAndroid::SaveCookie(int32_t asyncCallbackInfoId)
{
    WebCookieManagerJni::SaveCookie(asyncCallbackInfoId);
}
} // namespace OHOS::Plugin
