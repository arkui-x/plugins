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

#ifndef PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_RAM_H
#define PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_RAM_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "cache_space.h"

namespace OHOS::Plugin::CacheDownload {

// Forward declarations
class CacheManager;

/**
 * Maximum allowed size for a single cached resource after decompression.
 * Resources larger than this will not be stored in memory or file cache.
 */
constexpr uint64_t MAX_CACHE_SIZE = 20971520; // 20 MB

/**
 * In-memory cache implementation for download task data.
 *
 * Manages RAM-based storage for task data, including memory allocation
 * tracking through SpaceManager and automatic resource cleanup.
 *
 * The lifecycle is managed via shared_ptr: when the last reference is dropped,
 * the destructor releases allocated memory back to the SpaceManager.
 *
 * Ported from: request_request/common/cache_core/src/data/ram.rs → RamCache
 */
class RamCache {
public:
    /**
     * Creates a new RamCache for the specified task.
     *
     * Attempts to allocate the requested amount of memory from the SpaceManager.
     * If allocation fails, the cache is created without reserved memory and
     * data will not be eligible for RAM retention (only file cache).
     *
     * @param taskId Unique identifier for the task.
     * @param spaceMgr Reference to the SpaceManager for memory accounting.
     * @param sizeHint Optional size hint for pre-allocation (0 = no pre-allocation).
     */
    RamCache(const std::string& taskId,
             SpaceManager& spaceMgr,
             size_t sizeHint = 0);

    ~RamCache();

    // Non-copyable, movable
    RamCache(const RamCache&) = delete;
    RamCache& operator=(const RamCache&) = delete;
    RamCache(RamCache&& other) noexcept;
    RamCache& operator=(RamCache&& other) noexcept;

    /** Writes data to the underlying buffer. */
    void Write(const uint8_t* data, size_t size);

    /** Returns a read-only pointer to the data. */
    const uint8_t* Data() const;

    /** Returns the current size of the cached data in bytes. */
    size_t Size() const;

    /** Returns the task ID associated with this cache. */
    const std::string& TaskId() const;

    /** Returns true if this cache is eligible for RAM storage. */
    bool IsCached() const;

    /**
     * Finalizes the cache after writing is complete.
     * Checks if the data size is within limits and adjusts memory accounting.
     * Should be called once after all Write() calls.
     * @return true if the cache is eligible for RAM storage.
     */
    bool FinishWrite();

private:
    /**
     * Checks and adjusts the allocated memory based on current data size.
     * Handles three scenarios, as listed below.
     * - Data size equals allocated: no change needed
     * - Data size exceeds allocated: try to allocate more, or give up RAM
     * - Data size is below allocated: release the excess
     * @return true if RAM cache is valid.
     */
    bool CheckSize();

    std::string taskId_;
    std::vector<uint8_t> data_;
    uint64_t applied_;              // Amount of memory allocated (in bytes)
    SpaceManager* spaceMgr_;        // Non-owning pointer to the space manager
    bool isCached_;                 // Whether data is retained in RAM
};

} // namespace OHOS::Plugin::CacheDownload

#endif // PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_RAM_H
