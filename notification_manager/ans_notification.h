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

#ifndef BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_NOTIFICATION_H
#define BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_NOTIFICATION_H

#include <list>

#include "notification_request.h"

namespace OHOS {
namespace Notification {
using NotificationCallback = std::function<void(void*, const int32_t grantResult)>;
using EnableNotificationCallback = std::function<void(void*, const int32_t grantResult, bool isEnable)>;
class AnsNotification {
public:
    static std::shared_ptr<AnsNotification> GetInstance();

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
    virtual void PublishNotification(const NotificationRequest &request, void* asynccallbackinfo, NotificationCallback callback) = 0;

    /**
     * @brief Cancels a published notification.
     *
     * @param notificationId Indicates the unique notification ID in the application.
     *                       The value must be the ID of a published notification.
     *                       Otherwise, this method does not take effect.
     * @return Returns cancel notification result.
     */
    virtual ErrorCode CancelNotification(int32_t notificationId) = 0;

    /**
     * @brief Cancels all the published notifications.
     * @note To cancel a specified notification, see CancelNotification(int_32).
     *
     * @return Returns cancel all notifications result.
     */
    virtual ErrorCode CancelAllNotifications() = 0;

    /**
     * @brief Checks whether this application has permission to publish notifications.
     *
     * @param asynccallbackinfo AsyncCallbackInfoPublish
     * @param callback NotificationCallback
     */
    virtual void IsAllowedNotifySelf(void* asynccallbackinfo, EnableNotificationCallback callback) = 0;

    /**
     * @brief Set badge number.
     *
     * @param badgeNumber The badge number.
     * @return Returns set badge number result.
     */
    virtual ErrorCode SetBadgeNumber(int32_t badgeNumber) = 0;

    /**
     * @brief Request permission to publish notifications.
     *
     * @param asynccallbackinfo AsyncCallbackInfoPublish
     * @param callback NotificationCallback
     */
    virtual void RequestEnableNotification(void* asynccallbackinfo, NotificationCallback callback) = 0;
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_ANS_STANDARD_FRAMEWORKS_ANS_CORE_INCLUDE_ANS_NOTIFICATION_H
