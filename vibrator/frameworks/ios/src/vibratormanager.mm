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

#import "VibratorManager.h"
#import <UIKit/UIKit.h>

constexpr int32_t ERROR = -1;
constexpr int32_t SUCCESS = 0;
constexpr int32_t PARAMETER_ERROR = 401;
constexpr int32_t TIEM_OUT_MAX = 1800000;

@implementation VibratorInfoObject
@end

@implementation EffectInfo
@end

#pragma mark - VibratorManager 

@interface VibratorManager ()
@property (nonatomic, strong) CHHapticEngine *hapticEngine;
@property (nonatomic, strong) id<CHHapticPatternPlayer> patternPlayer;
@property (nonatomic, assign) BOOL isHapticEngineRunning;
@property (nonatomic, assign) BOOL isVibrating;
@property (nonatomic, strong) NSMutableArray<NSTimer *> *activeTimers;
@property (nonatomic, assign) dispatch_queue_t vibrationQueue;
@property (nonatomic, strong) NSDictionary<NSString *, NSDictionary *> *effectMapping;
@property (nonatomic, strong) UIImpactFeedbackGenerator *impactGenerator;
@property (nonatomic, strong) UINotificationFeedbackGenerator *notificationGenerator;
@property (nonatomic, strong) UISelectionFeedbackGenerator *selectionGenerator;
@end

@implementation VibratorManager

+ (instancetype)sharedInstance {
    static VibratorManager *shared = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        shared = [[self alloc] init];
    });
    return shared;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _vibrationQueue = dispatch_queue_create("com.arkuix.vibrator.queue", DISPATCH_QUEUE_SERIAL);
        _activeTimers = [NSMutableArray array];

        self.isVibrating = NO;

        [self setupEffectMapping];
        [self initIalizeHapticEngine];
        [self initIalizeFeedbackGenerators];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(appDidEnterBackground:)
                                                     name:UIApplicationDidEnterBackgroundNotification
                                                   object:nil];
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                 selector:@selector(appWillEnterForeground:)
                                                     name:UIApplicationWillEnterForegroundNotification
                                                   object:nil];
    }
    return self;
}

- (void)dealloc {
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    dispatch_async(self.vibrationQueue, ^{
        [self stopVibrator];
    });

    [self stopHapticEngine];
}

#pragma mark - Lifecycle Management

- (void)setupEffectMapping {
    self.effectMapping = @{
        @"haptic.clock.timer": @{@"type": @"continuous", @"intensity": @0.8, @"sharpness": @0.5, @"duration": @2.0},
        @"haptic.effect.hard": @{@"type": @"transient", @"intensity": @0.6, @"sharpness": @0.5, @"duration": @0.05},
        @"haptic.effect.soft": @{@"type": @"transient", @"intensity": @0.5, @"sharpness": @0.3, @"duration": @0.03},
        @"haptic.effect.sharp": @{@"type": @"transient", @"intensity": @0.5, @"sharpness": @0.8, @"duration": @0.02},
        @"haptic.notice.success": @{
            @"type": @"pattern",
            @"events": @[
                @{@"type": @"transient", @"time": @0.0, @"intensity": @0.8, @"sharpness": @0.6, @"duration": @0.17},
                @{@"type": @"transient", @"time": @0.25, @"intensity": @0.8, @"sharpness": @0.6, @"duration": @0.17}
            ],
        },
        @"haptic.notice.fail": @{
            @"type": @"pattern", 
            @"events": @[
                @{@"type": @"transient", @"time": @0.0, @"intensity": @0.9, @"sharpness": @0.3, @"duration": @0.25},
                @{@"type": @"transient", @"time": @0.2, @"intensity": @0.9, @"sharpness": @0.3, @"duration": @0.25}
            ],
        },
        @"haptic.notice.warning": @{
            @"type": @"pattern",
            @"events": @[
                @{@"type": @"continuous", @"time": @0.0, @"intensity": @0.9, @"sharpness": @0.5, @"duration": @0.2},
                @{@"type": @"continuous", @"time": @1.0, @"intensity": @0.9, @"sharpness": @0.5, @"duration": @0.2},
                @{@"type": @"continuous", @"time": @2.0, @"intensity": @0.9, @"sharpness": @0.5, @"duration": @0.2}
            ],
        }
    };
}

