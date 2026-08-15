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

#include "cache_download_client.h"

#include "cache_download_interface.h"
#include "cache_error.h"
#include "cache_ram.h"
#include "cache_task.h"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace OHOS::Plugin::CacheDownload {

namespace {

constexpr size_t MAX_RETRY_LIMIT = 10;
constexpr uint32_t MAX_NETWORK_CHECK_TIMEOUT_SEC = 20;
constexpr uint32_t MIN_HTTP_TOTAL_TIMEOUT_SEC = 1;

// Unique token generator for one-off callbacks registered by this client.
// Starts above JNI's token range to avoid collision.
std::atomic<int64_t> g_nextClientToken{100000};

CacheDownloadState ConvertTaskState(TaskState state)
{
    switch (state) {
        case TaskState::INIT:
            return CacheDownloadState::INIT;
        case TaskState::RUNNING:
            return CacheDownloadState::RUNNING;
        case TaskState::SUCCESS:
            return CacheDownloadState::SUCCESS;
        case TaskState::FAIL:
            return CacheDownloadState::FAIL;
        case TaskState::CANCEL:
            return CacheDownloadState::CANCEL;
        default:
            return CacheDownloadState::INIT;
    }
}

} // namespace

// ============================================================
// Task handle
// ============================================================

CacheDownloadTaskHandle::CacheDownloadTaskHandle(const std::string& url,
    std::weak_ptr<void> taskToken)
    : url_(url), taskToken_(taskToken)
{
}

CacheDownloadTaskHandle::~CacheDownloadTaskHandle() = default;

void CacheDownloadTaskHandle::Cancel()
{
    auto sp = std::static_pointer_cast<TaskHandle>(taskToken_.lock());
    if (sp != nullptr) {
        sp->Cancel();
    }
}

const std::string& CacheDownloadTaskHandle::GetUrl() const
{
    return url_;
}

bool CacheDownloadTaskHandle::IsFinished() const
{
    auto sp = std::static_pointer_cast<TaskHandle>(taskToken_.lock());
    if (sp == nullptr) {
        return true;
    }
    return sp->IsFinished();
}

CacheDownloadState CacheDownloadTaskHandle::GetState() const
{
    auto sp = std::static_pointer_cast<TaskHandle>(taskToken_.lock());
    if (sp == nullptr) {
        return CacheDownloadState::SUCCESS;
    }
    return ConvertTaskState(sp->GetState());
}

// ============================================================
// Client
// ============================================================

CacheDownloadClient& CacheDownloadClient::GetInstance()
{
    static CacheDownloadClient instance;
    return instance;
}

CacheDownloadClient::CacheDownloadClient() = default;

std::shared_ptr<CacheDownloadTaskHandle> CacheDownloadClient::Download(
    const std::string& url,
    std::shared_ptr<CacheDownloadCallback> callback,
    const CacheDownloadRequestOptions& options)
{
    auto& service = CacheDownloadService::GetInstance();

    // Register one-off callbacks before starting the download.
    // Each call gets a unique token so repeated Download() for the same URL
    // does not collide. The callback auto-removes itself after firing to
    // prevent accumulation (one-off semantics).
    if (callback != nullptr) {
        if (callback->onSuccess) {
            int64_t token = g_nextClientToken.fetch_add(1);
            service.OnDownloadSuccess(url, token, [cb = callback->onSuccess, url, token, &service]() {
                cb(url);
                service.OffDownloadSuccess(url, token);
            });
        }
        if (callback->onError) {
            int64_t token = g_nextClientToken.fetch_add(1);
            service.OnDownloadError(url, token,
                [cb = callback->onError, url, token, &service](const CacheDownloadError& error) {
                    cb(url, error);
                    service.OffDownloadError(url, token);
                });
        }
    }

    service.Download(url, options);

    // If the task was created, return a lightweight handle.
    auto task = service.GetActiveTask(url);
    if (task == nullptr) {
        return nullptr;
    }
    return std::shared_ptr<CacheDownloadTaskHandle>(
        new CacheDownloadTaskHandle(url, std::static_pointer_cast<void>(task)));
}

void CacheDownloadClient::Cancel(const std::string& url)
{
    CacheDownloadService::GetInstance().Cancel(url);
}

void CacheDownloadClient::Remove(const std::string& url)
{
    CacheDownloadService::GetInstance().Remove(url);
}

bool CacheDownloadClient::Contains(const std::string& url)
{
    return CacheDownloadService::GetInstance().Contains(url);
}

std::shared_ptr<RamCache> CacheDownloadClient::Fetch(const std::string& url)
{
    return CacheDownloadService::GetInstance().Fetch(url);
}

std::vector<uint8_t> CacheDownloadClient::FetchBytes(const std::string& url)
{
    auto cache = Fetch(url);
    if (cache == nullptr || cache->Data() == nullptr || cache->Size() == 0) {
        return {};
    }
    const uint8_t* data = cache->Data();
    return std::vector<uint8_t>(data, data + cache->Size());
}

std::optional<DownloadInfo> CacheDownloadClient::GetDownloadInfo(const std::string& url)
{
    return CacheDownloadService::GetInstance().GetDownloadInfo(url);
}

void CacheDownloadClient::SetMemoryCacheSize(uint64_t bytes)
{
    CacheDownloadService::GetInstance().SetMemoryCacheSize(bytes);
}

void CacheDownloadClient::SetFileCacheSize(uint64_t bytes)
{
    CacheDownloadService::GetInstance().SetFileCacheSize(bytes);
}

void CacheDownloadClient::SetDownloadInfoListSize(uint16_t size)
{
    CacheDownloadService::GetInstance().SetDownloadInfoListSize(size);
}

void CacheDownloadClient::SetFileCachePath(const std::string& path)
{
    CacheDownloadService::GetInstance().Initialize(path);
}

void CacheDownloadClient::ClearMemoryCache()
{
    CacheDownloadService::GetInstance().ClearMemoryCache();
}

void CacheDownloadClient::ClearFileCache()
{
    CacheDownloadService::GetInstance().ClearFileCache();
}

void CacheDownloadClient::SetGlobalRetryOptions(const RetryOptions& options)
{
    CacheDownloadService::GetInstance().SetGlobalRetryOptions(
        std::min(options.maxRetryCount, MAX_RETRY_LIMIT));
}

void CacheDownloadClient::SetGlobalTimeoutOptions(const TimeoutOptions& options)
{
    CacheDownloadService::GetInstance().SetGlobalTimeoutOptions(
        std::min(options.networkCheckTimeout, MAX_NETWORK_CHECK_TIMEOUT_SEC),
        std::max(options.httpTotalTimeout, MIN_HTTP_TOTAL_TIMEOUT_SEC));
}

} // namespace OHOS::Plugin::CacheDownload
