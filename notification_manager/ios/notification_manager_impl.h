/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_WIFI_MANAGER_IOS_WIFI_DEVICE_IMPL_H
#define PLUGINS_WIFI_MANAGER_IOS_WIFI_DEVICE_IMPL_H

#include "ans_notification.h"

namespace OHOS::Notification {
class NotificationManagerImpl : public AnsNotification {
public:
    /**
     * @brief Publishes a notification.
     * @note If a notification with the same ID has been published by the current application and has not been deleted,
     * this method will update the notification.
     *
     * @param request Indicates the NotificationRequest object for setting the notification content.
     *                This parameter must be specified.
     * @param asynccallbackinfo AsyncCallbackInfoPublish
     * @param callback NotificationCallback
     */
    virtual void PublishNotification(const NotificationRequest &request, void* asynccallbackinfo, NotificationCallback callback) override;

    /**
     * @brief Cancels a published notification.
     *
     * @param notificationId Indicates the unique notification ID in the application.
     *                       The value must be the ID of a published notification.
     *                       Otherwise, this method does not take effect.
     * @return Returns cancel notification result.
     */
    virtual ErrorCode CancelNotification(int32_t notificationId) override;

    /**
     * @brief Cancels all the published notifications.
     * @note To cancel a specified notification, see CancelNotification(int_32).
     *
     * @return Returns cancel all notifications result.
     */
    virtual ErrorCode CancelAllNotifications() override;

    /**
     * @brief Checks whether this application has permission to publish notifications.
     *
     * @param asynccallbackinfo AsyncCallbackInfoPublish
     * @param callback NotificationCallback
     */
    virtual void IsAllowedNotifySelf(void* asynccallbackinfo, EnableNotificationCallback callback) override;

    /**
     * @brief Set badge number.
     *
     * @param badgeNumber The badge number.
     * @return Returns set badge number result.
     */
    virtual ErrorCode SetBadgeNumber(int32_t badgeNumber) override;

    /**
     * @brief Request permission to publish notifications.
     *
     * @param asynccallbackinfo AsyncCallbackInfoPublish
     * @param callback NotificationCallback
     */
    virtual void RequestEnableNotification(void* asynccallbackinfo, NotificationCallback callback) override;
};
} // namespace OHOS::Plugin

#endif // PLUGINS_WIFI_MANAGER_IOS_WIFI_DEVICE_IMPL_H
