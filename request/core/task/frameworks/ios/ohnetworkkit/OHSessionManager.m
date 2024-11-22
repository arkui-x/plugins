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

#import "OHSessionManager.h"
#import "OHRequestResult.h"

#define OHNetworkLabel "com.oh.network.session"
#define OHConfigIdentifier "com.oh.config"
#define OHSessionLock "com.oh.seesion.lock"
#define OHQueueLabel "com.oh.network.queue"
#define OHOperationName "com.oh.network.operationQueue"

NSString * const OHNetworkErrorDomain = @"com.oh.network.error";

@interface OHSessionManager () <NSURLSessionDelegate, NSURLSessionDataDelegate, NSURLSessionDownloadDelegate>

@property (nonatomic, strong) NSURLSession *session;

@property (nonatomic, strong) NSLock *lock;

/// use taskIdentifier of NSURLSessionTask as key, save target Result as value
/// when SessionTask complete, remove bind result
@property (nonatomic, strong) NSMutableDictionary *taskResultMap;


@end

@implementation OHSessionManager

+ (instancetype)sharedInstance {
    static OHSessionManager *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[self alloc] initWithConfiguration:nil];
    });
    return sharedInstance;
}

- (instancetype)initWithConfiguration:(NSURLSessionConfiguration *)configuration {
    self = [super init];
    if (self) {
        
        if (!configuration) {
            configuration = [NSURLSessionConfiguration defaultSessionConfiguration];
        }
        
        dispatch_queue_t underlyingQueue = dispatch_queue_create(OHQueueLabel, DISPATCH_QUEUE_SERIAL);
        
        NSOperationQueue *queue =  [[NSOperationQueue alloc] init];
        queue.qualityOfService = NSQualityOfServiceDefault;
        queue.maxConcurrentOperationCount = 1;
        queue.underlyingQueue = underlyingQueue;
        queue.name = @OHOperationName;
        
        _lock = [[NSLock alloc] init];
        _lock.name = @OHSessionLock;
        
        _sslHandler = [OHSslHandler defaultHandler];
        
        _taskResultMap = [[NSMutableDictionary alloc] init];
        
        _session = [NSURLSession sessionWithConfiguration:configuration
                                                 delegate:self
                                            delegateQueue:queue];
        
    }
    return self;
}


#pragma mark -
- (OHRequestResult *)resultForTask:(NSURLSessionTask *)task {
    OHRequestResult *result = nil;
    [self.lock lock];
    result = [self.taskResultMap valueForKey:@(task.taskIdentifier).stringValue];
    [self.lock unlock];
    return result;
}

- (void)bindResultMapWithTask:(NSURLSessionTask *)task andResult:(OHRequestResult *)result {
    [self.lock lock];
    [self.taskResultMap setValue:result forKey:@(task.taskIdentifier).stringValue];
    [self.lock unlock];
}

- (void)removeResultMapForTask:(NSURLSessionTask *)task {
    [self.lock lock];
    [self.taskResultMap removeObjectForKey:@(task.taskIdentifier).stringValue];
    [self.lock unlock];
}

- (void)bindResultMapForDwonloadTask:(NSURLSessionDownloadTask *)task
                       progressBlock:(OHTaskProgressBlock)progressBlock
                         destination:(OHDownloadDestination)destination
                          completion:(OHDownloadCompletionBlock)completionHandler {
    OHDownloadRequestResult *result = [[OHDownloadRequestResult alloc] init];
    result.downloadProgressBlock = progressBlock;
    result.destination = destination;
    result.completionBlock = completionHandler;
    result.sessionManager = self;
    
    [self bindResultMapWithTask:task andResult:result];
}

- (void)bindResultMapForUploadTask:(NSURLSessionUploadTask *)task
                     progressBlock:(OHTaskProgressBlock)progressBlock
                        completion:(OHTaskCompletionBlock)completionHandler {
    OHUploadRequestResult *result = [[OHUploadRequestResult alloc] init];
    result.uploadProgressBlock = progressBlock;
    result.completionBlock = completionHandler;
    result.sessionManager = self;
    
    [self bindResultMapWithTask:task andResult:result];
}

#pragma mark - create task funcs

