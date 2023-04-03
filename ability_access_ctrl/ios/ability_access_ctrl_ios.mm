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

#include "ability_access_ctrl_ios.h"
#include "log.h"
#import <AVFoundation/AVCaptureDevice.h>
#import <AVFoundation/AVFoundation.h>

@implementation abilityAccessCtrlIOS

+ (instancetype)shareinstance{
    static dispatch_once_t onceToken;
    static abilityAccessCtrlIOS *instance = nil;
    dispatch_once(&onceToken, ^{
        instance = [abilityAccessCtrlIOS new];
    });
    return instance;
}

-(bool)CheckCameraPermission{
    AVAuthorizationStatus status = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
    switch (status) {
        case AVAuthorizationStatusNotDetermined:
        case AVAuthorizationStatusRestricted:
        case AVAuthorizationStatusDenied: {
            return NO;
        }
        case AVAuthorizationStatusAuthorized: {
            return YES;
        }
        default:
            break;
    }
    return NO;
}

-(bool)CheckMicrophonePermission{
  AVAuthorizationStatus microPhoneStatus = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio];
    switch (microPhoneStatus) {
        case AVAuthorizationStatusDenied:
        case AVAuthorizationStatusRestricted:
        case AVAuthorizationStatusNotDetermined: {
            return NO;
        }
        case AVAuthorizationStatusAuthorized: {
            return YES;
        }
        default:
            break;
    }
    return NO;
}

-(void)RequestMicrophonePermission: (IosCb)callback :(CallbackInfo*)data :(bool)isLast{
    AVAuthorizationStatus microPhoneStatus = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio];
    switch (microPhoneStatus) {
        case AVAuthorizationStatusNotDetermined: {
            [AVCaptureDevice requestAccessForMediaType:AVMediaTypeAudio completionHandler:^(BOOL granted) {
                dispatch_async(dispatch_get_main_queue(), ^{
                    callback(data, isLast, granted);
                });
            }];
            return;
        }
        case AVAuthorizationStatusAuthorized: {
            callback(data, isLast, 1);
            return;
        }
        case AVAuthorizationStatusDenied:
        case AVAuthorizationStatusRestricted: {
            callback(data, isLast, 0);
            return;
        }
        default:
            break;
    }
    callback(data, isLast, 0);
    return;
}

-(void)RequestCameraPermission: (IosCb)callback :(CallbackInfo*)data :(bool)isLast{
    AVAuthorizationStatus status = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
    switch (status) {
        case AVAuthorizationStatusNotDetermined: {
            [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL granted) {
                dispatch_async(dispatch_get_main_queue(), ^{
                    callback(data, isLast, granted);
                });
            }];
            return;
        }
        case AVAuthorizationStatusAuthorized: {
            callback(data, isLast, 1);
            return;
        }
        case AVAuthorizationStatusRestricted:
        case AVAuthorizationStatusDenied: {
            callback(data, isLast, 0);
            return;
        }
        default:
            break;
    }
    callback(data, isLast, 0);
    return;
}
@end

