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
@property (nullable, nonatomic, copy) NSString *proxy;
@property (nullable, nonatomic, copy) NSString *url;
@property (nullable, nonatomic, copy) NSString *data;
@property (nullable, nonatomic, copy) NSString *title;
@property (nullable, nonatomic, copy) NSString *desc;
@property (nonatomic, assign) int action;
@property (nonatomic, assign) int mode;
@property (nullable, nonatomic, copy) NSString *mimeType;
@property (nonatomic, assign) int state;
@property (nullable, nonatomic, copy) NSString *progress;
@property (nullable, nonatomic, copy) NSString *response;
@property (nonatomic, assign) int64_t ctime;
@property (nonatomic, assign) int64_t mtime;
@property (nonatomic, assign) int faults;
@property (nullable, nonatomic, copy) NSString *reason;
@property (nullable, nonatomic, copy) NSString *taskStates;
@property (nonatomic, assign) int downloadId;
@property (nullable, nonatomic, copy) NSString *token;
@property (nonatomic, assign) bool roaming;
@property (nonatomic, assign) bool metered;
@property (nonatomic, assign) int network;
@property (nullable, nonatomic, copy) NSString *headers;
@property (nonatomic, assign) int version;
@property (nonatomic, assign) int index;
@property (nonatomic, assign) int64_t begins;
@property (nonatomic, assign) int64_t ends;
@property (nonatomic, assign) bool overwrite;
@property (nonatomic, assign) bool retry;
@property (nonatomic, assign) bool redirect;
@property (nonatomic, assign) bool gauge;
@property (nonatomic, assign) bool precise;
@property (nonatomic, assign) bool background;
@property (nullable, nonatomic, copy) NSString *method;
@property (nullable, nonatomic, copy) NSString *forms;
@property (nullable, nonatomic, copy) NSString *files;
@property (nullable, nonatomic, copy) NSString *bodyFds;
@property (nullable, nonatomic, copy) NSString *bodyFileNames;
@property (nonatomic, assign) int tries;
@property (nonatomic, assign) int code;
@property (nonatomic, assign) bool withSystem;
@property (nullable, nonatomic, copy) NSString *extras;
@end

NS_ASSUME_NONNULL_END

#endif // PLUGINS_REQUEST_IOS_TASKINFO_H
