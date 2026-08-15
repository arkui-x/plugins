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

#include "cache_ram.h"

#include <algorithm>
#include <cstring>

namespace OHOS::Plugin::CacheDownload {

namespace {
    /** Default initial capacity for new cache vectors. */
    constexpr size_t DEFAULT_TRUNK_CAPACITY = 512;
}

RamCache::RamCache(const std::string& taskId,
                   SpaceManager& spaceMgr,
                   size_t sizeHint)
    : taskId_(taskId), applied_(0), spaceMgr_(&spaceMgr), isCached_(false)
{
    if (sizeHint > 0) {
        if (spaceMgr_->ApplyCacheSize(sizeHint)) {
            applied_ = sizeHint;
        }
        // If allocation fails, applied_ stays 0 — data won't be RAM-cached
    }
    data_.reserve(sizeHint > 0 ? sizeHint : DEFAULT_TRUNK_CAPACITY);
}

RamCache::~RamCache()
{
    if (applied_ > 0 && spaceMgr_ != nullptr) {
        spaceMgr_->Release(applied_);
    }
}

RamCache::RamCache(RamCache&& other) noexcept
    : taskId_(std::move(other.taskId_)), data_(std::move(other.data_)),
      applied_(other.applied_), spaceMgr_(other.spaceMgr_), isCached_(other.isCached_)
{
    // Prevent double-release by the moved-from object
    other.applied_ = 0;
    other.spaceMgr_ = nullptr;
}

RamCache& RamCache::operator=(RamCache&& other) noexcept
{
    if (this != &other) {
        // Release current allocation
        if (applied_ > 0 && spaceMgr_ != nullptr) {
            spaceMgr_->Release(applied_);
        }

        taskId_ = std::move(other.taskId_);
        data_ = std::move(other.data_);
        applied_ = other.applied_;
        spaceMgr_ = other.spaceMgr_;
        isCached_ = other.isCached_;

        other.applied_ = 0;
        other.spaceMgr_ = nullptr;
    }
    return *this;
}

void RamCache::Write(const uint8_t* data, size_t size)
{
    data_.insert(data_.end(), data, data + size);
}

const uint8_t* RamCache::Data() const
{
    return data_.data();
}

size_t RamCache::Size() const
{
    return data_.size();
}

const std::string& RamCache::TaskId() const
{
    return taskId_;
}

bool RamCache::IsCached() const
{
    return isCached_;
}

bool RamCache::FinishWrite()
{
    isCached_ = CheckSize();
    return isCached_;
}

bool RamCache::CheckSize()
{
    uint64_t dataLen = static_cast<uint64_t>(data_.size());
    if (dataLen == applied_) {
        return true;
    }

    if (dataLen > applied_) {
        // Data grew beyond pre-allocated space
        uint64_t diff = dataLen - applied_;

        if (dataLen > MAX_CACHE_SIZE) {
            // Exceeds maximum allowed size — release RAM
            if (applied_ > 0) {
                spaceMgr_->Release(applied_);
                applied_ = 0;
            }
            return false;
        }

        if (!spaceMgr_->ApplyCacheSize(diff)) {
            // Failed to allocate additional memory — release RAM
            if (applied_ > 0) {
                spaceMgr_->Release(applied_);
                applied_ = 0;
            }
            return false;
        }

        // Successfully allocated additional memory
        applied_ = dataLen;
        return true;
    }

    // Data is smaller than allocated — release excess
    uint64_t excess = applied_ - dataLen;
    spaceMgr_->Release(excess);
    applied_ = dataLen;
    return true;
}

} // namespace OHOS::Plugin::CacheDownload
