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

#include "cache_download_interface.h"
#include "cache_file.h"
#include "cache_task.h"
#include "cache_http_client.h"
#include "cache_log.h"

#include <algorithm>
#include <atomic>
#include <climits>
#include <chrono>
#include <thread>

namespace OHOS::Plugin::CacheDownload {

namespace {
    /** Maximum URL length in bytes. */
    constexpr size_t MAX_URL_LENGTH = 8192;

    /** Maximum resource size after decompression (20 MB). */
    constexpr uint64_t MAX_RESOURCE_SIZE = 20971520;

    /** Minimum interval between network retry checks (1 second). */
    constexpr uint32_t RETRY_CHECK_INTERVAL_SEC = 1;

    /** Global retry/timeout limits. */
    constexpr size_t GLOBAL_MAX_RETRY_LIMIT = 10;
    constexpr uint32_t GLOBAL_MAX_NETWORK_CHECK_TIMEOUT = 20;
    constexpr uint32_t GLOBAL_MIN_HTTP_TOTAL_TIMEOUT = 1;

    /** HTTP status code range. */
    constexpr int HTTP_STATUS_MIN = 100;
    constexpr int HTTP_STATUS_MAX = 600;

    /** Milliseconds per second. */
    static constexpr int32_t MILLIS_PER_SEC = 1000;
} // namespace

// ============================================================
// Download execution context
// ============================================================

struct DownloadContext : public CacheHttpRequestCallback {
    std::string url;
    std::shared_ptr<TaskHandle> handle;
    std::unique_ptr<RamCache> ramCache;
    CacheDownloadService* service = nullptr;
    std::shared_ptr<CacheHttpRequest> request;
    std::shared_ptr<DownloadContext> self;
    CacheDownloadOptions options;
    RetryOptions retryOptions;
    TimeoutOptions timeoutOptions;
    size_t attemptCount = 0;
    std::chrono::steady_clock::time_point startTime;
    int32_t responseCode = 0;
    CacheHttpMetrics metrics;
    // Cached brief task id — computed once per download to avoid repeated
    // full SHA-256 hashing in high-frequency OnData callbacks.
    std::string briefTaskId;

    bool IsFinished() const
    {
        return !handle || handle->IsFinished();
    }

    // CacheHttpRequestCallback implementation
    size_t OnData(const uint8_t* data, size_t size) override
    {
        if (IsFinished() || !ramCache) {
            return 0;
        }
        ramCache->Write(data, size);
        CDLOG_DEBUG("OnData taskId=" CDLOG_FMT_PUBLIC_S " chunk=" CDLOG_FMT_ZU, briefTaskId.c_str(), size);
        return size;
    }

    void OnHeader(const std::string& key, const std::string& value) override
    {
        if (IsFinished()) {
            return;
        }
        if (key == "Content-Length") {
            // Future: pre-allocate RamCache based on Content-Length.
        }
    }

    void OnProgress(uint64_t /* downloaded */, uint64_t /* total */) override
    {
        // Progress is tracked internally but not exposed to ArkTS API in the first version.
    }

    void OnMetrics(const CacheHttpMetrics& m) override
    {
        metrics = m;
    }

