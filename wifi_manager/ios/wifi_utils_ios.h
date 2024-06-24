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

#ifndef WIFI_UTILS_IOS_H_
#define WIFI_UTILS_IOS_H_

#import <Foundation/Foundation.h>

#include "wifi_errcode.h"
#include "wifi_msg.h"

NS_ASSUME_NONNULL_BEGIN

using WifiErrCode = OHOS::Plugin::ErrCode;
using LinkedInfo = OHOS::Plugin::WifiLinkedInfo;

@interface wifi_utils_ios : NSObject

/**
 * Single instance method
 */
+ (instancetype)shareManager;

/**
 * Get WiFi switch status
 * 
 * @param info Obtained current connection wifi information
 * @return Result and error code
 */
- (WifiErrCode)getLinkedInfo:(LinkedInfo*)info;

/**
 * Get WiFi switch status
 * 
 * @param bActive Obtained switch status
 * @return Result and error code
 */
- (WifiErrCode)isWifiActive:(bool*)bActive;

/**
 * Get WiFi connection status
 * 
 * @param isConnected Obtained WiFi connection status
 * @return Result and error code
 */
- (WifiErrCode)isConnected:(bool*)isConnected;

/**
 * lsnrctl start
 * 
 * @param key Type of listening wifiStateChange、wifiConnectionChange
 */
- (void)onChange:(NSString *)key;

/**
 * Cancel listening
 * 
 * @param key Type of listening wifiStateChange、wifiConnectionChange
 */
- (void)offChange:(NSString *)key;

@end

NS_ASSUME_NONNULL_END

#endif // WIFI_UTILS_IOS_H_
