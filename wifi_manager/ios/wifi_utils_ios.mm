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

#import "wifi_utils_ios.h"
#include "wifi_callback.h"

// Get the current connection WiFi introduction
#import <SystemConfiguration/CaptiveNetwork.h>

// Obtain the current WiFi switch introduction
#import <ifaddrs.h>
#import <net/if.h>

// Introduction of Reachable, detection of changes in network connection status, Reachability class library
#import <SystemConfiguration/SystemConfiguration.h>
#import <netinet/in.h>
typedef enum : NSInteger {
    NotReachable = 0,
    ReachableViaWiFi,
    ReachableViaWWAN
} NetworkStatus;

// Get WiFi list introduction
#import <NetworkExtension/NetworkExtension.h>

@interface wifi_utils_ios()

@property  BOOL isWifiActivity;
@property  BOOL isWifiActivityFirst;
@property (strong,nonatomic) NSTimer *stateTimer;

@end

@implementation wifi_utils_ios {
    // Listening for WiFi connection status usage
    SCNetworkReachabilityRef reachabilityOnConnect;
}

+ (instancetype)shareManager {
    static wifi_utils_ios* _manager = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _manager = [[wifi_utils_ios alloc] init];
        _manager.isWifiActivityFirst = YES;
    });
    return _manager;
}

- (WifiErrCode)getLinkedInfo:(LinkedInfo*)info {
    NSArray *interfaceNames = CFBridgingRelease(CNCopySupportedInterfaces());
    for(NSString *interfaceName in interfaceNames) {
        CFDictionaryRef dicRef = CNCopyCurrentNetworkInfo((__bridge  CFStringRef)(interfaceName));
        NSDictionary *networkInfo = (__bridge id)dicRef;
        if (networkInfo == nil) {
            NSLog(@"wifi_utils_ios GetLinkedInfo oc CNCopyCurrentNetworkInfo nil");
            continue;
        }
        NSString *ocssid = nil;
        NSString *ocbssid = nil;
        if (networkInfo[@"SSID"]) {
            ocssid = networkInfo[@"SSID"];
            if (networkInfo[@"BSSID"]) {
                ocbssid = networkInfo[@"BSSID"];
            }
            (*info).ssid = ocssid.UTF8String;
            (*info).bssid = ocbssid.UTF8String;
            break;
        }
    }
    return WifiErrCode::WIFI_OPT_SUCCESS;
}

- (WifiErrCode)isWifiActive:(bool*)bActive {
    *bActive = [self isSystemWifiActive];
    return WifiErrCode::WIFI_OPT_SUCCESS;
}

- (WifiErrCode)isConnected:(bool*)isConnected {
    *isConnected = [self isSystemWifiConnected];
    return WifiErrCode::WIFI_OPT_SUCCESS;
}

- (void)onChange:(NSString *)key {
    NSString *inKey = key;
    if ([inKey isEqualToString: @"wifiStateChange"]) {
        [self listenWifiActive];
    } else if ([inKey isEqualToString: @"wifiConnectionChange"]) {
        [self listenWifiConnect];
    } else {
        NSLog(@"wifi_utils_ios on oc in type fail");
    }
}

- (void)offChange:(NSString *)key {
    NSString *inKey = key;
    if ([inKey isEqualToString:@"wifiStateChange"]) {
        [self unListenWifiActive];
    } else if([inKey isEqualToString:@"wifiConnectionChange"]) {
        [self unListenWifiConnect];
    } else {
        NSLog(@"wifi_utils_ios off oc in type fail");
    }
}

- (BOOL)isSystemWifiActive {
    NSCountedSet * cset = [NSCountedSet new];
    struct ifaddrs *interfaces;
    int success = getifaddrs(&interfaces);
    if(!success){
        for(struct ifaddrs *interface = interfaces; interface; interface = interface->ifa_next) {
            if((interface->ifa_flags & IFF_UP) == IFF_UP) {
                [cset addObject:[NSString stringWithUTF8String:interface->ifa_name]];
            }
        }
    }
    return [cset countForObject:@"awdl0"] > 1 ? YES : NO;
}

- (BOOL)isSystemWifiConnected {
    // initialization SCNetworkReachabilityRef
    struct sockaddr_in zeroAddress;
    bzero(&zeroAddress, sizeof(zeroAddress));
    zeroAddress.sin_len = sizeof(zeroAddress);
    zeroAddress.sin_family = AF_INET;
    const struct sockaddr * hostAddress = (const struct sockaddr *) &zeroAddress;
    SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, hostAddress);
    if (reachability == NULL) {
        NSLog(@"wifi_utils_ios IsConnected oc isSystemWifiConnected create SCNetworkReachabilityRef fail");
        return NO;
    }

    // Get Status
    NetworkStatus netState = [self currentReachabilityStatus:reachability];
    return netState == ReachableViaWiFi ? YES : NO;
}