    void OnSuccess(const CacheHttpResponseInfo& info) override;
    void OnFailed(int errorCode, const std::string& message) override;
};

// ============================================================
// Singleton
// ============================================================

CacheDownloadService& CacheDownloadService::GetInstance()
{
    static CacheDownloadService instance;
    return instance;
}

CacheDownloadService::CacheDownloadService()
    : downloadInfos_(0)
{
    // Set up background task runner for file writes
    cacheManager_.SetTaskRunner([](std::function<void()> task) {
        std::thread([task = std::move(task)]() {
            task();
        }).detach();
    });
}

// ============================================================
// Initialization
// ============================================================

void CacheDownloadService::Initialize(const std::string& cacheDir)
{
    if (initialized_.load()) {
        return;
    }

    cacheManager_.InitCacheDir(cacheDir);
    cacheManager_.BuildCachedFilesIndex();
    initialized_.store(true);
    CDLOG_DEBUG("CacheDownloadService::Initialize dir=" CDLOG_FMT_PUBLIC_S, GetLastPathDir(cacheDir).c_str());
}

bool CacheDownloadService::IsInitialized() const
{
    return initialized_.load();
}

// ============================================================
// Core API
// ============================================================

void CacheDownloadService::Download(const std::string& url, const CacheDownloadOptions& options)
{
    CDLOG_INFO("Download taskId=" CDLOG_FMT_PUBLIC_S " strategy=" CDLOG_FMT_D,
        GetBriefTaskId(url).c_str(), (int)options.cacheStrategy);
    // Check if already cached with LAZY strategy
    if (options.cacheStrategy == CacheStrategy::LAZY && Contains(url)) {
        // Distinguish RAM-resident vs FILE-only cache hits — matches native
        // services.rs:614 "fetch success" (RAM) vs services.rs:623 "read local
        // success" (file loaded back to RAM).
        if (cacheManager_.HasInRam(url)) {
            CDLOG_INFO("Download cache hit (RAM) taskId=" CDLOG_FMT_PUBLIC_S,
                       GetBriefTaskId(url).c_str());
        } else {
            CDLOG_INFO("Download cache hit (FILE) taskId=" CDLOG_FMT_PUBLIC_S,
                       GetBriefTaskId(url).c_str());
        }
        NotifySuccess(url);
        return;
    }

    // Each non-LAZY-hit download() starts its own task. XTS multi-download tests
    // (e.g. testDownloadSuccess002) call download(url) twice and expect the
    // callback to fire for each call, so we must not collapse them into one.
    // (The previous AddRef "join running task" path was unfinished — refcount
    // was never released — and suppressed the second completion.)
    ExecuteDownload(url, options);
}

void CacheDownloadService::Cancel(const std::string& url)
{
    TaskHandlePtr handle;
    {
        std::lock_guard<std::mutex> lock(tasksMutex_);
        auto it = activeTasks_.find(url);
        if (it != activeTasks_.end()) {
            handle = it->second;
            activeTasks_.erase(it);
        }
    }

    CDLOG_INFO("Cancel taskId=" CDLOG_FMT_PUBLIC_S " found=" CDLOG_FMT_D, GetBriefTaskId(url).c_str(), handle ? 1 : 0);

    if (handle) {
        // Cancel() runs the cancel callback (aborts the HTTP request and resets
        // the context's self-reference). A cancelled task never fires success/
        // error callbacks; erasing activeTasks_ here prevents leaks/hangs.
        // ClearCancelCallback breaks the ctx ↔ handle reference cycle
        // (lambda captures ctx, ctx->handle holds the TaskHandle).
        handle->Cancel();
        handle->ClearCancelCallback();
    }
}

std::shared_ptr<RamCache> CacheDownloadService::Fetch(const std::string& url)
{
    return cacheManager_.Fetch(url);
}

// ============================================================
// Cache size management
// ============================================================

void CacheDownloadService::SetMemoryCacheSize(uint64_t bytes)
{
    cacheManager_.SetRamCacheSize(bytes);
}

void CacheDownloadService::SetFileCacheSize(uint64_t bytes)
{
    cacheManager_.SetFileCacheSize(bytes);
}

// ============================================================
// Download information
// ============================================================

std::optional<DownloadInfo> CacheDownloadService::GetDownloadInfo(const std::string& url)
{
    std::lock_guard<std::mutex> lock(infoMutex_);
    return downloadInfos_.Get(url);
}

void CacheDownloadService::SetDownloadInfoListSize(uint16_t size)
{
    std::lock_guard<std::mutex> lock(infoMutex_);
    infoListSize_ = size;
    downloadInfos_.SetMaxSize(size);
    // OH semantics: a list size of 0 means "store nothing" — drop existing info.
    if (size == 0) {
        downloadInfos_.Clear();
    }
}

// ============================================================
// Cache cleanup
// ============================================================

void CacheDownloadService::ClearMemoryCache()
{
    std::unordered_set<std::string> runningTasks;
    {
        std::lock_guard<std::mutex> lock(tasksMutex_);
        for (const auto& [url, handle] : activeTasks_) {
            if (!handle->IsFinished()) {
                runningTasks.insert(url);
            }
        }
    }
    cacheManager_.ClearMemoryCache(runningTasks);
}

void CacheDownloadService::ClearFileCache()
{
    std::unordered_set<std::string> runningTasks;
    {
        std::lock_guard<std::mutex> lock(tasksMutex_);
        for (const auto& [url, handle] : activeTasks_) {
            if (!handle->IsFinished()) {
                runningTasks.insert(url);
            }
        }
    }
    cacheManager_.ClearFileCache(runningTasks);
}

// ============================================================
// Callback management
// ============================================================

void CacheDownloadService::OnDownloadSuccess(const std::string& url, int64_t token, SuccessCallback callback)
{
    if (!callback) return;
    std::lock_guard<std::mutex> lock(successCbMutex_);
    successCallbacks_[url].emplace_back(token, std::move(callback));
}

void CacheDownloadService::OffDownloadSuccess(const std::string& url, int64_t token)
{
    std::lock_guard<std::mutex> lock(successCbMutex_);
    auto it = successCallbacks_.find(url);
    if (it == successCallbacks_.end()) {
        return;
    }
    if (token == 0) {
        successCallbacks_.erase(it);
        return;
    }
    auto& vec = it->second;
    vec.erase(std::remove_if(vec.begin(), vec.end(),
        [token](const std::pair<int64_t, SuccessCallback>& e) { return e.first == token; }),
        vec.end());
}

void CacheDownloadService::OnDownloadError(const std::string& url, int64_t token, ErrorCallback callback)
{
    if (!callback) return;
    std::lock_guard<std::mutex> lock(errorCbMutex_);
    errorCallbacks_[url].emplace_back(token, std::move(callback));
}

void CacheDownloadService::OffDownloadError(const std::string& url, int64_t token)
{
    std::lock_guard<std::mutex> lock(errorCbMutex_);
    auto it = errorCallbacks_.find(url);
    if (it == errorCallbacks_.end()) {
        return;
    }
    if (token == 0) {
        errorCallbacks_.erase(it);
        return;
    }
    auto& vec = it->second;
    vec.erase(std::remove_if(vec.begin(), vec.end(),
        [token](const std::pair<int64_t, ErrorCallback>& e) { return e.first == token; }),
        vec.end());
}

// ============================================================
// Global configuration
// ============================================================

void CacheDownloadService::SetGlobalRetryOptions(std::optional<size_t> maxRetry)
{
    if (maxRetry.has_value()) {
        globalMaxRetry_.store(std::min(*maxRetry, size_t(GLOBAL_MAX_RETRY_LIMIT)));
    }
}

void CacheDownloadService::SetGlobalTimeoutOptions(std::optional<uint32_t> networkCheckSec,
    std::optional<uint32_t> httpTotalSec)
{
    if (networkCheckSec.has_value()) {
        globalNetworkCheckTimeout_.store(std::min(*networkCheckSec, uint32_t(GLOBAL_MAX_NETWORK_CHECK_TIMEOUT)));
    }
    if (httpTotalSec.has_value()) {
        globalHttpTotalTimeout_.store(std::max(*httpTotalSec, uint32_t(GLOBAL_MIN_HTTP_TOTAL_TIMEOUT)));
    }
}

size_t CacheDownloadService::GetGlobalMaxRetry() const
{
    return globalMaxRetry_.load();
}

uint32_t CacheDownloadService::GetGlobalNetworkCheckTimeout() const
{
    return globalNetworkCheckTimeout_.load();
}

uint32_t CacheDownloadService::GetGlobalHttpTotalTimeout() const
{
    return globalHttpTotalTimeout_.load();
}

// ============================================================
// Internal
// ============================================================

bool CacheDownloadService::Contains(const std::string& url)
{
    return cacheManager_.Contains(url);
}

void CacheDownloadService::Remove(const std::string& url)
{
    cacheManager_.Remove(url);
}

void CacheDownloadService::OnNetworkAvailable()
{
    std::vector<TaskHandlePtr> tasks;
    {
        std::lock_guard<std::mutex> lock(tasksMutex_);
        for (const auto& [url, handle] : activeTasks_) {
            if (!handle->IsFinished()) {
                tasks.push_back(handle);
            }
        }
    }

    // Reset all active tasks to trigger retry
    for (auto& handle : tasks) {
        handle->Reset();
    }
}

CacheManager& CacheDownloadService::GetCacheManager()
{
    return cacheManager_;
}

TaskHandlePtr CacheDownloadService::GetActiveTask(const std::string& url) const
{
    std::lock_guard<std::mutex> lock(tasksMutex_);
    auto it = activeTasks_.find(url);
    if (it == activeTasks_.end()) {
        return nullptr;
    }
    return it->second;
}

// ============================================================
// Task execution
// ============================================================

void CacheDownloadService::ExecuteDownload(const std::string& url,
    const CacheDownloadOptions& options)
{
    // Build a cache key (URL is the primary key)
    std::string taskId = url;

    // Assign a monotonic sequence number — matches native task.rs:90
    //   info!("new task {} seq {}", task_id.brief(), seq)
    // NOTE: native task.rs uses `updater.seq + 1` as a per-task rebuild
    // generation for ABA protection; the port uses a global monotonic counter
    // as a unique task ordinal. Purpose diverges — kept for log correlation.
    static std::atomic<size_t> s_seq{0};
    size_t seq = s_seq.fetch_add(1, std::memory_order_relaxed);
    CDLOG_INFO("new task taskId=" CDLOG_FMT_PUBLIC_S " seq=" CDLOG_FMT_ZU,
               GetBriefTaskId(url).c_str(), seq);

    // Create task handle
    auto handle = std::make_shared<TaskHandle>(taskId, url, cacheManager_);
    handle->state_ = TASK_STATE_RUNNING;
    CDLOG_INFO("ExecuteDownload taskId=" CDLOG_FMT_PUBLIC_S, GetBriefTaskId(url).c_str());

    auto ctx = std::make_shared<DownloadContext>();
    ctx->url = url;
    ctx->briefTaskId = GetBriefTaskId(url);
    ctx->handle = handle;
    ctx->service = this;
    ctx->self = ctx;
    ctx->options = options;
    ctx->retryOptions = EffectiveRetryOptions(options);
    ctx->timeoutOptions = EffectiveTimeoutOptions(options);
    ctx->startTime = std::chrono::steady_clock::now();

    // Set up cancellation callback BEFORE publishing the handle to activeTasks_.
    // Otherwise Cancel() on another thread can read cancelCallback_ concurrently
    // with this write (data race on std::function → UB).
    handle->SetCancelCallback([ctx]() {
        if (ctx->request) {
            ctx->request->Cancel();
        }
        ctx->self.reset();
    });

    {
        std::lock_guard<std::mutex> lock(tasksMutex_);
        activeTasks_[url] = handle;
    }

    StartDownload(ctx);
}

void CacheDownloadService::StartDownload(const std::shared_ptr<DownloadContext>& ctx)
{
    if (!ctx || !ctx->handle || ctx->handle->IsFinished()) {
        return;
    }

    // Create a fresh RamCache for this attempt
    ctx->ramCache = cacheManager_.CreateRamCache(ctx->url);

    auto client = CreateCacheHttpClient();
    CDLOG_DEBUG("StartDownload taskId=" CDLOG_FMT_PUBLIC_S " attempt=" CDLOG_FMT_ZU,
        GetBriefTaskId(ctx->url).c_str(), ctx->attemptCount);
    if (!client) {
        ctx->handle->state_.store(TASK_STATE_FAIL);
        CDLOG_ERROR("StartDownload failed: HTTP client null taskId=" CDLOG_FMT_PUBLIC_S,
            GetBriefTaskId(ctx->url).c_str());
        NotifyError(ctx->url, CacheDownloadError(ErrorCode::OTHERS, -1, "Failed to create HTTP client"));
        ctx->handle->ClearCancelCallback();
        ctx->self.reset();
        return;
    }

    CacheHttpRequestOptions requestOptions;
    requestOptions.url = ctx->url;
    requestOptions.method = "GET";
    requestOptions.headers = ctx->options.headers;
    requestOptions.connectTimeoutMs = ctx->timeoutOptions.networkCheckTimeout * MILLIS_PER_SEC;
    // Cap readTimeoutMs at INT32_MAX — Java HttpURLConnection.setReadTimeout accepts int (signed 32-bit).
    // httpTotalTimeout up to 4294967 * 1000 overflows jint to negative, causing IllegalArgumentException.
    uint32_t readTimeoutMs = ctx->timeoutOptions.httpTotalTimeout * MILLIS_PER_SEC;
    requestOptions.readTimeoutMs = std::min(readTimeoutMs, uint32_t(INT32_MAX));
    requestOptions.sslType = ctx->options.sslType;
    requestOptions.caPath = ctx->options.caPath;

    ctx->request = client->NewRequest(requestOptions, ctx);
    if (!ctx->request) {
        ctx->handle->state_.store(TASK_STATE_FAIL);
        CDLOG_ERROR("StartDownload failed: HTTP request null taskId=" CDLOG_FMT_PUBLIC_S,
            GetBriefTaskId(ctx->url).c_str());
        NotifyError(ctx->url, CacheDownloadError(ErrorCode::OTHERS, -1, "Failed to create HTTP request"));
        ctx->handle->ClearCancelCallback();
        ctx->self.reset();
        return;
    }

    if (!ctx->request->Start()) {
        ctx->handle->state_.store(TASK_STATE_FAIL);
        CDLOG_ERROR("StartDownload failed: HTTP request start failed taskId=" CDLOG_FMT_PUBLIC_S,
            GetBriefTaskId(ctx->url).c_str());
        NotifyError(ctx->url, CacheDownloadError(ErrorCode::OTHERS, -1, "Failed to start HTTP request"));
        ctx->handle->ClearCancelCallback();
        ctx->self.reset();
        return;
    }
}

void CacheDownloadService::RetryDownload(const std::shared_ptr<DownloadContext>& ctx)
{
    if (!ctx || !ctx->handle) {
        return;
    }

    // Reset task state to allow retry
    ctx->handle->Reset();
    ctx->handle->state_.store(TASK_STATE_RUNNING);
    CDLOG_INFO("RetryDownload taskId=" CDLOG_FMT_PUBLIC_S " attempt=" CDLOG_FMT_ZU " maxRetry=" CDLOG_FMT_ZU,
        GetBriefTaskId(ctx->url).c_str(), ctx->attemptCount, ctx->retryOptions.maxRetryCount);

    // Retry after a short delay. In production this should be scheduled
    // asynchronously; for the first version we spawn a detached thread.
    auto self = ctx->self;
    std::thread([self]() {
        std::this_thread::sleep_for(std::chrono::seconds(RETRY_CHECK_INTERVAL_SEC));
        if (self && self->service && !self->IsFinished()) {
            self->service->StartDownload(self);
        }
    }).detach();
}

// Builds a DownloadInfo from captured metrics and computed total time.
DownloadInfo BuildDownloadInfo(int64_t size, const CacheHttpMetrics& metrics,
                               const std::chrono::steady_clock::time_point& startTime)
{
    DownloadInfo info;
    info.resource.size = size;
    info.network.ip = metrics.serverIp;
    info.network.dnsServers = metrics.dnsServers;
    info.performance.dnsTime = metrics.dnsTime;
    info.performance.connectTime = metrics.connectTime;
    info.performance.tlsTime = metrics.tlsTime;
    info.performance.firstSendTime = metrics.firstSendTime;
    info.performance.firstRecvTime = metrics.firstReceiveTime;
    info.performance.redirectTime = metrics.redirectTime;
    double total = metrics.totalTime;
    if (total <= 0.0) {
        auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - startTime).count();
        total = static_cast<double>(elapsedMs);
    }
    info.performance.totalTime = total;
    return info;
}

