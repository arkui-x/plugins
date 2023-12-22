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

#ifndef PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHNETSTATUSMONITOR_H
#define PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHNETSTATUSMONITOR_H

#import <Foundation/Foundation.h>
#import <SystemConfiguration/SystemConfiguration.h>

typedef NS_ENUM(NSInteger, OHNetStatus) {
    OHNetStatusUnknown          = -1,
    OHNetStatusNotReachable     = 0,
    OHNetStatusReachableViaWWAN = 1,
    OHNetStatusReachableViaWiFi = 2,
};

FOUNDATION_EXPORT NSString* OHStringFromNetworkReachabilityStatus(OHNetStatus status);

NS_ASSUME_NONNULL_BEGIN

@interface OHNetStatusMonitor : NSObject

@property (readonly, nonatomic, assign) OHNetStatus networkReachabilityStatus;
@property (readonly, nonatomic, assign, getter = isReachable) BOOL reachable;
@property (readonly, nonatomic, assign, getter = isReachableViaWWAN) BOOL reachableViaWWAN;
@property (readonly, nonatomic, assign, getter = isReachableViaWiFi) BOOL reachableViaWiFi;

+ (instancetype)sharedMonitor;
- (void)startMonitoring;
- (void)stopMonitoring;
- (void)setNetStatusChangeBlock:(nullable void (^)(OHNetStatus status))block;

+ (instancetype)new NS_UNAVAILABLE;
- (instancetype)init NS_UNAVAILABLE;

@end

NS_ASSUME_NONNULL_END
#endif // PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHNETSTATUSMONITOR_H