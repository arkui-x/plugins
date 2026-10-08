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

#ifndef PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_DOWNLOAD_INTERFACE_H
#define PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_DOWNLOAD_INTERFACE_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "cache_download_callback.h"
#include "cache_error.h"
#include "cache_manager.h"
#include "cache_task.h"
#include "cache_types.h"

namespace OHOS::Plugin::CacheDownload {

struct DownloadContext; // Defined in cache_download_service.cpp

/**
 * Singleton service that manages cache download operations.
 *
 * Provides the public API matching the ArkTS cacheDownload namespace:
 * - download() / cancel()
 * - setMemoryCacheSize() / setFileCacheSize()
 * - getDownloadInfo() / setDownloadInfoListSize()
 * - clearMemoryCache() / clearFileCache()
 * - onDownloadSuccess() / offDownloadSuccess()
 * - onDownloadError() / offDownloadError()
 * - setGlobalRetryOptions() / setGlobalTimeoutOptions()
 *
 * Ported from: request_request/.../cache_download/src/services.rs → CacheDownloadService
 */
class CacheDownloadService {
    // DownloadContext (defined in cache_download_service.cpp) drives task
    // execution and needs access to internal state.
    friend struct DownloadContext;
public:
    /** Returns the singleton instance. */
    static CacheDownloadService& GetInstance();

    /**
     * Initializes the service. Must be called once before use.
     * @param cacheDir Directory for file cache storage.
     */
    void Initialize(const std::string& cacheDir);

    /** Returns true if the service has been initialized. */
    bool IsInitialized() const;

    // ============================================================
    // Core API (matches ArkTS cacheDownload namespace)
    // ============================================================

    /**
     * Downloads a resource from the given URL and caches it.
     * Matches: cacheDownload.download(url, options): void
     *
     * Fire-and-forget: the result is delivered via onDownloadSuccess/Error callbacks.
     * If the URL is already cached (and cacheStrategy is not FORCE), the success
     * callback fires immediately.
     *
     * @param url Target resource URL (HTTP/HTTPS, max 8192 bytes).
     * @param options Download and cache configuration.
     */
    void Download(const std::string& url, const CacheDownloadOptions& options);

    /**
     * Cancels an ongoing download task by URL.
     * Matches: cacheDownload.cancel(url): void
     * Already-cached data is not affected.
     */
    void Cancel(const std::string& url);

    /**
     * Retrieves cached data for a URL (internal use by ArkUI components).
     * Not directly exposed in the ArkTS API but used by Image component internally.
     * @return Pointer to cached data, or nullptr if not cached.
     */
    std::shared_ptr<RamCache> Fetch(const std::string& url);

    // ============================================================
    // Cache size management
    // ============================================================

    /** Sets the RAM cache size limit. 0 = clear all. Max 1GB. */
    void SetMemoryCacheSize(uint64_t bytes);

    /** Sets the file cache size limit. 0 = delete all files. Max 4GB. */
    void SetFileCacheSize(uint64_t bytes);

    // ============================================================
    // Download information
    // ============================================================

    /**
     * Gets download information for a URL.
     * Matches: cacheDownload.getDownloadInfo(url): DownloadInfo | undefined
     * Requires GET_NETWORK_INFO permission (checked at platform layer).
     */
    std::optional<DownloadInfo> GetDownloadInfo(const std::string& url);

    /** Sets the download information list size. Range: [0, 8192]. */
    void SetDownloadInfoListSize(uint16_t size);

    // ============================================================
    // Cache cleanup
    // ============================================================

    /** Clears all memory-cached data. */
    void ClearMemoryCache();

    /** Clears all file-cached data. */
    void ClearFileCache();

    // ============================================================
    // Callback management (matches ArkTS on/off pattern)
    // ============================================================

    /**
     * Subscribes to pre-download completion events for a URL.
     * Matches: cacheDownload.onDownloadSuccess(url, callback)
     * @param token Caller-assigned identity for the callback, used by
     *              OffDownloadSuccess to remove this specific callback.
     */
    void OnDownloadSuccess(const std::string& url, int64_t token, SuccessCallback callback);

    /**
     * Unsubscribes from pre-download completion events.
     * Matches: cacheDownload.offDownloadSuccess(url, callback?)
     * @param token If 0, removes ALL success callbacks for the URL; otherwise
     *              removes only the callback registered with this token.
     */
    void OffDownloadSuccess(const std::string& url, int64_t token = 0);

    /**
     * Subscribes to pre-download error events for a URL.
     * Matches: cacheDownload.onDownloadError(url, callback)
     * @param token Caller-assigned identity for the callback.
     */
    void OnDownloadError(const std::string& url, int64_t token, ErrorCallback callback);

