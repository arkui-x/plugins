/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

package ohos.ace.plugin.i18nplugin;

import android.content.Context;
import android.content.res.Resources;
import android.content.SharedPreferences;
import android.os.Build;
import android.text.format.DateFormat;
import android.util.Log;

import java.util.Locale;
import java.util.TimeZone;

/**
 * INTL android plugin module
 *
 * @since 2023-04
 */
public class I18NPlugin {
    private static final String LOG_TAG = "I18NPlugin";
    private static final String LANGUAGE_SHARE_PREFERENCE = "language_prefs";
    private static final String KEY_LANGUAGE = "app_language";

    private static I18NSetAppPreferredLanguage mRestartFunc = null;

    private Context mContext;

    /**
     * INTLPlugin
     *
     * @param context context of the application
     */
    public I18NPlugin(Context context) {
        mContext = context;
        nativeInit();
    }

    /**
     * is24HourClock
     *
     * @return System use 24 hour clock or not
     */
    public boolean is24HourClock() {
        if (mContext == null) {
            Log.w(LOG_TAG, "I18NPlugin: context not registered");
            return true;
        }
        return DateFormat.is24HourFormat(mContext);
    }

    /**
     * getSystemLocale
     *
     * @return System locale
     */
    public String getSystemLocale() {
        Locale systemLocale = getSystemLocaleCompat();
        String localeTag = "";
        if (systemLocale != null) {
            localeTag += systemLocale.getLanguage();
            if (systemLocale.getScript().length() > 0) {
                localeTag += "-" + systemLocale.getScript();
            }
            if (systemLocale.getCountry().length() > 0) {
                localeTag += "-" + systemLocale.getCountry();
            }
        }
        return localeTag;
    }

    /**
     * getSystemLanguage
     *
     * @return System language
     */
    public String getSystemLanguage() {
        Locale systemLocale = getSystemLocaleCompat();
        String langugeTag = "";
        if (systemLocale != null) {
            langugeTag += systemLocale.getLanguage();
            if (systemLocale.getScript().length() > 0) {
                langugeTag += "-" + systemLocale.getScript();
            }
        }
        return langugeTag;
    }

    /**
     * getSystemRegion
     *
     * @return System region
     */
    public String getSystemRegion() {
        Locale systemLocale = getSystemLocaleCompat();
        if (systemLocale == null) {
            return "";
        }
        return systemLocale.getCountry();
    }

    /**
     * getSystemTimezone
     *
     * @return System time zone
     */
    public String getSystemTimezone() {
        return TimeZone.getDefault().getID();
    }

    /**
     * getAppPreferredLanguage
     *
     * @return app preferred language
     */
    public String getAppPreferredLanguage() {
        if (mContext == null) {
            Log.w(LOG_TAG, "I18NPlugin: context not registered");
            return getSystemLocale();
        }
        SharedPreferences prefs =
            mContext.getSharedPreferences(LANGUAGE_SHARE_PREFERENCE, Context.MODE_PRIVATE);
        String savedLang = prefs.getString(KEY_LANGUAGE, null);
        if (savedLang != null && !savedLang.isEmpty()) {
            return savedLang;
        } else {
            return getSystemLocale();
        }
    }

    /**
     * setAppPreferredLanguage
     *
     * @param languageTag app preferred language
     */
    public void setAppPreferredLanguage(String languageTag) {
        if (mContext == null) {
            Log.w(LOG_TAG, "I18NPlugin: context not registered");
            return;
        }
        SharedPreferences.Editor editor =
            mContext.getSharedPreferences(LANGUAGE_SHARE_PREFERENCE, Context.MODE_PRIVATE).edit();
        editor.putString(KEY_LANGUAGE, languageTag);
        editor.commit();
        if (mRestartFunc != null) {
            mRestartFunc.restartApp();
        }
    }

    /**
     * setRestartFunc
     *
     * @param func function to restart app
     */
    public static void setRestartFunc(I18NSetAppPreferredLanguage func) {
        mRestartFunc = func;
    }

    /**
     * clear Restart function
     */
    public static void clearRestartFunc() {
        mRestartFunc = null;
    }

    /**
     * nativeInit
     */
    protected native void nativeInit();

    private Locale getSystemLocaleCompat() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.N) {
            return Resources.getSystem().getConfiguration().getLocales().get(0);
        } else {
            return Locale.getDefault();
        }
    }
}
