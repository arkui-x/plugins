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

package ohos.ace.plugin.intlplugin;

import android.content.Context;
import android.content.res.Resources;
import android.icu.text.NumberingSystem;
import android.os.Build;
import android.os.LocaleList;
import android.text.format.DateFormat;
import android.util.Log;
import android.util.DisplayMetrics;
import android.view.Display;
import android.view.WindowManager;

import java.util.Locale;
import java.util.TimeZone;
import java.util.Calendar;

/**
 * INTL android plugin module
 *
 * @since 2023-04
 */
public class INTLPlugin {
    private static final String LOG_TAG = "INTLPlugin";
    private static final String DEVICE_TYPE_PHONES = "phones";
    private static final String DEVICE_TYPE_TABLET = "tablet";
    private static final int SQUARE = 2;
    private static final int THRESHOLD_DP = 600;
    private static final double THRESHOLD_INCH = 6.9;

    private Context mContext;

    /**
     * INTLPlugin
     *
     * @param context context of the application
     */
    public INTLPlugin(Context context) {
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
            Log.w(LOG_TAG, "INTLPlugin: context not registered");
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
     * getSystemTimezone
     *
     * @return System time zone
     */
    public String getSystemTimezone() {
        return TimeZone.getDefault().getID();
    }

    /**
     * getSystemCalendar
     *
     * @return System calendar
     */
    public String getSystemCalendar() {
        return Calendar.getInstance().getCalendarType();
    }

    /**
     * getNumberingSystem
     *
     * @return The digital system used by the system
     */
    public String getNumberingSystem() {
        Locale systemLocale = LocaleList.getDefault().get(0);
        NumberingSystem numberingSystem = NumberingSystem.getInstance(systemLocale);
        return numberingSystem.getName();
    }

    /**
     * Get device type.
     *
     * @return Device type.
     */
    public String getDeviceType() {
        String deviceType = DEVICE_TYPE_PHONES;
        if (mContext == null) {
            Log.e(LOG_TAG, "The mContext is null, getDeviceType failed.");
            return deviceType;
        }

        int minScreenWidth = mContext.getResources().getConfiguration().smallestScreenWidthDp;
        if (minScreenWidth >= THRESHOLD_DP) {
            deviceType = DEVICE_TYPE_TABLET;
        } else {
            deviceType = getDeviceTypeByPhysicalSize();
        }
        return deviceType;
    }

    private String getDeviceTypeByPhysicalSize() {
        String deviceType = DEVICE_TYPE_PHONES;
        if (mContext == null) {
            Log.e(LOG_TAG, "The mContext is null, getDeviceTypeByPhysicalSize failed.");
            return deviceType;
        }

        WindowManager windowManager = (WindowManager) mContext.getSystemService(Context.WINDOW_SERVICE);
        if (windowManager == null) {
            Log.e(LOG_TAG, "The mContext is null, getDeviceTypeByPhysicalSize failed.");
            return deviceType;
        }

        Display display = windowManager.getDefaultDisplay();
        if (display == null) {
            Log.e(LOG_TAG, "The display is null, getDeviceTypeByPhysicalSize failed.");
            return deviceType;
        }

        DisplayMetrics metrics = new DisplayMetrics();
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.JELLY_BEAN_MR1) {
            display.getRealMetrics(metrics);
        } else {
            display.getMetrics(metrics);
        }

        double width = metrics.widthPixels / metrics.xdpi;
        double height = metrics.heightPixels / metrics.ydpi;
        double diagonalSize = Math.sqrt(Math.pow(width, SQUARE) + Math.pow(height, SQUARE));

        if (diagonalSize >= THRESHOLD_INCH) {
            deviceType = DEVICE_TYPE_TABLET;
        }
        return deviceType;
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
