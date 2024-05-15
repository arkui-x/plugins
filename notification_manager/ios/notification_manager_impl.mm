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

#include <cstring>

#include "log.h"
#include "notification_manager_impl.h"
#include "ans_inner_errors.h"
#import "notification_utils_ios.h"

namespace OHOS::Notification {
std::shared_ptr<AnsNotification> AnsNotification::GetInstance()
{
    return std::make_shared<NotificationManagerImpl>();
}

void NotificationManagerImpl::PublishNotification(const NotificationRequest &request, void* asynccallbackinfo, NotificationCallback callback)
{
    [[notification_utils_ios shareManager] PublishNotification:&request WithCallback:^(int32_t errorCode) {
        callback(asynccallbackinfo, errorCode);
    }];
}

ErrorCode NotificationManagerImpl::CancelNotification(int32_t notificationId)
{
    return [[notification_utils_ios shareManager] CancelNotification:notificationId];
}

ErrorCode NotificationManagerImpl::CancelAllNotifications()
{
    return [[notification_utils_ios shareManager] CancelAllNotifications];
}

void NotificationManagerImpl::IsAllowedNotifySelf(void* asynccallbackinfo, EnableNotificationCallback callback)
{
    [[notification_utils_ios shareManager] IsAllowedNotifySelfWithCallback:^(int32_t errorCode, bool isEnabled) {
        callback(asynccallbackinfo, errorCode, isEnabled);
    }];
}

ErrorCode NotificationManagerImpl::SetBadgeNumber(int32_t badgeNumber)
{
    return [[notification_utils_ios shareManager] SetBadgeNumber:badgeNumber];
}

void NotificationManagerImpl::RequestEnableNotification(void* asynccallbackinfo, NotificationCallback callback)
{
    [[notification_utils_ios shareManager] RequestEnableNotificationWithCallback:^(int32_t errorCode) {
        callback(asynccallbackinfo, errorCode);
    }];
}
} // namespace OHOS::Plugin
