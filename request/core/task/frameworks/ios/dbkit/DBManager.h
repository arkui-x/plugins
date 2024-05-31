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

#ifndef PLUGINS_REQUEST_IOS_DBMANAGER_H
#define PLUGINS_REQUEST_IOS_DBMANAGER_H

#import <Foundation/Foundation.h>
#import "IosTaskInfo.h"
#import "IosTaskFilter.h"

NS_ASSUME_NONNULL_BEGIN

@interface DBManager : NSObject
+ (DBManager *)shareManager;

- (BOOL)initDB;
- (int64_t)insert:(IosTaskInfo *)taskInfo;
- (NSArray *)queryAll;
- (IosTaskInfo *)queryWithTaskId:(int64_t)taskId;
- (IosTaskInfo *)queryWithToken:(NSString *)token taskId:(int64_t)taskId;
- (NSArray *)queryWithFilter:(IosTaskFilter *)filter;
- (BOOL)update:(IosTaskInfo *)taskInfo;
- (BOOL)remove:(int64_t)taskId;

@end
NS_ASSUME_NONNULL_END

#endif // PLUGINS_REQUEST_IOS_DBMANAGER_H
