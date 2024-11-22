/*
 Copyright (C) 2016 Apple Inc. All Rights Reserved.
 See LICENSE.txt for this sample’s licensing information

 Abstract:
 Application delegate class.
 */

#import <arpa/inet.h>
#import <ifaddrs.h>
#import <netdb.h>
#import <netinet/in.h>
#import <sys/socket.h>
#import <CoreFoundation/CoreFoundation.h>

#import "net_status_listen.h"
#import "net_conn_client_adapter.h"

static void ReachabilityCallback(SCNetworkReachabilityRef target, SCNetworkReachabilityFlags flags, void* info)
{
    NSLog(@"ReachabilityCallback");
#pragma unused (target, flags)
    NSCAssert(info != NULL, @"info was NULL in ReachabilityCallback");
    NSCAssert([(__bridge NSObject*) info isKindOfClass: [NetStatusListen class]],
        @"info was wrong class in ReachabilityCallback");
    NetStatusListen* noteObject = (__bridge NetStatusListen *)info;
    OHOS::Plugin::NetConnClientAdapter::NotifyNetworkStatusChanged(noteObject);
}

#pragma mark - NetStatusListen implementation

@implementation NetStatusListen
{
    SCNetworkReachabilityRef _reachabilityRef;
}

+ (instancetype)reachabilityWithAddress:(const struct sockaddr *)hostAddress
{
    NetStatusListen* instance = NULL;
    SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault, hostAddress);
    if (reachability != NULL) {
        instance = [[self alloc] init];
        if (instance != NULL) {
            instance->_reachabilityRef = reachability;
        } else {
            CFRelease(reachability);
        }
    }
    return instance;
}

+ (instancetype)reachabilityForInternetConnection
{
    struct sockaddr_in zeroAddress;
    bzero(&zeroAddress, sizeof(zeroAddress));
    zeroAddress.sin_len = sizeof(zeroAddress);
    zeroAddress.sin_family = AF_INET;

    return [self reachabilityWithAddress: (const struct sockaddr *) &zeroAddress];
}

#pragma mark - Start and stop monitor

- (BOOL)startMonitor
{
    BOOL retCode = NO;
    SCNetworkReachabilityContext reachAbilityContext = {0, (__bridge void *)(self), NULL, NULL, NULL};

    if (SCNetworkReachabilitySetCallback(_reachabilityRef, ReachabilityCallback, &reachAbilityContext)) {
        if (SCNetworkReachabilityScheduleWithRunLoop(_reachabilityRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode)) {
            retCode = YES;
        }
    }

    return retCode;
}

- (void)stopMonitor
{
    if (_reachabilityRef != NULL) {
        SCNetworkReachabilityUnscheduleFromRunLoop(_reachabilityRef, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    }
}

- (void)dealloc
{
    [self stopMonitor];
    if (_reachabilityRef != NULL) {
        CFRelease(_reachabilityRef);
    }
}

#pragma mark - Network Flag Handling

- (NetworkStatus)getNetworkStatusForFlags:(SCNetworkReachabilityFlags)flags
{
    NetworkStatus retCode = NotReachable;

    if ((flags & kSCNetworkReachabilityFlagsReachable) == 0) {
        // The target host is not reachable.
        return NotReachable;
    }

    if ((flags & kSCNetworkReachabilityFlagsConnectionRequired) == 0) {
        // The target host is reachable via wifi.
        retCode = ReachableViaWiFi;
    }

    if ((((flags & kSCNetworkReachabilityFlagsConnectionOnDemand ) != 0) ||
        (flags & kSCNetworkReachabilityFlagsConnectionOnTraffic) != 0)) {
        if ((flags & kSCNetworkReachabilityFlagsInterventionRequired) == 0) {
            // The target host is reachable via wifi.
            retCode = ReachableViaWiFi;
        }
    }

    if ((flags & kSCNetworkReachabilityFlagsIsWWAN) == kSCNetworkReachabilityFlagsIsWWAN) {
        // The target host is reachable via wwan.
        retCode = ReachableViaWWAN;
    }

    return retCode;
}

- (NetworkStatus)getCurrentReachabilityStatus
{
    NSAssert(_reachabilityRef != NULL, @"getCurrentReachabilityStatus called with NULL SCNetworkReachabilityRef");
    NetworkStatus status = NotReachable;
    SCNetworkReachabilityFlags flags;
    if (SCNetworkReachabilityGetFlags(_reachabilityRef, &flags)) {
        status = [self getNetworkStatusForFlags:flags];
    }
    return status;
}
@end