- (NSURLSessionDownloadTask *)downloadWithRequest:(NSURLRequest *)request
                                    progressBlock:(OHTaskProgressBlock)progressBlock
                                      destination:(OHDownloadDestination)destination
                                       completion:(OHDownloadCompletionBlock)completionHandler {
    
    NSURLSessionDownloadTask *task = [self.session downloadTaskWithRequest:request];
    
    [self bindResultMapForDwonloadTask:task progressBlock:progressBlock destination:destination completion:completionHandler];
    
    return task;
}

- (NSURLSessionDownloadTask *)downloadTaskWithResumeData:(NSData *)resumeData
                                           progressBlock:(OHTaskProgressBlock)progressBlock
                                             destination:(OHDownloadDestination)destination
                                              completion:(OHDownloadCompletionBlock)completionHandler {
    
    NSURLSessionDownloadTask *task = [self.session downloadTaskWithResumeData:resumeData];
    
    [self bindResultMapForDwonloadTask:task progressBlock:progressBlock destination:destination completion:completionHandler];
    
    return task;
}

- (NSURLSessionUploadTask *)uploadWithRequest:(NSURLRequest *)request
                                     fromFile:(NSURL *)uploadFileURL
                                progressBlock:(OHTaskProgressBlock)progressBlock
                                   completion:(OHTaskCompletionBlock)completionHandler {
    
    NSURLSessionUploadTask *task = [self.session uploadTaskWithRequest:request fromFile:uploadFileURL];
    
    [self bindResultMapForUploadTask:task progressBlock:progressBlock completion:completionHandler];
    
    return task;
}

- (NSURLSessionUploadTask *)uploadWithRequest:(NSURLRequest *)request fromData:(NSData *)data progressBlock:(OHTaskProgressBlock)progressBlock completion:(OHTaskCompletionBlock)completionHandler {
    
    NSURLSessionUploadTask *task = [self.session uploadTaskWithRequest:request fromData:data];
    
    [self bindResultMapForUploadTask:task progressBlock:progressBlock completion:completionHandler];
    
    return task;
}

- (NSURLSessionUploadTask *)uploadWithStreamRequest:(NSURLRequest *)request progressBlock:(OHTaskProgressBlock)progressBlock completion:(OHTaskCompletionBlock)completionHandler {
    
    NSURLSessionUploadTask *task = [self.session uploadTaskWithStreamedRequest:request];
    
    [self bindResultMapForUploadTask:task progressBlock:progressBlock completion:completionHandler];
    
    return task;
}

- (NSError *)generateServerTrustErrorForServerTrust:(nullable SecTrustRef)serverTrust url:(nullable NSURL *)url {
    NSBundle *networkBundle = [NSBundle bundleWithIdentifier:@"com.apple.CFNetwork"];
    NSString *defaultValue = @"The certificate for this server is invalid. You might be connecting to a server \
        that is pretending to be “%@” which could put your confidential information at risk.";
    NSString *descriptionFormat = NSLocalizedStringWithDefaultValue(@"Err-1202.w", nil, networkBundle, defaultValue, @"") ?: defaultValue;
    NSString *localizedDescription = [descriptionFormat componentsSeparatedByString:@"%@"].count <= 2 ? \
        [NSString localizedStringWithFormat:descriptionFormat, url.host] : descriptionFormat;

    NSMutableDictionary *userInfo = [[NSMutableDictionary alloc] init];
    [userInfo setValue:localizedDescription forKey:NSLocalizedDescriptionKey];
    
    if (serverTrust) {
        [userInfo setValue:(__bridge id)serverTrust forKey:NSURLErrorFailingURLPeerTrustErrorKey];
    }
    if (url) {
        [userInfo setValue:url forKey:NSURLErrorFailingURLErrorKey];
        if (url.absoluteString) {
            [userInfo setValue:url.absoluteString forKey:NSURLErrorFailingURLStringErrorKey];
        }
    }
    return [NSError errorWithDomain:NSURLErrorDomain code:NSURLErrorServerCertificateUntrusted userInfo:userInfo];
}


#pragma mark - NSURLSessionDelegate


