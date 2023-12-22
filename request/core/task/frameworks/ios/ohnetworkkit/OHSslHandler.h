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

#ifndef PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHSSLHANDLER_H
#define PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHSSLHANDLER_H

#import <Foundation/Foundation.h>
#import <Security/Security.h>

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