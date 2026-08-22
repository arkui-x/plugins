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

#ifndef PLUGINS_REQUEST_CACHE_DOWNLOAD_INNERKITS_CACHE_DOWNLOAD_CLIENT_H
#define PLUGINS_REQUEST_CACHE_DOWNLOAD_INNERKITS_CACHE_DOWNLOAD_CLIENT_H

#include "cache_error.h"
#include "cache_ram.h"
#include "cache_types.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace OHOS::Plugin::CacheDownload {

/**
 * Public alias for request options.
 * Defined internally as CacheDownloadOptions (URL, headers, sslType, caPath,
 * cacheStrategy, retry, timeout).
 */
using CacheDownloadRequestOptions = CacheDownloadOptions;

/**
 * Running state of a cache-download task.
 */
enum class CacheDownloadState {
    INIT,
    RUNNING,
    SUCCESS,
    FAIL,
    CANCEL,
};

/**
 * Event callbacks for a cache-download task.
 *
 * Note: onCancel and onProgress are reserved in the first version and are not
 * triggered yet because CacheDownloadService currently only exposes success/error
 * callbacks through the NAPI surface.
 */
struct CacheDownloadCallback {
    /** Called when the download completes successfully. */
    std::function<void(const std::string& url)> onSuccess;
    /** Called when the download fails. */
    std::function<void(const std::string& url, const CacheDownloadError& error)> onError;
    /** Reserved: called when the download is cancelled. */
    std::function<void(const std::string& url)> onCancel;
    /** Reserved: called periodically with download progress. */
    std::function<void(const std::string& url, uint64_t downloaded, uint64_t total)> onProgress;
};

/**
 * Lightweight handle to an active cache-download task.
 */
class CacheDownloadTaskHandle {
public:
    ~CacheDownloadTaskHandle();

    /** Cancels the task if it is still active. */
    void Cancel();

    /** Returns the task URL. */
    const std::string& GetUrl() const;

    /** Returns true if the task has reached a terminal state. */
    bool IsFinished() const;

    /** Returns the current task state. */
    CacheDownloadState GetState() const;

private:
    friend class CacheDownloadClient;
    CacheDownloadTaskHandle(const std::string& url, std::weak_ptr<void> taskToken);

    std::string url_;
    std::weak_ptr<void> taskToken_;
};

/**
 * Public C++ innerkits client for cacheDownload.
 *
 * Internal components such as Image can use this singleton to preload resources,
 * query cache state, and synchronously fetch cached data.
 */
class CacheDownloadClient {
public:
    CacheDownloadClient(const CacheDownloadClient&) = delete;
    CacheDownloadClient& operator=(const CacheDownloadClient&) = delete;
    CacheDownloadClient(CacheDownloadClient&&) = delete;
    CacheDownloadClient& operator=(CacheDownloadClient&&) = delete;

    /** Returns the singleton instance. */
    static CacheDownloadClient& GetInstance();

    /**
     * Starts a cache download for the given URL.
     *
     * @param url Resource URL.
     * @param callback Optional event callbacks.
     * @param options Optional request configuration.
     * @return Task handle. The handle may be empty if the task finishes
     *         immediately due to a cache hit with LAZY strategy.
     */
    std::shared_ptr<CacheDownloadTaskHandle> Download(
        const std::string& url,
        std::shared_ptr<CacheDownloadCallback> callback = nullptr,
        const CacheDownloadRequestOptions& options = {});

    /** Cancels the active task for the given URL. */
    void Cancel(const std::string& url);

    /** Removes the cache entry and any active task for the given URL. */
    void Remove(const std::string& url);

    /** Returns true if the URL is cached or has an active task. */
    bool Contains(const std::string& url);

    /**
     * Fetches cached data for the given URL.
     * @return Shared pointer to the RAM cache, or nullptr if not cached.
     */
    std::shared_ptr<RamCache> Fetch(const std::string& url);

    /**
     * Fetches cached data as a byte vector.
     * @return The cached bytes, or an empty vector if not cached.
     */
    std::vector<uint8_t> FetchBytes(const std::string& url);

    /** Gets download statistics for the given URL if available. */
    std::optional<DownloadInfo> GetDownloadInfo(const std::string& url);

    /** Sets the RAM cache size limit. */
    void SetMemoryCacheSize(uint64_t bytes);

    /** Sets the file cache size limit. */
    void SetFileCacheSize(uint64_t bytes);

    /** Sets the maximum number of entries in the download info list. */
    void SetDownloadInfoListSize(uint16_t size);

    /** Sets the file cache directory. Must be called before first use. */
    void SetFileCachePath(const std::string& path);

    /** Clears all memory-cached data. */
    void ClearMemoryCache();

    /** Clears all file-cached data. */
    void ClearFileCache();

    /** Sets the global retry policy. */
    void SetGlobalRetryOptions(const RetryOptions& options);

    /** Sets the global timeout policy. */
    void SetGlobalTimeoutOptions(const TimeoutOptions& options);

private:
    CacheDownloadClient();
    ~CacheDownloadClient() = default;
};

} // namespace OHOS::Plugin::CacheDownload

#endif // PLUGINS_REQUEST_CACHE_DOWNLOAD_INNERKITS_CACHE_DOWNLOAD_CLIENT_H
