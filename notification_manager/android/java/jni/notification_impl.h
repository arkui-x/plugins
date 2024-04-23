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
#ifndef PLUGINS_NOTIFICATION_MANAGER_ANDROID_JAVA_JNI__IMPL_H
#define PLUGINS_NOTIFICATION_MANAGER_ANDROID_JAVA_JNI__IMPL_H

#include "plugins/notification_manager/ans_notification.h"

namespace OHOS {
namespace Notification {
class NotificationImpl : public AnsNotification {
public:
    NotificationImpl() = default;
    virtual ~NotificationImpl() = default;
    void PublishNotification(const NotificationRequest& request, void* data, NotificationCallback callBack) override;

    /**
     * @brief Cancels a published notification.
     *
     * @param notificationId Indicates the unique notification ID in the application.
     *                       The value must be the ID of a published notification.
     *                       Otherwise, this method does not take effect.
     * @return Returns cancel notification result.
     */
    ErrorCode CancelNotification(int32_t notificationId) override;

    /**
     * @brief Cancels all the published notifications.
     * @note To cancel a specified notification, see CancelNotification(int_32).
     *
     * @return Returns cancel all notifications result.
     */
    ErrorCode CancelAllNotifications() override;

    /**
     * @brief Checks whether this application has permission to publish notifications.
     *
     * @param  allowed True if this application has the permission; returns false otherwise
     * @return Returns is allowed notify result.
     */
    void IsAllowedNotifySelf(void* data, EnableNotificationCallback callBack) override;

    /**
     * @brief Set badge number.
     *
     * @param badgeNumber The badge number.
     * @return Returns set badge number result.
     */
    ErrorCode SetBadgeNumber(int32_t badgeNumber) override;

    /**
     * @brief Allow the current application to publish notifications on a specified device.
     *
     * @return Returns set notifications enabled for default bundle result.
     */
    void RequestEnableNotification(void* data, NotificationCallback callBack) override;
};
} // namespace Notification
} // namespace OHOS
#endif