- (NetworkStatus)currentReachabilityStatus:(SCNetworkReachabilityRef)_reachabilityRef {
    NSAssert(_reachabilityRef != NULL, @"currentNetworkStatus called with NULL SCNetworkReachabilityRef");
    NetworkStatus returnValue = NotReachable;
    SCNetworkReachabilityFlags flags;
    if (SCNetworkReachabilityGetFlags(_reachabilityRef, &flags)) {
        returnValue = [self networkStatusForFlags:flags];
    }
    return returnValue;
}

- (NetworkStatus)networkStatusForFlags:(SCNetworkReachabilityFlags)flags {
    if ((flags & kSCNetworkReachabilityFlagsReachable) == 0) {
        // The target host is not reachable.
        return NotReachable;
    }
    NetworkStatus returnValue = NotReachable;
    if ((flags & kSCNetworkReachabilityFlagsConnectionRequired) == 0) {
        // If the target host is reachable and no connection is required then we'll assume (for now) that you're on Wi-Fi...
        returnValue = ReachableViaWiFi;
    }
    if ((((flags & kSCNetworkReachabilityFlagsConnectionOnDemand ) != 0) ||
        (flags & kSCNetworkReachabilityFlagsConnectionOnTraffic) != 0)) {
        // ... and the connection is on-demand (or on-traffic) if the calling application is using the CFSocketStream or higher APIs...
        if ((flags & kSCNetworkReachabilityFlagsInterventionRequired) == 0) {
            // ... and no [user] intervention is needed...
            returnValue = ReachableViaWiFi;
        }
    }
    if ((flags & kSCNetworkReachabilityFlagsIsWWAN) == kSCNetworkReachabilityFlagsIsWWAN) {
        // ... but WWAN connections are OK if the calling application is using the CFNetwork APIs.
        returnValue = ReachableViaWWAN;
    }
    return returnValue;
}

- (void)listenWifiActive {
    if (self.stateTimer) {
        [self.stateTimer invalidate];
        self.stateTimer = nil;
        self.isWifiActivityFirst = YES;
    }
    self.stateTimer = [NSTimer scheduledTimerWithTimeInterval:1 repeats:YES block:^(NSTimer * _Nonnull timer) {
        BOOL isget = [self isSystemWifiActive];
        if (self.isWifiActivityFirst) {
            self.isWifiActivityFirst = NO;
            self.isWifiActivity = isget;
            int activityInt = 0;
            if (isget) {
                activityInt = 1;
            } else {
                activityInt = 0;
            }
            OHOS::Plugin::WifiCallback::GetInstance().SendCallback("wifiStateChange", activityInt);
        }
        if ((isget && self.isWifiActivity) || (!isget && !self.isWifiActivity)) {
            // Unchanged
        } else {
            // There are changes
            self.isWifiActivity = isget;
            int activityInt = 0;
            if (isget) {
                activityInt = 1;
            } else {
                activityInt = 0;
            }
            OHOS::Plugin::WifiCallback::GetInstance().SendCallback("wifiStateChange", activityInt);
        }
    }];
}

- (void)unListenWifiActive {
    if (self.stateTimer) {
        [self.stateTimer invalidate];
        self.stateTimer = nil;
        self.isWifiActivityFirst = YES;
    }
}

- (void)listenWifiConnect {
    // Set up sites for network detection
    NSString *remoteHostName = @"www.apple.com";
    reachabilityOnConnect = SCNetworkReachabilityCreateWithName(NULL, [remoteHostName UTF8String]);
    if (reachabilityOnConnect == NULL) {
        NSLog(@"wifi_utils_ios on oc wifiConnectionChange create SCNetworkReachabilityRef fail");
        return;
    }
    SCNetworkReachabilityContext context = {0, (__bridge void *)(self), NULL, NULL, NULL};
    if (SCNetworkReachabilitySetCallback(reachabilityOnConnect, ReachabilityCallback, &context)) {
        if (SCNetworkReachabilityScheduleWithRunLoop(reachabilityOnConnect, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode)) {
            NSLog(@"wifi_utils_ios on oc wifiConnectionChange listen success");
        }
    }
}

- (void)unListenWifiConnect {
    if (reachabilityOnConnect != NULL) {
        SCNetworkReachabilityUnscheduleFromRunLoop(reachabilityOnConnect, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
        reachabilityOnConnect = NULL;
    }
}

static void ReachabilityCallback(SCNetworkReachabilityRef target, SCNetworkReachabilityFlags flags, void* info) {
    NSCAssert(info != NULL, @"info was NULL in ReachabilityCallback");
    NSCAssert([(__bridge NSObject*) info isKindOfClass: [wifi_utils_ios class]], @"info was wrong class in ReachabilityCallback");
    wifi_utils_ios* noteObject = (__bridge wifi_utils_ios *)info;
    NetworkStatus netStatus = [noteObject currentReachabilityStatus:target];
    int connectedInt = 0;
    if (netStatus == ReachableViaWiFi) {
       // Successfully run
        connectedInt = 1;
    } else {
        connectedInt = 0;
    }
    OHOS::Plugin::WifiCallback::GetInstance().SendCallback("wifiConnectionChange", connectedInt);
}

@end
