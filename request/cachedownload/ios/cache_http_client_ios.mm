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
#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>
#include <string>

// Collects NSURLSessionTaskMetrics for a task created with a completion
// handler. A session-level delegate is required: the completionHandler API
// alone cannot expose task metrics. Apple guarantees
// -URLSession:task:didFinishCollectingMetrics: is delivered on the session's
// delegate queue before the completion handler runs, so metrics populated
// here are visible to the completion handler without extra locking.
@interface CacheHttpMetricsDelegate : NSObject <NSURLSessionDataDelegate>
/// Invoked when task metrics are collected. Runs on the session delegate queue.
@property (nonatomic, copy, nullable) void (^onTaskMetrics)(NSURLSessionTaskMetrics* taskMetrics);
/// Redirect guard. Invoked on every redirect hop with the proposed request.
/// Set before the task is created; nil disables the guard (auto-follow).
/// The block must return the request to follow (possibly a header-stripped
/// copy), or nil to reject the redirect and cancel the task.
@property (nonatomic, copy, nullable) NSURLRequest* (^onRedirect)(NSURLRequest* newRequest);
@end

@implementation CacheHttpMetricsDelegate

- (void)URLSession:(NSURLSession*)session task:(NSURLSessionTask*)task
    didFinishCollectingMetrics:(NSURLSessionTaskMetrics*)metrics
{
    (void)session;
    (void)task;
    if (self.onTaskMetrics != nil) {
        self.onTaskMetrics(metrics);
    }
}

- (void)URLSession:(NSURLSession*)session task:(NSURLSessionTask*)task
    willPerformHTTPRedirection:(NSHTTPURLResponse*)response
                    newRequest:(NSURLRequest*)request
             completionHandler:(void (^)(NSURLRequest*))completionHandler
{
    (void)session;
    (void)task;
    (void)response;
    if (self.onRedirect == nil) {
        completionHandler(request);
        return;
    }
    NSURLRequest* next = self.onRedirect(request);
    if (next == nil) {
        // Rejecting the redirect: cancel the task so the completion handler
        // fires with NSURLErrorCancelled instead of silently following.
        [task cancel];
    }
    completionHandler(request);
}

// The data task is created with a completion handler, so none of the
// NSURLSessionDataDelegate data methods are implemented — the delegate exists
// solely to receive task metrics and to guard redirect hops.

@end

namespace OHOS::Plugin::CacheDownload {

namespace {

// Max redirect hops to follow — 30 matches the libcurl default used by the
// OHOS netstack backend.
constexpr int MAX_REDIRECTS = 30;

// OHOS DownloadErrorCode::ERROR_TOO_MANY_REDIRECTS; the C++ service
// classifies it as OTHERS/non-retryable so redirect loops are not retried.
constexpr int ERROR_TOO_MANY_REDIRECTS = 6;

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
            // NSURLErrorTimedOut (-1001) lands here: a read/total timeout maps
            // to OTHERS, matching OHOS netstack where CURLE_OPERATION_TIMEDOUT
            // is ErrorKind::Others, not TCP.
            return static_cast<int>(ErrorCode::OTHERS);
    }
}

// Cumulative ms from the transaction's fetchStart to the given date.
// Returns 0 when the date is nil (stage never happened, e.g. no TLS for
// plain HTTP) so the metric stays unset like curl's behaviour.
double ElapsedMsSince(NSDate* fetchStart, NSDate* stageEnd)
{
    if (fetchStart == nil || stageEnd == nil) {
        return 0.0;
    }
    return [stageEnd timeIntervalSinceDate:fetchStart] * 1000.0;
}

