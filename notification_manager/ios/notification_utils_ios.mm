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

#import "notification_utils_ios.h"
#import <UIKit/UIKit.h>

#include <string.h>

@interface notification_utils_ios()

@end

@implementation notification_utils_ios {

}

+ (instancetype)shareManager {
    static notification_utils_ios* _manager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _manager = [[notification_utils_ios alloc] init];
    });
    return _manager;
}

- (void)PublishNotification:(const NotificationRequest*)request WithCallback:(void(^)(int32_t errorCode))callback {
    if (request == nil || request->GetContent() == nil || request->GetContent()->GetNotificationContent() == nil) {
        NSLog(@"PublishNotification request param is invalid");
        callback(ErrorCode::ERR_ANS_INVALID_PARAM);
    }
    [self IsAllowedNotifySelfWithCallback:^(int32_t errorCode, bool isEnabled) {
        if (!isEnabled) {
            callback(ErrorCode::ERR_ANS_NOT_ALLOWED);
            return;
        }
        UNMutableNotificationContent *content = [[UNMutableNotificationContent alloc]init];
        
        NSString *text = [NSString stringWithCString:request->GetContent()->GetNotificationContent()->GetText().c_str() 
                                            encoding:NSUTF8StringEncoding];
        NSString *title = [NSString stringWithCString:request->GetContent()->GetNotificationContent()->GetTitle().c_str()
                                            encoding:NSUTF8StringEncoding];
        content.body = text;
        content.title = title;
        content.sound = [UNNotificationSound defaultSound];

        int32_t requestId = request->GetNotificationId();
        int32_t badgeNumber = request->GetBadgeNumber();
        if (badgeNumber >= 0) {
            content.badge = [NSNumber numberWithInt:badgeNumber];
        }

        std::string groupName = request->GetGroupName();
        if (!groupName.empty()) {
            content.threadIdentifier = [NSString stringWithCString:groupName.c_str() encoding:NSUTF8StringEncoding];
        }

        UNNotificationRequest *uNNotificationRequest = [UNNotificationRequest requestWithIdentifier:[NSString stringWithFormat:@"%d", requestId]
                                                                                            content:content
                                                                                            trigger:nil];
        UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];
        [center addNotificationRequest:uNNotificationRequest withCompletionHandler:nil]; 
        callback(ErrorCode::ERR_ANS_OK);
    }];
}

- (ErrorCode)CancelNotification:(int32_t)notificationId {
    NSString *requestIdStr = [NSString stringWithFormat:@"%d", notificationId];
    [[UNUserNotificationCenter currentNotificationCenter] removeDeliveredNotificationsWithIdentifiers: @[requestIdStr]];
    return ErrorCode::ERR_ANS_OK;
}

- (ErrorCode)CancelAllNotifications {
    [[UNUserNotificationCenter currentNotificationCenter] removeAllDeliveredNotifications];
    return ErrorCode::ERR_ANS_OK;
}

- (ErrorCode)SetBadgeNumber:(int32_t)badgeNumber {
    dispatch_async(dispatch_get_main_queue(), ^{
        [UIApplication sharedApplication].applicationIconBadgeNumber = badgeNumber;
    });
    return ErrorCode::ERR_ANS_OK;
}

- (void)RequestEnableNotificationWithCallback:(void(^)(int32_t errorCode))callback {
    UNUserNotificationCenter *center = [UNUserNotificationCenter currentNotificationCenter];

    [center requestAuthorizationWithOptions:UNAuthorizationOptionAlert | UNAuthorizationOptionSound | UNAuthorizationOptionBadge 
                          completionHandler:^(BOOL granted, NSError * _Nullable error) {
        dispatch_async(dispatch_get_main_queue(), ^{
            ErrorCode result = granted ? ErrorCode::ERR_ANS_OK : ErrorCode::ERR_ANS_NOT_ALLOWED;
            callback(result);
        });
    }];
}

- (void)IsAllowedNotifySelfWithCallback:(void(^)(int32_t errorCode, bool isEnabled))callback {
   [UNUserNotificationCenter.currentNotificationCenter getNotificationSettingsWithCompletionHandler:^(UNNotificationSettings * _Nonnull settings) {
        bool isEnabled = false;
        switch (settings.authorizationStatus) {
            case UNAuthorizationStatusAuthorized:
                isEnabled = true;
                break;
            default:
                isEnabled = false;
                break;
        }
        dispatch_async(dispatch_get_main_queue(), ^{
            callback(ErrorCode::ERR_ANS_OK, isEnabled);
        });
   }];
}


@end
