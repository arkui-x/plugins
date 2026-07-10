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
import android.os.Handler;
import android.os.Looper;
import android.util.Log;
import android.webkit.CookieManager;
import android.webkit.ValueCallback;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.net.HttpCookie;
import java.net.URI;
import java.net.URISyntaxException;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.LinkedBlockingQueue;
import java.util.concurrent.ThreadPoolExecutor;
import java.util.concurrent.TimeUnit;

/**
 * WebCookiePlugin is used to manage cookies
 *
 * @since 2024-05-31
 */
public class WebCookiePlugin {
    private static final String LOG_TAG = "WebCookiePlugin";
    private static final String ACE_WEB_PLUGIN_BASE_CLASS_NAME = "ohos.ace.adapter.capability.web.AceWebPluginBase";
    private static final int NO_ERROR = 0;
    private static final int INVALID_URL = 17100002;
    private static final int INVALID_COOKIE_VALUE = 17100005;
    private static final int MAX_COOKIE_SIZE = 4096;

    // Cached reflection references — resolved once on first use.
    private static Class<?> sCachedPluginBaseClass = null;
    private static Method sCachedSetAcceptCookieMethod = null;
    private static Method sCachedSetAcceptThirdPartyCookieMethod = null;
    private static Method sCachedGetObjectMapMethod = null;
    private static final Map<Class<?>, Method> sCachedThirdPartyCookieMethods = new HashMap<>();

    private static final ThreadPoolExecutor COOKIE_EXECUTOR = new ThreadPoolExecutor(
        1, 1, 0L, TimeUnit.MILLISECONDS, new LinkedBlockingQueue<>());

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
     * Set cookie value asynchronously with option parameters.
     *
     * <p>Android public WebKit APIs only expose the shared cookie jar. The incognito and
     * includeHttpOnly flags are accepted for API compatibility but cannot be enforced with
     * independent jar semantics or precise HttpOnly overwrite control.</p>
     *
     * @param url url
     * @param stringCookie cookie
     * @param incognito whether to use incognito jar
     * @param includeHttpOnly whether HttpOnly overwrite should be allowed
     * @param asyncCallbackInfoId callback id
     */
    public void configCookie(String url, String stringCookie, boolean incognito, boolean includeHttpOnly,
            long asyncCallbackInfoId) {
        int validateResult = validateCookie(url, stringCookie);
        if (validateResult != NO_ERROR) {
            onReceiveCookieError(validateResult, asyncCallbackInfoId);
            return;
        }
        if (incognito) {
            Log.w(LOG_TAG, "configCookie ignores incognito on Android and uses the shared cookie store.");
        }
        if (!includeHttpOnly) {
            Log.w(LOG_TAG, "configCookie cannot enforce includeHttpOnly=false on Android.");
        }
        CookieManager.getInstance().setCookie(url, stringCookie, new ValueCallback<Boolean>() {
            @Override
            public void onReceiveValue(Boolean value) {
                if (Boolean.TRUE.equals(value)) {
                    onReceiveCookieValue(true, asyncCallbackInfoId);
                } else {
                    onReceiveCookieError(INVALID_COOKIE_VALUE, asyncCallbackInfoId);
                }
            }
        });
    }

