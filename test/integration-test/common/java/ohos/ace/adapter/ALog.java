/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * ALog stub for testing - delegates to android.util.Log.
 */
package ohos.ace.adapter;

import android.util.Log;

public class ALog {
    public static void i(String tag, String msg) {
        Log.i(tag, msg);
    }

    public static void d(String tag, String msg) {
        Log.d(tag, msg);
    }

    public static void e(String tag, String msg) {
        Log.e(tag, msg);
    }

    public static void w(String tag, String msg) {
        Log.w(tag, msg);
    }
}
