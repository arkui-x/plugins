/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "cache_http_client_ios.h"
#include "cache_error.h"

#undef LOG_DOMAIN
#undef LOG_TAG
#define LOG_DOMAIN 0xD002B01
#define LOG_TAG "CacheDownload"
#include "hilog/log.h"
#include "cache_log.h"

#import <Foundation/Foundation.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <chrono>
#include <string>

namespace OHOS::Plugin::CacheDownload {

namespace {

std::string NSStringToStdString(NSString* str)
{
    if (str == nil) {
        return {};
    }
    return std::string([str UTF8String]);
}

NSString* StdStringToNSString(const std::string& str)
{
    return [NSString stringWithUTF8String:str.c_str()];
}

// Resolves the host of a URL to a stringified IP (first address). Returns an
// empty string if resolution fails. Used to populate DownloadInfo.network.ip.
std::string ResolveHostToIp(const std::string& urlStr)
{
    NSURL* url = [NSURL URLWithString:StdStringToNSString(urlStr)];
    if (url == nil || url.host == nil) {
        return {};
    }
    std::string host = NSStringToStdString(url.host);
    if (host.empty()) {
        return {};
    }
    struct addrinfo hints = {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo* res = nullptr;
    if (getaddrinfo(host.c_str(), nullptr, &hints, &res) != 0 || res == nullptr) {
        return {};
    }
    std::string ip;
    char buf[INET6_ADDRSTRLEN] = {0};
    for (struct addrinfo* p = res; p != nullptr; p = p->ai_next) {
        if (p->ai_family == AF_INET) {
            auto* s4 = reinterpret_cast<struct sockaddr_in*>(p->ai_addr);
            if (inet_ntop(AF_INET, &s4->sin_addr, buf, sizeof(buf)) != nullptr) {
                ip = buf;
                break;
            }
        } else if (p->ai_family == AF_INET6) {
            auto* s6 = reinterpret_cast<struct sockaddr_in6*>(p->ai_addr);
            if (inet_ntop(AF_INET6, &s6->sin6_addr, buf, sizeof(buf)) != nullptr) {
                ip = buf;
                break;
            }
        }
    }
    freeaddrinfo(res);
    return ip;
}

// Maps an NSError to a CacheDownload ErrorCode value (DNS/TCP/SSL/OTHERS) per
// the XTS ErrorCode enum. HTTP status errors are handled separately by the
// caller (passed as a numeric status code).
int MapNSErrorToErrorCode(NSError* error)
{
    if (error == nil) {
        return static_cast<int>(ErrorCode::OTHERS);
    }
    NSInteger code = error.code;
    switch (code) {
        case NSURLErrorCannotFindHost:        // -1003
        case NSURLErrorDNSLookupFailed:       // -1006
            return static_cast<int>(ErrorCode::DNS);
        case NSURLErrorCannotConnectToHost:   // -1004
        case NSURLErrorNetworkConnectionLost: // -1005
        case NSURLErrorNotConnectedToInternet:// -1009
        case NSURLErrorTimedOut:              // -1001
            return static_cast<int>(ErrorCode::TCP);
        case NSURLErrorSecureConnectionFailed:   // -1200
        case NSURLErrorServerCertificateHasBadDate:        // -1201
        case NSURLErrorServerCertificateUntrusted:         // -1202
        case NSURLErrorServerCertificateHasUnknownRoot:    // -1203
        case NSURLErrorServerCertificateNotYetValid:       // -1204
        case NSURLErrorClientCertificateRejected:          // -1205
        case NSURLErrorClientCertificateRequired:          // -1206
            return static_cast<int>(ErrorCode::SSL);
        default:
            return static_cast<int>(ErrorCode::OTHERS);
    }
}

class CacheHttpRequestIOS : public CacheHttpRequest {
public:
    CacheHttpRequestIOS(const CacheHttpRequestOptions& options,
                        const std::shared_ptr<CacheHttpRequestCallback>& callback)
        : options_(options)
        , callback_(callback)
    {
    }

    ~CacheHttpRequestIOS() override
    {
        Cancel();
    }

    bool Start() override;
    void Cancel() override;

private:
    NSMutableURLRequest* BuildRequest();
    NSURLSessionConfiguration* BuildSessionConfig();
    void HandleResponse(NSData* data, NSURLResponse* response, NSError* error,
                       const std::shared_ptr<CacheHttpRequestCallback>& callback,
                       CacheHttpRequestIOS* strongSelf);

    CacheHttpRequestOptions options_;
    std::shared_ptr<CacheHttpRequestCallback> callback_;
    NSURLSession* session_ = nil;
    NSURLSessionDataTask* task_ = nil;
    bool cancelled_ = false;
    std::chrono::steady_clock::time_point startTime_;
    std::string resolvedIp_;
};

} // namespace

NSMutableURLRequest* CacheHttpRequestIOS::BuildRequest()
{
    NSURL* url = [NSURL URLWithString:StdStringToNSString(options_.url)];
    NSMutableURLRequest* request = [NSMutableURLRequest requestWithURL:url];
    request.HTTPMethod = StdStringToNSString(options_.method);
    request.timeoutInterval = options_.readTimeoutMs / 1000.0;

    for (const auto& [key, value] : options_.headers) {
        [request setValue:StdStringToNSString(value) forHTTPHeaderField:StdStringToNSString(key)];
    }
    return request;
}

NSURLSessionConfiguration* CacheHttpRequestIOS::BuildSessionConfig()
{
    NSURLSessionConfiguration* config = [NSURLSessionConfiguration defaultSessionConfiguration];
    config.timeoutIntervalForRequest = options_.readTimeoutMs / 1000.0;
    config.timeoutIntervalForResource = options_.readTimeoutMs / 1000.0;
    return config;
}

void CacheHttpRequestIOS::HandleResponse(NSData* data, NSURLResponse* response, NSError* error,
    const std::shared_ptr<CacheHttpRequestCallback>& callback, CacheHttpRequestIOS* strongSelf)
{
    NSHTTPURLResponse* httpResponse = (NSHTTPURLResponse*)response;
    NSInteger statusCode = (httpResponse != nil) ? httpResponse.statusCode : 0;
    CDLOG_INFO("CacheHttpRequestIOS::HandleResponse statusCode=" CDLOG_FMT_D " taskId=" CDLOG_FMT_PUBLIC_S,
               (int)statusCode, GetBriefTaskId(strongSelf->options_.url).c_str());

    CacheHttpMetrics metrics;
    metrics.serverIp = strongSelf->resolvedIp_;
    auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - strongSelf->startTime_).count();
    metrics.totalTime = static_cast<double>(elapsedUs) / 1000.0;
    metrics.statusCode = static_cast<int>(statusCode);
    callback->OnMetrics(metrics);

