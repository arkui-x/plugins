// Copyright (c) 2011–2016 Alamofire Software Foundation ( http://alamofire.org/ )
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

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