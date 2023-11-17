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

public class WebCookiePlugin {
    private static final String LOG_TAG = "WebCookiePlugin";

    public WebCookiePlugin(Context context) {
        nativeInit();
    }

    public void configCookie(String url, String StringCookie) {
        CookieManager.getInstance().setCookie(url, StringCookie, new ValueCallback<Boolean>() {
            @Override
            public void onReceiveValue(Boolean value) {
                onReceiveCookieValue(true);
            }
        });
    }

    public void fetchCookie(String url) {
        String cookies = CookieManager.getInstance().getCookie(url);
        if (cookies == null) {
            cookies = "";
        }
        onReceiveCookieValue(cookies);
    }

    public void clearAllCookies() {
        CookieManager.getInstance().removeAllCookies(new ValueCallback<Boolean>() {
            @Override
            public void onReceiveValue(Boolean value) {
                onReceiveCookieValue();
            }
        });
    }

    protected native void nativeInit();

    protected native static void onReceiveCookieValue(String value);

    protected native static void onReceiveCookieValue(boolean value);

    protected native static void onReceiveCookieValue();
}