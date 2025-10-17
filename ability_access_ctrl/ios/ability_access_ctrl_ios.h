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

#ifndef PLUGINS_IOS_ABILITYACCESSCTRL_ABILITY_ACCESS_CTRL_H
#define PLUGINS_IOS_ABILITYACCESSCTRL_ABILITY_ACCESS_CTRL_H

#include <string>
#include <vector>
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN
using RequestPermissionCallbackIos =
    std::function<void(void*, const std::vector<std::string> permissions, const std::vector<int> grantResults)>;

typedef struct {
    void *data;
    RequestPermissionCallbackIos napiCb;
    std::vector<std::string> permissionList;
    std::vector<int> grantResults;
    size_t index;
} CallbackInfo;

typedef enum GrantResultType {
    /** permission has been denied by user. */
    DENIED_BY_USER = -1,
    /** permission is granted. */
    GRANTED = 0,
    /** permission need dynamic pop-up windows to grant it. */
    NOT_DETERMINED = 1,
    /** invalid operation, something is wrong or the app is not permmited to use the permission. */
    INVALID_OPER = 2,
} GrantResultType;

@interface abilityAccessCtrlIOS : NSObject

typedef void (*IosCb)(CallbackInfo* info, bool isLast, int result);

+(instancetype)shareinstance;

-(bool)CheckCameraPermission;
-(void)RequestCameraPermission:(IosCb)callback :(CallbackInfo*)data :(bool)isLast;

-(bool)CheckMicrophonePermission;
-(void)RequestMicrophonePermission:(IosCb)callback :(CallbackInfo*)data :(bool)isLast;

-(bool)CheckPhotoPermission;
-(void)RequestPhotoPermission:(IosCb)callback :(CallbackInfo*)data :(bool)isLast;

-(bool)CheckLocationPermission;
-(void)RequestLocationPermission:(IosCb)callback :(CallbackInfo*)data :(bool)isLast;
@end

NS_ASSUME_NONNULL_END

#endif  // PLUGINS_IOS_ABILITYACCESSCTRL_ABILITY_ACCESS_CTRL_H
