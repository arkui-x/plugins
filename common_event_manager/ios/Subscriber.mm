/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#import "Subscriber.h"

#include "inner_api/plugin_utils_inner.h"
#include "inner_api/plugin_utils_napi.h"
#include "plugins/common_event_manager/common_event_support.h"
#include "plugins/common_event_manager/ios/common_event_receiver.h"

#import <UIKit/UIDevice.h>
#import <UIKit/UIApplication.h>

using CommonEventSupport = OHOS::Plugin::CommonEventSupport;
static const float batteryLowValue = 20.f;
@interface Subscriber () {
    BOOL _batteryLowFlag, _batteryNormalFlag;
}
@end

@implementation Subscriber
NSString* getOCstring(const std::string& c_string) {
    return [NSString stringWithCString:c_string.c_str() encoding:NSUTF8StringEncoding];
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _batteryLowFlag = NO;
        _batteryNormalFlag = NO;
    }
    return self;
}

- (void)subscribeEvents:(NSArray*)events key:(NSString*)key {
    NSLog(@"%s, events : %@, key : %@", __func__, events, key);
    self.events = events;
    self.key = key;

    for (NSString* eventName in events) {
        if ([eventName containsString:getOCstring(CommonEventSupport::COMMON_EVENT_BATTERY_LOW)]
                    || [eventName containsString:getOCstring(CommonEventSupport::COMMON_EVENT_BATTERY_OKAY)]) {
            [[UIDevice currentDevice] setBatteryMonitoringEnabled:YES];
            [[NSNotificationCenter defaultCenter] addObserver:self
                                                     selector:@selector(systemNotificationBatteryLevelsChanged)
                                                         name:UIDeviceBatteryLevelDidChangeNotification
                                                       object:nil];
        } else if ([eventName containsString:getOCstring(CommonEventSupport::COMMON_EVENT_TIME_CHANGED)]) {
            [[NSNotificationCenter defaultCenter] addObserver:self
                                                     selector:@selector(systemNotificationClockDidChange)
                                                         name:NSSystemClockDidChangeNotification
                                                       object:nil];
        } else if ([eventName containsString:getOCstring(CommonEventSupport::COMMON_EVENT_TIMEZONE_CHANGED)]) {
            [[NSNotificationCenter defaultCenter] addObserver:self
                                                     selector:@selector(systemNotificationTimeZoneDidChange)
                                                         name:NSSystemTimeZoneDidChangeNotification
                                                       object:nil];
        } else if ([eventName containsString:getOCstring(CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED)]) {
            [[NSNotificationCenter defaultCenter] addObserver:self
                                                     selector:@selector(systemNotificationChangePowerMode)
                                                         name:NSProcessInfoPowerStateDidChangeNotification
                                                       object:nil];
        }
    }
}

- (void)unSubscribeEvents:(void*)ptr {
    NSLog(@"%s", __func__);
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    [[UIDevice currentDevice] setBatteryMonitoringEnabled:NO];

    if (ptr) {
        OHOS::Plugin::AsyncCallbackInfo* callbackPtr = (OHOS::Plugin::AsyncCallbackInfo*)ptr;
        callbackPtr->status = true;
    }
}

- (void)systemNotificationBatteryLevelsChanged {
    float batteryLevel = [UIDevice currentDevice].batteryLevel * 100;

    if (batteryLevel <= batteryLowValue && !_batteryLowFlag) {
        NSLog(@"%s, batteryLevel enter low : %.02lf", __func__, batteryLevel);
        OHOS::Plugin::CommonEventReceiver::ReceiveCommonEvent(self.key.UTF8String,
            CommonEventSupport::COMMON_EVENT_BATTERY_LOW, "");
        _batteryLowFlag = YES;
        _batteryNormalFlag = NO;
    }
    if (batteryLevel > batteryLowValue && !_batteryNormalFlag) {
        NSLog(@"%s, batteryLevel enter ok : %.02lf", __func__, batteryLevel);
        OHOS::Plugin::CommonEventReceiver::ReceiveCommonEvent(self.key.UTF8String,
            CommonEventSupport::COMMON_EVENT_BATTERY_OKAY, "");
        _batteryNormalFlag = YES;
        _batteryLowFlag = NO;
    }
}

- (void)systemNotificationClockDidChange {
    NSDate* date = [NSDate date];
    NSTimeInterval timeInterval = [date timeIntervalSinceNow];
    NSDate* currentDate = [[NSDate alloc] initWithTimeIntervalSinceNow:timeInterval];
    NSDateFormatter* dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setDateFormat:@"yyyy-MM-dd HH-mm-ss"];
    NSString* timeString = [dateFormatter stringFromDate:currentDate];

    NSLog(@"%s, current date : %@", __func__, timeString);
    OHOS::Plugin::CommonEventReceiver::ReceiveCommonEvent(self.key.UTF8String,
        CommonEventSupport::COMMON_EVENT_TIME_CHANGED, "");
}

- (void)systemNotificationTimeZoneDidChange {
    NSArray* zoneNames = [NSTimeZone knownTimeZoneNames];
    NSTimeZone* zone = [NSTimeZone systemTimeZone];
    NSString* strZoneName = [zone name];

    NSLog(@"%s, system timezone name : %@", __func__, strZoneName);
    OHOS::Plugin::CommonEventReceiver::ReceiveCommonEvent(self.key.UTF8String,
        CommonEventSupport::COMMON_EVENT_TIMEZONE_CHANGED, "");
}

- (void)systemNotificationChangePowerMode {
    NSLog(@"%s", __func__);
    OHOS::Plugin::CommonEventReceiver::ReceiveCommonEvent(self.key.UTF8String,
        CommonEventSupport::COMMON_EVENT_POWER_SAVE_MODE_CHANGED, "");
}
@end
