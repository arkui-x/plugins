/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#import <UIKit/UIKit.h>
#import "accessibilitySystemAbilityClient.h"

typedef void (^ScribeStateBlock)(bool state);

@interface AccessibilitySystemAbilityClient ()
@property(nonatomic, copy) ScribeStateBlock scribeStateBlock;
@property(nonatomic, assign) BOOL lastVoiceOverState;
@property(nonatomic, assign) BOOL observerRegistered;
@end
@implementation AccessibilitySystemAbilityClient

+ (instancetype)sharedInstance
{
    static AccessibilitySystemAbilityClient* sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] init];
    });
    return sharedInstance;
}

- (instancetype)init
{
    self = [super init];
    if (self) {
        self.lastVoiceOverState = UIAccessibilityIsVoiceOverRunning();
        self.scribeStateBlock = nil;
        self.observerRegistered = false;
    }
    return self;
}

- (bool)getAccessibilityEnabled
{
    return UIAccessibilityIsVoiceOverRunning();
}

- (void)subscribeState:(void (^)(bool state))block
{
    if (!block || self.observerRegistered) {
        return;
    }
    
    self.scribeStateBlock = block;
    NSNotificationCenter* center = [NSNotificationCenter defaultCenter];
    if (@available(iOS 11.0, *)) {
        [center addObserver:self
                   selector:@selector(voiceOverStatusChanged:)
                       name:UIAccessibilityVoiceOverStatusDidChangeNotification
                     object:nil];
    } else {
        [center addObserver:self
                   selector:@selector(voiceOverStatusChanged:)
                       name:UIAccessibilityVoiceOverStatusChanged
                     object:nil];
    }
    self.lastVoiceOverState = UIAccessibilityIsVoiceOverRunning();
    self.observerRegistered = true;
}

- (void)unsubscribeState:(void (^)(bool state))block
{
    if (!block) {
        self.scribeStateBlock = nil;
    }

    if (self.observerRegistered) {
        [[NSNotificationCenter defaultCenter] removeObserver:self];
        self.observerRegistered = false;
    }
}

- (void)voiceOverStatusChanged:(NSNotification*)notification
{
    bool state = UIAccessibilityIsVoiceOverRunning();
    if (self.scribeStateBlock && state != self.lastVoiceOverState) {
        self.scribeStateBlock(state);
    }
    self.lastVoiceOverState = state;
}

@end