- (void)initIalizeFeedbackGenerators {
    if (@available(iOS 10.0, *)) {
        _impactGenerator = [[UIImpactFeedbackGenerator alloc] initWithStyle:UIImpactFeedbackStyleMedium];
        [_impactGenerator prepare];
        _notificationGenerator = [[UINotificationFeedbackGenerator alloc] init];
        [_notificationGenerator prepare];
        _selectionGenerator = [[UISelectionFeedbackGenerator alloc] init];
        [_selectionGenerator prepare];
    }
}

- (void)initIalizeHapticEngine {
    NSLog(@"initIalizeHapticEngine enter");
    if (@available(iOS 13.0, *)) {
        if (![CHHapticEngine.capabilitiesForHardware supportsHaptics]) {
            NSLog(@"Device does not support Core Haptics");
            return;
        }
        NSError *error = nil;
        _hapticEngine = [[CHHapticEngine alloc] initAndReturnError:&error];
        if (error) {
            NSLog(@"Haptic engine creation error: %@", error.localizedDescription);
            return;
        }

        if (!_hapticEngine) {
            NSLog(@"Haptic engine is nil after initialization attempts");
            return;
        }
        NSValue *selfValue = [NSValue valueWithPointer:(const void *)self];
        _hapticEngine.resetHandler = ^{
            dispatch_async(dispatch_get_main_queue(), ^{
                VibratorManager *manager = (VibratorManager *)[selfValue pointerValue];
                if (manager) {
                    [manager startHapticEngine];
                }
            });
        };

        _hapticEngine.stoppedHandler = ^(CHHapticEngineStoppedReason reason) {
            NSLog(@"Haptic engine stopped: %ld", (long)reason);
        };

        [self startHapticEngine];
    }
    NSLog(@"initIalizeHapticEngine leave");
}

- (void)startHapticEngine {
    NSLog(@"startHapticEngine enter");
    if (@available(iOS 13.0, *)) {
        if (!_hapticEngine) {
            NSLog(@"Haptic engine is not initialized,_hapticEngine is nil");
            return;
        }
        NSError *error = nil;
        [_hapticEngine startAndReturnError:&error];
        if (error) {
            NSLog(@"Failed to start haptic engine: %@", error.localizedDescription);
        } else {
            _isHapticEngineRunning = YES;
        }
    }
    NSLog(@"startHapticEngine leave");
}

- (void)stopHapticEngine {
    NSLog(@"stopHapticEngine enter");
    if (@available(iOS 13.0, *)) {
        if (!_hapticEngine) {
            NSLog(@"Haptic engine is not initialized,_hapticEngine is nil");
            return;
        }
        [_hapticEngine stopWithCompletionHandler:^(NSError *error) {
            if (error) {
                NSLog(@"Error stopping haptic engine: %@", error.localizedDescription);
            } else {
                self->_isHapticEngineRunning = NO;
            }
        }];
    }
    NSLog(@"stopHapticEngine leave");
}

- (int32_t)vibrateWithTimeOut:(int32_t)timeOut {
    NSLog(@"vibrateWithTimeOut enter,timeOut:%dms, usage: %d", timeOut);
    [self setCurrentMode:VibratorModeTime];
    if (timeOut <= 0 || timeOut > TIEM_OUT_MAX) {
        NSLog(@"vibrateWithTimeOut timeOut invalid, timeOut:%dms", timeOut);
        return PARAMETER_ERROR;
    }
    if (self.isVibrating) {
        NSLog(@"vibrateWithTimeOut already vibrating, stop first.");
        [self stopVibrator];
    }

    self.isVibrating = YES;
    
    if (@available(iOS 13.0, *)) {
        [self playVibrationWithDuration:timeOut/1000.0];
    } else {
        AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
    }
    NSLog(@"vibrateWithTimeOut leave");
    return SUCCESS;
}

- (void)playVibrationWithDuration:(NSTimeInterval)duration {
    NSLog(@"playVibrationWithDuration enter,duration:%d", duration);
    CGFloat intensity = 1.0;
    CGFloat sharpness = 1.0;
    
    [self playVibrationCoreWithDuration:duration intensity:intensity sharpness:sharpness];
}

