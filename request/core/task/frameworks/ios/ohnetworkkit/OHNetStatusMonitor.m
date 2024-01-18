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

#import "OHNetStatusMonitor.h"
#import <netinet/in.h>
#import <netinet6/in6.h>
#import <arpa/inet.h>
#import <ifaddrs.h>
#import <netdb.h>


typedef void (^OHNetStatusBlock)(OHNetStatus status);
typedef OHNetStatusMonitor * (^OHNetStatusCallback)(OHNetStatus status);

NSString * OHStringFromNetworkReachabilityStatus(OHNetStatus status) {
    switch (status) {
        case OHNetStatusNotReachable:
            return NSLocalizedStringFromTable(@"Not Reachable", @"OHNetworkKit", nil);
        case OHNetStatusReachableViaWWAN:
            return NSLocalizedStringFromTable(@"Reachable via WWAN", @"OHNetworkKit", nil);
        case OHNetStatusReachableViaWiFi:
            return NSLocalizedStringFromTable(@"Reachable via WiFi", @"OHNetworkKit", nil);
        case OHNetStatusUnknown:
        default:
            return NSLocalizedStringFromTable(@"Unknown", @"OHNetworkKit", nil);
    }
}

static OHNetStatus OHNetStatusForFlags(SCNetworkReachabilityFlags flags) {
    BOOL isReachable = ((flags & kSCNetworkReachabilityFlagsReachable) != 0);
    BOOL needsConnection = ((flags & kSCNetworkReachabilityFlagsConnectionRequired) != 0);
    BOOL canConnectionAutomatically = (((flags & kSCNetworkReachabilityFlagsConnectionOnDemand ) != 0) ||
        ((flags & kSCNetworkReachabilityFlagsConnectionOnTraffic) != 0));
    BOOL canConnectWithoutUserInteraction = (canConnectionAutomatically &&
        (flags & kSCNetworkReachabilityFlagsInterventionRequired) == 0);
    BOOL isNetworkReachable = (isReachable && (!needsConnection || canConnectWithoutUserInteraction));

    OHNetStatus status = OHNetStatusUnknown;
    if (isNetworkReachable == NO) {
        status = OHNetStatusNotReachable;
    }
    else if ((flags & kSCNetworkReachabilityFlagsIsWWAN) != 0) {
        status = OHNetStatusReachableViaWWAN;
    }
    else {
        status = OHNetStatusReachableViaWiFi;
    }

    return status;
}

static void OHPostReachabilityStatusChange(SCNetworkReachabilityFlags flags, OHNetStatusCallback block) {
    OHNetStatus status = OHNetStatusForFlags(flags);
    dispatch_async(dispatch_get_main_queue(), ^{
        OHNetStatusMonitor *monitor = nil;
        if (block) {
            monitor = block(status);
        }
    });
}

static void OHNetworkReachabilityCallback(SCNetworkReachabilityRef __unused target,
    SCNetworkReachabilityFlags flags, void *info) {
    OHPostReachabilityStatusChange(flags, (__bridge OHNetStatusCallback)info);
}

static const void * OHNetworkReachabilityRetainCallback(const void *info) {
    return Block_copy(info);
}

static void OHNetworkReachabilityReleaseCallback(const void *info) {
    if (info) {
        Block_release(info);
    }
}


#pragma mark - OHNetStatusMonitor
@interface OHNetStatusMonitor ()

@property (readonly, nonatomic, assign) SCNetworkReachabilityRef networkReachability;
@property (readwrite, nonatomic, assign) OHNetStatus networkReachabilityStatus;
@property (readwrite, nonatomic, copy) OHNetStatusBlock networkReachabilityStatusBlock;

@end


@implementation OHNetStatusMonitor

+ (instancetype)sharedMonitor {
    static OHNetStatusMonitor *_sharedMonitor = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        _sharedMonitor = [self monitor];
    });

    return _sharedMonitor;
}

+ (instancetype)monitorForDomain:(NSString *)domain {
    SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithName(kCFAllocatorDefault,
        domain.UTF8String);
    OHNetStatusMonitor *monitor = [[self alloc] initWithReachability:reachability];
    CFRelease(reachability);
    return monitor;
}

+ (instancetype)monitorForAddress:(const void *)address {
    SCNetworkReachabilityRef reachability = SCNetworkReachabilityCreateWithAddress(kCFAllocatorDefault,
        (const struct sockaddr *)address);
    OHNetStatusMonitor *monitor = [[self alloc] initWithReachability:reachability];
    CFRelease(reachability);
    return monitor;
}

+ (instancetype)monitor {
#if (defined(__IPHONE_OS_VERSION_MIN_REQUIRED) && __IPHONE_OS_VERSION_MIN_REQUIRED >= 90000)
    struct sockaddr_in6 address;
    bzero(&address, sizeof(address));
    address.sin6_len = sizeof(address);
    address.sin6_family = AF_INET6;
#else
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_len = sizeof(address);
    address.sin_family = AF_INET;
#endif
    return [self monitorForAddress:&address];
}

- (instancetype)initWithReachability:(SCNetworkReachabilityRef)reachability {
    self = [super init];
    if (!self) {
        return nil;
    }

    _networkReachability = CFRetain(reachability);
    self.networkReachabilityStatus = OHNetStatusUnknown;
    return self;
}

- (instancetype)init {
    @throw [NSException exceptionWithName:NSGenericException
                                   reason:@"`-init` unavailable. Use `-initWithReachability:` instead"
                                 userInfo:nil];
    return nil;
}

- (void)dealloc {
    [self stopMonitoring];
    if (_networkReachability != NULL) {
        CFRelease(_networkReachability);
    }
}

#pragma mark -
- (BOOL)isReachable {
    return [self isReachableViaWWAN] || [self isReachableViaWiFi];
}

- (BOOL)isReachableViaWWAN {
    return self.networkReachabilityStatus == OHNetStatusReachableViaWWAN;
}

- (BOOL)isReachableViaWiFi {
    return self.networkReachabilityStatus == OHNetStatusReachableViaWiFi;
}

#pragma mark - Methods
- (void)startMonitoring {
    [self stopMonitoring];
    if (!self.networkReachability) {
        return;
    }

    __weak __typeof(self)weakSelf = self;
    OHNetStatusCallback callback = ^(OHNetStatus status) {
        __strong __typeof(weakSelf)strongSelf = weakSelf;
        strongSelf.networkReachabilityStatus = status;
        if (strongSelf.networkReachabilityStatusBlock) {
            strongSelf.networkReachabilityStatusBlock(status);
        }
        return strongSelf;
    };

    SCNetworkReachabilityContext context = {0, (__bridge void *)callback,
        OHNetworkReachabilityRetainCallback, OHNetworkReachabilityReleaseCallback, NULL};
    SCNetworkReachabilitySetCallback(self.networkReachability, OHNetworkReachabilityCallback, &context);
    SCNetworkReachabilityScheduleWithRunLoop(self.networkReachability, CFRunLoopGetMain(), kCFRunLoopCommonModes);

    dispatch_async(dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_LOW, 0),^{
        SCNetworkReachabilityFlags flags;
        if (SCNetworkReachabilityGetFlags(self.networkReachability, &flags)) {
            OHPostReachabilityStatusChange(flags, callback);
        }
    });
}

- (void)stopMonitoring {
    if (!self.networkReachability) {
        return;
    }
    SCNetworkReachabilityUnscheduleFromRunLoop(self.networkReachability, CFRunLoopGetMain(), kCFRunLoopCommonModes);
}

- (void)setNetStatusChangeBlock:(void (^)(OHNetStatus status))block {
    self.networkReachabilityStatusBlock = block;
}

#pragma mark - NSKeyValueObserving
+ (NSSet *)keyPathsForValuesAffectingValueForKey:(NSString *)key {
    if ([key isEqualToString:@"reachable"] || [key isEqualToString:@"reachableViaWWAN"] ||
        [key isEqualToString:@"reachableViaWiFi"]) {
        return [NSSet setWithObject:@"networkReachabilityStatus"];
    }
    return [super keyPathsForValuesAffectingValueForKey:key];
}

@end