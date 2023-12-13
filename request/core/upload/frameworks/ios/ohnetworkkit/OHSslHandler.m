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

#import "OHSslHandler.h"
#import <AssertMacros.h>


static BOOL OHSecKeyIsEqualToKey(SecKeyRef key1, SecKeyRef key2) {
    return [(__bridge id)key1 isEqual:(__bridge id)key2];
}

static id OHPublicKeyForCertificate(NSData *certificate) {
    id allowedPublicKey = nil;
    SecCertificateRef allowedCertificate;
    SecPolicyRef policy = nil;
    SecTrustRef allowedTrust = nil;
    SecTrustResultType result;
    allowedCertificate = SecCertificateCreateWithData(NULL, (__bridge CFDataRef)certificate);
    __Require_Quiet(allowedCertificate != NULL, _out);
    policy = SecPolicyCreateBasicX509();
    __Require_noErr_Quiet(SecTrustCreateWithCertificates(allowedCertificate, policy, &allowedTrust), _out);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    __Require_noErr_Quiet(SecTrustEvaluate(allowedTrust, &result), _out);
#pragma clang diagnostic pop
    allowedPublicKey = (__bridge_transfer id)SecTrustCopyPublicKey(allowedTrust);

_out:
    if (allowedTrust) {
        CFRelease(allowedTrust);
    }
    if (policy) {
        CFRelease(policy);
    }
    if (allowedCertificate) {
        CFRelease(allowedCertificate);
    }
    return allowedPublicKey;
}

static BOOL OHServerTrustIsValid(SecTrustRef serverTrust) {
    BOOL isValid = NO;
    SecTrustResultType result;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    __Require_noErr_Quiet(SecTrustEvaluate(serverTrust, &result), _out);
#pragma clang diagnostic pop
    isValid = (result == kSecTrustResultUnspecified || result == kSecTrustResultProceed);

_out:
    return isValid;
}

static NSArray * OHCertificateTrustChainForServerTrust(SecTrustRef serverTrust) {
    CFIndex certificateCount = SecTrustGetCertificateCount(serverTrust);
    NSMutableArray *trustChain = [NSMutableArray arrayWithCapacity:(NSUInteger)certificateCount];
    for (CFIndex i = 0; i < certificateCount; i++) {
        SecCertificateRef certificate = SecTrustGetCertificateAtIndex(serverTrust, i);
        [trustChain addObject:(__bridge_transfer NSData *)SecCertificateCopyData(certificate)];
    }
    return [NSArray arrayWithArray:trustChain];
}

static NSArray * OHPublicKeyTrustChainForServerTrust(SecTrustRef serverTrust) {
    SecPolicyRef policy = SecPolicyCreateBasicX509();
    CFIndex certificateCount = SecTrustGetCertificateCount(serverTrust);
    NSMutableArray *trustChain = [NSMutableArray arrayWithCapacity:(NSUInteger)certificateCount];
    for (CFIndex i = 0; i < certificateCount; i++) {
        SecCertificateRef certificate = SecTrustGetCertificateAtIndex(serverTrust, i);

        SecCertificateRef someCertificates[] = {certificate};
        CFArrayRef certificates = CFArrayCreate(NULL, (const void **)someCertificates, 1, NULL);

        SecTrustRef trust;
        __Require_noErr_Quiet(SecTrustCreateWithCertificates(certificates, policy, &trust), _out);
        SecTrustResultType result;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
        __Require_noErr_Quiet(SecTrustEvaluate(trust, &result), _out);
#pragma clang diagnostic pop
        [trustChain addObject:(__bridge_transfer id)SecTrustCopyPublicKey(trust)];

    _out:
        if (trust) {
            CFRelease(trust);
        }

        if (certificates) {
            CFRelease(certificates);
        }
        continue;
    }
    CFRelease(policy);
    return [NSArray arrayWithArray:trustChain];
}

#pragma mark -
@interface OHSslHandler()

@property (readwrite, nonatomic, assign) OHSslType sslType;
@property (readwrite, nonatomic, strong) NSSet *pinnedPublicKeys;

@end


@implementation OHSslHandler

+ (NSSet *)certsInBundle:(NSBundle *)bundle {
    NSArray *paths = [bundle pathsForResourcesOfType:@"cer" inDirectory:@"."];
    NSMutableSet *certificates = [NSMutableSet setWithCapacity:[paths count]];
    for (NSString *path in paths) {
        NSData *certificateData = [NSData dataWithContentsOfFile:path];
        [certificates addObject:certificateData];
    }
    return [NSSet setWithSet:certificates];
}

