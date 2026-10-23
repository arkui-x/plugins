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

#include "cache_space.h"
#include "cache_log.h"

namespace OHOS::Plugin::CacheDownload {

SpaceManager::SpaceManager(uint64_t capacity)
    : totalCapacity_(capacity), usedCapacity_(0)
{
}

bool SpaceManager::ApplyCacheSize(uint64_t applySize)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // A capacity of 0 means "no capacity" (matching OH space.rs): any apply
    // (except 0) fails. Callers that want "unlimited" must set a large limit.
    // When capacity is 0 but bytes are still stored, report failure even for a
    // 0-byte apply so the caller's eviction loop runs and drains usedCapacity_
    // to 0 — otherwise setFileCacheSize(0)/setMemoryCacheSize(0) would shrink
    // the limit without evicting anything, leaving stale entries visible to
    // LAZY cache hits (testCacheStrategy002).
    if (totalCapacity_ == 0) {
        CDLOG_WARN("SpaceManager apply rejected (capacity=0) apply=" CDLOG_FMT_LLU, (uint64_t)applySize);
        return applySize == 0 && usedCapacity_ == 0;
    }

    if (applySize + usedCapacity_ > totalCapacity_) {
        CDLOG_WARN("SpaceManager apply rejected (over capacity) apply=" CDLOG_FMT_LLU " used=" CDLOG_FMT_LLU
            " total=" CDLOG_FMT_LLU,
            (uint64_t)applySize, (uint64_t)usedCapacity_, (uint64_t)totalCapacity_);
        return false;
    }
    usedCapacity_ += applySize;
    return true;
}

void SpaceManager::Release(uint64_t size)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (size > usedCapacity_) {
        usedCapacity_ = 0;
    } else {
        usedCapacity_ -= size;
    }
}

void SpaceManager::ChangeTotalSize(uint64_t size)
{
    std::lock_guard<std::mutex> lock(mutex_);
    totalCapacity_ = size;
    CDLOG_INFO("SpaceManager ChangeTotalSize size=" CDLOG_FMT_LLU, (uint64_t)size);
}

uint64_t SpaceManager::TotalCapacity() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return totalCapacity_;
}

uint64_t SpaceManager::UsedCapacity() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return usedCapacity_;
}

uint64_t SpaceManager::AvailableCapacity() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (totalCapacity_ == 0) {
        return 0; // no capacity configured
    }
    return totalCapacity_ > usedCapacity_ ? totalCapacity_ - usedCapacity_ : 0;
}

} // namespace OHOS::Plugin::CacheDownload