- (int32_t)stopVibrator {
    NSLog(@"stopVibrator enter");
    if (!self.isVibrating) {
        NSLog(@"No active vibration to stop");
        return SUCCESS;
    }
    for (NSTimer *timer in self.activeTimers) {
        if ([timer isValid]) {
            [timer invalidate];
            NSLog(@"Invalidated timer: %@", timer);
        }
    }
    [self.activeTimers removeAllObjects];
    if (@available(iOS 13.0, *) && self.patternPlayer) {
        NSError *error = nil;
        [self.patternPlayer stopAtTime:0 error:&error];
        NSLog(@"Error stopping pattern player: %@", error.localizedDescription);
        self.patternPlayer = nil;
    }
    self.isVibrating = NO;
    NSLog(@"stopVibrator leave");
    return SUCCESS;
}

- (int32_t)playVibratorEffect:(NSString *)effect loopCount:(int32_t)loopCount {
    NSLog(@"playVibratorEffect enter");
    if (!@available(iOS 13.0, *)) {
        NSLog(@"playVibratorEffect error: version not support ");
        return ERROR;
    }

    [self setCurrentMode:VibratorModePreset];
    if (!effect || effect.length == 0) {
        NSLog(@"playVibratorEffect error: effect is nil or empty");
        return PARAMETER_ERROR;
    }

    NSDictionary *effectConfig = self.effectMapping[effect];
    if (!effectConfig) {
        NSLog(@"playVibratorEffect error: effect config not found: %@", effect);
        return ERROR;
    }
    self.isVibrating = YES;
    NSString *effectType = effectConfig[@"type"];
    if ([effectType isEqualToString:@"pattern"]) {
        NSArray *events = effectConfig[@"events"];
        return [self playHapticPattern:events loopCount:loopCount];
    } else {
        float intensity = [effectConfig[@"intensity"] floatValue];
        float sharpness = [effectConfig[@"sharpness"] floatValue];
        float duration = [effectConfig[@"duration"] floatValue];
        return [self playHaptic:intensity sharpness:sharpness duration:duration loopCount:loopCount];
    }
}

- (int32_t)playHapticPattern:(NSArray *)events loopCount:(int32_t)loopCount {
    NSLog(@"playHapticPattern enter ");
    if (!_hapticEngine || !_isHapticEngineRunning) {
        NSLog(@"HapticEngine not available");
        return ERROR;
    }
    NSError *error = nil;
    NSMutableArray<CHHapticEvent *> *hapticEvents = [NSMutableArray array];
    for (int loop = 0; loop < MAX(1, loopCount); loop++) {
        NSTimeInterval loopStartTime = 0;
        if (loop > 0) {
            loopStartTime = loop * [self calculatePatternDuration:events];
        }
        for (NSDictionary *eventConfig in events) {
            NSTimeInterval eventTime = loopStartTime + [eventConfig[@"time"] doubleValue];
            float intensity = [eventConfig[@"intensity"] floatValue];
            float sharpness = [eventConfig[@"sharpness"] floatValue];
            float duration = [eventConfig[@"duration"] floatValue];
            CHHapticEventType eventType;
            NSString *eventTypeStr = eventConfig[@"type"];
            if (eventTypeStr) {
                if ([eventTypeStr isEqualToString:@"continuous"]) {
                    eventType = CHHapticEventTypeHapticContinuous;
                } else {
                    eventType = CHHapticEventTypeHapticTransient;
                }
            }

            CHHapticEventParameter *intensityParam = [[CHHapticEventParameter alloc] 
                initWithParameterID:CHHapticEventParameterIDHapticIntensity value:intensity];
            CHHapticEventParameter *sharpnessParam = [[CHHapticEventParameter alloc] 
                initWithParameterID:CHHapticEventParameterIDHapticSharpness value:sharpness];
            
            CHHapticEvent *event = [[CHHapticEvent alloc] 
                initWithEventType:eventType
                parameters:@[intensityParam, sharpnessParam]
                relativeTime:eventTime
                duration:duration];
            
            [hapticEvents addObject:event];
        }
    }

    CHHapticPattern *pattern = [[CHHapticPattern alloc] initWithEvents:hapticEvents parameters:@[] error:&error];

    if (error) {
        NSLog(@"Error creating pattern: %@", error);
        return ERROR;
    }

    id<CHHapticPatternPlayer> player = [_hapticEngine createPlayerWithPattern:pattern error:&error];
    if (error) {
        NSLog(@"Error creating player: %@", error);
        return ERROR;
    }

    [player startAtTime:0 error:&error];
    if (error) {
        NSLog(@"Error starting player: %@", error);
        return ERROR;
    }

    self.patternPlayer = player;
    return SUCCESS;
}