+ (instancetype)defaultHandler {
    OHSslHandler *handler = [[self alloc] init];
    handler.sslType = OHSslTypeNone;
    return handler;
}

+ (instancetype)handlerWithSslType:(OHSslType)sslType {
    NSSet <NSData *> *defaultCert = [self certsInBundle:[NSBundle mainBundle]];
    return [self handlerWithSslType:sslType withSslCerts:defaultCert];
}

+ (instancetype)handlerWithSslType:(OHSslType)sslType
    withSslCerts:(NSSet *)certList {
    OHSslHandler *handler = [[self alloc] init];
    handler.sslType = sslType;
    [handler setCerts:certList];
    return handler;
}

- (instancetype)init {
    self = [super init];
    if (!self) {
        return nil;
    }
    self.allowIvdHostDomain = YES;
    return self;
}

- (void)setCerts:(NSSet *)certList {
    _certList = certList;

    if (self.certList) {
        NSMutableSet *pubKeys = [NSMutableSet setWithCapacity:[self.certList count]];
        for (NSData *certificate in self.certList) {
            id publicKey = OHPublicKeyForCertificate(certificate);
            if (!publicKey) {
                continue;
            }
            [pubKeys addObject:publicKey];
        }
        self.pinnedPublicKeys = [NSSet setWithSet:pubKeys];
    } else {
        self.pinnedPublicKeys = nil;
    }
}

#pragma mark -
- (BOOL)evaluateServerTrust:(SecTrustRef)serverTrust
                  forDomain:(NSString *)domain {
    if (domain && self.allowIvdCerts && self.allowIvdHostDomain &&
        (self.sslType == OHSslTypeNone || [self.certList count] == 0)) {
        NSLog(@"In order to validate a domain name for self signed certificates, you MUST use pinning.");
        return NO;
    }

    NSMutableArray *policies = [NSMutableArray array];
    if (self.allowIvdHostDomain) {
        [policies addObject:(__bridge_transfer id)SecPolicyCreateSSL(true, (__bridge CFStringRef)domain)];
    } else {
        [policies addObject:(__bridge_transfer id)SecPolicyCreateBasicX509()];
    }
    SecTrustSetPolicies(serverTrust, (__bridge CFArrayRef)policies);
    if (self.sslType == OHSslTypeNone) {
        return self.allowIvdCerts || OHServerTrustIsValid(serverTrust);
    } else if (!self.allowIvdCerts && !OHServerTrustIsValid(serverTrust)) {
        return NO;
    }

    switch (self.sslType) {
        case OHSslTypeCert: {
            NSMutableArray *certList = [NSMutableArray array];
            for (NSData *certificateData in self.certList) {
                [certList addObject:(__bridge_transfer id)SecCertificateCreateWithData(NULL,
                    (__bridge CFDataRef)certificateData)];
            }
            SecTrustSetAnchorCertificates(serverTrust, (__bridge CFArrayRef)certList);
            if (!OHServerTrustIsValid(serverTrust)) {
                return NO;
            }
            // obtain the chain after being validated, which *should* contain the pinned certificate
            // in the last position (if it's the Root CA)
            NSArray *serverCertificates = OHCertificateTrustChainForServerTrust(serverTrust);
            for (NSData *trustChainCertificate in [serverCertificates reverseObjectEnumerator]) {
                if ([self.certList containsObject:trustChainCertificate]) {
                    return YES;
                }
            }
            return NO;
        }
        case OHSslTypePubKey: {
            NSUInteger trustedPublicKeyCount = 0;
            NSArray *publicKeys = OHPublicKeyTrustChainForServerTrust(serverTrust);
            for (id trustChainPublicKey in publicKeys) {
                for (id pinnedPublicKey in self.pinnedPublicKeys) {
                    if (OHSecKeyIsEqualToKey((__bridge SecKeyRef)trustChainPublicKey,
                        (__bridge SecKeyRef)pinnedPublicKey)) {
                        trustedPublicKeyCount += 1;
                    }
                }
            }
            return trustedPublicKeyCount > 0;
        }
        default:
            return NO;
    }
    
    return NO;
}

@end