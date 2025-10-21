/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef VIBRATOR_MANAGER_H
#define VIBRATOR_MANAGER_H

#import <AVFoundation/AVFoundation.h>
#import <CoreHaptics/CoreHaptics.h>
#import <Foundation/Foundation.h>

#include "vibrator_infos.h"

NS_ASSUME_NONNULL_BEGIN

typedef NS_ENUM(NSInteger, VibratorMode)
{
    VibratorModeTime = 0,
    VibratorModePreset = 1,
};

@interface VibratorInfoObject : NSObject
@property (nonatomic, assign) NSInteger deviceId;
@property (nonatomic, assign) NSInteger vibratorId;
@property (nonatomic, copy) NSString *deviceName;
@property (nonatomic, assign) BOOL isSupportHdHaptic;
@property (nonatomic, assign) BOOL isLocalVibrator;
@property (nonatomic, assign) NSInteger position;
@end

@interface EffectInfo : NSObject
@property (nonatomic, strong) NSNumber *duration;
@property (nonatomic, assign) BOOL isSupportEffect;
@end

@interface VibratorManager : NSObject {
    VibratorMode _currentMode;
}

+ (instancetype)sharedInstance;

- (int32_t)vibrateWithTimeOut:(int32_t)timeOut;
- (int32_t)stopVibrator;
- (int32_t)playVibratorEffect:(NSString *)effect loopCount:(int32_t)loopCount;
- (BOOL)isSupportEffect:(NSString *)effect;
- (NSArray<VibratorInfoObject *> *)getVibratorList;
- (EffectInfo *)getEffectInfo:(NSString *)effectType;
- (int32_t)playHaptic:(float)intensity sharpness:(float)sharpness duration:(float)duration loopCount:(int32_t)loopCount;
- (void)playVibrationWithDuration:(NSTimeInterval)duration;
- (void)playPattern:(const OHOS::Sensors::VibratePattern &)pattern;
- (void)playPatterns:(const std::vector<OHOS::Sensors::VibratePattern> &)patterns;
- (void)SetCurrentMode:(VibratorMode)mode;
- (int32_t)stopVibratorByMode:(NSString *)mode;
@property (nonatomic, readonly) BOOL supportsCoreHaptics;
@end
NS_ASSUME_NONNULL_END
#endif // VIBRATOR_MANAGER_H