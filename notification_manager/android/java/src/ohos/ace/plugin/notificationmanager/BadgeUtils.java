/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

package ohos.ace.plugin.notificationmanager;

import android.app.Notification;
import android.content.ComponentName;
import android.content.ContentValues;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.List;

/**
 * BadgeUtils set badge number for different manufacturers of mobile phones
 *
 * @since 2024-06-24
 */
public class BadgeUtils {
    private static final String TAG = BadgeUtils.class.getSimpleName();

    /**
     * set badge number
     *
     * @param context application context
     * @param count badge number
     * @param notification notification
     */
    public static void setBadgeNumber(Context context, int count, Notification notification) {
        if (context == null) {
            return;
        }
        String manufacturer = Build.MANUFACTURER.toLowerCase();
        Log.d(TAG, "manufacturer:" + manufacturer);
        if (manufacturer.contains("xiaomi")) {
            setXiaoMiBadge(count, context, notification);
        } else {
            setBadgeNumber(context, count);
        }
    }

    /**
     * set badge number
     *
     * @param count   badge number
     * @param context application context
     */
    public static void setBadgeNumber(Context context, int count) {
        if (context == null) {
            return;
        }
        String manufacturer = Build.MANUFACTURER.toLowerCase();
        if (manufacturer.contains("huawei") || manufacturer.contains("honor")) {
            setHWOrHonorBadge(count, context);
        } else if (manufacturer.contains("samsung")) {
            setSamsungBadge(count, context);
        } else if (manufacturer.contains("oppo")) {
            setOPPOBadge(count, context);
        } else if (manufacturer.contains("vivo")) {
            setVivoBadge(count, context);
        } else {
            Log.d(TAG, "other phone");
            setBadgeCount(count, context);
        }
    }

    private static void setBadgeCount(int count, Context context) {
        try {
            Uri uri = Uri.parse("content://com.android.badge/badge");
            ContentValues contentValues = new ContentValues();
            contentValues.put("package", context.getPackageName());
            contentValues.put("class", getLauncherClassName(context));
            contentValues.put("badgecount", count);
            context.getContentResolver().update(uri, contentValues, null, null);
        } catch (RuntimeException except) {
            Log.e(TAG, "setBadgeCount except");
        }
    }

    /**
     * set badge number for xiaomi phone
     *
     * @param count   badge number
     * @param context applicationcontext
     * @param notification notification
     */
    private static void setXiaoMiBadge(int count, Context context, Notification notification) {
        if (notification == null) {
            return;
        }
        try {
            Field field = notification.getClass().getDeclaredField("extraNotification");
            Object extraNotification = field.get(notification);
            Method method = extraNotification.getClass().getDeclaredMethod("setMessageCount", int.class);
            method.invoke(extraNotification, count);
        } catch (IllegalAccessException exception) {
            exception.printStackTrace();
        } catch (IllegalArgumentException exception) {
            exception.printStackTrace();
        } catch (InvocationTargetException exception) {
            exception.printStackTrace();
        } catch (NoSuchMethodException exception) {
            exception.printStackTrace();
        } catch (SecurityException exception) {
            exception.printStackTrace();
        } catch (NoSuchFieldException exception) {
            exception.printStackTrace();
        }
    }

    private static void setHWOrHonorBadge(int count, Context context) {
        try {
            Bundle bunlde = new Bundle();
            bunlde.putString("package", context.getPackageName());
            String launchClassName = context.getPackageManager().getLaunchIntentForPackage(context.getPackageName())
                    .getComponent().getClassName();
            bunlde.putString("class", launchClassName);
            bunlde.putInt("badgenumber", count);
            context.getContentResolver().call(Uri.parse("content://com.huawei.android.launcher.settings/badge/"),
                    "change_badge", null, bunlde);
        } catch (RuntimeException except) {
            Log.e(TAG, "setBadgeCount except");
        }
    }

    private static void setSamsungBadge(int count, Context context) {
        String launcherClassName = getLauncherClassName(context);
        if (TextUtils.isEmpty(launcherClassName)) {
            return;
        }
        Intent intent = new Intent("android.intent.action.BADGE_COUNT_UPDATE");
        intent.putExtra("badge_count", count);
        intent.putExtra("badge_count_package_name", context.getPackageName());
        intent.putExtra("badge_count_class_name", launcherClassName);
        context.sendBroadcast(intent);
    }

    private static void setOPPOBadge(int count, Context context) {
        Intent intent = new Intent("com.oppo.unsettledevent");
        intent.putExtra("packageName", context.getPackageName());
        intent.putExtra("number", count);
        intent.putExtra("upgradeNumber", count);
        PackageManager packageManager = context.getPackageManager();

        List<ResolveInfo> receivers = packageManager.queryBroadcastReceivers(intent, 0);
        if (receivers != null && receivers.size() > 0) {
            context.sendBroadcast(intent);
        } else {
            setAppBadgeCount(count, context);
        }
    }

    private static void setAppBadgeCount(int count, Context context) {
        try {
            Bundle extras = new Bundle();
            extras.putInt("app_badge_count", count);
            context.getContentResolver().call(Uri.parse("content://com.android.badge/badge"),
                    "setAppBadgeCount", String.valueOf(count), extras);
        } catch (RuntimeException except) {
            Log.e(TAG, "setAppBadgeCount except");
        }
    }

    private static void setVivoBadge(int count, Context context) {
        String launcherClassName = getLauncherClassName(context);
        if (TextUtils.isEmpty(launcherClassName)) {
            return;
        }
        Intent intent = new Intent("launcher.action.CHANGE_APPLICATION_NOTIFICATION_NUM");
        intent.putExtra("packageName", context.getPackageName());
        intent.putExtra("className", launcherClassName);
        intent.putExtra("notificationNum", count);
        context.sendBroadcast(intent);
    }

    private static String getLauncherClassName(Context context) {
        ComponentName launchComponent = getLauncherComponentName(context);
        if (launchComponent == null) {
            return "";
        } else {
            return launchComponent.getClassName();
        }
    }

    private static ComponentName getLauncherComponentName(Context context) {
        Intent launchIntent = context.getPackageManager().getLaunchIntentForPackage(context
                .getPackageName());
        if (launchIntent != null) {
            return launchIntent.getComponent();
        } else {
            return null;
        }
    }
}
