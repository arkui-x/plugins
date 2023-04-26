/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
import android.os.LocaleList;
import android.text.format.DateFormat;
import android.util.Log;
import java.util.Locale;
import java.util.TimeZone;

public class INTLPlugin {
    private static final String LOG_TAG = "INTLPlugin";
    private Context mContext;

    public INTLPlugin(Context context) {
        mContext = context;
        nativeInit();
    }

    public boolean is24HourClock() {
        if (mContext == null) {
            Log.w(LOG_TAG, "INTLPlugin: context not registered");
            return true;
        }
        return DateFormat.is24HourFormat(mContext);
    }

    public String getSystemLocale() {
        Locale systemLocale =  LocaleList.getDefault().get(0);
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

    public String getSystemTimezone() {
        return TimeZone.getDefault().getID();
    }

    protected native void nativeInit();
}
