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
#ifndef PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHHTTPSESSION_TASK_DELEGATE_H
#define PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHHTTPSESSION_TASK_DELEGATE_H

#import <Foundation/Foundation.h>


NS_ASSUME_NONNULL_BEGIN

@class OHHttpSessionController;

typedef NSURL * (^OHURLSessionDownloadTaskDidFinishDownloadingBlock)(NSURLSession *session,
    NSURLSessionDownloadTask *downloadTask, NSURL *location);
typedef void (^OHURLSessionTaskProgressBlock)(NSProgress *);
typedef void (^OHURLSessionTaskCompletionHandler)(NSURLResponse *response, id responseObject, NSError *error);
FOUNDATION_EXPORT const void * const AuthenticationChallengeErrorKey;


@interface OHHttpSessionTaskDelegate : NSObject <NSURLSessionTaskDelegate, NSURLSessionDownloadDelegate>

@property (nonatomic, weak) OHHttpSessionController *sessionCtrl_;
@property (nonatomic, strong) NSMutableData *mutableData;
@property (nonatomic, strong) NSProgress *uploadProgress;
@property (nonatomic, strong) NSProgress *downloadProgress;
@property (nonatomic, copy) NSURL *downloadFileURL;
@property (nonatomic, copy) OHURLSessionDownloadTaskDidFinishDownloadingBlock downloadTaskDidFinishDownloading;
@property (nonatomic, copy) OHURLSessionTaskProgressBlock uploadProgressBlock;
@property (nonatomic, copy) OHURLSessionTaskProgressBlock downloadProgressBlock;
@property (nonatomic, copy) OHURLSessionTaskCompletionHandler completionHandler;

- (instancetype)initWithTask:(NSURLSessionTask *)task;

@end

NS_ASSUME_NONNULL_END
#endif // PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHHTTPSESSION_TASK_DELEGATE_H