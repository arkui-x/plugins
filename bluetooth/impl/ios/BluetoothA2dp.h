/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_BLUETOOTH_STANDARD_BLUETOOTHA2DP_H
#define OHOS_BLUETOOTH_STANDARD_BLUETOOTHA2DP_H

#import <AVFAudio/AVFAudio.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreBluetooth/CoreBluetooth.h>
#import <Foundation/Foundation.h>

#include "bluetooth_def.h"

using namespace OHOS::Bluetooth;

typedef void (^ConnectStateChangeBlock)(NSString* strDeviceId, int32_t state);

@interface BluetoothA2dp : NSObject

@property(nonatomic, copy) ConnectStateChangeBlock connectStateChangeBlock;

+ (BluetoothA2dp*)sharedInstance;
+ (int)GetDeviceState:(NSString*)strDeviceId;
+ (NSMutableArray<NSString*>*)GetDevicesByStates;

- (void)setNotifyRouteChange;
- (void)closeNotifyRouteChange;

@end

#endif // OHOS_BLUETOOTH_STANDARD_BLUETOOTHA2DP_H