class CacheHttpRequestIOS : public CacheHttpRequest,
                            public std::enable_shared_from_this<CacheHttpRequestIOS> {
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
    void CollectMetrics(NSURLSessionTaskMetrics* taskMetrics);

private:
    NSMutableURLRequest* BuildRequest();
    NSURLSessionConfiguration* BuildSessionConfig();
    void ConfigureRedirectGuard(CacheHttpMetricsDelegate* metricsDelegate);
    void StartTask(NSMutableURLRequest* request, NSURLSessionConfiguration* config,
        CacheHttpMetricsDelegate* metricsDelegate,
        const std::shared_ptr<CacheHttpRequestCallback>& callback);
    void HandleResponse(NSData* data, NSURLResponse* response, NSError* error,
                       const std::shared_ptr<CacheHttpRequestCallback>& callback);
    void FinishSession();

    CacheHttpRequestOptions options_;
    std::shared_ptr<CacheHttpRequestCallback> callback_;
    NSURLSession* session_ = nil;
    NSURLSessionDataTask* task_ = nil;
    // Set by Cancel() on the caller thread; read by the metrics block and the
    // completion handler on the session delegate queue. Atomic so those
    // cross-thread reads/writes are well-defined.
    std::atomic<bool> cancelled_ {false};
    // Serializes the session_/task_ teardown in Cancel() (caller thread) and
    // FinishSession() (delegate queue) so they cannot double-invalidate or
    // race on the pointer swaps. Critical sections stay tiny.
    std::mutex stateMutex_;
    // Redirect-hop counter, incremented in onRedirect on the session delegate
    // queue and only read there (the completion handler runs on the same
    // serial queue), so a plain int is sufficient.
    int redirectHopCount_ = 0;
    // Set when onRedirect rejected a hop (limit exceeded); read by the
    // completion handler on the same serial delegate queue.
    bool tooManyRedirects_ = false;
    std::chrono::steady_clock::time_point startTime_;
    std::string resolvedIp_;
    CacheHttpMetrics metrics_;
};

} // namespace

void CacheHttpRequestIOS::CollectMetrics(NSURLSessionTaskMetrics* taskMetrics)
{
    if (taskMetrics == nil || taskMetrics.transactionMetrics.count == 0) {
        return;
    }
    NSURLSessionTaskTransactionMetrics* finalTransaction =
        taskMetrics.transactionMetrics.lastObject;
    // Cumulative ms since the ORIGINAL request start (curl NAMELOOKUP/CONNECT/
    // APPCONNECT/PRETRANSFER/STARTTRANSFER semantics). firstObject is the
    // original hop; under redirects its fetchStartDate is the chain's start.
    NSDate* fetchStart = taskMetrics.transactionMetrics.firstObject.fetchStartDate;
    if (fetchStart == nil) {
        // Defensive fallback: mirrors the pre-redirect-chain behavior when the
        // original hop's start is unavailable.
        fetchStart = finalTransaction.fetchStartDate;
    }
    if (fetchStart == nil) {
        return;
    }
    // finalTransaction's own stage ends give the cumulative stage boundaries of
    // the final hop (dnsTime=NAMELOOKUP, connectTime=CONNECT, tlsTime=APPCONNECT,
    // firstSendTime≈PRETRANSFER via requestStartDate,
    // firstReceiveTime=STARTTRANSFER via responseStartDate).
    metrics_.dnsTime = ElapsedMsSince(fetchStart, finalTransaction.domainLookupEndDate);
    metrics_.connectTime = ElapsedMsSince(fetchStart, finalTransaction.connectEndDate);
    metrics_.tlsTime = ElapsedMsSince(fetchStart, finalTransaction.secureConnectionEndDate);
    metrics_.firstSendTime = ElapsedMsSince(fetchStart, finalTransaction.requestStartDate);
    metrics_.firstReceiveTime = ElapsedMsSince(fetchStart, finalTransaction.responseStartDate);
    // redirectTime = "original request start -> final request begins" (curl
    // REDIRECT_TIME_T ends when the final request starts, NOT when its response
    // arrives). chainEnd is therefore the final transaction's fetchStartDate;
    // the final hop's own transfer time is covered by firstSendTime/totalTime.
    // Stays 0 when no redirect happened.
    if (taskMetrics.redirectCount > 0 && taskMetrics.transactionMetrics.count > 1) {
        NSDate* chainStart = taskMetrics.transactionMetrics.firstObject.fetchStartDate;
        NSDate* chainEnd = finalTransaction.fetchStartDate;
        if (chainStart != nil && chainEnd != nil) {
            metrics_.redirectTime = [chainEnd timeIntervalSinceDate:chainStart] * 1000.0;
        }
    }
}

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
    const std::shared_ptr<CacheHttpRequestCallback>& callback)
{
    NSHTTPURLResponse* httpResponse = (NSHTTPURLResponse*)response;
    NSInteger statusCode = (httpResponse != nil) ? httpResponse.statusCode : 0;
    CDLOG_INFO("CacheHttpRequestIOS::HandleResponse statusCode=" CDLOG_FMT_D " taskId=" CDLOG_FMT_PUBLIC_S,
               (int)statusCode, GetBriefTaskId(options_.url).c_str());

    CacheHttpMetrics metrics = metrics_;
    metrics.serverIp = resolvedIp_;
    auto elapsedUs = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now() - startTime_).count();
    metrics.totalTime = static_cast<double>(elapsedUs) / 1000.0;
    metrics.statusCode = static_cast<int>(statusCode);
    callback->OnMetrics(metrics);

    if (error != nil) {
        int ec = MapNSErrorToErrorCode(error);
        std::string message = NSStringToStdString(error.localizedDescription);
        if (tooManyRedirects_ && error.code == NSURLErrorCancelled) {
            // Redirect chain exceeded MAX_REDIRECTS (onRedirect rejected the
            // hop and cancelled the task). Use the OHOS
            // DownloadErrorCode::ERROR_TOO_MANY_REDIRECTS value — the C++
            // service classifies it as OTHERS/non-retryable, so an A->B->A
            // loop fails instead of being retried.
            ec = ERROR_TOO_MANY_REDIRECTS;
            message = "Too many redirects";
        }
        callback->OnFailed(ec, message);
        CDLOG_ERROR("CacheHttpRequestIOS error code=" CDLOG_FMT_D " taskId=" CDLOG_FMT_PUBLIC_S,
                    ec, GetBriefTaskId(options_.url).c_str());
        FinishSession();
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
            FinishSession();
            return;
        }
    }

    if (statusCode >= 200 && statusCode < 300) {
        callback->OnSuccess(info);
    } else {
        callback->OnFailed(static_cast<int>(statusCode), std::to_string(statusCode));
    }
    FinishSession();
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

    StartTask(BuildRequest(), BuildSessionConfig(),
              [[CacheHttpMetricsDelegate alloc] init], callback_);
    return true;
}

