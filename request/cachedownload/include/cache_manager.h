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

#ifndef PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_MANAGER_H
#define PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_MANAGER_H

#include <atomic>
#include <condition_variable>
#include <cstdint>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "cache_lru.h"
#include "cache_ram.h"
#include "cache_space.h"

namespace OHOS::Plugin::CacheDownload {

/**
 * Serialized file operation queue for a single task.
 *
 * Ensures that file read/write/remove operations for the same task ID
 * are executed sequentially to prevent file corruption.
 *
 * Ported from: Rust NotifyCondition pattern in manage.rs. Each queued slot
 * carries its own (mutex, condvar, ready flag) so wakeups are durable and a
 * notify that races ahead of a waiter's wait() is not lost.
 */
class TaskOperationQueue {
public:
    /** A single queued operation's synchronization state. */
    struct OpSlot {
        std::mutex mutex;
        std::condition_variable cv;
        bool ready = false;
    };

    TaskOperationQueue() = default;

    /**
     * Enqueues an operation and returns whether it is first in the queue.
     * The caller must call NotifyNext() when the operation completes.
     * @return pair (is_first, slot). If is_first, the caller can proceed
     *         immediately. Otherwise, the caller must wait on slot->cv
     *         (looping while !slot->ready) before proceeding.
     */
    std::pair<bool, std::shared_ptr<OpSlot>> Enqueue();

    /** Signals the next waiting operation in the queue. */
    void NotifyNext();

private:
    std::mutex mutex_;
    std::deque<std::shared_ptr<OpSlot>> queue_;
};

/**
 * Central manager for coordinating RAM and file caches.
 *
 * Manages dual-layer caching:
 * - RAM cache: LRU with SpaceManager for capacity tracking (default 20MB)
 * - File cache: LRU with SpaceManager for capacity tracking (default 100MB)
 * - Serialized file operations per task to prevent corruption
 * - File→RAM lazy loading with Once semantics
 * - Background file writing on download completion
 *
 * Ported from: request_request/common/cache_core/src/manage.rs → CacheManager
 */
class CacheManager {
public:
    /** Default maximum size for RAM-based cache storage. */
    static constexpr uint64_t DEFAULT_RAM_CACHE_SIZE = 1024 * 1024 * 20;   // 20 MB

    /** Default maximum size for file-based cache storage. */
    static constexpr uint64_t DEFAULT_FILE_CACHE_SIZE = 1024 * 1024 * 100; // 100 MB

    /** Callback type for spawning background tasks (file writes). */
    using TaskRunner = std::function<void(std::function<void()>)>;

    CacheManager();
    ~CacheManager();

    // Non-copyable, non-movable (singleton-like, owned by CacheDownloadService)
    CacheManager(const CacheManager&) = delete;
    CacheManager& operator=(const CacheManager&) = delete;

    // ============================================================
    // Cache size management
    // ============================================================

    /** Sets the maximum RAM cache size (bytes). Triggers LRU eviction. */
    void SetRamCacheSize(uint64_t size);

    /** Sets the maximum file cache size (bytes). Triggers LRU eviction. */
    void SetFileCacheSize(uint64_t size);

    /** Returns the current RAM cache total capacity. */
    uint64_t RamCacheSize() const;

    /** Returns the current file cache total capacity. */
    uint64_t FileCacheSize() const;

    // ============================================================
    // Cache operations
    // ============================================================

    /**
     * Fetches a cache entry by URL/task ID.
     * Checks: primary RAM → backup RAM → file cache (lazy load).
     * @return Pointer to RamCache data, or nullptr if not found.
     */
    std::shared_ptr<RamCache> Fetch(const std::string& taskId);

    /** Checks if a cache entry exists (any tier). */
    bool Contains(const std::string& taskId) const;

    /**
     * Removes a cache entry from all storage tiers.
     * @param taskId The task ID to remove.
     */
    void Remove(const std::string& taskId);

    /**
     * Creates a new RamCache for writing.
     * Attempts to pre-allocate memory from the RAM space manager.
     */
    std::unique_ptr<RamCache> CreateRamCache(const std::string& taskId, size_t sizeHint = 0);

    /**
     * Called when a download completes and data is fully written to RamCache.
     * Registers the cache in RAM LRU and triggers a background file write.
     */
    void OnDownloadComplete(std::unique_ptr<RamCache> cache);

    /**
     * Reads a task's cached file into RAM without inserting it into the cache.
     * Used during service startup to serve from local files before restore.
     */
    std::unique_ptr<RamCache> ReadLocalFile(const std::string& taskId);

    // ============================================================
    // Cache cleanup
    // ============================================================

