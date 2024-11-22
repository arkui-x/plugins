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

#ifndef BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_INTERFACES_INNER_API_NOTIFICATION_REQUEST_H
#define BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_INTERFACES_INNER_API_NOTIFICATION_REQUEST_H

#include "ans_const_define.h"
#include "notification_content.h"
#include "ans_inner_errors.h"
#include <string>

namespace OHOS {
namespace Notification {

class NotificationRequest {
public:
    NotificationRequest() = default;

    /**
     * @brief A constructor used to create a NotificationRequest instance with the input parameter notificationId
     * passed.
     *
     * @param notificationId Indicates notification ID.
     */
    explicit NotificationRequest(int32_t notificationId);

    /**
     * @brief A constructor used to create a NotificationRequest instance by copying parameters from an existing one.
     *
     * @param other Indicates the existing object.
     */
    NotificationRequest(const NotificationRequest &other);

    /**
     * @brief A constructor used to create a NotificationRequest instance by copying parameters from an existing one.
     *
     * @param other Indicates the existing object.
     */
    NotificationRequest &operator=(const NotificationRequest &other);

    virtual ~NotificationRequest();

    /**
     * @brief Checks whether this notification is in progress.
     *
     * @return Returns true if this notification is in progress; returns false otherwise.
     */
    bool IsInProgress() const;

    /**
     * @brief Sets whether this notification is in progress.
     * Users cannot directly dismiss notifications in progress because
     * they usually contain some ongoing background services such as music playback.
     *
     * @param isOngoing Specifies whether this notification is in progress.
     */
    void SetInProgress(bool isOngoing);

    /**
     * @brief Sets the number to be displayed for this notification.
     *
     * @param number Indicates the number to set.
     */
    void SetBadgeNumber(uint32_t number);

    /**
     * @brief Obtains the number to be displayed for this notification.
     *
     * @return Returns the number to be displayed for this notification.
     */
    uint32_t GetBadgeNumber() const;

    /**
     * @brief Sets the current notification ID to uniquely identify the notification in the application.
     * After a notification is received, its ID is obtained by using the getNotificationId() method.
     *
     * @param notificationId Indicates the ID of the notification to be set.
     */
    void SetNotificationId(int32_t notificationId);

    /**
     * @brief Obtains the notification ID, which is unique in the current application.
     *
     * @return the notification ID.
     */
    int32_t GetNotificationId() const;

    /**
     * @brief Sets the time to deliver a notification.
     *
     * @param deliveryTime Indicates the time in milliseconds.
     */
    void SetDeliveryTime(int64_t deliveryTime);

    /**
     * @brief Obtains the time when a notification is delivered.
     *
     * @return Returns the time in milliseconds.
     */
    int64_t GetDeliveryTime() const;

    /**
     * @brief Checks whether the notification delivery time is displayed for this notification.
     *
     * @return Returns true if the time is displayed; returns false otherwise.
     */
    bool IsShowDeliveryTime() const;

    /**
     * @brief Sets whether to show the notification delivery time for this notification.
     * This method is valid only when the notification delivery time has been set by calling setDeliveryTime(int64_t).
     *
     * @param showDeliveryTime Specifies whether to show the notification delivery time.
     */
    void SetShowDeliveryTime(bool showDeliveryTime);

    /**
     * @brief Checks whether this notification is set to alert only once,
     * which means that sound or vibration will no longer be played
     * for notifications with the same ID upon their updates.
     *
     * @return Returns true if this notification is set to alert only once; returns false otherwise.
     */
    bool IsAlertOneTime() const;

    /**
     * @brief Sets whether to have this notification alert only once.
     * If a notification alerts only once, sound or vibration will no longer be played
     * for notifications with the same ID upon their updates after they are published.
     *
     * @param isAlertOnce Specifies whether to have this notification alert only once.
     */
    void SetAlertOneTime(bool isAlertOnce);

    /**
     * @brief Sets the time to delete a notification.
     *
     * @param deletedTime Indicates the time in milliseconds.
     * The default value is 0, indicating that the notification will not be automatically deleted.
     * To enable the notification to be automatically deleted, set this parameter to an integer greater than 0.
     */
    void SetAutoDeletedTime(int64_t deletedTime);

    /**
     * @brief Obtains the period during which a notification is deleted.
     *
     * @return Returns the period in milliseconds.
     */
    int64_t GetAutoDeletedTime() const;

    /**
     * @brief Sets the notification content type to NotificationNormalContent, NotificationLongTextContent,
     * or NotificationPictureContent etc.
     * Each content type indicates a particular notification content.
     *
     * @param content Indicates the notification content type.
     */
    void SetContent(const std::shared_ptr<NotificationContent> &content);

    /**
     * @brief Obtains the notification content set by calling the setContent(NotificationContent) method.
     *
     * @return Returns the notification content.
     */
    const std::shared_ptr<NotificationContent> GetContent() const;

