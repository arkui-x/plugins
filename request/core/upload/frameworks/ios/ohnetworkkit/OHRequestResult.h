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

#ifndef PLUGINS_REQUEST_IOS_OHREQUESTRESULT_H
#define PLUGINS_REQUEST_IOS_OHREQUESTRESULT_H

#import <Foundation/Foundation.h>
#import "OHSessionManager.h"

NS_ASSUME_NONNULL_BEGIN


@interface OHRequestResult : NSObject

@property (nonatomic, copy) OHTaskCompletionBlock completionBlock;

@property (nonatomic, weak) OHSessionManager *sessionManager;

- (void)task:(NSURLSessionTask *)task didCompleteWithError:(NSError *)error;

- (void)task:(NSURLSessionTask *)task didReceiveData:(NSData *)data;

- (void)task:(NSURLSessionTask *)task didChallengeFailed:(NSError *)error;

@end


#pragma mark - Download
@interface OHDownloadRequestResult : OHRequestResult

@property (nonatomic, copy) OHDownloadDestination destination;
@property (nonatomic, copy) OHTaskProgressBlock downloadProgressBlock;

- (instancetype)init;

- (void)downloadFinished:(NSURLSessionDownloadTask *)task toPath:(nullable NSURL *)path withError:(nullable NSError *)error;

- (void)updateProgressWithBytesWritten:(int64_t)bytesWritten totalBytesExpectedToWrite:(int64_t)totalBytesExpectedToWrite;

@end


#pragma mark - Upload
@interface OHUploadRequestResult : OHRequestResult

@property (nonatomic, copy) OHTaskProgressBlock uploadProgressBlock;

- (instancetype)init;

- (void)upadteProgressWithTotalBytesSent:(int64_t)totalBytesSent totalBytesExpectedToSend:(int64_t)totalBytesExpectedToSend;

@end

NS_ASSUME_NONNULL_END
#endif // PLUGINS_REQUEST_IOS_OHREQUESTRESULT_H
