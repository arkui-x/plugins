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

#import "BluetoothA2dp.h"

@implementation BluetoothA2dp
static NSString* UIDPREFIXS = @"00002909-0000-1000-8000-";

+ (BluetoothA2dp*)sharedInstance
{
    static BluetoothA2dp* a2dp = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
      a2dp = [[BluetoothA2dp alloc] init];
    });

    return a2dp;
}

- (void)setNotifyRouteChange
{
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(handleAudioRouteChange:)
                                                 name:AVAudioSessionRouteChangeNotification
                                               object:[AVAudioSession sharedInstance]];
}

- (void)closeNotifyRouteChange
{
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:AVAudioSessionRouteChangeNotification
                                                  object:[AVAudioSession sharedInstance]];
}

- (void)handleAudioRouteChange:(NSNotification*)notification
{
    int32_t reason = [[notification.userInfo valueForKey:AVAudioSessionRouteChangeReasonKey] integerValue];
    AVAudioSessionRouteDescription* routDescription =
        [notification.userInfo valueForKey:AVAudioSessionRouteChangePreviousRouteKey];
    if (routDescription.outputs.count != 0 && self.connectStateChangeBlock) {
        NSString* strUID = routDescription.outputs.lastObject.UID;
        strUID = [BluetoothA2dp getUIDBits32:strUID];
        self.connectStateChangeBlock(strUID, reason);
    }
}

+ (NSString*)getUIDBits32:(NSString*)strUid
{
    strUid = [strUid stringByReplacingOccurrencesOfString:@"-tacl" withString:@""];
    strUid = [strUid stringByReplacingOccurrencesOfString:@":" withString:@""];
    strUid = [NSString stringWithFormat:@"%@%@", UIDPREFIXS, strUid];
    return strUid;
}

+ (int)GetDeviceState:(NSString*)strDeviceId
{
    strDeviceId = [strDeviceId stringByReplacingOccurrencesOfString:UIDPREFIXS withString:@""];
    AVAudioSession* session = [AVAudioSession sharedInstance];
    [session setCategory:AVAudioSessionCategoryPlayAndRecord
                    mode:AVAudioSessionModeDefault
                 options:AVAudioSessionCategoryOptionAllowBluetooth | AVAudioSessionCategoryOptionAllowBluetoothA2DP |
                         AVAudioSessionCategoryOptionInterruptSpokenAudioAndMixWithOthers
                   error:nil];
    int state = static_cast<int>(BTConnectState::DISCONNECTED);
    for (AVAudioSessionPortDescription* portDescription in session.availableInputs) {
        NSString* strUID = [portDescription.UID stringByReplacingOccurrencesOfString:@":" withString:@""];
        if ([strUID containsString:strDeviceId]) {
            state = static_cast<int>(BTConnectState::CONNECTED);
        }
    }
    return state;
}

+ (NSMutableArray<NSString*>*)GetDevicesByStates
{
    AVAudioSession* session = [AVAudioSession sharedInstance];
    [session setCategory:AVAudioSessionCategoryPlayAndRecord
                    mode:AVAudioSessionModeDefault
                 options:AVAudioSessionCategoryOptionAllowBluetooth | AVAudioSessionCategoryOptionAllowBluetoothA2DP |
                         AVAudioSessionCategoryOptionInterruptSpokenAudioAndMixWithOthers
                   error:nil];
    AVAudioSessionRouteDescription* routeDescription = [session currentRoute];
    NSMutableArray* arrPort = [NSMutableArray array];
    for (AVAudioSessionPortDescription* portDescription in routeDescription.outputs) {
        NSString* portType = [portDescription portType];
        if (portType && [portType isEqualToString:@"BluetoothA2DPOutput"]) {
            NSString* strUID = [BluetoothA2dp getUIDBits32:portDescription.UID];
            [arrPort addObject:strUID];
        }
    }
    return arrPort;
}

@end