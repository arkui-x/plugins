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

namespace OHOS::Notification {
std::shared_ptr<AnsNotification> AnsNotification::GetInstance()
{
    return std::make_shared<NotificationManagerImpl>();
}

void NotificationManagerImpl::PublishNotification(const NotificationRequest &request, void* asynccallbackinfo, NotificationCallback callback)
{

}

ErrorCode NotificationManagerImpl::CancelNotification(int32_t notificationId)
{
    return ErrorCode::ERR_ANS_OK;
}

ErrorCode NotificationManagerImpl::CancelAllNotifications()
{
    return ErrorCode::ERR_ANS_OK;
}

void NotificationManagerImpl::IsAllowedNotifySelf(void* asynccallbackinfo, EnableNotificationCallback callback)
{

}

ErrorCode NotificationManagerImpl::SetBadgeNumber(int32_t badgeNumber)
{
    return ErrorCode::ERR_ANS_OK;
}

void NotificationManagerImpl::RequestEnableNotification(void* asynccallbackinfo, NotificationCallback callback)
{
}
} // namespace OHOS::Plugin