    if (error != nil) {
        int ec = MapNSErrorToErrorCode(error);
        callback->OnFailed(ec, NSStringToStdString(error.localizedDescription));
        CDLOG_ERROR("CacheHttpRequestIOS error code=" CDLOG_FMT_D " taskId=" CDLOG_FMT_PUBLIC_S,
                    ec, GetBriefTaskId(strongSelf->options_.url).c_str());
        strongSelf->session_ = nil;
        return;
    }

    CacheHttpResponseInfo info;
    info.statusCode = static_cast<int>(statusCode);
    if (httpResponse != nil && httpResponse.allHeaderFields != nil) {
        for (NSString* key in httpResponse.allHeaderFields) {
            NSString* value = [httpResponse.allHeaderFields objectForKey:key];
            if (value != nil) {
                info.headers[NSStringToStdString(key)] = NSStringToStdString(value);
            }
        }
    }

    if (data != nil && data.length > 0) {
        if (callback->OnData(static_cast<const uint8_t*>(data.bytes),
                             static_cast<size_t>(data.length)) == 0) {
            strongSelf->session_ = nil;
            return;
        }
    }

    if (statusCode >= 200 && statusCode < 300) {
        callback->OnSuccess(info);
    } else {
        callback->OnFailed(static_cast<int>(statusCode), std::to_string(statusCode));
    }
    strongSelf->session_ = nil;
}

bool CacheHttpRequestIOS::Start()
{
    if (cancelled_ || !callback_) {
        return false;
    }

    // TLCP (GM/T 0024) is not supported by the iOS platform TLS stack. Fail
    // explicitly with an SSL-class error instead of silently downgrading to
    // plain HTTPS — matches OHOS, where a TLCP handshake against a non-TLCP
    // peer fails (SUB_Request_cacheDownload_stateSecret_0400).
    if (options_.sslType == SslType::TLCP) {
        HILOG_ERROR(LOG_CORE,
            "cacheDownload: TLCP sslType is not supported on iOS, fail explicitly. taskId=%{public}s",
            GetBriefTaskId(options_.url).c_str());
        callback_->OnFailed(static_cast<int>(ErrorCode::SSL), "TLCP is not supported on iOS");
        return false;
    }

    startTime_ = std::chrono::steady_clock::now();
    CDLOG_INFO("CacheHttpRequestIOS::Start taskId=" CDLOG_FMT_PUBLIC_S, GetBriefTaskId(options_.url).c_str());
    resolvedIp_ = ResolveHostToIp(options_.url);

    NSMutableURLRequest* request = BuildRequest();
    NSURLSessionConfiguration* config = BuildSessionConfig();

    auto callback = callback_;
    __weak CacheHttpRequestIOS* weakSelf = this;

    session_ = [NSURLSession sessionWithConfiguration:config];
    task_ = [session_ dataTaskWithRequest:request
                       completionHandler:^(NSData* data, NSURLResponse* response, NSError* error) {
        CacheHttpRequestIOS* strongSelf = weakSelf;
        if (strongSelf == nil || strongSelf->cancelled_) {
            return;
        }
        strongSelf->HandleResponse(data, response, error, callback, strongSelf);
    }];

    [task_ resume];
    return true;
}

void CacheHttpRequestIOS::Cancel()
{
    CDLOG_INFO("CacheHttpRequestIOS::Cancel taskId=" CDLOG_FMT_PUBLIC_S, GetBriefTaskId(options_.url).c_str());
    cancelled_ = true;
    if (task_ != nil) {
        [task_ cancel];
        task_ = nil;
    }
    session_ = nil;
}

CacheHttpClientIOS::CacheHttpClientIOS() = default;

CacheHttpClientIOS::~CacheHttpClientIOS() = default;

std::shared_ptr<CacheHttpRequest> CacheHttpClientIOS::NewRequest(
    const CacheHttpRequestOptions& options,
    const std::shared_ptr<CacheHttpRequestCallback>& callback)
{
    return std::make_shared<CacheHttpRequestIOS>(options, callback);
}

std::shared_ptr<CacheHttpClient> CreateCacheHttpClient()
{
    return std::make_shared<CacheHttpClientIOS>();
}

} // namespace OHOS::Plugin::CacheDownload