- (NSTimeInterval)calculatePatternDuration:(NSArray *)events {
    NSTimeInterval maxTime = 0;
    for (NSDictionary *eventConfig in events) {
        NSTimeInterval eventEndTime = [eventConfig[@"time"] doubleValue] + [eventConfig[@"duration"] doubleValue];
        if (eventEndTime > maxTime) {
            maxTime = eventEndTime;
        }
    }
    return maxTime;
}

- (int32_t)playHaptic:(float)intensity sharpness:(float)sharpness duration:(float)duration loopCount:(int32_t)loopCount {
    NSLog(@"playHaptic enter");
    if (!_hapticEngine || !_isHapticEngineRunning) {
        NSLog(@"HapticEngine create failed or not running");
        return ERROR;
    }
    NSError *error = nil;
    NSMutableArray<CHHapticEvent *> *events = [NSMutableArray array];
    float gap = 0.1;
    for (int i = 0; i < MAX(1, loopCount); i++) {
        CHHapticEventParameter *intensityParam = [[CHHapticEventParameter alloc] 
                                                    initWithParameterID:CHHapticEventParameterIDHapticIntensity 
                                                    value:intensity];
        CHHapticEventParameter *sharpnessParam = [[CHHapticEventParameter alloc] 
                                                    initWithParameterID:CHHapticEventParameterIDHapticSharpness 
                                                    value:sharpness];
        float startTime = i * (duration + gap);
        CHHapticEvent *event = [[CHHapticEvent alloc] 
                                initWithEventType:CHHapticEventTypeHapticContinuous 
                                parameters:@[intensityParam, sharpnessParam] 
                                relativeTime:startTime duration:duration];
        [events addObject:event];
    }
    CHHapticPattern *pattern = [[CHHapticPattern alloc] initWithEvents:events parameters:@[] error:&error];
    if (!error) {
        id<CHHapticPatternPlayer> player = [_hapticEngine createPlayerWithPattern:pattern error:&error];
        if (player == nil || error) {
            NSLog(@"player is nul or error: %@", error);
            return ERROR;
        }
        [player startAtTime:0 error:&error];
        NSLog(@"Error playHaptic starting player: %@", error);
        if (!error) {
            self.patternPlayer = player;
            NSLog(@"playHaptic player: %@", error);
            return SUCCESS;
        }
    }
    return SUCCESS;
}

- (BOOL)isSupportEffect:(NSString *)effect {
    NSLog(@"isSupportEffect enter, effect:%@", effect);
    BOOL supported = self.effectMapping[effect] != nil;
    return supported;
}

- (NSArray<VibratorInfoObject *> *)getVibratorList {
    NSLog(@"getVibratorList enter");
    VibratorInfoObject *vibrator = [[VibratorInfoObject alloc] init];
    vibrator.deviceId = -1;
    vibrator.vibratorId = 0;
    vibrator.deviceName = @"iOS Taptic Engine";
    vibrator.isSupportHdHaptic = [self supportsCoreHaptics];
    vibrator.isLocalVibrator = YES;
    vibrator.position = 0;
    return @[vibrator];
}

- (EffectInfo *)getEffectInfo:(NSString *)effectType {
    NSLog(@"getEffectInfo enter, effectType:%@", effectType);
    EffectInfo *info = [[EffectInfo alloc] init];

    NSDictionary *effectConfig = self.effectMapping[effectType];
    if (effectConfig) {
        NSLog(@"getEffectInfo effectConfig found for: %@", effectType);
        info.duration = @([effectConfig[@"duration"] floatValue]);
        info.isSupportEffect = YES;
    } else {
        NSLog(@"getEffectInfo effectConfig not found for: %@", effectType);
        info.isSupportEffect = NO;
    }
    return info;
}

- (BOOL)supportsCoreHaptics {
    NSLog(@"supportsCoreHaptics enter");
    if (@available(iOS 13.0, *)) {
        return [CHHapticEngine.capabilitiesForHardware supportsHaptics];
    }
    return NO;
}

