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

typedef NS_ENUM(NSUInteger, OHSSLMode) {
    OHSSLModeNone,
    OHSSLModePubKey,
    OHSSLModeCert,
};


NS_ASSUME_NONNULL_BEGIN

@interface OHSslHandler : NSObject <NSSecureCoding, NSCopying>

@property (readonly, nonatomic, assign) OHSSLMode sslMode;
@property (nonatomic, strong, nullable) NSSet <NSData *> *pinCerts;
@property (nonatomic, assign) BOOL passInvalidCerts;
@property (nonatomic, assign) BOOL allowValidatesDomain;


+ (NSSet <NSData *> *)certsInBundle:(NSBundle *)bundle;
+ (instancetype)defaultHandler;
+ (instancetype)handlerWithPinningMode:(OHSSLMode)pinningMode;
- (BOOL)evaluateServerTrust:(SecTrustRef)serverTrust
                  forDomain:(nullable NSString *)domain;

@end

NS_ASSUME_NONNULL_END
#endif // PLUGINS_REQUEST_IOS_OHNETWORKKIT_OHSSLHANDLER_H