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

#include "cache_proxy.h"

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "base64_utils.h"
#include "constant.h"
#include "http_exec.h"
#include "lru_cache_disk_handler.h"
#include "netstack_common_utils.h"
#include "netstack_log.h"
#include "request_context.h"

static constexpr int32_t WRITE_INTERVAL = 60;

namespace OHOS::NetStack::Http {
std::mutex g_diskCacheMutex;
std::mutex g_cacheNeedRunMutex;
std::atomic_bool g_cacheNeedRun(false);
std::atomic_bool g_cacheIsRunning(false);
std::condition_variable g_cacheThreadCondition;
std::condition_variable g_cacheNeedRunCondition;
static std::shared_ptr<LRUCacheDiskHandler> DISK_LRU_CACHE = nullptr;

CacheProxy::CacheProxy(HttpRequestOptions &requestOptions) : strategy_(requestOptions)
{
    std::string str = requestOptions.GetUrl() + HttpConstant::HTTP_LINE_SEPARATOR +
                    CommonUtils::ToLower(requestOptions.GetMethod()) + HttpConstant::HTTP_LINE_SEPARATOR;
    for (const auto &p : requestOptions.GetHeader()) {
        str += p.first + HttpConstant::HTTP_HEADER_SEPARATOR + p.second + HttpConstant::HTTP_LINE_SEPARATOR;
    }
    str += std::to_string(requestOptions.GetHttpVersion());
    key_ = OHOS::NetStack::Base64::Encode(str);
}

bool CacheProxy::ReadResponseFromCache(RequestContext* context)
{
    if (!g_cacheIsRunning.load()) {
        return false;
    }

    if (!strategy_.CouldUseCache()) {
        NETSTACK_LOGI("only GET/HEAD method or header has [Range] can use cache");
        return false;
    }

    if (DISK_LRU_CACHE == nullptr) {
        DISK_LRU_CACHE = std::make_shared<LRUCacheDiskHandler>(HttpExec::GetCacheFileName(), 0);
    }

    auto responseFromCache = DISK_LRU_CACHE->Get(key_);
    if (responseFromCache.empty()) {
        NETSTACK_LOGI("no cache with this request");
        return false;
    }
    HttpResponse cachedResponse;
    cachedResponse.SetRawHeader(OHOS::NetStack::Base64::Decode(responseFromCache[HttpConstant::RESPONSE_KEY_HEADER]));
    cachedResponse.SetResult(OHOS::NetStack::Base64::Decode(responseFromCache[HttpConstant::RESPONSE_KEY_RESULT]));
    cachedResponse.SetCookies(OHOS::NetStack::Base64::Decode(responseFromCache[HttpConstant::RESPONSE_KEY_COOKIES]));
    cachedResponse.SetResponseTime(OHOS::NetStack::Base64::Decode(responseFromCache[HttpConstant::RESPONSE_TIME]));
    cachedResponse.SetRequestTime(OHOS::NetStack::Base64::Decode(responseFromCache[HttpConstant::REQUEST_TIME]));
    cachedResponse.SetResponseCode(static_cast<uint32_t>(ResponseCode::OK));
    cachedResponse.ParseHeaders();

    CacheStatus status = strategy_.RunStrategy(cachedResponse);
    if (status == CacheStatus::FRESH) {
        context->response = cachedResponse;
        NETSTACK_LOGI("cache is FRESH");
        return true;
    }
    if (status == CacheStatus::STALE) {
        NETSTACK_LOGI("cache is STATE, we try to talk to the server");
        context->SetCacheResponse(cachedResponse);
        return false;
    }
    return false;
}

void CacheProxy::WriteResponseToCache(const HttpResponse &response)
{
    if (!g_cacheIsRunning.load()) {
        return;
    }

    if (!strategy_.IsCacheable(response)) {
        NETSTACK_LOGE("do not cache this response");
        return;
    }
    std::unordered_map<std::string, std::string> cacheResponse;
    cacheResponse[HttpConstant::RESPONSE_KEY_HEADER] = OHOS::NetStack::Base64::Encode(response.GetRawHeader());
    cacheResponse[HttpConstant::RESPONSE_KEY_RESULT] = OHOS::NetStack::Base64::Encode(response.GetResult());
    cacheResponse[HttpConstant::RESPONSE_KEY_COOKIES] = OHOS::NetStack::Base64::Encode(response.GetCookies());
    cacheResponse[HttpConstant::RESPONSE_TIME] = OHOS::NetStack::Base64::Encode(response.GetResponseTime());
    cacheResponse[HttpConstant::REQUEST_TIME] = OHOS::NetStack::Base64::Encode(response.GetRequestTime());

    if (DISK_LRU_CACHE == nullptr) {
        DISK_LRU_CACHE = std::make_shared<LRUCacheDiskHandler>(HttpExec::GetCacheFileName(), 0);
    }
    DISK_LRU_CACHE->Put(key_, cacheResponse);
}

void CacheProxy::RunCache()
{
    RunCacheWithSize(MAX_DISK_CACHE_SIZE);
}

void CacheProxy::RunCacheWithSize(size_t capacity)
{
    if (g_cacheIsRunning.load()) {
        return;
    }
    if (DISK_LRU_CACHE == nullptr) {
        DISK_LRU_CACHE = std::make_shared<LRUCacheDiskHandler>(HttpExec::GetCacheFileName(), 0);
    }
    DISK_LRU_CACHE->SetCapacity(capacity);

    g_cacheNeedRun.store(true);

    DISK_LRU_CACHE->ReadCacheFromJsonFile();

    std::thread([]() {
        g_cacheIsRunning.store(true);
        while (g_cacheNeedRun.load()) {
            std::unique_lock<std::mutex> lock(g_cacheNeedRunMutex);
            g_cacheNeedRunCondition.wait_for(
                lock, std::chrono::seconds(WRITE_INTERVAL), [] { return !g_cacheNeedRun.load(); });

            if (DISK_LRU_CACHE == nullptr) {
                DISK_LRU_CACHE = std::make_shared<LRUCacheDiskHandler>(HttpExec::GetCacheFileName(), 0);
            }
            DISK_LRU_CACHE->WriteCacheToJsonFile();
        }

        g_cacheIsRunning.store(false);
        g_cacheThreadCondition.notify_all();
    }).detach();
}

void CacheProxy::FlushCache()
{
    if (!g_cacheIsRunning.load()) {
        return;
    }
    if (DISK_LRU_CACHE == nullptr) {
        DISK_LRU_CACHE = std::make_shared<LRUCacheDiskHandler>(HttpExec::GetCacheFileName(), 0);
    }
    DISK_LRU_CACHE->WriteCacheToJsonFile();
}

void CacheProxy::StopCacheAndDelete()
{
    if (!g_cacheIsRunning.load()) {
        return;
    }
    g_cacheNeedRun.store(false);
    g_cacheNeedRunCondition.notify_all();

    std::unique_lock<std::mutex> lock(g_diskCacheMutex);
    g_cacheThreadCondition.wait(lock, [] { return !g_cacheIsRunning.load(); });
    if (DISK_LRU_CACHE == nullptr) {
        DISK_LRU_CACHE = std::make_shared<LRUCacheDiskHandler>(HttpExec::GetCacheFileName(), 0);
    }
    DISK_LRU_CACHE->Delete();
}
} // namespace OHOS::NetStack::Http
