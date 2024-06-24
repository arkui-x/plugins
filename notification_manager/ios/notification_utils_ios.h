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
#ifndef notification_utils_ios_h
#define notification_utils_ios_h


#import <Foundation/Foundation.h>
#import "UserNotifications/UserNotifications.h"

#include "ans_inner_errors.h"
#include "notification_request.h"

NS_ASSUME_NONNULL_BEGIN

using ErrorCode = OHOS::Notification::ErrorCode;
using NotificationRequest = OHOS::Notification::NotificationRequest;

@interface notification_utils_ios : NSObject

/**
 * Single instance method
 */
+ (instancetype)shareManager;

- (void)PublishNotification:(const NotificationRequest*)request WithCallback:(void(^)(int32_t errorCode))callback;

- (ErrorCode)CancelNotification:(int32_t)notificationId;

- (ErrorCode)CancelAllNotifications;

- (ErrorCode)SetBadgeNumber:(int32_t)badgeNumber;

- (void)RequestEnableNotificationWithCallback:(void(^)(int32_t errorCode))callback;

- (void)IsAllowedNotifySelfWithCallback:(void(^)(int32_t errorCode, bool isEnabled))callback;

@end

NS_ASSUME_NONNULL_END

#endif /* notification_utils_ios_h */