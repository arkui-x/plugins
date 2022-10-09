/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
import android.text.format.DateFormat;
import android.util.Log;

public class I18NPlugin {
    private static final String LOG_TAG = "I18NPlugin";
    private Context mContext;

    public I18NPlugin(Context context) {
        mContext = context;
        nativeInit();
    }

    public boolean is24HourClock() {
        if (mContext == null) {
            Log.w(LOG_TAG, "I18NPlugin: context not registered");
            return true;
        }
        return DateFormat.is24HourFormat(mContext);
    }

    protected native void nativeInit();
}
