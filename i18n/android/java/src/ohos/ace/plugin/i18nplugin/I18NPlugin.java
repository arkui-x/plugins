/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
import android.content.SharedPreferences;
import android.content.res.Resources;
import android.icu.text.NumberingSystem;
import android.os.Build;
import android.os.LocaleList;
import android.text.format.DateFormat;
import android.util.Log;

import java.util.HashMap;
import java.util.LinkedHashSet;
import java.util.Locale;
import java.util.Map;
import java.util.Set;
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

    private static final Map<String, String> localDigitMap = new HashMap<String, String>() {{
        put("ar", "arab");
        put("as", "beng");
        put("bn", "beng");
        put("fa", "arabext");
        put("mr", "deva");
        put("my", "mymr");
        put("ne", "deva");
        put("ur", "latn");
    }};

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
            Log.e(LOG_TAG, "I18NPlugin: context not registered");
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
        String languageTag = "";
        if (systemLocale != null) {
            languageTag += systemLocale.getLanguage();
            if (systemLocale.getScript().length() > 0) {
                languageTag += "-" + systemLocale.getScript();
            }
        }
        return languageTag;
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

    /**
     * getSystemLanguages
     *
     * @return System languages
     */
    public String[] getSystemLanguages() {
        Locale[] availableLocales = Locale.getAvailableLocales();
        Set<String> languageSet = new LinkedHashSet<>();
        for (int index = 0; index < availableLocales.length; index++) {
            String languageTag = "";
            if (availableLocales[index] != null) {
                languageTag += availableLocales[index].getLanguage();
                if (availableLocales[index].getScript().length() > 0) {
                    languageTag += "-" + availableLocales[index].getScript();
                }
                languageSet.add(languageTag);
            }
        }
        return languageSet.toArray(new String[languageSet.size()]);
    }

    /**
     * getAvailableIDs
     *
     * @return System time zone IDs
     */
    public String[] getAvailableIDs() {
        return TimeZone.getAvailableIDs();
    }

    /**
     * getSystemCountries
     *
     * @param language language
     * @return Countries
     */
    public String[] getSystemCountries(String language) {
        Locale[] availableLocales = Locale.getAvailableLocales();
        Set<String> countrySet = new LinkedHashSet<>();
        for (int index = 0; index < availableLocales.length; index++) {
            if (availableLocales[index] == null || availableLocales[index].getCountry() == null ||
                    availableLocales[index].getCountry().isEmpty()) {
                continue;
            }

            if (availableLocales[index].getCountry().matches("\\d{3}")) {
                continue;
            }

            if (language == null || language.isEmpty()) {
                countrySet.add(availableLocales[index].getCountry().toUpperCase(Locale.ROOT));
                continue;
            }

            if (availableLocales[index].getLanguage().toUpperCase(Locale.ROOT)
                    .equals(language.toUpperCase(Locale.ROOT))) {
                countrySet.add(availableLocales[index].getCountry().toUpperCase(Locale.ROOT));
            }
        }
        return countrySet.toArray(new String[countrySet.size()]);
    }

    private boolean checkParamValid(String param) {
        if (param == null || param.isEmpty()) {
            return false;
        }
        return true;
    }

    private boolean checkLocaleValid(Locale locale) {
        if (locale == null ||
                locale.getLanguage() == null ||
                locale.getLanguage().isEmpty() ||
                locale.getCountry() == null ||
                locale.getCountry().isEmpty()) {
            return false;
        }
        return true;
    }

    /**
     * isSuggested
     *
     * @param language language
     * @param region region
     * @return Is suggested language
     */
    public boolean isSuggested(String language, String region) {
        if (!checkParamValid(language)) {
            return false;
        }

        String resolvedRegion = region;
        if (!checkParamValid(resolvedRegion)) {
            Locale systemLocale = getSystemLocaleCompat();
            if (systemLocale == null) {
                return false;
            }
            resolvedRegion = systemLocale.getCountry();
        }

        Locale[] availableLocales = Locale.getAvailableLocales();
        for (int index = 0; index < availableLocales.length; index++) {
            if (!checkLocaleValid(availableLocales[index])) {
                continue;
            }

            if (availableLocales[index].getCountry().equals(resolvedRegion) &&
                    availableLocales[index].getLanguage().equals(language)) {
                return true;
            }
        }

        return false;
    }

    /**
     * getPreferredLanguageList
     *
     * @return The preferred language list
     */
    public String[] getPreferredLanguageList() {
        // LocaleList is a user-set preference
        LocaleList locales = LocaleList.getDefault();
        String[] localesResult = new String[locales.size()];
        for (int index = 0; index < locales.size(); index++) {
            String languageTag = "";
            if (locales.get(index) != null) {
                languageTag += locales.get(index).getLanguage();
                if (locales.get(index).getScript().length() > 0) {
                    languageTag += "-" + locales.get(index).getScript();
                }
                localesResult[index] = languageTag;
            }
        }
        return localesResult;
    }

    /**
     * getFirstPreferredLanguage
     *
     * @return The preferred language
     */
    public String getFirstPreferredLanguage() {
        String languageTag = "";
        LocaleList locales = LocaleList.getDefault();
        String[] localesResult = new String[locales.size()];
        for (int index = 0; index < locales.size(); index++) {
            if (locales.get(index) != null) {
                languageTag += locales.get(index).getLanguage();
                if (locales.get(index).getScript().length() > 0) {
                    languageTag += "-" + locales.get(index).getScript();
                }
                return languageTag;
            }
        }
        return languageTag;
    }

    /**
     * getUsingLocalDigit
     *
     * @return Using local digit
     */
    public boolean getUsingLocalDigit() {
        Locale locale = getSystemLocaleCompat();
        if (locale == null) {
            return false;
        }

        if (localDigitMap.containsKey(locale.getLanguage())) {
            NumberingSystem numberingSystem = NumberingSystem.getInstance(locale);
            if (numberingSystem == null) {
                return false;
            }
            return localDigitMap.get(locale.getLanguage()).equals(numberingSystem.getName());
        }
        return false;
    }
}
