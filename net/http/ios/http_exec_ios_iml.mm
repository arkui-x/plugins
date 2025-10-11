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

#include "http_exec_ios_iml.h"
#import "http_ios_request.h"

namespace OHOS::NetStack::Http {
SuccessCallback HttpExecIosIml::successResponseCallback_ = nullptr;
FailedCallback HttpExecIosIml::failedResponseCallback_ = nullptr;
ProgressCallback HttpExecIosIml::progressCallback_ = nullptr;
WritingBodyCallback HttpExecIosIml::writingBodyCallback_ = nullptr;
WritingHeaderCallback HttpExecIosIml::writingHeaderCallback_ = nullptr;

HttpExecIosIml::~HttpExecIosIml()
{
    http_ios_request* request = (__bridge http_ios_request*)request_;
    if (!request) {
        return;
    }
    [request deInitialize];
}

std::string HttpExecIosIml::GetCacheFileName()
{
    NSString * stringPath = [http_ios_request getBaseCachePath];
    std::string path = [stringPath UTF8String];
    return path;
}

bool HttpExecIosIml::SendRequest(HttpRequestOptions& requestOptions,void* userData)
{
    http_ios_param* requestParam = [[http_ios_param alloc] init];
    const char* url = requestOptions.GetUrl().c_str();
    const char* method = requestOptions.GetMethod().c_str();
    requestParam.urlPath = [NSString stringWithCString:url encoding:NSUTF8StringEncoding];
    requestParam.method = [NSString stringWithCString:method encoding:NSUTF8StringEncoding];

    std::string proxyHost;
    int32_t proxprot;
    std::string exclusionList;
    NapiUtils::SecureData username;
    NapiUtils::SecureData password;
    requestOptions.GetSpecifiedHttpProxy(proxyHost, proxprot, exclusionList, username, password);

    requestParam.proxyhost = [NSString stringWithCString:proxyHost.c_str() encoding:NSUTF8StringEncoding];
    requestParam.proxyport = (NSInteger)proxprot;
    requestParam.exclusionList = [NSString stringWithCString:exclusionList.c_str() encoding:NSUTF8StringEncoding];
    requestParam.returnDataType = (NSInteger)requestOptions.GetHttpDataType();
    const char* body = requestOptions.GetBody().c_str();
    requestParam.bodyParam = [NSString stringWithCString:body encoding:NSUTF8StringEncoding];
    requestParam.context = userData;

    NSMutableDictionary* headerDic = [NSMutableDictionary dictionary];
    for (std::map<std::string,
        std::string>::const_iterator it = requestOptions.GetHeader().begin();
        it != requestOptions.GetHeader().end(); it++) {
        NSString* key = [NSString stringWithUTF8String:it->first.c_str()];
        NSString* value = [NSString stringWithUTF8String:it->second.c_str()];
        headerDic[key] = value;
    }

    requestParam.headerJson = headerDic;
    requestParam.priority = requestOptions.GetPriority();
    requestParam.readTimeout = (NSTimeInterval)requestOptions.GetReadTimeout()/1000;
    requestParam.connectTimeout = (NSTimeInterval)requestOptions.GetConnectTimeout()/1000;
    requestParam.httpVersion = requestOptions.GetHttpVersion();

    std::string cert;
    std::string certType;
    std::string key;
    Secure::SecureChar keyPasswd;
    requestOptions.GetClientCert(cert, certType, key, keyPasswd);

    requestParam.ca = [NSString stringWithCString:cert.c_str() encoding:NSUTF8StringEncoding];;
    requestParam.caType = [NSString stringWithCString:certType.c_str() encoding:NSUTF8StringEncoding];;
    requestParam.key = [NSString stringWithCString:key.c_str() encoding:NSUTF8StringEncoding];;
    requestParam.password = [NSString stringWithCString:keyPasswd.Data() encoding:NSUTF8StringEncoding];;
    requestParam.usingHttpProxyType = static_cast<NSInteger>(requestOptions.GetUsingHttpProxyType());    

    http_ios_request* request = [[http_ios_request alloc] init];
    request_ = (__bridge void*)request;
    RequestCallBack();

    [request sendRequestWith:requestParam];
    
    return true;
}

void HttpExecIosIml::RequestCallBack()
{
    http_ios_request* request = (__bridge http_ios_request*)request_;
    [request setMemoryHeaderBlock:^(NSDictionary* _Nonnull headers, void* _Nonnull userData) {
        NSData* data = [NSJSONSerialization dataWithJSONObject:headers options:NSJSONWritingPrettyPrinted error:nil];
        if (!data) {
            return;
        }
        const void* buf = [data bytes];
        size_t size = [data length];

        HttpExecIosIml::writingHeaderCallback_(buf, size, userData);
    }];

    [request setMemoryBodyBlock:^(NSData* _Nonnull data, void* _Nonnull userData) {
        const void* buf = [data bytes];
        size_t size = [data length];
        HttpExecIosIml::writingBodyCallback_(buf, size, userData);
    }];

    [request setUploadProgress:^(void* _Nonnull userData, long total, long now) {
        HttpExecIosIml::progressCallback_(userData, 0, 0, total, now);
    }];

    [request setDownloadProgress:^(void* _Nonnull userData, long total, long now) {
        HttpExecIosIml::progressCallback_(userData, total, now, 0, 0);
    }];

    [request setResponseBlock:^(NSURLSessionTask* _Nonnull task,
                                NSHTTPURLResponse* _Nullable response,
                                NSData* _Nullable data,
                                void* _Nonnull userData) {
        NSInteger code = response.statusCode;
        uint32_t statueCode = (uint32_t)code;
        NSDictionary* headerDic = response.allHeaderFields;

        std::string header;
        for (NSString* key in headerDic.allKeys) {
            NSString* value = [headerDic valueForKey:key];
            header += [[NSString stringWithFormat:@"%@: %@", key, value] UTF8String];
            header += "\r\n";
        }
        
        NSHTTPCookieStorage* cookieJar = [NSHTTPCookieStorage sharedHTTPCookieStorage];
        std::string cookiesStr;
        for (NSHTTPCookie* cookie in [cookieJar cookies]) {
            cookiesStr += [[NSString stringWithFormat:@"%@: %@", cookie.name, cookie.value] UTF8String];
            cookiesStr += "\r\n";
        }

        HttpResponse httpRespone;
        std::string dataChat((const char*)[data bytes], [data length]);
        httpRespone.SetResult(dataChat);
        httpRespone.SetResponseCode(statueCode);
        httpRespone.SetRawHeader(header);
        httpRespone.SetCookies(cookiesStr);
        HttpExecIosIml::successResponseCallback_(httpRespone, userData);

        headerDic = nil;
    }];

    [request setFailBlock:^(NSInteger errorCode, NSString* _Nonnull errorMessage, void* _Nonnull userData) {
        uint32_t code = (uint32_t)errorCode;
        std::string errormsg = [errorMessage UTF8String];
        HttpExecIosIml::failedResponseCallback_(code, errormsg, userData);
    }];
}

void HttpExecIosIml::SetSuccessCallback(SuccessCallback successResponseCallback)
{
    if (successResponseCallback_ == nullptr) {
        successResponseCallback_ = successResponseCallback;
    }
}

void HttpExecIosIml::SetFailedCallback(FailedCallback failedResponseCallback)
{
    if (failedResponseCallback_ == nullptr) {
        failedResponseCallback_ = failedResponseCallback;
    }
}

void HttpExecIosIml::SetProgressCallback(ProgressCallback progressCallback)
{
    if (progressCallback_ == nullptr) {
        progressCallback_ = progressCallback;
    }
}

void HttpExecIosIml::SetWritingBodyCallback(WritingBodyCallback writingBodyCallback)
{
    if (writingBodyCallback_ == nullptr) {
        writingBodyCallback_ = writingBodyCallback;
    }
}

void HttpExecIosIml::SetWritingHeaderCallback(WritingHeaderCallback writingHeaderCallback)
{
    if (writingHeaderCallback_ == nullptr) {
        writingHeaderCallback_ = writingHeaderCallback;
    }
}
} // namespace OHOS::NetStack::Http
