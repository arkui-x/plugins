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

@interface OHHttpSessionController : NSObject <NSURLSessionDelegate,
    NSURLSessionTaskDelegate, NSURLSessionDownloadDelegate>

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
    completionHandler:(nullable void (^)(NSURLResponse *response, id _Nullable responseObject,
    NSError  * _Nullable error))completionHandler;

- (NSURLSessionUploadTask *)uploadTaskWithStreamedRequest:(NSURLRequest *)request
    progress:(nullable void (^)(NSProgress *uploadProgress))uploadProgressBlock
    completionHandler:(nullable void (^)(NSURLResponse *response, id _Nullable responseObject,
    NSError * _Nullable error))completionHandler;

- (NSURLSessionDownloadTask *)downloadTaskWithRequest:(NSURLRequest *)request
    progress:(nullable void (^)(NSProgress *downloadProgress))downloadProgressBlock
    destination:(nullable NSURL * (^)(NSURL *targetPath, NSURLResponse *response))destination
    completionHandler:(nullable void (^)(NSURLResponse *response, NSURL * _Nullable filePath,
    NSError * _Nullable error))completionHandler;

- (NSURLSessionDownloadTask *)downloadTaskWithResumeData:(NSData *)resumeData
    progress:(nullable void (^)(NSProgress *downloadProgress))downloadProgressBlock
    destination:(nullable NSURL * (^)(NSURL *targetPath, NSURLResponse *response))destination
    completionHandler:(nullable void (^)(NSURLResponse *response, NSURL * _Nullable filePath,
    NSError * _Nullable error))completionHandler;

- (void)setDidFinishEventsForBackgroundURLSessionBlock:(nullable void (^)(NSURLSession *session))block;

@end
NS_ASSUME_NONNULL_END

#endif // PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHHTTPSESSION_CONTROLLER_H