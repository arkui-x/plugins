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