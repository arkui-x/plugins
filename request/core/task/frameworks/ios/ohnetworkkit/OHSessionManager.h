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

#ifndef PLUGINS_REQUEST_IOS_OHSESSIONMANAGER_H
#define PLUGINS_REQUEST_IOS_OHSESSIONMANAGER_H

#import <Foundation/Foundation.h>
#import "OHSslHandler.h"

NS_ASSUME_NONNULL_BEGIN

//common completion block
typedef void (^OHTaskCompletionBlock)(NSURLResponse * _Nullable response, id _Nullable object,  NSError * _Nullable error);
typedef void (^OHTaskProgressBlock)(NSProgress *progess);
typedef NSURL * _Nullable (^OHDownloadDestination)(NSURLResponse *response, NSURL *temporaryURL);
typedef void (^OHDownloadCompletionBlock)(NSURLResponse * _Nullable response, NSURL * _Nullable filepath, NSError * _Nullable error);


@interface OHSessionManager : NSObject

@property (nonatomic, nullable) dispatch_queue_t completionQueue;

@property (nonatomic, strong) OHSslHandler *sslHandler;

+ (instancetype)sharedInstance;

- (instancetype)initWithConfiguration: (nullable NSURLSessionConfiguration *)configuration;


- (NSURLSessionDownloadTask *)downloadWithRequest:(NSURLRequest *)request
                                    progressBlock:(OHTaskProgressBlock)progressBlock
                                      destination:(OHDownloadDestination)destination
                                       completion:(OHDownloadCompletionBlock)completionHandler;

- (NSURLSessionDownloadTask *)downloadTaskWithResumeData:(NSData *)resumeData
                                           progressBlock:(OHTaskProgressBlock)progressBlock
                                             destination:(OHDownloadDestination)destination
                                              completion:(OHDownloadCompletionBlock)completionHandler;


- (NSURLSessionUploadTask *)uploadWithRequest:(NSURLRequest *)request
                                     fromFile:(NSURL *)uploadFileURL
                                progressBlock:(OHTaskProgressBlock)progressBlock
                                   completion:(OHTaskCompletionBlock)completionHandler;

- (NSURLSessionUploadTask *)uploadWithRequest:(NSURLRequest *)request
                                     fromData:(NSData *)data
                                progressBlock:(OHTaskProgressBlock)progressBlock
                                   completion:(OHTaskCompletionBlock)completionHandler;

- (NSURLSessionUploadTask *)uploadWithStreamRequest:(NSURLRequest *)request
                                      progressBlock:(OHTaskProgressBlock)progressBlock
                                         completion:(OHTaskCompletionBlock)completionHandler;

@end

NS_ASSUME_NONNULL_END
#endif // PLUGINS_REQUEST_IOS_OHSESSIONMANAGER_H
