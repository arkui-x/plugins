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

import android.app.Activity;
import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;

import android.app.PendingIntent;
import android.content.ActivityNotFoundException;
import android.content.ComponentName;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Bundle;
import android.text.TextUtils;
import android.util.Log;
import android.content.Context;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.util.Map;

public class NotificationPlugin {
    private static final String TAG = NotificationPlugin.class.getSimpleName();
    private static final String PERMISSION = "android.permission.POST_NOTIFICATIONS";
    private static final String CHANNER_ID = "arkuix";
    private NotificationManager mManager;
    Notification.Builder mBuilder;
    private NotificationChannel mNotificationChannel;
    private Context context;
    private NotificationRequest request;

    public NotificationPlugin(Context context) {
        if (context == null) {
            Log.e(TAG, " Notification context is null");
            return;
        }
        this.context = context;
        mManager = (NotificationManager) context.getSystemService(context.NOTIFICATION_SERVICE);

        mNotificationChannel = mManager.getNotificationChannel(CHANNER_ID);
        if (mNotificationChannel == null) {
            mNotificationChannel = new NotificationChannel(CHANNER_ID, context.getPackageName(),
                    NotificationManager.IMPORTANCE_DEFAULT);
            mNotificationChannel.setShowBadge(true);
            mManager.createNotificationChannel(mNotificationChannel);
        }

        mManager.createNotificationChannel(mNotificationChannel);
        nativeInit();
    }

    protected native void nativeInit();

    protected native void nativeReceiveCallback(String key, long code);

    public void requestEnableNotification() {
        Activity activity = getActivity();
        if (activity == null) {
            Log.d(TAG, "requestEnableNotification activity null");
            return;
        }
        Log.d(TAG, "requestEnableNotification requestPermissions " + activity);
        activity.requestPermissions(new String[] { PERMISSION }, 1);
    }

    public boolean isHighVersion() {
        // Build.VERSION_CODES.TIRAMISU
        Log.d(TAG, "isHighVersion version " + Build.VERSION.SDK_INT);
        return Build.VERSION.SDK_INT >= 33;
    }

    public boolean checkPermission() {
        if (!isHighVersion()) {
            return mManager.areNotificationsEnabled();
        }
        PackageManager pm = context.getPackageManager();
        if (pm.checkPermission(PERMISSION, context.getPackageName()) == PackageManager.PERMISSION_GRANTED) {
            return true;
        } else {
            return false;
        }
    }

    public void publish(String jsonString) {
        Log.d(TAG, "publish:" + jsonString);
        if (TextUtils.isEmpty(jsonString)) {
            return;
        }
        request = JsonUtils.getNotificationRequest(jsonString);
        createNotification();
    }

    public void publish(NotificationRequest detail) {
        request = detail;
        createNotification();
    }

    public void setBadgeNumber(int count) {
        BadgeUtils.setBadgeNumber(count, context);
    }

    public void cancel(int id, String tag) {
        if (!TextUtils.isEmpty(tag)) {
            mManager.cancel(tag, id);
        } else {
            mManager.cancel(id);
        }
    }

    public void cancelAll() {
        mManager.cancelAll();
    }

    private void createNotification() {
        mBuilder = getBuilder();
        Notification notification = mBuilder.build();
        if (request.badgeNumber != null && request.badgeNumber >= 0) {
            BadgeUtils.setBadgeNumber(request.badgeNumber, context, notification);
        }

        if (!TextUtils.isEmpty(request.label)) {
            mManager.notify(request.label, request.id, notification);
        } else {
            mManager.notify(request.id, notification);
        }
    }

    private Notification.Builder getBuilder() {
        Notification.Builder builder = new Notification.Builder(context);
        builder.setChannelId(CHANNER_ID)
                .setOngoing(request.isOngoing)
                .setUsesChronometer(request.isStopwatch)
                .setChronometerCountDown(request.isCountdown)
                .setShowWhen(request.showDeliveryTime)
                .setOnlyAlertOnce(request.isAlertOnce)
                .setAutoCancel(request.tapDismissed)
                .setSmallIcon(NotificationTools.getIconFromFilePath(context, request.smallIcon));

        if (request.content != null && request.content.content != null) {
            builder.setContentTitle(request.content.content.title);
            builder.setContentText(request.content.content.text);
        }
        if (!TextUtils.isEmpty(request.largeIcon)) {
            builder.setLargeIcon(NotificationTools.getBitmapFromFilePath(request.largeIcon));
        }
        if (request.deliveryTime != null && request.deliveryTime > 0) {
            builder.setWhen(request.deliveryTime);
        }
        if (request.autoDeletedTime != null && request.autoDeletedTime > 0) {
            builder.setTimeoutAfter(request.autoDeletedTime - System.currentTimeMillis());
        }
        if (request.badgeNumber != null && request.badgeNumber >= 0) {
            builder.setNumber(request.badgeNumber);
        }
        Notification.Style style = NotificationTools.getStyle(context, request.content);
        if (style != null) {
            builder.setStyle(style);
        }
        return builder;
    }

    private Activity getActivity() {
        try {
            Class activityThreadClass = Class.forName("android.app.ActivityThread");
            Object activityThread = activityThreadClass.getMethod("currentActivityThread").invoke(null);
            Field mActivities = activityThreadClass.getDeclaredField("mActivities");
            mActivities.setAccessible(true);
            Map activitiesMap = (Map) mActivities.get(activityThread);
            for (Object activityClientRecord : activitiesMap.values()) {
                Class activityClientRecordClass = activityClientRecord.getClass();
                Field paused = activityClientRecordClass.getDeclaredField("paused");
                paused.setAccessible(true);
                if (!paused.getBoolean(activityClientRecord)) {
                    Field activityField = activityClientRecordClass.getDeclaredField("activity");
                    activityField.setAccessible(true);
                    Activity activity = (Activity) activityField.get(activityClientRecord);
                    return activity;
                }
            }
        } catch (ClassNotFoundException ex) {
            ex.printStackTrace();
        } catch (InvocationTargetException ex) {
            ex.printStackTrace();
        } catch (NoSuchMethodException ex) {
            ex.printStackTrace();
        } catch (NoSuchFieldException ex) {
            ex.printStackTrace();
        } catch (IllegalAccessException ex) {
            ex.printStackTrace();
        }
        return null;
    }
}