- (void)playVibrationCoreWithDuration:(float)duration intensity:(float)intensity sharpness:(float)sharpness {
    NSLog(@"playVibrationCoreWithDuration enter");
    NSError *error = nil;
    CHHapticEventParameter *intensityParam = [[CHHapticEventParameter alloc] 
                                                initWithParameterID:CHHapticEventParameterIDHapticIntensity 
                                                value:intensity];
    CHHapticEventParameter *sharpnessParam = [[CHHapticEventParameter alloc] 
                                                initWithParameterID:CHHapticEventParameterIDHapticSharpness 
                                                value:sharpness];
    CHHapticEvent *event = [[CHHapticEvent alloc] 
                            initWithEventType:CHHapticEventTypeHapticContinuous 
                            parameters:@[intensityParam, sharpnessParam] 
                            relativeTime:0 duration:duration];
    CHHapticPattern *pattern = [[CHHapticPattern alloc] initWithEvents:@[event] parameters:@[] error:&error];
    NSLog(@"Error creating pattern: %@", error);
    if (!error) {
        id<CHHapticPatternPlayer> player = [_hapticEngine createPlayerWithPattern:pattern error:&error];
        if (player == nil || error) {
            NSLog(@"player is nul or error: %@", error);
            return;
        }
        [player startAtTime:0 error:&error];
        NSLog(@"Error starting player: %@", error);
        if (!error) {
            self.patternPlayer = player;
            return;
        }
    }
}

- (void)playPattern:(const OHOS::Sensors::VibratePattern &)pattern {
    NSLog(@"playPattern enter");
    if (self.isVibrating) {
        NSLog(@"playPattern already vibrating, stop first.");
        [self stopVibrator];
    }
    NSMutableArray<CHHapticEvent *> *hapticEvents = [NSMutableArray array];
    NSMutableArray<CHHapticParameterCurve *> *parameterCurves = [NSMutableArray array];

    for (const OHOS::Sensors::VibrateEvent &event : pattern.events) {
        NSTimeInterval eventTime = event.time / 1000.0;
        NSTimeInterval eventDuration = event.duration / 1000.0;

        CHHapticEventType eventType = (event.tag == OHOS::Sensors::EVENT_TAG_CONTINUOUS) 
            ? CHHapticEventTypeHapticContinuous 
            : CHHapticEventTypeHapticTransient;

        float intensity = event.intensity / 100.0f;
        float sharpness = event.frequency / 100.0f;
        NSArray<CHHapticEventParameter *> *parameters = @[
            [[CHHapticEventParameter alloc] initWithParameterID:CHHapticEventParameterIDHapticIntensity value:intensity],
            [[CHHapticEventParameter alloc] initWithParameterID:CHHapticEventParameterIDHapticSharpness value:sharpness]
        ];

        CHHapticEvent *hapticEvent = [[CHHapticEvent alloc] 
            initWithEventType:eventType
            parameters:parameters
            relativeTime:eventTime
            duration:eventDuration];
        [hapticEvents addObject:hapticEvent];

        if (!event.points.empty()) {
            std::vector<OHOS::Sensors::VibrateCurvePoint> sortedPoints = event.points;
            std::sort(sortedPoints.begin(), sortedPoints.end());
            NSMutableArray<CHHapticParameterCurveControlPoint *> *intensityPoints = [NSMutableArray array];
            NSMutableArray<CHHapticParameterCurveControlPoint *> *sharpnessPoints = [NSMutableArray array];
            for (const OHOS::Sensors::VibrateCurvePoint &point : sortedPoints) {
                CHHapticParameterCurveControlPoint *intensityControlPoint = [[CHHapticParameterCurveControlPoint alloc]
                    initWithRelativeTime:point.time / 1000.0
                    value:point.intensity / 100.0f];
                [intensityPoints addObject:intensityControlPoint];
                CHHapticParameterCurveControlPoint *sharpnessControlPoint = [[CHHapticParameterCurveControlPoint alloc]
                    initWithRelativeTime:point.time / 1000.0
                    value:point.frequency / 100.0f];
                [sharpnessPoints addObject:sharpnessControlPoint];
            }
            CHHapticParameterCurve *intensityCurve = [[CHHapticParameterCurve alloc]
                initWithParameterID:CHHapticDynamicParameterIDHapticIntensityControl
                controlPoints:intensityPoints
                relativeTime:eventTime];
            [parameterCurves addObject:intensityCurve];
            CHHapticParameterCurve *sharpnessCurve = [[CHHapticParameterCurve alloc]
                initWithParameterID:CHHapticDynamicParameterIDHapticSharpnessControl
                controlPoints:sharpnessPoints
                relativeTime:eventTime];
            [parameterCurves addObject:sharpnessCurve];
        }
    }

    NSError *error = nil;
    CHHapticPattern *hapticPattern = [[CHHapticPattern alloc] 
                                     initWithEvents:hapticEvents 
                                     parameterCurves:parameterCurves 
                                     error:&error];
    if (!error) {
        id<CHHapticPatternPlayer> player = [_hapticEngine createPlayerWithPattern:hapticPattern error:&error];
        if (player == nil || error) {
            NSLog(@"player is nul or error: %@", error);
            return;
        }
        [player startAtTime:CHHapticTimeImmediate error:&error];
        NSLog(@"Error playPattern starting player: %@", error);
        if (!error) {
            self.patternPlayer = player;
            self.isVibrating = YES;
            return;
        }
    }
}

