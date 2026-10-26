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
#include <utility>

struct PendingLocationRequest {
    IosCb callback;
    CallbackInfo* data;
    bool isLast;
    bool accurate;
};

@interface abilityAccessCtrlIOS () <CLLocationManagerDelegate> {
    CLLocationManager* locationManager_;
    std::vector<PendingLocationRequest> pendingLocationRequests_;
}

- (int)locationResultForAccuracy:(bool)accurate;
- (void)completeLocationRequests;
@end

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

- (bool)CheckPhotoPermission {
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

- (int)locationResultForAccuracy:(bool)accurate {
    if (locationManager_ == nil) {
        locationManager_ = [[CLLocationManager alloc] init];
        locationManager_.delegate = self;
    }
    CLAuthorizationStatus status;
    if (@available(iOS 14.0, *)) {
        status = locationManager_.authorizationStatus;
    } else {
        status = [CLLocationManager authorizationStatus];
    }
    switch (status) {
        case kCLAuthorizationStatusNotDetermined:
            return GrantResultType::NOT_DETERMINED;
        case kCLAuthorizationStatusRestricted:
            return GrantResultType::INVALID_OPER;
        case kCLAuthorizationStatusDenied: {
            return GrantResultType::DENIED_BY_USER;
        }
        case kCLAuthorizationStatusAuthorizedAlways:
        case kCLAuthorizationStatusAuthorizedWhenInUse: {
            if (accurate) {
                if (@available(iOS 14.0, *)) {
                    return locationManager_.accuracyAuthorization == CLAccuracyAuthorizationFullAccuracy ?
                        GrantResultType::GRANTED : GrantResultType::DENIED_BY_USER;
                }
            }
            return GrantResultType::GRANTED;
        }
        default:
            break;
    }
    return GrantResultType::INVALID_OPER;
}

-(bool)CheckLocationPermission:(bool)accurate {
    __block bool granted = false;
    if ([NSThread isMainThread]) {
        granted = [self locationResultForAccuracy:accurate] == GrantResultType::GRANTED;
    } else {
        dispatch_sync(dispatch_get_main_queue(), ^{
            granted = [self locationResultForAccuracy:accurate] == GrantResultType::GRANTED;
        });
    }
    return granted;
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

- (void)completeLocationRequests {
    if (pendingLocationRequests_.empty() ||
        [self locationResultForAccuracy:false] == GrantResultType::NOT_DETERMINED) {
        return;
    }

    auto requests = std::move(pendingLocationRequests_);
    pendingLocationRequests_.clear();
    for (const auto& request : requests) {
        request.callback(request.data, request.isLast, [self locationResultForAccuracy:request.accurate]);
    }
}

- (void)locationManagerDidChangeAuthorization:(CLLocationManager*)manager API_AVAILABLE(ios(14.0)) {
    [self completeLocationRequests];
}

- (void)locationManager:(CLLocationManager*)manager didChangeAuthorizationStatus:(CLAuthorizationStatus)status {
    if (@available(iOS 14.0, *)) {
        return;
    }
    [self completeLocationRequests];
}

-(void)RequestLocationPermission:(bool)accurate callback:(IosCb)callback data:(CallbackInfo*)data isLast:(bool)isLast {
    if (![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self RequestLocationPermission:accurate callback:callback data:data isLast:isLast];
        });
        return;
    }

    int result = [self locationResultForAccuracy:accurate];
    if (result != GrantResultType::NOT_DETERMINED) {
        callback(data, isLast, result);
        return;
    }
    if ([[NSBundle mainBundle] objectForInfoDictionaryKey:@"NSLocationWhenInUseUsageDescription"] == nil) {
        LOGE("NSLocationWhenInUseUsageDescription is missing from Info.plist.");
        callback(data, isLast, GrantResultType::INVALID_OPER);
        return;
    }
    if (![CLLocationManager locationServicesEnabled]) {
        callback(data, isLast, GrantResultType::INVALID_OPER);
        return;
    }

    bool shouldRequest = pendingLocationRequests_.empty();
    pendingLocationRequests_.push_back({callback, data, isLast, accurate});
    if (shouldRequest) {
        if (locationManager_ == nil) {
            locationManager_ = [[CLLocationManager alloc] init];
            locationManager_.delegate = self;
        }
        [locationManager_ requestWhenInUseAuthorization];
    }
}
@end
