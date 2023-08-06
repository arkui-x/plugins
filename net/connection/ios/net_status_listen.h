/*
 Copyright (C) 2016 Apple Inc. All Rights Reserved.
 See LICENSE.txt for this sample’s licensing information

 Abstract:
 Application delegate class.
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