    /**
     * @brief Obtains the notification type.
     *
     * @return Returns the type of the current notification, which can be
     * NotificationContent::Type::BASIC_TEXT,
     * NotificationContent::Type::LONG_TEXT,
     * NotificationContent::Type::PICTURE,
     * NotificationContent::Type::CONVERSATION,
     * NotificationContent::Type::MULTILINE,
     * NotificationContent::Type::MEDIA,
     * or NotificationContent::Type::LIVE_VIEW
     */
    NotificationContent::Type GetNotificationType() const;

    /**
     * @brief Checks whether the notification creation time is displayed as a countdown timer.
     *
     * @return Returns true if the time is displayed as a countdown timer; returns false otherwise.
     */
    bool IsCountdownTimer() const;

    /**
     * @brief Sets whether to show the notification creation time as a countdown timer.
     * This method is valid only when setShowStopwatch(boolean) is set to true.
     *
     * @param isCountDown Specifies whether to show the notification creation time as a countdown timer.
     */
    void SetCountdownTimer(bool isCountDown);

    /**
     * @brief Sets the group information for this notification.
     * If no groups are set for notifications, all notifications from the same application will appear
     * in the notification bar as one stack with the number of stacked notifications displayed.
     * If notifications are grouped and there are multiple groups identified by different groupName,
     * notifications with different groupName will appear in separate stacks.
     * Note that one of the notifications in a group must be set as the overview of its group by calling
     * setGroupOverview(bool), and other notifications are considered as child notifications.
     * Otherwise, notifications will not be displayed as one group even if they are assigned the same groupName by
     * calling setGroupName(string).
     *
     * @param groupName Specifies whether to set this notification as the group overview.
     */
    void SetGroupName(const std::string &groupName);

    /**
     * @brief Obtains the group information about this notification.
     *
     * @return Returns the group information about this notification.
     */
    std::string GetGroupName() const;

    /**
     * @brief Checks whether the notification creation time is displayed as a stopwatch.
     *
     * @return Returns true if the time is displayed as a stopwatch; returns false otherwise.
     */
    bool IsShowStopwatch() const;

    /**
     * @brief Sets whether to show the notification creation time as a stopwatch.
     * This method is valid only when the notification creation time has been set by calling setDeliveryTime(int64_t).
     * When the notification creation time is set to be shown as a stopwatch, the interval between the current time
     * and the creation time set by setDeliveryTime(int64_t) is dynamically displayed for this notification
     * in Minutes: Seconds format. If the interval is longer than 60 minutes, it will be displayed
     * in Hours: Minutes: Seconds format. If this method and setShowDeliveryTime(boolean) are both set to true, only
     * this method takes effect, that is, the notification creation time will be shown as a stopwatch.
     *
     * @param isShow Specifies whether to show the notification creation time as a stopwatch.
     */
    void SetShowStopwatch(bool isShow);

    /**
     * @brief Checks whether the current notification will be automatically dismissed after being tapped.
     *
     * @return Returns true if the notification will be automatically dismissed; returns false otherwise.
     */
    bool IsTapDismissed() const;

    /**
     * @brief Sets whether to automatically dismiss a notification after being tapped.
     * If you set tapDismissed to true,
     * you must call the setWantAgent(WantAgent) method to make the settings take effect.
     *
     * @param isDismissed Specifies whether a notification will be automatically dismissed after being tapped.
     */
    void SetTapDismissed(bool isDismissed);

    /**
     * @brief Converts a NotificationRequest object into a Json.
     *
     * @param jsonObject Indicates the Json object.
     * @return Returns true if succeed; returns false otherwise.
     */
    bool ToJson(nlohmann::json& jsonObject) const;

private:

    /**
     * @brief Obtains the current system time in milliseconds.
     *
     * @return Returns the current system time in milliseconds.
     */
    int64_t GetNowSysTime();

    void CopyBase(const NotificationRequest& other);
    void CopyOther(const NotificationRequest& other);

    bool ConvertObjectsToJson(nlohmann::json& jsonObject) const;

private:
    int32_t notificationId_ {0};
    uint32_t badgeNumber_ {-1};
    int64_t deliveryTime_ {0};

    int64_t autoDeletedTime_ {-1};
    std::string groupName_ {};
    NotificationContent::Type notificationContentType_ {NotificationContent::Type::NONE};

    bool showDeliveryTime_ { false };
    bool tapDismissed_ { true };
    bool alertOneTime_ { false };
    bool showStopwatch_ { false };
    bool isCountdown_ { false };
    bool inProgress_ { false };
    std::shared_ptr<NotificationContent> notificationContent_ {};
};
}  // namespace Notification
}  // namespace OHOS

#endif  // BASE_NOTIFICATION_DISTRIBUTED_NOTIFICATION_SERVICE_INTERFACES_INNER_API_NOTIFICATION_REQUEST_H