#pragma mark - NSURLSessionTaskDelegate

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task didReceiveChallenge:(NSURLAuthenticationChallenge *)challenge completionHandler:(void (^)(NSURLSessionAuthChallengeDisposition, NSURLCredential * _Nullable))completionHandler {
    
    BOOL isEvaluateServerTrust = false;
    NSURLSessionAuthChallengeDisposition disposition = NSURLSessionAuthChallengePerformDefaultHandling;
    NSURLCredential *credential = nil;
    isEvaluateServerTrust = [challenge.protectionSpace.authenticationMethod
                             isEqualToString:NSURLAuthenticationMethodServerTrust];
    if (isEvaluateServerTrust) {
        BOOL sslHandlerResult = [self.sslHandler evaluateServerTrust:challenge.protectionSpace.serverTrust
                                                           forDomain:challenge.protectionSpace.host];
        if (sslHandlerResult) {
            disposition = NSURLSessionAuthChallengeUseCredential;
            credential = [NSURLCredential credentialForTrust:challenge.protectionSpace.serverTrust];
        } else {
            OHRequestResult *result = [self resultForTask:task];
            NSError *error = [self generateServerTrustErrorForServerTrust:challenge.protectionSpace.serverTrust
                                                                      url:task.currentRequest.URL];
            [result task:task didChallengeFailed:error];
            disposition = NSURLSessionAuthChallengeCancelAuthenticationChallenge;
        }
    }
    if (completionHandler) {
        completionHandler(disposition, credential);
    }
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task didCompleteWithError:(NSError *)error {
    OHRequestResult *result = [self resultForTask:task];
    if (result) {
        [self removeResultMapForTask:task];
    }
    [result task:task didCompleteWithError:error];
    
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task didSendBodyData:(int64_t)bytesSent totalBytesSent:(int64_t)totalBytesSent totalBytesExpectedToSend:(int64_t)totalBytesExpectedToSend {
    
    OHRequestResult *result = [self resultForTask:task];
    if (result && [result isKindOfClass:[OHUploadRequestResult class]]) {
        [(OHUploadRequestResult *)result upadteProgressWithTotalBytesSent:totalBytesSent totalBytesExpectedToSend:totalBytesExpectedToSend];
    }
}

#pragma mark - NSURLSessionDataDelegate
- (void)URLSession:(NSURLSession *)session dataTask:(NSURLSessionDataTask *)dataTask didReceiveData:(NSData *)data {
    OHRequestResult *result = [self resultForTask:dataTask];
    [result task:dataTask didReceiveData:data];
}


#pragma mark - NSURLSessionDownloadDelegate
- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask didFinishDownloadingToURL:(NSURL *)location {
    
    OHRequestResult *result = [self resultForTask:downloadTask];
    if (result && [result isKindOfClass:[OHDownloadRequestResult class]]) {
        OHDownloadRequestResult *downloadResult = (OHDownloadRequestResult *)result;
        
        NSURL *filePath = downloadResult.destination(downloadTask.response, location);

        //the path for move after download is nil, stop move
        if (!filePath) {
            NSDictionary *userInfo = @{
                NSLocalizedDescriptionKey: @"No destination url for download task",
            };
            NSError *error = [NSError errorWithDomain:OHNetworkErrorDomain code:NSURLErrorCannotWriteToFile userInfo:userInfo];
            [downloadResult downloadFinished:downloadTask toPath:filePath withError:error];
            return;
        }
        
        NSError *error;
        
        //just remove old
        if ([[NSFileManager defaultManager] fileExistsAtPath:filePath.path]) {
            [[NSFileManager defaultManager] removeItemAtURL:filePath error:&error];
        }
        
        [[NSFileManager defaultManager] moveItemAtURL:location toURL:filePath error:&error];
        [downloadResult downloadFinished:downloadTask toPath:filePath withError:error];
    }
}

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask didWriteData:(int64_t)bytesWritten totalBytesWritten:(int64_t)totalBytesWritten totalBytesExpectedToWrite:(int64_t)totalBytesExpectedToWrite {
    
    OHRequestResult *result = [self resultForTask:downloadTask];
    if (result && [result isKindOfClass:[OHDownloadRequestResult class]]) {
        [(OHDownloadRequestResult *)result updateProgressWithBytesWritten:bytesWritten totalBytesExpectedToWrite:totalBytesExpectedToWrite];
    }
}

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask didResumeAtOffset:(int64_t)fileOffset expectedTotalBytes:(int64_t)expectedTotalBytes {
    
    OHRequestResult *result = [self resultForTask:downloadTask];
    if (result && [result isKindOfClass:[OHDownloadRequestResult class]]) {
        [(OHDownloadRequestResult *)result updateProgressWithBytesWritten:fileOffset totalBytesExpectedToWrite:expectedTotalBytes];
    }
}

@end
