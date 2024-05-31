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

#import "ios_certificate_utils.h"
#include <mutex>
#include "log.h"

namespace OHOS::Plugin::Request::CertificateUtils {

void GetCerList(NSMutableArray *derList)
{
    NSSet *cerSet = [OHSslHandler certsInBundle:[NSBundle mainBundle]]; // *.cer
    if ([cerSet count] == 0) {
        REQUEST_HILOGD("no .cer files");
        return;
    }

    for (NSData *cer in cerSet) {
        [derList addObject:cer];
    }
}

void GetDerList(NSMutableArray *derList)
{
    static NSMutableArray *cacheDerList = nil;
    std::once_flag onceFlag;
    std::call_once(onceFlag, [&] {
        if (cacheDerList == nil) {
            cacheDerList = [[NSMutableArray alloc] init];
            // CA Certificates in mainBundle: include privacy certificate (*.pem) and ohos certificate(cacert.pem)
            NSArray<NSString *> *paths = [[NSBundle mainBundle] pathsForResourcesOfType:@"pem" inDirectory:nil];
            if ([paths count] == 0) {
                REQUEST_HILOGD("no .pem files");
                GetCerList(derList);
                [cacheDerList addObjectsFromArray:derList];
                return;
            }

            for (NSString *certPath in paths) {
                NSString *pems = [NSString stringWithContentsOfFile:certPath
                    encoding:NSUTF8StringEncoding error:nil];
                NSString *begCert = @"-----BEGIN CERTIFICATE-----";
                NSString *endCert = @"-----END CERTIFICATE-----";
                NSUInteger loc = 0;
                do {
                    NSRange posRange = NSMakeRange(loc, pems.length - loc);
                    NSRange begRange = [pems rangeOfString:begCert options:NSCaseInsensitiveSearch range:posRange];
                    NSRange endRange = [pems rangeOfString:endCert options:NSCaseInsensitiveSearch range:posRange];
                    if (begRange.length > 0 && endRange.length > 0) {
                        NSUInteger pos = begRange.location + begRange.length;
                        NSString *pemContent = [pems substringWithRange:NSMakeRange(pos, endRange.location - pos)];
                        NSData *base64 = [NSData dataWithBytes:pemContent.UTF8String length:pemContent.length];
                        NSData *derContent = [[NSData alloc] initWithBase64EncodedData:base64 options:NSDataBase64DecodingIgnoreUnknownCharacters]; // base64 Decoded
                        [cacheDerList addObject:derContent];
                    }
                    loc = endRange.location + endRange.length;
                } while (loc < pems.length - 1);
            }
            GetCerList(cacheDerList);
        }
    });
    [derList addObjectsFromArray:cacheDerList];
}

void InstallCertificateChain(OHSessionManager *sessionCtrl_)
{
    NSMutableArray *derList = [[NSMutableArray alloc] init];
    OHOS::Plugin::Request::CertificateUtils::GetDerList(derList);
    NSMutableSet *certSet = [[NSMutableSet alloc] init];
    for (NSData *der in derList) {
        [certSet addObject:der];
    }

    REQUEST_HILOGD("Certificate chain count:%{public}d", certSet.count);
    if ([certSet count] == 0) {
        return; // use default: OHSslTypeNone
    }
    OHSslHandler *sslHandler = [OHSslHandler handlerWithSslType:OHSslTypeCert];
    [sslHandler setAllowIvdCerts:YES];
    [sslHandler setAllowIvdHostDomain:NO];
    [sslHandler setCerts:certSet];
    sessionCtrl_.sslHandler = sslHandler;
}

} // OHOS::Plugin::Request::CertificateUtils