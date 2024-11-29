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

#ifndef HTTP_IOS_REQUEST_H
#define HTTP_IOS_REQUEST_H
#import <Foundation/Foundation.h>
NS_ASSUME_NONNULL_BEGIN

typedef void (^HTTPRequestSuccessBlock)(NSURLSessionTask* _Nonnull task,
                                        NSHTTPURLResponse* _Nullable response,
                                        NSData* _Nullable data, void* userData);
typedef void (^HTTPRequestFailureBlock)(NSInteger errorCode, NSString* errorMessage, void* userData);
typedef void (^HTTPRequestMemoryBodyBlock)(NSData* data, void* userData);
typedef void (^HTTPRequestMemoryHeaderBlock)(NSDictionary* headers, void* userData);
typedef void (^HTTPRequestProgressBlock)(void* userData, long total, long now);
typedef NSURLRequest* _Nullable (^HTTPRequestWillRedirectionBlock)(NSURLSession* session,
                                                                NSURLSessionTask* task,
                                                                NSURLResponse* response,
                                                                NSURLRequest* request);

@interface http_ios_param : NSObject
@property (nonatomic, copy) NSString* urlPath;
@property (nonatomic, copy) NSString* method;
@property (nonatomic, strong) id bodyParam;
@property (nonatomic, assign) NSInteger returnDataType;
@property (nonatomic, strong) NSDictionary* headerJson;
@property (nonatomic, assign) NSTimeInterval readTimeout;
@property (nonatomic, assign) NSTimeInterval connectTimeout;
@property (nonatomic, assign) NSInteger httpVersion;
@property (nonatomic, assign) NSInteger priority;
@property (nonatomic, assign) NSUInteger usingHttpProxyType;
@property (nonatomic, copy) NSString* proxyhost;
@property (nonatomic, assign) NSInteger proxyport;
@property (nonatomic, copy) NSString* exclusionList;
@property (nonatomic) void* context;
@property (nonatomic, copy) NSString* ca;
@property (nonatomic, copy) NSString* caType;
@property (nonatomic, copy) NSString* key;
@property (nonatomic, copy) NSString* password;
@end

@interface http_ios_request : NSObject
@property (nonatomic, copy) HTTPRequestMemoryHeaderBlock memoryHeaderBlock;
@property (nonatomic, copy) HTTPRequestMemoryBodyBlock memoryBodyBlock;
@property (nonatomic, copy) HTTPRequestProgressBlock uploadProgress;
@property (nonatomic, copy) HTTPRequestProgressBlock downloadProgress;
@property (nonatomic, copy) HTTPRequestSuccessBlock responseBlock;
@property (nonatomic, copy) HTTPRequestFailureBlock failBlock;
@property (nonatomic, copy) HTTPRequestWillRedirectionBlock redirectionBlock;

+ (NSString*)getBaseCachePath;
- (NSURLSessionDataTask*)sendRequestWith:(nullable http_ios_param*)requestParam;
- (void)deInitialize;
@end

NS_ASSUME_NONNULL_END
#endif