- (void)playPatterns:(const std::vector<OHOS::Sensors::VibratePattern> &)patterns {
    NSLog(@"playPatterns enter");
    if (self.isVibrating) {
        NSLog(@"playPatterns already vibrating, stop first.");
        [self stopVibrator];
    }
    NSDictionary *hapticComponents = [self convertPatternsToHapticComponents:patterns];
    NSArray<CHHapticEvent *> *allHapticEvents = hapticComponents[@"events"];
    NSArray<CHHapticParameterCurve *> *allParameterCurves = hapticComponents[@"curves"];
    [self createAndPlayHapticPatternWithEvents:allHapticEvents curves:allParameterCurves];
    NSLog(@"playPatterns leave");
}

- (NSDictionary *)convertPatternsToHapticComponents:(const std::vector<OHOS::Sensors::VibratePattern> &)patterns {
    NSLog(@"convertPatternsToHapticComponents enter");
    NSMutableArray<CHHapticEvent *> *allHapticEvents = [NSMutableArray array];
    NSMutableArray<CHHapticParameterCurve *> *allParameterCurves = [NSMutableArray array];
    for (const OHOS::Sensors::VibratePattern &pattern : patterns) {
        [self processVibratePattern:pattern
                   intoHapticEvents:allHapticEvents
                 andParameterCurves:allParameterCurves];
    }
    return @{
        @"events": allHapticEvents,
        @"curves": allParameterCurves
    };
}

- (void)processVibratePattern:(const OHOS::Sensors::VibratePattern &)pattern
             intoHapticEvents:(NSMutableArray<CHHapticEvent *> *)hapticEvents
           andParameterCurves:(NSMutableArray<CHHapticParameterCurve *> *)parameterCurves {
    NSTimeInterval patternStartTime = (NSTimeInterval)pattern.startTime / 1000.0;
    for (const OHOS::Sensors::VibrateEvent &event : pattern.events) {
        NSTimeInterval eventTime = patternStartTime + ((NSTimeInterval)event.time / 1000.0);
        CHHapticEvent *hapticEvent = [self createHapticEventFromVibrateEvent:event
                                                                    eventTime:eventTime];
        [hapticEvents addObject:hapticEvent];
        if (!event.points.empty()) {
            [self addParameterCurvesForEvent:event
                                   eventTime:eventTime
                     toParameterCurvesArray:parameterCurves];
        }
    }
}

- (CHHapticEvent *)createHapticEventFromVibrateEvent:(const OHOS::Sensors::VibrateEvent &)event
                                           eventTime:(NSTimeInterval)eventTime {
    NSTimeInterval eventDuration = (NSTimeInterval)event.duration / 1000.0;
    float intensity = event.intensity / 100.0f;
    float sharpness = [self mapFrequencyToSharpness:event.frequency];
    NSArray<CHHapticEventParameter *> *parameters = @[
        [[CHHapticEventParameter alloc] initWithParameterID:CHHapticEventParameterIDHapticIntensity value:intensity],
        [[CHHapticEventParameter alloc] initWithParameterID:CHHapticEventParameterIDHapticSharpness value:sharpness]
    ];
    if (event.tag == OHOS::Sensors::EVENT_TAG_CONTINUOUS) {
        return [[CHHapticEvent alloc] initWithEventType:CHHapticEventTypeHapticContinuous
                                            parameters:parameters
                                            relativeTime:eventTime
                                            duration:eventDuration];
    } else {
        return [[CHHapticEvent alloc] initWithEventType:CHHapticEventTypeHapticTransient
                                            parameters:parameters
                                            relativeTime:eventTime];
    }
}

