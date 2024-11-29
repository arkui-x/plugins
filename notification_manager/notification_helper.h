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

#ifndef BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_INTERFACES_INNER_API_NOTIFICATION_HELPER_H
#define BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_INTERFACES_INNER_API_NOTIFICATION_HELPER_H

#include "notification_request.h"
#include "ans_notification.h"

namespace OHOS {
namespace Notification {
class NotificationHelper {
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
    static void PublishNotification(const NotificationRequest &request, void* asynccallbackinfo, NotificationCallback callback);

    /**
     * @brief Cancels a published notification matching the specified label and notificationId.
     *
     * @param label Indicates the label of the notification to cancel.
     * @param notificationId Indicates the ID of the notification to cancel.
     * @return Returns cancel notification result.
     */
    static ErrorCode CancelNotification(const std::string &label, int32_t notificationId);

    /**
     * @brief Cancels all the published notifications.
     *
     * @note To cancel a specified notification, see CancelNotification(int_32).
     * @return Returns cancel all notifications result.
     */
    static ErrorCode CancelAllNotifications();

    /**
     * @brief Checks whether this application has permission to publish notifications.
     *
     * @param asynccallbackinfo AsyncCallbackInfoPublish
     * @param callback NotificationCallback
     */
    static void IsAllowedNotifySelf(void* asynccallbackinfo, EnableNotificationCallback callback);

    /**
     * @brief Set badge number.
     *
     * @param badgeNumber The badge number.
     * @return Returns set badge number result.
     */
    static ErrorCode SetBadgeNumber(int32_t badgeNumber);

    /**
     * @brief Request permission to publish notifications.
     *
     * @param asynccallbackinfo AsyncCallbackInfoPublish
     * @param callback NotificationCallback
     */
    static void RequestEnableNotification(void* asynccallbackinfo, NotificationCallback callback);
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_INTERFACES_INNER_API_NOTIFICATION_HELPER_H