// Creates the session, wires the metrics/redirect guards on the delegate and
// resumes the data task. Both the metrics block and the completion handler
// capture a strong shared_ptr (see LIFECYCLE in ConfigureRedirectGuard) so
// they stay safe past Cancel()/service-layer teardown.
void CacheHttpRequestIOS::StartTask(NSMutableURLRequest* request, NSURLSessionConfiguration* config,
    CacheHttpMetricsDelegate* metricsDelegate,
    const std::shared_ptr<CacheHttpRequestCallback>& callback)
{
    // LIFECYCLE: both blocks below capture a strong shared_ptr to this request,
    // extending its lifetime past Cancel()/service-layer teardown until the
    // delegate queue drains and the blocks are released. This makes the blocks
    // safe even when the service drops its ctx->request reference first (which
    // also breaks the ctx<->request reference cycle at the service layer).
    // After Cancel() sets cancelled_, the blocks observe it (atomic) and no-op;
    // Cancel()/FinishSession() invalidate the session, so no further metrics
    // or completion callbacks are delivered. Start() is only called on an
    // object owned by shared_ptr (created via make_shared in NewRequest), so
    // shared_from_this() is valid here.
    std::shared_ptr<CacheHttpRequestIOS> strongSelf = shared_from_this();

    // Session-level delegate is required to collect NSURLSessionTaskMetrics.
    // The session retains its delegate; the delegate only lives to forward
    // metrics into this request. Both the metrics block and the completion
    // handler run on the session's serial delegate queue (delegateQueue:nil),
    // and Apple guarantees didFinishCollectingMetrics precedes the completion
    // handler — CollectMetrics writes are visible to HandleResponse without
    // extra locking.
    metricsDelegate.onTaskMetrics = ^(NSURLSessionTaskMetrics* taskMetrics) {
        if (!strongSelf->cancelled_) {
            strongSelf->CollectMetrics(taskMetrics);
        }
    };
    ConfigureRedirectGuard(metricsDelegate);
    session_ = [NSURLSession sessionWithConfiguration:config
                                              delegate:metricsDelegate
                                         delegateQueue:nil];
    task_ = [session_ dataTaskWithRequest:request
                       completionHandler:^(NSData* data, NSURLResponse* response, NSError* error) {
        if (strongSelf->cancelled_) {
            // Cancel() may have run before session_/task_ were assigned, so
            // its teardown saw nil and invalidated nothing. Break the
            // request→session→delegate→block retain cycle here (no-op if
            // Cancel()/FinishSession() already invalidated the session).
            strongSelf->FinishSession();
            return;
        }
        if (strongSelf->tooManyRedirects_
            && error != nil && error.code == NSURLErrorCancelled) {
            // Redirect limit exceeded and onRedirect cancelled the task.
            // Report it via the normal failure path — HandleResponse maps the
            // cancelled+tooManyRedirects_ case to the OHOS
            // ERROR_TOO_MANY_REDIRECTS code instead of the silent user-cancel
            // drop above.
            strongSelf->HandleResponse(data, response, error, callback);
            return;
        }
        strongSelf->HandleResponse(data, response, error, callback);
    }];

    [task_ resume];
}