// Error classification: transport errors (DNS/TCP/SSL/HTTP/OTHERS enum values)
// are retryable; HTTP status codes (100..599) and unknown codes are not.
struct ErrorClassification {
    ErrorCode ec;
    bool isNonRetryable;
    std::string effectiveMessage;
};

ErrorClassification ClassifyDownloadError(int errorCode, const std::string& message)
{
    if (errorCode == static_cast<int>(ErrorCode::DNS) ||
        errorCode == static_cast<int>(ErrorCode::TCP) ||
        errorCode == static_cast<int>(ErrorCode::SSL) ||
        errorCode == static_cast<int>(ErrorCode::HTTP) ||
        errorCode == static_cast<int>(ErrorCode::OTHERS)) {
        return {static_cast<ErrorCode>(errorCode), false, message};
    }
    if (errorCode >= HTTP_STATUS_MIN && errorCode < HTTP_STATUS_MAX) {
        return {ErrorCode::HTTP, true, std::to_string(errorCode)};
    }
    return {ErrorCode::OTHERS, true, message};
}

void DownloadContext::OnSuccess(const CacheHttpResponseInfo& info)
{
    if (IsFinished() || !service || !ramCache) {
        return;
    }

    responseCode = info.statusCode;
    CDLOG_INFO("OnSuccess taskId=" CDLOG_FMT_PUBLIC_S " statusCode=" CDLOG_FMT_D " size=" CDLOG_FMT_ZU,
        GetBriefTaskId(url).c_str(), responseCode, ramCache->Size());
    ramCache->FinishWrite();

    DownloadInfo downloadInfo = BuildDownloadInfo(
        static_cast<int64_t>(ramCache->Size()), metrics, startTime);

    {
        std::lock_guard<std::mutex> lock(service->infoMutex_);
        // OH semantics: a list size of 0 means "do not store".
        if (service->infoListSize_ > 0) {
            service->downloadInfos_.Put(url, std::move(downloadInfo));
            // Matches native info.rs:557: info!("Insert {} info", task_id.brief())
            CDLOG_INFO("InsertDownloadInfo taskId=" CDLOG_FMT_PUBLIC_S,
                       GetBriefTaskId(url).c_str());
        }
    }

    handle->state_.store(TASK_STATE_SUCCESS);
    service->cacheManager_.OnDownloadComplete(std::move(ramCache));
    service->NotifySuccess(url);
    handle->ClearCancelCallback();
    self.reset();
}