- (void)addParameterCurvesForEvent:(const OHOS::Sensors::VibrateEvent &)event
                         eventTime:(NSTimeInterval)eventTime
           toParameterCurvesArray:(NSMutableArray<CHHapticParameterCurve *> *)parameterCurves {
    std::vector<OHOS::Sensors::VibrateCurvePoint> sortedPoints = event.points;
    std::sort(sortedPoints.begin(), sortedPoints.end());
    NSMutableArray<CHHapticParameterCurveControlPoint *> *intensityPoints = [NSMutableArray array];
    NSMutableArray<CHHapticParameterCurveControlPoint *> *sharpnessPoints = [NSMutableArray array];

    for (const OHOS::Sensors::VibrateCurvePoint &point : sortedPoints) {
        NSTimeInterval pointTime = eventTime + (point.time / 1000.0);
        float pointIntensity = point.intensity / 100.0f;
        float pointSharpness = [self mapFrequencyToSharpness:point.frequency];
        CHHapticParameterCurveControlPoint *intensityControlPoint = [[CHHapticParameterCurveControlPoint alloc]
            initWithRelativeTime:pointTime
            value:pointIntensity];
        [intensityPoints addObject:intensityControlPoint];
        CHHapticParameterCurveControlPoint *sharpnessControlPoint = [[CHHapticParameterCurveControlPoint alloc]
            initWithRelativeTime:pointTime
            value:pointSharpness];
        [sharpnessPoints addObject:sharpnessControlPoint];
    }
    if (intensityPoints.count > 0) {
        CHHapticParameterCurve *intensityCurve = [[CHHapticParameterCurve alloc]
            initWithParameterID:CHHapticDynamicParameterIDHapticIntensityControl
            controlPoints:intensityPoints
            relativeTime:eventTime];
        [parameterCurves addObject:intensityCurve];
    }
    if (sharpnessPoints.count > 0) {
        CHHapticParameterCurve *sharpnessCurve = [[CHHapticParameterCurve alloc]
            initWithParameterID:CHHapticDynamicParameterIDHapticSharpnessControl
            controlPoints:sharpnessPoints
            relativeTime:eventTime];
        [parameterCurves addObject:sharpnessCurve];
    }
}

- (void)createAndPlayHapticPatternWithEvents:(NSArray<CHHapticEvent *> *)events
                                      curves:(NSArray<CHHapticParameterCurve *> *)curves {

    NSError *error = nil;
    CHHapticPattern *hapticPattern = [[CHHapticPattern alloc] 
                                    initWithEvents:events
                                    parameterCurves:curves
                                    error:&error];
    if (error) {
        NSLog(@"Error creating haptic pattern: %@", error);
        return;
    }
    id<CHHapticPatternPlayer> player = [_hapticEngine createPlayerWithPattern:hapticPattern error:&error];
    if (player == nil || error) {
        NSLog(@"player is nul or error: %@", error);
        return;
    }
    [player startAtTime:CHHapticTimeImmediate error:&error];
    if (error) {
        NSLog(@"Error starting haptic player: %@", error);
        return;
    }
    self.patternPlayer = player;
    self.isVibrating = YES;
}

- (float)mapFrequencyToSharpness:(int32_t)frequency {
    int32_t absFrequency = std::abs(frequency);
    float baseSharpness = absFrequency / 100.0f;

    if (frequency < 0) {
        return 0.1f + (baseSharpness * 0.4f);
    } else {
        return 0.5f + (baseSharpness * 0.5f);
    }
}

- (void)appDidEnterBackground:(NSNotification *)notification {
    [self stopVibrator];
    [self stopHapticEngine];
}

- (void)appWillEnterForeground:(NSNotification *)notification {
    [self startHapticEngine];
}

- (void)setCurrentMode:(VibratorMode)mode {
    _currentMode = mode;
    self.isVibrating = YES;
}

- (int32_t)stopVibratorByMode:(NSString *)mode {
    NSLog(@"Stop vibration mode: %@", mode);
    NSString *modeLower = [mode lowercaseString];
    VibratorMode vibratorMode;
    if ([modeLower isEqualToString:@"time"]) {
        vibratorMode = VibratorModeTime;
    } 
    else if ([modeLower isEqualToString:@"preset"]) {
        vibratorMode = VibratorModePreset;
    } else {
        return [self stopVibrator];
    }
    if (_currentMode == vibratorMode ) {
        return [self stopVibrator];
    } else {
        return SUCCESS;
    }
}
@end