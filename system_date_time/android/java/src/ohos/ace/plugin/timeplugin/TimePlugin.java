/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

package ohos.ace.plugin.timeplugin;

import java.util.TimeZone;
import android.content.Context;

/**
 * The TimePlugin class provides the function to obtain time zones.
 *
 * @since 2025/1/10
 */
public class TimePlugin {
    private static final String LOG_TAG = "TimePlugin";
    private Context mContext_ = null;

    /**
     * Constructs a TestPlugin instance with the given context.
     *
     * @param context The context of the activity.
     */
    public TimePlugin(Context context) {
        mContext_ = context;
        nativeInit();
    }

    /**
     * Getting the time zone name.
     *
     * @param timezoneId Used to receive the obtained time zone name.
     * @return 0 Return the call result. 0 indicates success.
     */
    public int getTimeZone(String[] timezoneId) {
        TimeZone timeZone = TimeZone.getDefault();
        timezoneId[0] = timeZone.getID();
        return 0;
    }

    /**
     * Initializes the native layer by calling the corresponding C++ function to register JNI.
     */
    public native void nativeInit();
}
