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

#ifndef PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_TYPES_H
#define PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_TYPES_H

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace OHOS::Plugin::CacheDownload {

// ============================================================
// Enums matching ArkTS definitions
// ============================================================

/** Secure communication protocol. */
enum class SslType : uint8_t {
    TLS  = 0,
    TLCP = 1,
};

/** Cache update strategies. */
enum class CacheStrategy : uint8_t {
    FORCE = 0,  // Force update, regardless of existing cache
    LAZY  = 1,  // Update only when cache does not exist
};

/** Task state machine states. */
enum class TaskState : uint8_t {
    INIT    = 0,
    RUNNING = 1,
    SUCCESS = 2,
    FAIL    = 3,
    CANCEL  = 4,
};

// ============================================================
// Configuration options
// ============================================================

/** Task retry configuration. */
struct RetryOptions {
    /** Maximum number of retry attempts. Default: 1, Range: [0, 10]. */
    size_t maxRetryCount = 1;
};

/** Task timeout configuration. */
struct TimeoutOptions {
    /** Network availability check timeout in seconds. Default: 20, Range: [0, 20]. */
    uint32_t networkCheckTimeout = 20;
    /** Complete HTTP request-response cycle timeout in seconds. Default: 60, Min: 1. */
    uint32_t httpTotalTimeout = 60;
};

/**
 * Provides configuration options for download and cache.
 * Matches the ArkTS CacheDownloadOptions interface.
 */
struct CacheDownloadOptions {
    /** HTTP request headers. */
    std::map<std::string, std::string> headers;
    /** Secure communication protocol. Default: TLS. */
    SslType sslType = SslType::TLS;
    /** CA certificate path (.pem format). Empty = use system default. */
    std::string caPath;
    /** Cache update strategy. Default: FORCE. */
    CacheStrategy cacheStrategy = CacheStrategy::FORCE;
    /** Per-task retry configuration. */
    std::optional<RetryOptions> retry;
    /** Per-task timeout configuration. */
    std::optional<TimeoutOptions> timeout;
};

// ============================================================
// Download information (returned by getDownloadInfo)
// ============================================================

/** Pre-downloaded resource information. */
struct ResourceInfo {
    /** Size of resource after decompression in bytes. -1 = download failed. */
    int64_t size = -1;
};

/** Pre-downloaded network information. */
struct NetworkInfo {
    /** DNS servers used for downloading. */
    std::vector<std::string> dnsServers;
    /** IP address of the URL. Empty if DNS resolution failed. */
    std::string ip;
};

/** Pre-downloaded performance information. All times in milliseconds. */
struct PerformanceInfo {
    double dnsTime         = 0.0;
    double connectTime     = 0.0;
    double tlsTime         = 0.0;
    double firstSendTime   = 0.0;
    double firstRecvTime   = 0.0;
    double totalTime       = 0.0;
    double redirectTime    = 0.0;
};

/** Pre-downloaded download information. */
struct DownloadInfo {
    ResourceInfo resource;
    NetworkInfo network;
    PerformanceInfo performance;
};

} // namespace OHOS::Plugin::CacheDownload

#endif // PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_TYPES_H
