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
#ifndef PLUGINS_REQUEST_IOS_TASKFILTER_H
#define PLUGINS_REQUEST_IOS_TASKFILTER_H

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface IosTaskFilter : NSObject
@property (nonatomic, assign) int64_t before;
@property (nonatomic, assign) int64_t after;
@property (nonatomic, assign) int state;
@property (nonatomic, assign) int action;
@property (nonatomic, assign) int mode;
@end

@interface IosTaskProgress : NSObject
@property (nonatomic, assign) int state;
@property (nonatomic, assign) int index;
@property (nonatomic, assign) int64_t processed;
@property (nonatomic, assign) int64_t totalProcessed;
@property (nullable, nonatomic, strong) NSArray *sizes;
@property (nullable, nonatomic, strong) NSDictionary *extras;
@property (nullable, nonatomic, strong) NSArray *bodyBytes;

- (nullable NSString *)toJsonString;
@end

@interface IosTaskState : NSObject
@property (nullable, nonatomic, strong) NSString *path;
@property (nonatomic, assign) int responseCode;
@property (nullable, nonatomic, strong) NSString *message;

- (nullable NSString *)toJsonString;
@end

NS_ASSUME_NONNULL_END

#endif // PLUGINS_REQUEST_IOS_TASKFILTER_H