    /**
     * Set cookie value synchronously.
     *
     * <p>Android public WebKit APIs only expose the shared cookie jar. The incognito and
     * includeHttpOnly flags are accepted for API compatibility but cannot be enforced with
     * independent jar semantics or precise HttpOnly overwrite control.</p>
     *
     * @param url url
     * @param stringCookie cookie value in Set-Cookie format
     * @param incognito whether to use incognito jar
     * @param includeHttpOnly whether HttpOnly overwrite should be allowed
     * @return error code
     */
    public int configCookieSync(String url, String stringCookie, boolean incognito, boolean includeHttpOnly) {
        int validateResult = validateCookie(url, stringCookie);
        if (validateResult != NO_ERROR) {
            return validateResult;
        }
        if (incognito) {
            Log.w(LOG_TAG, "configCookieSync ignores incognito on Android and uses the shared cookie store.");
        }
        if (!includeHttpOnly) {
            Log.w(LOG_TAG, "configCookieSync cannot enforce includeHttpOnly=false on Android.");
        }
        CookieManager cookieManager = CookieManager.getInstance();
        cookieManager.setCookie(url, stringCookie);
        return NO_ERROR;
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
     * Get cookie value synchronously.
     *
     * <p>Android public WebKit APIs only expose the shared cookie jar. The incognito flag is
     * accepted for API compatibility but cannot be enforced with an independent incognito store.</p>
     *
     * @param url url
     * @param incognito whether to use incognito jar
     * @return cookie string, or null when url is invalid
     */
    public String fetchCookieSync(String url, boolean incognito) {
        if (!isValidCookieUrl(url)) {
            return null;
        }
        if (incognito) {
            Log.w(LOG_TAG, "fetchCookieSync ignores incognito on Android and uses the shared cookie store.");
        }
        String cookies = CookieManager.getInstance().getCookie(url);
        return cookies == null ? "" : cookies;
    }

    /**
     * Update cookie access policy for the shared cookie store.
     *
     * @param accept whether cookies are allowed
     */
    public void putAcceptCookieEnabled(boolean accept) {
        updateAcceptCookieDefault(accept);
        CookieManager.getInstance().setAcceptCookie(accept);
    }

    /**
     * Update third-party cookie policy for current and future WebView instances.
     *
     * @param accept whether third-party cookies are allowed
     */
    public void putAcceptThirdPartyCookieEnabled(boolean accept) {
        updateThirdPartyCookieDefault(accept);
        List<Object> snapshot = new ArrayList<>(getRegisteredWebObjects().values());
        for (Object webObject : snapshot) {
            invokeSetAcceptThirdPartyCookies(webObject, accept);
        }
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
     * Clear cookies synchronously.
     *
     * <p>Android does not expose an independent incognito cookie jar through public WebKit APIs.
     * This implementation treats {@code incognito=true} as best-effort in-memory cookie cleanup by
     * removing session cookies, and {@code incognito=false} as shared-store cleanup by removing all
     * cookies.</p>
     *
     * <p><b>Platform limitation (Android):</b> {@code removeAllCookies} / {@code removeSessionCookies}
     * are asynchronous; their callbacks run on the WebView's Looper (the caller thread). Blocking to
     * wait would deadlock, so this method returns before the removal completes. A subsequent
     * {@link #existCookie} may transiently return stale cookies.</p>
     *
     * @param incognito whether to clear incognito-mode cookies
     */
    public void clearAllCookiesSync(boolean incognito) {
        CookieManager cookieManager = CookieManager.getInstance();
        if (incognito) {
            // Android has no public API to clear cookies of a specific incognito WebView instance.
            // Best-effort: remove all session cookies (which are the only cookies written by
            // incognito WebViews on Android). Persistent cookies from normal browsing are NOT
            // affected. This is a known platform limitation.
            cookieManager.removeSessionCookies(null);
        } else {
            cookieManager.removeAllCookies(null);
        }
    }

    /**
     * Clear session cookies synchronously.
     *
     * <p>Uses {@code removeSessionCookies(null)} which is the non-deprecated API (API 21+).</p>
     *
     * <p><b>Platform limitation (Android):</b> {@code removeSessionCookies} is asynchronous; its
     * callback runs on the WebView's Looper (the caller thread). Blocking to wait would deadlock,
     * so this method returns before the removal completes. A subsequent {@link #existCookie} may
     * transiently return stale cookies.</p>
     */
    public void clearSessionCookieSync() {
        CookieManager.getInstance().removeSessionCookies(null);
    }

    /**
     * Flush cookies to disk asynchronously.
     *
     * <p>Android {@code CookieManager.flush()} has no async variant. This implementation runs
     * flush on a background thread and notifies the caller via the native callback once done.</p>
     *
     * @param asyncCallbackInfoId callback id
     */
    public void saveCookie(long asyncCallbackInfoId) {
        COOKIE_EXECUTOR.execute(() -> {
            CookieManager.getInstance().flush();
            new Handler(Looper.getMainLooper()).post(() -> onReceiveSaveCookieValue(asyncCallbackInfoId));
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
     * Native error callback function.
     *
     * @param errorCode Error code.
     * @param asyncCallbackInfoId Asynchronous callback information identification.
     */
    protected static native void onReceiveCookieError(int errorCode, long asyncCallbackInfoId);

    /**
     * Native callback function.
     *
     * @param asyncCallbackInfoId Asynchronous callback information identification.
     */
    protected native static void onReceiveCookieValue(long asyncCallbackInfoId);

    /**
     * Native callback function for SaveCookie completion.
     *
     * @param asyncCallbackInfoId Asynchronous callback information identification.
     */
    protected static native void onReceiveSaveCookieValue(long asyncCallbackInfoId);

    private void updateAcceptCookieDefault(boolean accept) {
        try {
            Method method = getSetAcceptCookieMethod();
            if (method != null) {
                method.invoke(null, accept);
            }
        } catch (IllegalAccessException | InvocationTargetException e) {
            Log.e(LOG_TAG, "Failed to update cookie access default.", e);
        }
    }

    private void updateThirdPartyCookieDefault(boolean accept) {
        try {
            Method method = getSetAcceptThirdPartyCookieMethod();
            if (method != null) {
                method.invoke(null, accept);
            }
        } catch (IllegalAccessException | InvocationTargetException e) {
            Log.e(LOG_TAG, "Failed to update third-party cookie default.", e);
        }
    }

    private Map<?, ?> getRegisteredWebObjects() {
        try {
            Method method = getGetObjectMapMethod();
            if (method != null) {
                Object objectMap = method.invoke(null);
                if (objectMap instanceof Map<?, ?>) {
                    return (Map<?, ?>) objectMap;
                }
            }
        } catch (IllegalAccessException | InvocationTargetException e) {
            Log.e(LOG_TAG, "Failed to get registered WebView objects.", e);
        }
        return java.util.Collections.emptyMap();
    }

    private static synchronized Class<?> getPluginBaseClass() {
        if (sCachedPluginBaseClass == null) {
            try {
                sCachedPluginBaseClass = Class.forName(ACE_WEB_PLUGIN_BASE_CLASS_NAME);
            } catch (ClassNotFoundException e) {
                Log.e(LOG_TAG, "AceWebPluginBase class not found.", e);
            }
        }
        return sCachedPluginBaseClass;
    }

    private static synchronized Method getSetAcceptCookieMethod() {
        if (sCachedSetAcceptCookieMethod == null) {
            Class<?> cls = getPluginBaseClass();
            if (cls != null) {
                try {
                    sCachedSetAcceptCookieMethod = cls.getMethod("setAcceptCookieEnabled", boolean.class);
                } catch (NoSuchMethodException e) {
                    Log.e(LOG_TAG, "setAcceptCookieEnabled method not found.", e);
                }
            }
        }
        return sCachedSetAcceptCookieMethod;
    }

    private static synchronized Method getSetAcceptThirdPartyCookieMethod() {
        if (sCachedSetAcceptThirdPartyCookieMethod == null) {
            Class<?> cls = getPluginBaseClass();
            if (cls != null) {
                try {
                    sCachedSetAcceptThirdPartyCookieMethod =
                        cls.getMethod("setAcceptThirdPartyCookieEnabled", boolean.class);
                } catch (NoSuchMethodException e) {
                    Log.e(LOG_TAG, "setAcceptThirdPartyCookieEnabled method not found.", e);
                }
            }
        }
        return sCachedSetAcceptThirdPartyCookieMethod;
    }

    private static synchronized Method getGetObjectMapMethod() {
        if (sCachedGetObjectMapMethod == null) {
            Class<?> cls = getPluginBaseClass();
            if (cls != null) {
                try {
                    sCachedGetObjectMapMethod = cls.getMethod("getObjectMap");
                } catch (NoSuchMethodException e) {
                    Log.e(LOG_TAG, "getObjectMap method not found.", e);
                }
            }
        }
        return sCachedGetObjectMapMethod;
    }

    private static synchronized Method getSetThirdPartyCookieMethod(Class<?> cls) {
        if (cls == null) {
            return null;
        }
        Method cached = sCachedThirdPartyCookieMethods.get(cls);
        if (cached != null) {
            return cached;
        }
        try {
            Method method = cls.getMethod("setAcceptThirdPartyCookies", boolean.class);
            sCachedThirdPartyCookieMethods.put(cls, method);
            return method;
        } catch (NoSuchMethodException e) {
            Log.e(LOG_TAG, "setAcceptThirdPartyCookies method not found.", e);
            return null;
        }
    }

    private void invokeSetAcceptThirdPartyCookies(Object webObject, boolean accept) {
        if (webObject == null) {
            return;
        }
        try {
            Method method = getSetThirdPartyCookieMethod(webObject.getClass());
            if (method != null) {
                method.invoke(webObject, accept);
            }
        } catch (IllegalAccessException | InvocationTargetException | IllegalArgumentException e) {
            Log.e(LOG_TAG, "Failed to update third-party cookie for WebView.", e);
        }
    }

    private int validateCookie(String url, String stringCookie) {
        if (!isValidCookieUrl(url)) {
            return INVALID_URL;
        }
        if (!isValidCookieValue(stringCookie)) {
            return INVALID_COOKIE_VALUE;
        }
        if (stringCookie.getBytes(StandardCharsets.UTF_8).length > MAX_COOKIE_SIZE) {
            return INVALID_COOKIE_VALUE;
        }
        if (containsAttribute(stringCookie, "Secure") && !isHttpsUrl(url)) {
            return INVALID_URL;
        }
        return NO_ERROR;
    }

    private boolean isValidCookieUrl(String url) {
        if (url == null || url.isEmpty()) {
            return false;
        }
        try {
            URI parsedUrl = new URI(url);
            String scheme = parsedUrl.getScheme();
            return scheme != null;
        } catch (URISyntaxException exception) {
            return false;
        }
    }

    private boolean isHttpsUrl(String url) {
        try {
            URI parsedUrl = new URI(url);
            return "https".equalsIgnoreCase(parsedUrl.getScheme());
        } catch (URISyntaxException exception) {
            return false;
        }
    }

    private boolean isValidCookieValue(String stringCookie) {
        if (stringCookie == null || stringCookie.isEmpty()) {
            return false;
        }
        try {
            List<HttpCookie> cookies = HttpCookie.parse(stringCookie);
            return cookies != null && cookies.size() == 1;
        } catch (IllegalArgumentException exception) {
            return false;
        }
    }

    private boolean containsAttribute(String stringCookie, String attribute) {
        String[] segments = stringCookie.split(";");
        for (String segment : segments) {
            if (attribute.equalsIgnoreCase(segment.trim())) {
                return true;
            }
        }
        return false;
    }
}
