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

#import "OHHttpSessionTaskDelegate.h"
#import "OHHttpSessionController.h"
#import <objc/runtime.h>


const void * const AuthenticationChallengeErrorKey = &AuthenticationChallengeErrorKey;

static dispatch_queue_t url_session_ctrl_processing_queue() {
    static dispatch_queue_t oh_session_ctrl_processing_queue;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        oh_session_ctrl_processing_queue = dispatch_queue_create("com.oh.networkkit.session.processing", DISPATCH_QUEUE_CONCURRENT);
    });
    return oh_session_ctrl_processing_queue;
}

static dispatch_group_t url_session_completion_group() {
    static dispatch_group_t oh_session_ctrl_completion_group;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        oh_session_ctrl_completion_group = dispatch_group_create();
    });
    return oh_session_ctrl_completion_group;
}


@implementation OHHttpSessionTaskDelegate

- (instancetype)initWithTask:(NSURLSessionTask *)task {
    self = [super init];
    if (!self) {
        return nil;
    }
    _mutableData = [NSMutableData data];
    _uploadProgress = [[NSProgress alloc] initWithParent:nil userInfo:nil];
    _downloadProgress = [[NSProgress alloc] initWithParent:nil userInfo:nil];
    __weak __typeof__(task) weakTask = task;
    for (NSProgress *progress in @[ _uploadProgress, _downloadProgress ]) {
        progress.totalUnitCount = NSURLSessionTransferSizeUnknown;
        progress.cancellable = YES;
        progress.cancellationHandler = ^{
            [weakTask cancel];
        };
        progress.pausable = YES;
        progress.pausingHandler = ^{
            [weakTask suspend];
        };
#if OH_CAN_USE_AT_AVAILABLE
        if (@available(macOS 10.11, *))
#else
        if ([progress respondsToSelector:@selector(setResumingHandler:)])
#endif
        {
            progress.resumingHandler = ^{
                [weakTask resume];
            };
        }
        [progress addObserver:self
                   forKeyPath:NSStringFromSelector(@selector(fractionCompleted))
                      options:NSKeyValueObservingOptionNew
                      context:NULL];
    }
    return self;
}

- (void)dealloc {
    [self.downloadProgress removeObserver:self forKeyPath:NSStringFromSelector(@selector(fractionCompleted))];
    [self.uploadProgress removeObserver:self forKeyPath:NSStringFromSelector(@selector(fractionCompleted))];
}

#pragma mark - NSProgress Tracking
- (void)observeValueForKeyPath:(NSString *)keyPath ofObject:(id)object change:(NSDictionary<NSString *,id> *)change context:(void *)context {
   if ([object isEqual:self.downloadProgress]) {
        if (self.downloadProgressBlock) {
            self.downloadProgressBlock(object);
        }
    }
    else if ([object isEqual:self.uploadProgress]) {
        if (self.uploadProgressBlock) {
            self.uploadProgressBlock(object);
        }
    }
}

#pragma mark - NSURLSessionTaskDelegate
- (void)URLSession:(__unused NSURLSession *)session
              task:(NSURLSessionTask *)task
didCompleteWithError:(NSError *)error
{
    error = objc_getAssociatedObject(task, AuthenticationChallengeErrorKey) ?: error;
    __strong OHHttpSessionController *sessionCtrl_ = self.sessionCtrl_;
    __block id responseObject = nil;
    NSData *data = nil;
    if (self.mutableData) {
        data = [self.mutableData copy];
        self.mutableData = nil;
    }

    if (error) {
        dispatch_group_async(url_session_completion_group(), dispatch_get_main_queue(), ^{
            if (self.completionHandler) {
                self.completionHandler(task.response, responseObject, error);
            }
        });
    } else {
        dispatch_async(url_session_ctrl_processing_queue(), ^{
            if (self.downloadFileURL) {
                responseObject = self.downloadFileURL;
            }
            dispatch_group_async(url_session_completion_group(), dispatch_get_main_queue(), ^{
                if (self.completionHandler) {
                    self.completionHandler(task.response, responseObject, error);
                }
            });
        });
    }
}

- (void)URLSession:(NSURLSession *)session task:(NSURLSessionTask *)task
   didSendBodyData:(int64_t)bytesSent
    totalBytesSent:(int64_t)totalBytesSent
totalBytesExpectedToSend:(int64_t)totalBytesExpectedToSend {
    self.uploadProgress.totalUnitCount = task.countOfBytesExpectedToSend;
    self.uploadProgress.completedUnitCount = task.countOfBytesSent;
}

#pragma mark - NSURLSessionDownloadDelegate
- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask
      didWriteData:(int64_t)bytesWritten
 totalBytesWritten:(int64_t)totalBytesWritten
totalBytesExpectedToWrite:(int64_t)totalBytesExpectedToWrite {
    self.downloadProgress.totalUnitCount = totalBytesExpectedToWrite;
    self.downloadProgress.completedUnitCount = totalBytesWritten;
}

- (void)URLSession:(NSURLSession *)session downloadTask:(NSURLSessionDownloadTask *)downloadTask
 didResumeAtOffset:(int64_t)fileOffset
expectedTotalBytes:(int64_t)expectedTotalBytes {
    self.downloadProgress.totalUnitCount = expectedTotalBytes;
    self.downloadProgress.completedUnitCount = fileOffset;
}

- (void)URLSession:(NSURLSession *)session
      downloadTask:(NSURLSessionDownloadTask *)downloadTask
didFinishDownloadingToURL:(NSURL *)location {
    self.downloadFileURL = nil;
    if (self.downloadTaskDidFinishDownloading) {
        self.downloadFileURL = self.downloadTaskDidFinishDownloading(session, downloadTask, location);
        if (self.downloadFileURL) {
            NSError *fileManagerError = nil;
            [[NSFileManager defaultManager] moveItemAtURL:location toURL:self.downloadFileURL error:&fileManagerError];
        }
    }
}

@end