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

public class NotificationRequest {
    public Integer id = 1;
    public String label;
    public Long deliveryTime;
    public Long autoDeletedTime;
    public boolean tapDismissed;
    public boolean isAlertOnce;
    public boolean isStopwatch;
    public boolean isCountdown;
    public boolean isOngoing;
    public boolean showDeliveryTime;
    public String smallIcon;
    public String largeIcon;
    public Integer badgeNumber;
    public String notificationContentType;
    public Integer progressMax;
    public Integer progressValue;
    public NotificationContent content;

    public static class NotificationContent {
        public int contentType;
        public NotificationBasicContent content;
    }

    public static class NotificationBasicContent {
        public String title;
        public String text;
    }

    public static class NotificationLongTextContent extends NotificationBasicContent {
        public String longText;
        public String briefText;
        public String expandedTitle;
    }

    public static class NotificationMultiLineContent extends NotificationBasicContent {
        public String briefText;
        public String expandedTitle;
        public String[] allLines;
    }

    public static class NotificationPictureContent extends NotificationBasicContent {
        public String briefText;
        public String expandedTitle;
        public String picture;
    }
}