    /** Clears RAM cache entries not in the running tasks set. */
    void ClearMemoryCache(const std::unordered_set<std::string>& runningTasks);

    /** Clears file cache entries not in the running tasks set. */
    void ClearFileCache(const std::unordered_set<std::string>& runningTasks);

    // ============================================================
    // Index management
    // ============================================================

    /**
     * Initializes the file cache directory and restores cached file index.
     * Should be called once during CacheDownloadService initialization.
     */
    void InitCacheDir(const std::string& cacheDir);

    /**
     * Builds the cached files index from disk.
     * Scans the cache directory, restores valid files into the file LRU.
     */
    void BuildCachedFilesIndex();

    // ============================================================
    // Background task runner
    // ============================================================

    /** Sets the task runner for background file operations. */
    void SetTaskRunner(TaskRunner runner);

private:
    // ============================================================
    // Internal helpers
    // ============================================================

    /** Attempts to allocate RAM cache space, evicting LRU entries if needed. */
    bool ApplyRamSpace(uint64_t size);

    /** Attempts to allocate file cache space, evicting LRU entries if needed. */
    bool ApplyFileSpace(uint64_t size, std::vector<std::string>& evictedTasks);

    /** Updates the primary RAM LRU with a cache entry. */
    void UpdateRamCache(std::shared_ptr<RamCache> cache);

    /** Background: writes RamCache data to a file. Aborts if the entry was
     * invalidated (epochs changed) after the write was queued. */
    void WriteToFileInBackground(const std::string& taskId,
                                  std::shared_ptr<RamCache> cache,
                                  uint64_t globalEpoch, uint64_t taskEpoch);

    /** Lazy-load RAM from file cache with Once semantics. */
    std::shared_ptr<RamCache> LoadRamFromFile(const std::string& taskId);

    /** Result of checking file cache state before loading. */
    enum class FileLoadAction {
        kReturnBackup,  // Backup RAM cache found, return it directly
        kSkip,          // Already loaded or no file on disk
        kContinue,      // Proceed with file read
    };

    /** Checks file cache state under fileMutex_. Determines whether
     *  LoadRamFromFile should return a backup, skip, or continue. */
    FileLoadAction PrepareFileForLoad(const std::string& taskId,
                                      std::vector<std::string>& evictedOnPopulate,
                                      std::shared_ptr<RamCache>& backupRam);

    /** Enqueues a serialized file read for the task, waiting for prior ops. */
    void EnqueueAndWaitForFileRead(const std::string& taskId);

    /** Signals the next queued file operation for the task. */
    void SignalNextFileOp(const std::string& taskId);

    /** Finalizes a loaded RamCache: updates RAM LRU on success, cleans up on failure. */
    std::shared_ptr<RamCache> FinalizeLoadedRam(const std::string& taskId,
                                                 std::unique_ptr<RamCache> ramCache);

    /** Removes a file from disk (waits for prior operations). */
    void RemoveFileWithQueue(const std::string& taskId);

    // ============================================================
    // Members
    // ============================================================

    // RAM cache
    mutable std::mutex ramMutex_;
    LRUCache<std::shared_ptr<RamCache>> ramCache_;
    SpaceManager ramSpace_;

    // File cache
    mutable std::mutex fileMutex_;
    LRUCache<uint64_t> fileCache_;          // taskId → file size
    SpaceManager fileSpace_;
    std::unordered_map<std::string, std::shared_ptr<RamCache>> backupRams_;
    std::unordered_map<std::string, bool> fileLoadOnce_;  // taskId → has been loaded
    // Write invalidation epochs (guarded by fileMutex_): a background file
    // write queued by OnDownloadComplete captures these and aborts if an
    // eviction/removal bumped them before the write started — otherwise the
    // stale write would resurrect an entry that setFileCacheSize(0)/Remove/
    // ClearFileCache had logically cleared (testCacheStrategy002 LAZY miss).
    uint64_t writeEpochGlobal_ = 0;
    std::unordered_map<std::string, uint64_t> writeEpochs_;

    // Hashes of files found on disk at startup (BuildCachedFilesIndex).
    // Used to avoid double-counting space when LoadRamFromFile lazily
    // indexes a pre-existing file (the space was already reserved at startup).
    std::unordered_set<std::string> startupFileHashes_;

    // Serialized file operations per task
    mutable std::mutex opMutex_;
    std::unordered_map<std::string, std::shared_ptr<TaskOperationQueue>> taskOps_;

    // Background task runner
    TaskRunner taskRunner_;
};

} // namespace OHOS::Plugin::CacheDownload

#endif // PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_MANAGER_H