void DownloadContext::OnFailed(int errorCode, const std::string& message)
{
    if (IsFinished() || !service) {
        return;
    }

    // The raw message is deliberately NOT logged: transport error texts (e.g.
    // iOS localizedDescription) often embed URL fragments — an indirect URL
    // leak channel. The callback contract still carries the full message; only
    // the log line drops it (matches native, which logs brief id + code only).
    CDLOG_ERROR("OnFailed taskId=" CDLOG_FMT_PUBLIC_S " errorCode=" CDLOG_FMT_D " attempt=" CDLOG_FMT_ZU,
        GetBriefTaskId(url).c_str(), errorCode, attemptCount);

    // Error classification contract with the platform HTTP client:
    //   - HTTP status errors: errorCode = HTTP status (100..599), message = status string.
    //   - Transport errors:   errorCode = ErrorCode enum value (DNS/TCP/SSL/OTHERS).
    // ErrorCode::OTHERS (0xFF = 255) lies inside the HTTP status range, so the
    // enum values MUST be matched before the range check — otherwise every
    // OTHERS transport error is misreported as HTTP (48) and wrongly skips
    // retry (SUB_Request_cacheDownload_ListeningError_0800 expects 255).
    auto cls = ClassifyDownloadError(errorCode, message);
    if (!cls.isNonRetryable && attemptCount < retryOptions.maxRetryCount) {
        attemptCount++;
        CDLOG_INFO("OnFailed retrying taskId=" CDLOG_FMT_PUBLIC_S " attempt=" CDLOG_FMT_ZU,
            GetBriefTaskId(url).c_str(), attemptCount);
        service->RetryDownload(self);
        return;
    }

    handle->state_.store(TASK_STATE_FAIL);

    CacheDownloadError error(cls.ec, errorCode, cls.effectiveMessage);

    DownloadInfo downloadInfo = BuildDownloadInfo(-1, metrics, startTime);

    {
        std::lock_guard<std::mutex> lock(service->infoMutex_);
        if (service->infoListSize_ > 0) {
            service->downloadInfos_.Put(url, std::move(downloadInfo));
            // Matches native info.rs:557: info!("Insert {} info", task_id.brief())
            // Symmetric with OnSuccess — failed downloads also store info.
            CDLOG_INFO("InsertDownloadInfo taskId=" CDLOG_FMT_PUBLIC_S,
                       GetBriefTaskId(url).c_str());
        }
    }

    service->NotifyError(url, error);
    handle->ClearCancelCallback();
    self.reset();
}

