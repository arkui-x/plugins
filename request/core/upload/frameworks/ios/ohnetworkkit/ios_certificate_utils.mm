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

#import "ios_certificate_utils.h"
#include <mutex>
#include "upload_hilog_wrapper.h"


namespace OHOS::Plugin::Request::CertificateUtils {

void GetCerList(NSMutableArray *derList)
{
    NSSet *cerSet = [OHSslHandler certsInBundle:[NSBundle mainBundle]]; // *.cer
    if ([cerSet count] == 0) {
        UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "no .cer files");
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
                UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "no .pem files");
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

    UPLOAD_HILOGD(UPLOAD_MODULE_FRAMEWORK, "Certificate chain count:%{public}d", certSet.count);
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