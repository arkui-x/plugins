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