// Copyright (c) 2011–2016 Alamofire Software Foundation ( http://alamofire.org/ )
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

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