    /**
     * Unsubscribes from pre-download error events.
     * Matches: cacheDownload.offDownloadError(url, callback?)
     * @param token If 0, removes ALL error callbacks for the URL; otherwise
     *              removes only the callback registered with this token.
     */
    void OffDownloadError(const std::string& url, int64_t token = 0);

    // ============================================================
    // Global configuration
    // ============================================================

    /**
     * Sets global retry options for all tasks.
     * Used when per-task retry config is not set.
     * Matches: cacheDownload.setGlobalRetryOptions(options?)
     */
    void SetGlobalRetryOptions(std::optional<size_t> maxRetry);

    /**
     * Sets global timeout options for all tasks.
     * Used when per-task timeout config is not set.
     * Matches: cacheDownload.setGlobalTimeoutOptions(options?)
     */
    void SetGlobalTimeoutOptions(std::optional<uint32_t> networkCheckSec,
                                  std::optional<uint32_t> httpTotalSec);

    /** Returns the global max retry count. */
    size_t GetGlobalMaxRetry() const;

    /** Returns the global network check timeout in seconds. */
    uint32_t GetGlobalNetworkCheckTimeout() const;

    /** Returns the global HTTP total timeout in seconds. */
    uint32_t GetGlobalHttpTotalTimeout() const;

    // ============================================================
    // Internal
    // ============================================================

    /**
     * Checks if a URL exists in any cache tier.
     */
    bool Contains(const std::string& url);

    /**
     * Removes a URL from all cache tiers and deletes its cached file.
     */
    void Remove(const std::string& url);

    /**
     * Called by NetObserver when network connectivity is restored.
     * Resets all active tasks to trigger retry.
     */
    void OnNetworkAvailable();

    /**
     * Returns the active task handle for the URL, or nullptr if none.
     */
    TaskHandlePtr GetActiveTask(const std::string& url) const;

    /** Returns the CacheManager reference (for internal components). */
    CacheManager& GetCacheManager();

private:
    CacheDownloadService();
    ~CacheDownloadService() = default;
    CacheDownloadService(const CacheDownloadService&) = delete;
    CacheDownloadService& operator=(const CacheDownloadService&) = delete;

    // ============================================================
    // Task execution
    // ============================================================

    /** Executes an HTTP download for the given URL. */
    void ExecuteDownload(const std::string& url, const CacheDownloadOptions& options);

    /** Notifies all registered success callbacks for a URL. */
    void NotifySuccess(const std::string& url);

    /** Notifies all registered error callbacks for a URL. */
    void NotifyError(const std::string& url, const CacheDownloadError& error);

    /** Starts a download attempt using the given context. */
    void StartDownload(const std::shared_ptr<DownloadContext>& ctx);

    /** Retries a failed download attempt. */
    void RetryDownload(const std::shared_ptr<DownloadContext>& ctx);

    /** Applies the effective retry/timeout config (task → global → default). */
    RetryOptions EffectiveRetryOptions(const CacheDownloadOptions& options) const;
    TimeoutOptions EffectiveTimeoutOptions(const CacheDownloadOptions& options) const;

    // ============================================================
    // Members
    // ============================================================

    std::atomic<bool> initialized_{false};

    // Cache engine
    CacheManager cacheManager_;

    // Active tasks (URL → TaskHandle)
    mutable std::mutex tasksMutex_;
    std::unordered_map<std::string, TaskHandlePtr> activeTasks_;

    // Success callbacks per URL (token, callback). Persist across downloads
    // (removed only via OffDownloadSuccess) so a callback fires on every
    // completion of its URL — matching XTS multi-download callback tests.
    mutable std::mutex successCbMutex_;
    std::unordered_map<std::string, std::vector<std::pair<int64_t, SuccessCallback>>> successCallbacks_;

    // Error callbacks per URL (token, callback).
    mutable std::mutex errorCbMutex_;
    std::unordered_map<std::string, std::vector<std::pair<int64_t, ErrorCallback>>> errorCallbacks_;

    // Download info storage (LRU, URL → DownloadInfo)
    mutable std::mutex infoMutex_;
    LRUCache<DownloadInfo> downloadInfos_;
    uint16_t infoListSize_{0};

    // Global defaults
    std::atomic<size_t> globalMaxRetry_{1};
    std::atomic<uint32_t> globalNetworkCheckTimeout_{20};
    std::atomic<uint32_t> globalHttpTotalTimeout_{60};
};

} // namespace OHOS::Plugin::CacheDownload

#endif // PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_DOWNLOAD_INTERFACE_H
