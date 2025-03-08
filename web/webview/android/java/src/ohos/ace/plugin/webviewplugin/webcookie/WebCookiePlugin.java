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

package ohos.ace.plugin.webviewplugin.webcookie;

import android.content.Context;
import android.webkit.ValueCallback;
import android.webkit.CookieManager;

/**
 * WebCookiePlugin is used to manage cookies
 *
 * @since 2024-05-31
 */
public class WebCookiePlugin {
    private static final String LOG_TAG = "WebCookiePlugin";

    public WebCookiePlugin(Context context) {
        nativeInit();
    }

    /**
     * Set cookie value
     *
     * @param url url
     * @param StringCookie cookie
     * @param asyncCallbackInfoId callback id
     */
    public void configCookie(String url, String StringCookie, long asyncCallbackInfoId) {
        CookieManager.getInstance().setCookie(url, StringCookie, new ValueCallback<Boolean>() {
            @Override
            public void onReceiveValue(Boolean value) {
                onReceiveCookieValue(true, asyncCallbackInfoId);
            }
        });
    }

    /**
     * Get cookie value
     *
     * @param url url
     * @param asyncCallbackInfoId callback id
     */
    public void fetchCookie(String url, long asyncCallbackInfoId) {
        String cookies = CookieManager.getInstance().getCookie(url);
        if (cookies == null) {
            cookies = "";
        }
        onReceiveCookieValue(cookies, asyncCallbackInfoId);
    }

    /**
     * Clear all cookies
     *
     * @param asyncCallbackInfoId callback id
     */
    public void clearAllCookies(long asyncCallbackInfoId) {
        CookieManager.getInstance().removeAllCookies(new ValueCallback<Boolean>() {
            @Override
            public void onReceiveValue(Boolean value) {
                onReceiveCookieValue(asyncCallbackInfoId);
            }
        });
    }

    /**
     * Determine whether cookies exist.
     *
     * Function description: This method is used to check for the presence of cookies.
     *
     * @param incognito Is it in incognito mode.
     * @return If cookies exist, return true; otherwise, return false.
     */
    public boolean existCookie(boolean incognito) {
        CookieManager.getInstance().flush();
        return CookieManager.getInstance().hasCookies();
    }

    /**
     * Clear session cookies.
     *
     * @param asyncCallbackInfoId Asynchronous callback information identification.
     */
    public void clearSessionCookie(long asyncCallbackInfoId) {
        CookieManager.getInstance().removeSessionCookies(new ValueCallback<Boolean>() {
            @Override
            public void onReceiveValue(Boolean value) {
                onReceiveCookieValue(asyncCallbackInfoId);
            }
        });
    }

    /**
     * Initialize the native environment.
     */
    protected native void nativeInit();

    /**
     * Native callback function.
     *
     * @param value Value.
     * @param asyncCallbackInfoId Asynchronous callback information identification.
     */
    protected native static void onReceiveCookieValue(String value, long asyncCallbackInfoId);

    /**
     * Native callback function.
     *
     * @param value Value.
     * @param asyncCallbackInfoId Asynchronous callback information identification.
     */
    protected native static void onReceiveCookieValue(boolean value, long asyncCallbackInfoId);

    /**
     * Native callback function.
     *
     * @param asyncCallbackInfoId Asynchronous callback information identification.
     */
    protected native static void onReceiveCookieValue(long asyncCallbackInfoId);
}