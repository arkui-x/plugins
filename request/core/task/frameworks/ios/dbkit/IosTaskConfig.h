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
#ifndef PLUGINS_REQUEST_IOS_TASKCONFIG_H
#define PLUGINS_REQUEST_IOS_TASKCONFIG_H

#import <Foundation/Foundation.h>
#import "IosTaskInfo.h"

NS_ASSUME_NONNULL_BEGIN

@interface IosTaskConfig : NSObject
@property (nonatomic, assign) int action;
@property (nullable, nonatomic, copy) NSString *url;
@property (nonatomic, assign) int version;
@property (nonatomic, assign) int mode;
@property (nonatomic, assign) int network;
@property (nonatomic, assign) int index;
@property (nonatomic, assign) int begins;
@property (nonatomic, assign) int ends;
@property (nonatomic, assign) bool overwrite;
@property (nonatomic, assign) bool metered;
@property (nonatomic, assign) bool roaming;
@property (nonatomic, assign) bool retry;
@property (nonatomic, assign) bool redirect;
@property (nonatomic, assign) bool gauge;
@property (nonatomic, assign) bool precise;
@property (nonatomic, assign) bool background;
@property (nullable, nonatomic, copy) NSString *title;
@property (nullable, nonatomic, copy) NSString *saveas;
@property (nullable, nonatomic, copy) NSString *proxy;
@property (nullable, nonatomic, copy) NSString *realPath;
@property (nullable, nonatomic, copy) NSString *method;
@property (nullable, nonatomic, copy) NSString *token;
@property (nullable, nonatomic, copy) NSString *desc;
@property (nullable, nonatomic, copy) NSString *data;
@property (nullable, nonatomic, strong) NSDictionary *headers;
@property (nullable, nonatomic, strong) NSArray *forms;
@property (nullable, nonatomic, strong) NSArray *files;
@property (nullable, nonatomic, strong) NSArray *bodyFds;
@property (nullable, nonatomic, strong) NSArray *bodyFileNames;
@property (nullable, nonatomic, strong) NSDictionary *extras;

+ (_Nullable instancetype)initWithJsonString:(NSString *)jsonString;

- (IosTaskInfo *)getTaskInfo;

@end

NS_ASSUME_NONNULL_END

#endif // PLUGINS_REQUEST_IOS_TASKCONFIG_H

