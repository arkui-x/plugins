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

@interface abilityAccessCtrlIOS : NSObject

typedef void (*IosCb)(CallbackInfo* info, bool isLast, bool granted);

+(instancetype)shareinstance;

-(bool)CheckCameraPermission;
-(void)RequestCameraPermission:(IosCb)callback :(CallbackInfo*)data :(bool)isLast;

-(bool)CheckMicrophonePermission;
-(void)RequestMicrophonePermission:(IosCb)callback :(CallbackInfo*)data :(bool)isLast;
@end

NS_ASSUME_NONNULL_END

#endif  // PLUGINS_IOS_ABILITYACCESSCTRL_ABILITY_ACCESS_CTRL_H
