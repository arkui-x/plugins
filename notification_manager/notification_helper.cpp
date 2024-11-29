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

#include "notification_helper.h"
#include "singleton.h"

namespace OHOS {
namespace Notification {
std::shared_ptr<AnsNotification> notificationPtr = AnsNotification::GetInstance();
void NotificationHelper::PublishNotification(const NotificationRequest &request, void* asynccallbackinfo, NotificationCallback callback)
{
    notificationPtr->PublishNotification(request, asynccallbackinfo, callback);
}

ErrorCode NotificationHelper::CancelNotification(const std::string &label, int32_t notificationId)
{
    return notificationPtr->CancelNotification(notificationId);
}

ErrorCode NotificationHelper::CancelAllNotifications()
{
    return notificationPtr->CancelAllNotifications();
}

void NotificationHelper::IsAllowedNotifySelf(void* asynccallbackinfo, EnableNotificationCallback callback)
{
    notificationPtr->IsAllowedNotifySelf(asynccallbackinfo, callback);
}

ErrorCode NotificationHelper::SetBadgeNumber(int32_t badgeNumber)
{
    return notificationPtr->SetBadgeNumber(badgeNumber);
}

void NotificationHelper::RequestEnableNotification(void* asynccallbackinfo, NotificationCallback callback)
{
    notificationPtr->RequestEnableNotification(asynccallbackinfo, callback);
}
}  // namespace Notification
}  // namespace OHOS
