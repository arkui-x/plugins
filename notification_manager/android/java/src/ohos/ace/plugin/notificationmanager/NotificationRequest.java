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

/**
 * NotificationRequest
 *
 * @since 2024-06-24
 */
public class NotificationRequest {
    /**
     * id is the id of the notification.
     */
    public Integer id = 1;

    /**
     * label is the label of the notification.
     */
    public String label;

    /**
     * deliveryTime is the time when the notification is delivered to the user.
     */
    public Long deliveryTime;

    /**
     * autoDeletedTime is the time when the notification is automatically deleted.
     */
    public Long autoDeletedTime;

    /**
     * tapDismissed is whether the notification is dismissed when the user taps it.
     */
    public boolean tapDismissed;

    /**
     * isAlertOnce is whether the notification is only alerted once.
     */
    public boolean isAlertOnce;

    /**
     * isStopwatch is whether the notification is a stopwatch.
     */
    public boolean isStopwatch;

    /**
     * isCountdown is whether the notification is a countdown.
     */
    public boolean isCountdown;

    /**
     * isOngoing is whether the notification is ongoing.
     */
    public boolean isOngoing;

    /**
     * showDeliveryTime is whether the notification shows the delivery time.
     */
    public boolean showDeliveryTime;

    /**
     * smallIcon is the small icon of the notification.
     */
    public String smallIcon;

    /**
     * largeIcon is the large icon of the notification.
     */
    public String largeIcon;

    /**
     * badgeNumber is the badge number of the notification.
     */
    public Integer badgeNumber;

    /**
     * notificationContentType is the content type of the notification.
     */
    public String notificationContentType;

    /**
     * progressMax is the maximum progress value of the notification.
     */
    public Integer progressMax;

    /**
     * progressValue is the progress value of the notification.
     * */
    public Integer progressValue;

    /**
     * content is the content of the notification.
     */
    public NotificationContent content;

    /**
     * NotificationContent is the content of the notification.
     */
    public static class NotificationContent {
        /**
         * contentType is the content type of the notification.
         */
        public int contentType;

        /**
         * content is the content of the notification.
         */
        public NotificationBasicContent content;
    }

    /**
     * NotificationBasicContent is the basic content of the notification.
     */
    public static class NotificationBasicContent {
        /**
         * title is the title of the notification.
         */
        public String title;

        /**
         * text is the text of the notification.
         */
        public String text;
    }

    /**
     * NotificationLongTextContent is the long text content of the notification.
     */
    public static class NotificationLongTextContent extends NotificationBasicContent {
        /**
         * longText is the long text of the notification.
         */
        public String longText;

        /**
         * briefText is the brief text of the notification.
         */
        public String briefText;

        /**
         * expandedTitle is the expanded title of the notification.
         */
        public String expandedTitle;
    }

    /**
     * NotificationMultiLineContent is the multi-line content of the notification.
     */
    public static class NotificationMultiLineContent extends NotificationBasicContent {
        /**
         * briefText is the brief text of the notification.
         */
        public String briefText;

        /**
         * expandedTitle is the expanded title of the notification.
         */
        public String expandedTitle;

        /**
         * allLines is the all lines of the notification.
         */
        public String[] allLines;
    }

    /**
     * NotificationPictureContent is the picture content of the notification.
     */
    public static class NotificationPictureContent extends NotificationBasicContent {
        /**
         * briefText is the brief text of the notification.
         */
        public String briefText;

        /**
         * expandedTitle is the expanded title of the notification.
         */
        public String expandedTitle;

        /**
         * picture is the picture of the notification.
         */
        public String picture;
    }
}
