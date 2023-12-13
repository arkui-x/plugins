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

#ifndef PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHSSLHANDLER_H
#define PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHSSLHANDLER_H

#import <Security/Security.h>
#import <Foundation/Foundation.h>


typedef NS_ENUM(NSUInteger, OHSslType) {
    OHSslTypePubKey,
    OHSslTypeCert,
    OHSslTypeNone
};


NS_ASSUME_NONNULL_BEGIN

@interface OHSslHandler : NSObject

@property (nonatomic, strong, nullable) NSSet <NSData *> *certList;
@property (nonatomic, assign) BOOL allowIvdHostDomain;
@property (nonatomic, assign) BOOL allowIvdCerts;
@property (readonly, nonatomic, assign) OHSslType sslType;


+ (NSSet <NSData *> *)certsInBundle:(NSBundle *)bundle;
+ (instancetype)defaultHandler;
+ (instancetype)handlerWithSslType:(OHSslType)sslType;
- (BOOL)evaluateServerTrust:(SecTrustRef)serverTrust
                  forDomain:(nullable NSString *)domain;
- (void)setCerts:(NSSet *)certList;

@end

NS_ASSUME_NONNULL_END
#endif // PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHSSLHANDLER_H