// Installs the redirect guard on the metrics delegate: strips sensitive
// headers (Authorization/Proxy-Authorization/Cookie) once the chain leaves
// the original origin (cross-host or https->http downgrade), and caps the
// chain at MAX_REDIRECTS hops — Foundation would otherwise auto-follow an
// A->B->A loop until the resource timeout. Runs on the same serial delegate
// queue as the completion handler.
void CacheHttpRequestIOS::ConfigureRedirectGuard(CacheHttpMetricsDelegate* metricsDelegate)
{
    std::shared_ptr<CacheHttpRequestIOS> strongSelf = shared_from_this();
    NSURL* originalUrl = [NSURL URLWithString:StdStringToNSString(options_.url)];
    metricsDelegate.onRedirect = ^NSURLRequest*(NSURLRequest* newRequest) {
        if (strongSelf->redirectHopCount_ >= MAX_REDIRECTS) {
            strongSelf->tooManyRedirects_ = true;
            return nil; // delegate cancels the task
        }
        strongSelf->redirectHopCount_++;
        NSURL* newUrl = newRequest.URL;
        BOOL crossHost = (newUrl.host == nil || originalUrl.host == nil
            || ![newUrl.host isEqualToString:originalUrl.host]);
        BOOL schemeDowngrade = ([originalUrl.scheme isEqualToString:@"https"]
            && [newUrl.scheme isEqualToString:@"http"]);
        if (!crossHost && !schemeDowngrade) {
            return newRequest; // same origin: follow unchanged
        }
        NSMutableURLRequest* stripped = [newRequest mutableCopy];
        for (NSString* field in @[ @"Authorization", @"Proxy-Authorization", @"Cookie" ]) {
            [stripped setValue:nil forHTTPHeaderField:field];
        }
        return stripped;
    };
}

void CacheHttpRequestIOS::Cancel()
{
    CDLOG_INFO("CacheHttpRequestIOS::Cancel taskId=" CDLOG_FMT_PUBLIC_S, GetBriefTaskId(options_.url).c_str());
    cancelled_ = true;
    // Guard the session_/task_ teardown against a concurrent FinishSession()
    // running on the session delegate queue (HandleResponse calls it there).
    // The lock prevents double-invalidate and racy pointer swaps; the second
    // entrant sees nil pointers and no-ops.
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (task_ != nil) {
        [task_ cancel];
        task_ = nil;
    }
    if (session_ != nil) {
        // Cancel path: abort in-flight work immediately (finishTasks would
        // let a running transfer continue past the cancel request).
        [session_ invalidateAndCancel];
        session_ = nil;
    }
}

// Tears down the delegate-based session after a completed/failed request.
// Invalidation is required for delegate-based sessions: without it the
// session (and the metrics delegate it retains) leaks.
void CacheHttpRequestIOS::FinishSession()
{
    // See Cancel(): the mutex serializes teardown with a concurrent Cancel()
    // on the caller thread.
    std::lock_guard<std::mutex> lock(stateMutex_);
    // Drop the task reference BEFORE invalidating: the task retains the
    // completion handler, which captures a shared_ptr to this request —
    // keeping task_ would create a request→task→block→request retain cycle
    // that leaks the request (and the ctx its block captures) forever.
    // Releasing the block's captures while it is executing is safe: the
    // running block stays alive on the stack until HandleResponse returns.
    task_ = nil;
    if (session_ != nil) {
        [session_ finishTasksAndInvalidate];
        session_ = nil;
    }
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
