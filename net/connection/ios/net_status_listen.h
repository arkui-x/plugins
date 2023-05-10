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

#ifndef NET_CONNECTION_IOS_NET_STATUS_LISTEN_H
#define NET_CONNECTION_IOS_NET_STATUS_LISTEN_H

#import <Foundation/Foundation.h>
#import <SystemConfiguration/SystemConfiguration.h>
#import <netinet/in.h>

typedef enum : NSInteger {
    NotReachable = 0, // The target host is not reachable.
    ReachableViaWiFi, // The target host is reachable via wifi.
    ReachableViaWWAN  // The target host is reachable via wwan.
} NetworkStatus;

@interface NetStatusListen : NSObject

// Use to check the reachability of a given IP address.
+ (instancetype)reachabilityWithAddress:(const struct sockaddr *)hostAddress;

// Checks whether the default route is available.
+ (instancetype)reachabilityForInternetConnection;

// Start listening for reachability notifications on the current run loop.
- (BOOL)startMonitor;
- (void)stopMonitor;

// Get the status of the current reachability.
- (NetworkStatus)getCurrentReachabilityStatus;
@end
#endif // NET_CONNECTION_IOS_NET_STATUS_LISTEN_H