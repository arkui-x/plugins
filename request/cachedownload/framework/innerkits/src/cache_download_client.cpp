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
#include "cache_log.h"

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
        CDLOG_DEBUG("TaskHandle::Cancel taskId=" CDLOG_FMT_PUBLIC_S, GetBriefTaskId(url_).c_str());
        sp->Cancel();
    } else {
        CDLOG_DEBUG("TaskHandle::Cancel taskId=" CDLOG_FMT_PUBLIC_S " already finished", GetBriefTaskId(url_).c_str());
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
    CDLOG_INFO("CacheDownloadClient::Download taskId=" CDLOG_FMT_PUBLIC_S " strategy=" CDLOG_FMT_D,
        GetBriefTaskId(url).c_str(), (int)options.cacheStrategy);

    // If the task was created, return a lightweight handle.
    auto task = service.GetActiveTask(url);
    if (task == nullptr) {
        CDLOG_WARN("CacheDownloadClient::Download taskId=" CDLOG_FMT_PUBLIC_S " no active task",
            GetBriefTaskId(url).c_str());
        return nullptr;
    }
    return std::shared_ptr<CacheDownloadTaskHandle>(
        new CacheDownloadTaskHandle(url, std::static_pointer_cast<void>(task)));
}

void CacheDownloadClient::Cancel(const std::string& url)
{
    CDLOG_INFO("CacheDownloadClient::Cancel taskId=" CDLOG_FMT_PUBLIC_S, GetBriefTaskId(url).c_str());
    CacheDownloadService::GetInstance().Cancel(url);
}

void CacheDownloadClient::Remove(const std::string& url)
{
    CDLOG_INFO("CacheDownloadClient::Remove taskId=" CDLOG_FMT_PUBLIC_S, GetBriefTaskId(url).c_str());
    CacheDownloadService::GetInstance().Remove(url);
}

bool CacheDownloadClient::Contains(const std::string& url)
{
    bool found = CacheDownloadService::GetInstance().Contains(url);
    CDLOG_DEBUG("CacheDownloadClient::Contains taskId=" CDLOG_FMT_PUBLIC_S " found=" CDLOG_FMT_D,
        GetBriefTaskId(url).c_str(), (int)found);
    return found;
}

std::shared_ptr<RamCache> CacheDownloadClient::Fetch(const std::string& url)
{
    auto cache = CacheDownloadService::GetInstance().Fetch(url);
    if (cache != nullptr) {
        CDLOG_DEBUG("CacheDownloadClient::Fetch taskId=" CDLOG_FMT_PUBLIC_S " hit size=" CDLOG_FMT_ZU,
            GetBriefTaskId(url).c_str(), cache->Size());
    } else {
        CDLOG_DEBUG("CacheDownloadClient::Fetch taskId=" CDLOG_FMT_PUBLIC_S " miss", GetBriefTaskId(url).c_str());
    }
    return cache;
}

std::vector<uint8_t> CacheDownloadClient::FetchBytes(const std::string& url)
{
    auto cache = Fetch(url);
    if (cache == nullptr || cache->Data() == nullptr || cache->Size() == 0) {
        CDLOG_WARN("CacheDownloadClient::FetchBytes empty taskId=" CDLOG_FMT_PUBLIC_S, GetBriefTaskId(url).c_str());
        return {};
    }
    const uint8_t* data = cache->Data();
    CDLOG_INFO("CacheDownloadClient::FetchBytes taskId=" CDLOG_FMT_PUBLIC_S " size=" CDLOG_FMT_ZU,
        GetBriefTaskId(url).c_str(), cache->Size());
    return std::vector<uint8_t>(data, data + cache->Size());
}

std::optional<DownloadInfo> CacheDownloadClient::GetDownloadInfo(const std::string& url)
{
    auto info = CacheDownloadService::GetInstance().GetDownloadInfo(url);
    CDLOG_DEBUG("CacheDownloadClient::GetDownloadInfo taskId=" CDLOG_FMT_PUBLIC_S " found=" CDLOG_FMT_D,
        GetBriefTaskId(url).c_str(), (int)info.has_value());
    return info;
}

void CacheDownloadClient::SetMemoryCacheSize(uint64_t bytes)
{
    CDLOG_INFO("CacheDownloadClient::SetMemoryCacheSize bytes=" CDLOG_FMT_LLU, bytes);
    CacheDownloadService::GetInstance().SetMemoryCacheSize(bytes);
}

void CacheDownloadClient::SetFileCacheSize(uint64_t bytes)
{
    CDLOG_INFO("CacheDownloadClient::SetFileCacheSize bytes=" CDLOG_FMT_LLU, bytes);
    CacheDownloadService::GetInstance().SetFileCacheSize(bytes);
}

void CacheDownloadClient::SetDownloadInfoListSize(uint16_t size)
{
    CDLOG_INFO("CacheDownloadClient::SetDownloadInfoListSize size=" CDLOG_FMT_D, (int)size);
    CacheDownloadService::GetInstance().SetDownloadInfoListSize(size);
}

void CacheDownloadClient::SetFileCachePath(const std::string& path)
{
    CDLOG_DEBUG("CacheDownloadClient::SetFileCachePath dir=" CDLOG_FMT_PUBLIC_S, GetLastPathDir(path).c_str());
    CacheDownloadService::GetInstance().Initialize(path);
}

void CacheDownloadClient::ClearMemoryCache()
{
    CDLOG_INFO("CacheDownloadClient::ClearMemoryCache");
    CacheDownloadService::GetInstance().ClearMemoryCache();
}

void CacheDownloadClient::ClearFileCache()
{
    CDLOG_INFO("CacheDownloadClient::ClearFileCache");
    CacheDownloadService::GetInstance().ClearFileCache();
}

void CacheDownloadClient::SetGlobalRetryOptions(const RetryOptions& options)
{
    auto clamped = std::min(options.maxRetryCount, MAX_RETRY_LIMIT);
    CDLOG_INFO("CacheDownloadClient::SetGlobalRetryOptions maxRetry=" CDLOG_FMT_ZU " clamped=" CDLOG_FMT_ZU,
        options.maxRetryCount, clamped);
    CacheDownloadService::GetInstance().SetGlobalRetryOptions(clamped);
}

void CacheDownloadClient::SetGlobalTimeoutOptions(const TimeoutOptions& options)
{
    auto netCheck = std::min(options.networkCheckTimeout, MAX_NETWORK_CHECK_TIMEOUT_SEC);
    auto httpTotal = std::max(options.httpTotalTimeout, MIN_HTTP_TOTAL_TIMEOUT_SEC);
    CDLOG_INFO("CacheDownloadClient::SetGlobalTimeoutOptions networkCheck=" CDLOG_FMT_D " httpTotal=" CDLOG_FMT_D,
        (int)netCheck, (int)httpTotal);
    CacheDownloadService::GetInstance().SetGlobalTimeoutOptions(netCheck, httpTotal);
}

} // namespace OHOS::Plugin::CacheDownload
