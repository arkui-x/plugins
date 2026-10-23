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

#ifndef PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_SPACE_H
#define PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_SPACE_H

#include <cstdint>
#include <mutex>

namespace OHOS::Plugin::CacheDownload {

/**
 * Manages resource capacities for the caching system.
 *
 * Tracks total available capacity and currently used capacity,
 * providing methods to apply for additional resources and release unused ones.
 * Thread-safe: all public methods are protected by a mutex.
 *
 * Ported from: request_request/common/cache_core/src/data/space.rs → SpaceManager
 */
class SpaceManager {
public:
    /**
     * Creates a new SpaceManager with the specified total capacity.
     * @param capacity Total available capacity in bytes. 0 = no limit.
     */
    explicit SpaceManager(uint64_t capacity = 0);

    /**
     * Attempts to allocate additional cache space.
     * @param applySize Amount of space to allocate in bytes.
     * @return true if allocation succeeded, false if insufficient capacity.
     */
    bool ApplyCacheSize(uint64_t applySize);

    /**
     * Releases previously allocated resource space.
     * @param size Amount of space to release in bytes.
     */
    void Release(uint64_t size);

    /**
     * Updates the total capacity.
     * @param size New total capacity in bytes. 0 = no limit.
     */
    void ChangeTotalSize(uint64_t size);

    /** Returns the total capacity in bytes. */
    uint64_t TotalCapacity() const;

    /** Returns the currently used capacity in bytes. */
    uint64_t UsedCapacity() const;

    /** Returns the available capacity in bytes. */
    uint64_t AvailableCapacity() const;

private:
    mutable std::mutex mutex_;
    uint64_t totalCapacity_;
    uint64_t usedCapacity_;
};

} // namespace OHOS::Plugin::CacheDownload

#endif // PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_SPACE_H