void CacheDownloadService::NotifySuccess(const std::string& url)
{
    // Clean up task
    {
        std::lock_guard<std::mutex> lock(tasksMutex_);
        activeTasks_.erase(url);
    }

    // Snapshot the registered callbacks (they PERSIST across downloads — removed
    // only via OffDownloadSuccess — so a callback fires on every completion of
    // its URL, matching XTS multi-download callback tests).
    std::vector<SuccessCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(successCbMutex_);
        auto it = successCallbacks_.find(url);
        if (it != successCallbacks_.end()) {
            for (const auto& entry : it->second) {
                callbacks.push_back(entry.second);
            }
        }
    }
    CDLOG_INFO("NotifySuccess taskId=" CDLOG_FMT_PUBLIC_S " callbacks=" CDLOG_FMT_ZU,
        GetBriefTaskId(url).c_str(), callbacks.size());

    for (auto& cb : callbacks) {
        if (cb) {
            cb();
        }
    }
}

void CacheDownloadService::NotifyError(const std::string& url, const CacheDownloadError& error)
{
    // Clean up task
    {
        std::lock_guard<std::mutex> lock(tasksMutex_);
        activeTasks_.erase(url);
    }

    std::vector<ErrorCallback> callbacks;
    {
        std::lock_guard<std::mutex> lock(errorCbMutex_);
        auto it = errorCallbacks_.find(url);
        if (it != errorCallbacks_.end()) {
            for (const auto& entry : it->second) {
                callbacks.push_back(entry.second);
            }
        }
    }
    CDLOG_INFO("NotifyError taskId=" CDLOG_FMT_PUBLIC_S " errorCode=" CDLOG_FMT_D " callbacks=" CDLOG_FMT_ZU,
        GetBriefTaskId(url).c_str(), (int)error.errorCode, callbacks.size());

    for (auto& cb : callbacks) {
        if (cb) {
            cb(error);
        }
    }
}

RetryOptions CacheDownloadService::EffectiveRetryOptions(const CacheDownloadOptions& options) const
{
    if (options.retry.has_value()) {
        return *options.retry;
    }
    return RetryOptions{globalMaxRetry_.load()};
}

TimeoutOptions CacheDownloadService::EffectiveTimeoutOptions(const CacheDownloadOptions& options) const
{
    if (options.timeout.has_value()) {
        return *options.timeout;
    }
    return TimeoutOptions{
        globalNetworkCheckTimeout_.load(),
        globalHttpTotalTimeout_.load()
    };
}

} // namespace OHOS::Plugin::CacheDownload
