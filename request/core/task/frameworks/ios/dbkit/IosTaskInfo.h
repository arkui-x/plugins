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
#ifndef PLUGINS_REQUEST_IOS_TASKINFO_H
#define PLUGINS_REQUEST_IOS_TASKINFO_H

#import <Foundation/Foundation.h>
#import "Constants.h"
#import "IosTaskFilter.h"

NS_ASSUME_NONNULL_BEGIN

@interface IosTaskInfo : NSObject
@property (nonatomic, assign) int64_t tid;
@property (nullable, nonatomic, copy) NSString *saveas;
@property (nullable, nonatomic, copy) NSString *url;
@property (nullable, nonatomic, copy) NSString *data;
@property (nullable, nonatomic, copy) NSString *title;
@property (nullable, nonatomic, copy) NSString *desc;
@property (nonatomic, assign) int action;
@property (nonatomic, assign) int mode;
@property (nullable, nonatomic, copy) NSString *mimeType;
@property (nonatomic, assign) int state;
@property (nullable, nonatomic, copy) NSString *progress;
@property (nonatomic) int64_t ctime;
@property (nonatomic) int64_t mtime;
@property (nonatomic) int faults;
@property (nullable, nonatomic, copy) NSString *reason;
@property (nullable, nonatomic, copy) NSString *taskStates;
@property (nullable, nonatomic, copy) NSString *token;
@end

NS_ASSUME_NONNULL_END

#endif // PLUGINS_REQUEST_IOS_TASKINFO_H
