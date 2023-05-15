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
#ifndef PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHHTTPSESSION_CONTROLLER_H
#define PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHHTTPSESSION_CONTROLLER_H

#import <Foundation/Foundation.h>
#import <SystemConfiguration/SystemConfiguration.h>
#import <TargetConditionals.h>
#import "OHHttpRequestHandler.h"
#import "OHSslHandler.h"


#if __has_warning("-Wunguarded-availability-new")
    #define OH_CAN_USE_AT_AVAILABLE 1
#else
    #define OH_CAN_USE_AT_AVAILABLE 0
#endif


NS_ASSUME_NONNULL_BEGIN

@interface OHHttpSessionController : NSObject <NSURLSessionDelegate, NSURLSessionTaskDelegate, NSURLSessionDownloadDelegate>

@property (readonly, nonatomic, strong) NSURLSession *session;
@property (readonly, nonatomic, strong) NSOperationQueue *operationQueue;
@property (nonatomic, strong) OHSslHandler *sslHandler;
@property (readonly, nonatomic, strong, nullable) NSURL *baseURL;
@property (nonatomic, strong) OHHttpRequestHandler *requestHandler;

- (instancetype)initWithBaseURL:(nullable NSURL *)url
           sessionConfiguration:(nullable NSURLSessionConfiguration *)configuration NS_DESIGNATED_INITIALIZER;

- (NSURLSessionUploadTask *)uploadTaskWithRequest:(NSURLRequest *)request
                                         fromFile:(NSURL *)fileURL
                                         progress:(nullable void (^)(NSProgress *uploadProgress))uploadProgressBlock
                                completionHandler:(nullable void (^)(NSURLResponse *response, id _Nullable responseObject, NSError  * _Nullable error))completionHandler;

- (NSURLSessionUploadTask *)uploadTaskWithStreamedRequest:(NSURLRequest *)request
                                                 progress:(nullable void (^)(NSProgress *uploadProgress))uploadProgressBlock
                                        completionHandler:(nullable void (^)(NSURLResponse *response, id _Nullable responseObject, NSError * _Nullable error))completionHandler;

- (NSURLSessionDownloadTask *)downloadTaskWithRequest:(NSURLRequest *)request
                                             progress:(nullable void (^)(NSProgress *downloadProgress))downloadProgressBlock
                                          destination:(nullable NSURL * (^)(NSURL *targetPath, NSURLResponse *response))destination
                                    completionHandler:(nullable void (^)(NSURLResponse *response, NSURL * _Nullable filePath, NSError * _Nullable error))completionHandler;

- (NSURLSessionDownloadTask *)downloadTaskWithResumeData:(NSData *)resumeData
                                                progress:(nullable void (^)(NSProgress *downloadProgress))downloadProgressBlock
                                             destination:(nullable NSURL * (^)(NSURL *targetPath, NSURLResponse *response))destination
                                       completionHandler:(nullable void (^)(NSURLResponse *response, NSURL * _Nullable filePath, NSError * _Nullable error))completionHandler;

- (void)setDidFinishEventsForBackgroundURLSessionBlock:(nullable void (^)(NSURLSession *session))block;

@end
NS_ASSUME_NONNULL_END

#endif // PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHHTTPSESSION_CONTROLLER_H