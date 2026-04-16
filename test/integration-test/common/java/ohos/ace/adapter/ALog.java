/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
package ohos.ace.adapter;

import android.util.Log;

/**
 * ALog is a log utility that delegates to android.util.Log.
 *
 * @since 1
 */
public class ALog {
    /**
     * Log a message at the INFO level.
     *
     * @param tag tag for the log message
     * @param msg the message to log
     */
    public static void i(String tag, String msg) {
        Log.i(tag, msg);
    }

    /**
     * Log a message at the DEBUG level.
     *
     * @param tag tag for the log message
     * @param msg the message to log
     */
    public static void d(String tag, String msg) {
        Log.d(tag, msg);
    }

    /**
     * Log a message at the ERROR level.
     *
     * @param tag tag for the log message
     * @param msg the message to log
     */
    public static void e(String tag, String msg) {
        Log.e(tag, msg);
    }

    /**
     * Log a message at the WARN level.
     *
     * @param tag tag for the log message
     * @param msg the message to log
     */
    public static void w(String tag, String msg) {
        Log.w(tag, msg);
    }
}
