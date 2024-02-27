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
 * 单例方法
 */
+ (instancetype)shareManager;

/**
 * 获取连接wifi信息
 * @param info 获取到的当前连接wifi信息
 * @return 结果以及错误码
 */
- (WifiErrCode)getLinkedInfo:(LinkedInfo*)info;

/**
 * 获取wifi开关状态
 * @param bActive 获取到的开关状态
 * @return 结果以及错误码
 */
- (WifiErrCode)isWifiActive:(bool*)bActive;

/**
 * 获取wifi连接状态
 * @param isConnected 获取到的wifi连接状态
 * @return 结果以及错误码
 */
- (WifiErrCode)isConnected:(bool*)isConnected;

/**
 * 启动监听
 * @param key 监听的类型 wifiStateChange、wifiConnectionChange
 */
- (void)onChange:(NSString *)key;

/**
 * 取消监听
 * @param key 监听的类型 wifiStateChange、wifiConnectionChange
 */
- (void)offChange:(NSString *)key;

@end

NS_ASSUME_NONNULL_END

#endif // WIFI_UTILS_IOS_H_