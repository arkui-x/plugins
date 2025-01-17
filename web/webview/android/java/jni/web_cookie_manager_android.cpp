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

void WebCookieManagerAndroid::FetchCookie(const std::string& url, int32_t asyncCallbackInfoId)
{
    WebCookieManagerJni::FetchCookie(url, asyncCallbackInfoId);
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
} // namespace OHOS::Plugin