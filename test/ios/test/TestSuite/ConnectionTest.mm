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

#import <XCTest/XCTest.h>
#import <libace_ios/net_status_observer.h>
#import <libace_ios/net_status_listen.h>

#define FAILED_MESSAGE @"Test failed!"

NetStatusObserver* observer;
@interface ConnectionTest : XCTestCase

@end

@implementation ConnectionTest
- (void)setUp {
    observer = [NetStatusObserver instanceNetStatusObserver];
}

- (void)tearDown {
    observer = nil;
}

- (void)testInit_0100 {
    XCTAssertNotNil(observer, FAILED_MESSAGE);
}

- (void)testReachabilityForInternetConnection_0100 {
    NetStatusListen *netStatusListen = [NetStatusListen reachabilityForInternetConnection];
    XCTAssertNotNil(netStatusListen, FAILED_MESSAGE);
}

- (void)testReachabilityWithAddress_0100 {
    struct sockaddr_in zeroAddress;
    bzero(&zeroAddress, sizeof(zeroAddress));
    zeroAddress.sin_len = sizeof(zeroAddress);
    zeroAddress.sin_family = AF_INET;

    NetStatusListen *netStatusListen = [NetStatusListen reachabilityWithAddress:(const struct sockaddr *) &zeroAddress];
    XCTAssertNotNil(netStatusListen, FAILED_MESSAGE);
}

- (void)testGetCurrentReachabilityStatus_0100 {
    NetStatusListen *netStatusListen = [NetStatusListen reachabilityForInternetConnection];
    XCTAssertNotNil(netStatusListen, FAILED_MESSAGE);
    NetworkStatus status = [netStatusListen getCurrentReachabilityStatus];
    bool result = (status == NotReachable) || (status == ReachableViaWiFi) || (status == ReachableViaWWAN);
    XCTAssertTrue(result, FAILED_MESSAGE);
}

- (void)testStartMonitor_0100 {
    NetStatusListen *netStatusListen = [NetStatusListen reachabilityForInternetConnection];
    bool result = [netStatusListen startMonitor];
    XCTAssertTrue(result, FAILED_MESSAGE);
}

- (void)testStopMonitor_0100 {
    NetStatusListen *netStatusListen = [NetStatusListen reachabilityForInternetConnection];
    bool result = [netStatusListen startMonitor];
    XCTAssertTrue(result, FAILED_MESSAGE);
    [netStatusListen stopMonitor];
}
@end
