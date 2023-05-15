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
#ifndef PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHHTTPREQUEST_HANDLER_H
#define PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHHTTPREQUEST_HANDLER_H

#import <Foundation/Foundation.h>
#import <TargetConditionals.h>
#import <UIKit/UIKit.h>


NS_ASSUME_NONNULL_BEGIN

FOUNDATION_EXPORT NSString* OHPercentEscapedStringFromString(NSString *string);
FOUNDATION_EXPORT NSString* OHQueryStringFromParameters(NSDictionary *parameters);

typedef NS_ENUM(NSUInteger, OHHTTPRequestQueryStringSerializationStyle) {
    OHHTTPRequestQueryStringDefaultStyle = 0,
};

@protocol OHMultiFormData;

@interface OHHttpRequestHandler : NSObject <NSObject>
@property (nonatomic, assign) NSStringEncoding encoding;
@property (readonly, nonatomic, strong) NSDictionary <NSString *, NSString *> *requestHeaders;
@property (nonatomic, strong) NSSet <NSString *> *httpMethodEncodeParamInUri;


+ (instancetype)instance;

- (void)setValue:(nullable NSString *)value forHeaderField:(NSString *)field;

- (nullable NSString *)valueForHeaderField:(NSString *)field;

- (void)clearAuthHeader;

- (void)setQryStrSeriWithStyle:(OHHTTPRequestQueryStringSerializationStyle)style;

- (void)setQryStrSeriWithBlock:(nullable NSString * _Nullable (^)(NSURLRequest *request,
    id parameters, NSError * __autoreleasing *error))block;

- (nullable NSMutableURLRequest *)requestWithMethod:(NSString *)method
                                          urlString:(NSString *)urlString
                                         parameters:(nullable id)parameters
                                              error:(NSError * _Nullable __autoreleasing *)error;

- (NSMutableURLRequest *)multipartFormRequestWithMethod:(NSString *)method
                                              urlString:(NSString *)urlString
                                             parameters:(nullable NSDictionary <NSString *, id> *)parameters
                              constructingBodyWithBlock:(nullable void (^)(id <OHMultiFormData> formData))block
                                                  error:(NSError * _Nullable __autoreleasing *)error;

- (nullable NSURLRequest *)requestBySeriReq:(NSURLRequest *)request
                               withParameters:(nullable id)parameters
                                        error:(NSError * _Nullable __autoreleasing *)error NS_SWIFT_NOTHROW;

@end

NS_ASSUME_NONNULL_END
#endif // OHNETWORK_KIT_OHHTTPREQUEST_HANDLER_H