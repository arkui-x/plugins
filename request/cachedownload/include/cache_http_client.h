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

#ifndef PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_HTTP_CLIENT_H
#define PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_HTTP_CLIENT_H

#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cache_types.h"

namespace OHOS::Plugin::CacheDownload {

/**
 * Request options for the cache-download HTTP client.
 *
 * This is a deliberately small subset of HTTP features needed by cacheDownload:
 * URL, method, headers, timeouts, sslType, and optional CA path. It does not
 * depend on plugins/net/http or netstack types.
 */
struct CacheHttpRequestOptions {
    /** Request URL. */
    std::string url;

    /** HTTP method (e.g. "GET"). */
    std::string method = "GET";

    /** Request headers. */
    std::map<std::string, std::string> headers;

    /** Connect timeout in milliseconds. */
    uint32_t connectTimeoutMs = 20000;

    /** Read / total request timeout in milliseconds. */
    uint32_t readTimeoutMs = 60000;

    /** Secure communication protocol. Default: TLS. */
    SslType sslType = SslType::TLS;

    /** Optional CA certificate path. Empty = use system defaults. */
    std::string caPath;
};

/**
 * Response metadata delivered on success.
 */
struct CacheHttpResponseInfo {
    /** HTTP status code. */
    int statusCode = 0;

    /** Response headers. */
    std::map<std::string, std::string> headers;
};

/**
 * Connection/transport metrics collected during a request.
 *
 * Populated by the platform HTTP client (e.g. from NSURLSessionTaskMetrics on
 * iOS) and delivered via OnMetrics before OnSuccess/OnFailed. Used to fill the
 * DownloadInfo performance/network fields expected by XTS.
 */
struct CacheHttpMetrics {
    /** Resolved server IP (empty if DNS/connection failed). */
    std::string serverIp;
    /** DNS servers consulted (may be empty). */
    std::vector<std::string> dnsServers;
    /** DNS resolution time (ms). */
    double dnsTime = 0.0;
    /** TCP connect time (ms). */
    double connectTime = 0.0;
    /** TLS handshake time (ms, 0 for plain HTTP). */
    double tlsTime = 0.0;
    /** Time from request start to request fully sent (ms). */
    double firstSendTime = 0.0;
    /** Time from request start to first response byte (ms). */
    double firstReceiveTime = 0.0;
    /** Total request time (ms). */
    double totalTime = 0.0;
    /** Time spent in redirects (ms). */
    double redirectTime = 0.0;
    /** Best-effort captured HTTP status code (0 if none received). */
    int statusCode = 0;
};

/**
 * Callback interface for an active HTTP request.
 *
 * The platform-specific CacheHttpRequest implementation invokes these methods
 * on arbitrary background threads. Implementations must be thread-safe and
 * must return quickly; heavy work should be offloaded.
 */
class CacheHttpRequestCallback {
public:
    virtual ~CacheHttpRequestCallback() = default;

    /**
     * Called when a chunk of response body data is received.
     * @return Number of bytes consumed. Returning 0 aborts the download.
     */
    virtual size_t OnData(const uint8_t* data, size_t size) = 0;

    /** Called for each response header line. */
    virtual void OnHeader(const std::string& key, const std::string& value) = 0;

    /** Called periodically with download progress. */
    virtual void OnProgress(uint64_t downloaded, uint64_t total) = 0;

    /**
     * Called when transport metrics become available (before OnSuccess/OnFailed).
     * Default no-op so platform clients that don't collect metrics are unaffected.
     */
    virtual void OnMetrics(const CacheHttpMetrics& /* metrics */) {}

    /** Called when the request completes with an HTTP success response. */
    virtual void OnSuccess(const CacheHttpResponseInfo& info) = 0;

    /** Called when the request fails at the network/transport layer. */
    virtual void OnFailed(int errorCode, const std::string& message) = 0;
};

/**
 * Handle to an in-flight HTTP request.
 */
class CacheHttpRequest {
public:
    virtual ~CacheHttpRequest() = default;

    /** Starts the request asynchronously. */
    virtual bool Start() = 0;

    /** Cancels the request. Already-received data remains in the cache. */
    virtual void Cancel() = 0;
};

/**
 * Platform-agnostic HTTP client used by CacheDownloadService.
 */
class CacheHttpClient {
public:
    virtual ~CacheHttpClient() = default;

    /**
     * Creates a new HTTP request.
     *
     * The request does not start until Start() is called. The callback must
     * outlive the request (a shared_ptr is recommended).
     */
    virtual std::shared_ptr<CacheHttpRequest> NewRequest(
        const CacheHttpRequestOptions& options,
        const std::shared_ptr<CacheHttpRequestCallback>& callback) = 0;
};

/**
 * Creates the platform-specific HTTP client.
 *
 * Returns a client backed by HttpURLConnection via JNI, or by NSURLSession,
 * depending on the target platform.
 */
std::shared_ptr<CacheHttpClient> CreateCacheHttpClient();

} // namespace OHOS::Plugin::CacheDownload

#endif // PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_HTTP_CLIENT_H
