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
import android.content.Context;
import android.content.pm.PackageManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Canvas;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.Icon;
import android.text.TextUtils;
import android.util.Log;

/**
 * NotificationTools class is used to create notification style
 *
 * @since 2024-06-24
 */
public class NotificationTools {
    private static final String TAG = NotificationTools.class.getSimpleName();

    /**
     * create style from notification content
     */
    public enum ContentType {
        NONE(0),
        BASIC_TEXT(1),
        CONVERSATION(2),
        LONG_TEXT(3),
        MEDIA(4),
        MULTILINE(5),
        PICTURE(6),
        LOCAL_LIVE_VIEW(7),
        LIVE_VIEW(8);
        private int value;

        private ContentType(int typeValue) {
            value = typeValue;
        }

        public int getValue() {
            return value;
        }
    };

    /**
     * create style from notification content
     *
     * @param context application context
     * @param content notification content
     * @return Notification.Style
     */
    public static Notification.Style getStyle(Context context, NotificationRequest.NotificationContent content) {
        if (context == null) {
            return null;
        }
        if (content == null || content.content == null) {
            return null;
        }
        if (content.contentType == ContentType.LONG_TEXT.getValue()) {
            if (!(content.content instanceof NotificationRequest.NotificationLongTextContent)) {
                return null;
            }
            NotificationRequest.NotificationLongTextContent longTextContent =
                (NotificationRequest.NotificationLongTextContent) content.content;
            Notification.BigTextStyle style = new Notification.BigTextStyle();
            style.bigText(longTextContent.longText);
            style.setSummaryText(longTextContent.briefText);
            style.setBigContentTitle(longTextContent.expandedTitle);
            return style;
        }
        if (content.contentType == ContentType.MULTILINE.getValue()) {
            if (!(content.content instanceof NotificationRequest.NotificationMultiLineContent)) {
                return null;
            }
            NotificationRequest.NotificationMultiLineContent linesTextContent =
                (NotificationRequest.NotificationMultiLineContent) content.content;
            Notification.InboxStyle inboxStyle = new Notification.InboxStyle();
            if (linesTextContent.allLines != null) {
                for (int i = 0; i < linesTextContent.allLines.length; i++) {
                    inboxStyle.addLine(linesTextContent.allLines[i]);
                }
            }
            inboxStyle.setSummaryText(linesTextContent.briefText);
            inboxStyle.setBigContentTitle(linesTextContent.expandedTitle);
            return inboxStyle;
        }
        if (content.contentType == ContentType.PICTURE.getValue()) {
            if (!(content.content instanceof NotificationRequest.NotificationPictureContent)) {
                return null;
            }
            NotificationRequest.NotificationPictureContent pictureContent =
                (NotificationRequest.NotificationPictureContent) content.content;
            Notification.BigPictureStyle pictureStyle = new Notification.BigPictureStyle();
            pictureStyle.setSummaryText(pictureContent.briefText);
            pictureStyle.setBigContentTitle(pictureContent.expandedTitle);
            pictureStyle.bigPicture(getBitmapFromFilePath(pictureContent.picture));
            return pictureStyle;
        }
        return null;
    }

    /**
     * get Bitmap from file path
     *
     * @param filePath the picture Path
     * @return Bitmap
     */
    public static Bitmap getBitmapFromFilePath(String filePath) {
        Bitmap bitmap = null;
        if (TextUtils.isEmpty(filePath)) {
            return bitmap;
        }
        try {
            bitmap = BitmapFactory.decodeFile(filePath);
        } catch (OutOfMemoryError except) {
            Log.e(TAG, "getBitmapFromFilePath except");
        }
        return bitmap;
    }

    /**
     * get Icon from file path
     *
     * @param context the context
     * @param filePath the picture Path
     * @return Icon
     */
    public static Icon getIconFromFilePath(Context context, String filePath) {
        if (TextUtils.isEmpty(filePath)) {
            return getAppIcon(context);
        }
        Icon icon = Icon.createWithBitmap(getBitmapFromFilePath(filePath));
        return icon;
    }

    private static Icon getAppIcon(Context context) {
        Icon icon = null;
        if (context == null) {
            return icon;
        }
        PackageManager packageManager = context.getPackageManager();
        Drawable drawable = null;
        try {
            drawable = packageManager.getApplicationIcon(context.getPackageName());
        } catch (PackageManager.NameNotFoundException e) {
            Log.e(TAG, "getAppIcon error");
            return icon;
        }
        icon = Icon.createWithBitmap(drawableToBitmap(drawable));
        return icon;
    }

    private static Bitmap drawableToBitmap(Drawable drawable) {
        if (drawable instanceof BitmapDrawable) {
            return ((BitmapDrawable) drawable).getBitmap();
        }

        Bitmap bitmap = Bitmap.createBitmap(drawable.getIntrinsicWidth(),
                drawable.getIntrinsicHeight(), Bitmap.Config.ARGB_8888);
        Canvas canvas = new Canvas(bitmap);
        drawable.setBounds(0, 0, canvas.getWidth(), canvas.getHeight());
        drawable.draw(canvas);

        return bitmap;
    }

    /**
     * get First Upper String
     *
     * @param input the input string
     * @return String
     */
    public static String getFirstUpperStr(String input) {
        if (TextUtils.isEmpty(input) || input.length() < 1) {
            return input;
        }
        String output = input.substring(0, 1).toUpperCase();
        if (input.length() > 1) {
            output = output + input.substring(1);
        }
        return output;
    }
}
