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
#ifndef PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHMULTIPARTFORMDATA_H
#define PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHMULTIPARTFORMDATA_H

#import <Foundation/Foundation.h>
#import <TargetConditionals.h>
#import <UIKit/UIKit.h>


NS_ASSUME_NONNULL_BEGIN

// OHMultiFormData
@protocol OHMultiFormData

- (void)addHeaders:(nullable NSDictionary <NSString *, NSString *> *)headers
    body:(NSData *)body;

- (void)addFormData:(NSData *)data
    name:(NSString *)name;

- (BOOL)addFile:(NSURL *)url
    name:(NSString *)name
    mime:(NSString *)mime
    fileName:(NSString *)fileName
    error:(NSError * _Nullable __autoreleasing *)error;

@end

// OHQueryStringPair
@interface OHQueryStringPair : NSObject

@property (readwrite, nonatomic, strong) id field;
@property (readwrite, nonatomic, strong) id value;

- (instancetype)initWithField:(id)field value:(id)value;
- (NSString *)URLEncodedStringValue;

@end

// OHStreamMulFormData
@interface OHStreamMulFormData : NSObject <OHMultiFormData>

- (instancetype)initWithUrlReq:(NSMutableURLRequest *)urlRequest
                    encoding:(NSStringEncoding)encoding;
- (NSMutableURLRequest *)requestByFinMultiFormData;

@end

NS_ASSUME_NONNULL_END
#endif // PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHMULTIPARTFORMDATA_H