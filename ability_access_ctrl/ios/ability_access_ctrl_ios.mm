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
#import <Photos/Photos.h>
#import <CoreLocation/CoreLocation.h>

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

- (bool)checkPhotoPermission {
    PHAuthorizationStatus status = [PHPhotoLibrary authorizationStatus];
    switch (status) {
        case PHAuthorizationStatusNotDetermined:
        case PHAuthorizationStatusRestricted:
        case PHAuthorizationStatusDenied: {
            return NO;
        }
        case PHAuthorizationStatusLimited:
        case PHAuthorizationStatusAuthorized: {
            return YES;
        }
        default:
            break;
    }
    return NO;
}

-(bool)CheckLocationPermission{
    CLAuthorizationStatus status = [CLLocationManager authorizationStatus];
    switch (status) {
        case kCLAuthorizationStatusNotDetermined:
        case kCLAuthorizationStatusRestricted:
        case kCLAuthorizationStatusDenied: {
            return NO;
        }
        case kCLAuthorizationStatusAuthorizedAlways:
        case kCLAuthorizationStatusAuthorizedWhenInUse: {
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
                    int result = granted ? GrantResultType::GRANTED : GrantResultType::DENIED_BY_USER;
                    callback(data, isLast, result);
                });
            }];
            return;
        }
        case AVAuthorizationStatusAuthorized: {
            callback(data, isLast, GrantResultType::GRANTED);
            return;
        }
        case AVAuthorizationStatusDenied: {
            callback(data, isLast, GrantResultType::DENIED_BY_USER);
            return;
        }
        case AVAuthorizationStatusRestricted:
        default:
            break;
    }
    callback(data, isLast, GrantResultType::INVALID_OPER);
    return;
}

-(void)RequestCameraPermission: (IosCb)callback :(CallbackInfo*)data :(bool)isLast{
    AVAuthorizationStatus status = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeVideo];
    switch (status) {
        case AVAuthorizationStatusNotDetermined: {
            [AVCaptureDevice requestAccessForMediaType:AVMediaTypeVideo completionHandler:^(BOOL granted) {
                dispatch_async(dispatch_get_main_queue(), ^{
                    int result = granted ? GrantResultType::GRANTED : GrantResultType::DENIED_BY_USER;
                    callback(data, isLast, result);
                });
            }];
            return;
        }
        case AVAuthorizationStatusAuthorized: {
            callback(data, isLast, GrantResultType::GRANTED);
            return;
        }
        case AVAuthorizationStatusDenied: {
            callback(data, isLast, GrantResultType::DENIED_BY_USER);
            return;
        }
        case AVAuthorizationStatusRestricted:
        default:
            break;
    }
    callback(data, isLast, GrantResultType::INVALID_OPER);
    return;
}

-(void)RequestPhotoPermission: (IosCb)callback :(CallbackInfo*)data :(bool)isLast{
    PHAuthorizationStatus status = [PHPhotoLibrary authorizationStatus];
    switch (status) {
        case PHAuthorizationStatusNotDetermined: {
            [PHPhotoLibrary requestAuthorization:^(PHAuthorizationStatus status) {
                dispatch_async(dispatch_get_main_queue(), ^{
                    switch (status) {
                        case PHAuthorizationStatusLimited:
                        case PHAuthorizationStatusAuthorized: {
                            callback(data, isLast, GrantResultType::GRANTED);
                            return;
                        }
                        case PHAuthorizationStatusDenied: {
                            callback(data, isLast, GrantResultType::DENIED_BY_USER);
                            return;
                        }
                        case PHAuthorizationStatusRestricted:
                        default:
                            break;
                    }
                    callback(data, isLast, GrantResultType::INVALID_OPER);
                });
            }];
            return;
        }
        case PHAuthorizationStatusLimited:
        case PHAuthorizationStatusAuthorized: {
            callback(data, isLast, GrantResultType::GRANTED);
            return;
        }
        case PHAuthorizationStatusDenied: {
            callback(data, isLast, GrantResultType::DENIED_BY_USER);
            return;
        }
        case PHAuthorizationStatusRestricted:
        default:
            break;
    }
    callback(data, isLast, GrantResultType::INVALID_OPER);
    return;
}

-(void)RequestLocationPermission: (IosCb)callback :(CallbackInfo*)data :(bool)isLast{
    CLAuthorizationStatus status = [CLLocationManager authorizationStatus];
    switch (status) {
        case kCLAuthorizationStatusNotDetermined: {
            CLLocationManager* locationManager = [[CLLocationManager alloc] init];
            [locationManager requestWhenInUseAuthorization];
            callback(data, isLast, GrantResultType::GRANTED);
            return;
        }
        case kCLAuthorizationStatusAuthorizedAlways:
        case kCLAuthorizationStatusAuthorizedWhenInUse: {
            callback(data, isLast, GrantResultType::GRANTED);
            return;
        }
        case kCLAuthorizationStatusDenied: {
            callback(data, isLast, GrantResultType::DENIED_BY_USER);
            return;
        }
        case kCLAuthorizationStatusRestricted:
        default:
            break;
    }
    callback(data, isLast, GrantResultType::INVALID_OPER);
    return;
}
@end

