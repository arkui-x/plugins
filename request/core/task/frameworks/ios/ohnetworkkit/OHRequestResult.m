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

#import "OHRequestResult.h"

NSString * const OHNetworkResponseErrorDomain = @"com.oh.network.response.error";

@interface OHRequestResult ()

@property (nonatomic, strong) NSError *error;

@property (nonatomic, strong) NSMutableData *responseData;

@end

@implementation OHRequestResult

- (void)task:(NSURLSessionTask *)task didCompleteWithError:(NSError *)error {
    //do nothing, for subclass to realize
}

- (void)task:(NSURLSessionTask *)task didReceiveData:(NSData *)data {
    if (!self.responseData) {
        self.responseData = [[NSMutableData alloc] initWithData:data];
        return;
    }
    [self.responseData appendData:data];
}

- (void)task:(NSURLSessionTask *)task didChallengeFailed:(NSError *)error {
    if (!_error) {
        _error = error;
    }
}

- (BOOL)checkStatusCode:(NSUInteger)statusCode {
    NSIndexSet *vaildStatusCode = [NSIndexSet indexSetWithIndexesInRange:NSMakeRange(200, 100)];
    if ([vaildStatusCode containsIndex:statusCode]) {
        return true;
    }
    return false;
}

- (nullable NSError *)validateResponseWithTask:(NSURLSessionTask *)task {
    
    NSError *validError = nil;
    if (task.response != nil && [task.response isKindOfClass:[NSHTTPURLResponse class]]) {
        NSHTTPURLResponse *response = (NSHTTPURLResponse *)task.response;
        if ([self checkStatusCode:response.statusCode] == false) {
            NSDictionary *userInfo = @{
                NSLocalizedDescriptionKey: [NSString stringWithFormat:@"Request failed cause by %@ (%ld)",[NSHTTPURLResponse localizedStringForStatusCode:response.statusCode], (long)response.statusCode],
                NSURLErrorFailingURLErrorKey: response.URL,
            };
            validError = [NSError errorWithDomain:OHNetworkResponseErrorDomain code:NSURLErrorBadServerResponse userInfo:userInfo];
        }
    }
    return validError;
}

@end


#pragma mark - OHDownloadRequestResult
@interface OHDownloadRequestResult ()

@property (nonatomic, strong) NSProgress *progress;
@property (nonatomic, strong) NSURL *targetFilePath;

@end

@implementation OHDownloadRequestResult

- (instancetype)init {
    self = [super init];
    if (self) {
        _progress = [NSProgress progressWithTotalUnitCount: 0];
    }
    return self;
}

- (void)updateProgressWithBytesWritten:(int64_t)bytesWritten totalBytesExpectedToWrite:(int64_t)totalBytesExpectedToWrite {
    _progress.totalUnitCount = totalBytesExpectedToWrite;
    _progress.completedUnitCount += bytesWritten;
    
    if (_downloadProgressBlock) {
        self.downloadProgressBlock(_progress);
    }
}

- (void)downloadFinished:(NSURLSessionDownloadTask *)task toPath:(NSURL *)path withError:(NSError *)error {
    //ignore file save error ...
    self.targetFilePath = path;
}

- (void)task:(NSURLSessionTask *)task didCompleteWithError:(NSError *)error {
    if (error) {
        self.error = error;
    }
    if (!self.error) {
        self.error =  [self validateResponseWithTask:task];
    }
    dispatch_queue_t queue = self.sessionManager.completionQueue ?: dispatch_get_main_queue();
    dispatch_async(queue, ^{
        self.completionBlock(task.response, self.targetFilePath, self.error);
    });
}

@end


#pragma mark - OHDownloadRequestResult
@interface OHUploadRequestResult ()

@property (nonatomic, strong) NSProgress *progress;

@end

@implementation OHUploadRequestResult

- (instancetype)init {
    self = [super init];
    if (self) {
        _progress = [NSProgress progressWithTotalUnitCount: 0];
    }
    return self;
}

- (void)upadteProgressWithTotalBytesSent:(int64_t)totalBytesSent totalBytesExpectedToSend:(int64_t)totalBytesExpectedToSend {
    _progress.totalUnitCount = totalBytesExpectedToSend;
    _progress.completedUnitCount = totalBytesSent;
    
    if (_uploadProgressBlock) {
        _uploadProgressBlock(_progress);
    }
}

- (void)task:(NSURLSessionTask *)task didCompleteWithError:(NSError *)error {
    if (error) {
        self.error = error;
    }
    if (!self.error) {
        self.error =  [self validateResponseWithTask:task];
    }
    dispatch_queue_t queue = self.sessionManager.completionQueue ?: dispatch_get_main_queue();
    dispatch_async(queue, ^{
        self.completionBlock(task.response, self.responseData, self.error);
    });
}

@end
