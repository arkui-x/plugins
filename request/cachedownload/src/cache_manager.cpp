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

#include "cache_manager.h"
#include "cache_file.h"

#include <algorithm>

namespace OHOS::Plugin::CacheDownload {

namespace {
    /** Hard backstop to prevent infinite eviction loops. */
    constexpr size_t EVICTION_GUARD_LIMIT = 100000;
} // namespace

// ============================================================
// TaskOperationQueue
// ============================================================

std::pair<bool, std::shared_ptr<TaskOperationQueue::OpSlot>> TaskOperationQueue::Enqueue()
{
    std::lock_guard<std::mutex> lock(mutex_);
    auto slot = std::make_shared<OpSlot>();
    bool isFirst = queue_.empty();
    slot->ready = isFirst; // first in queue proceeds immediately
    queue_.push_back(slot);
    return {isFirst, slot};
}

void TaskOperationQueue::NotifyNext()
{
    std::shared_ptr<OpSlot> next;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!queue_.empty()) {
            queue_.pop_front();
        }
        if (!queue_.empty()) {
            next = queue_.front();
        }
    }
    if (next) {
        // Set ready under the slot's own mutex so the waiter cannot miss the
        // wakeup (durable flag), matching the Rust NotifyCondition pattern.
        std::lock_guard<std::mutex> slotLock(next->mutex);
        next->ready = true;
        next->cv.notify_one();
    }
}

// ============================================================
// CacheManager
// ============================================================

CacheManager::CacheManager()
    : ramCache_(0), ramSpace_(DEFAULT_RAM_CACHE_SIZE),
      fileCache_(0), fileSpace_(DEFAULT_FILE_CACHE_SIZE)
{
}

CacheManager::~CacheManager() = default;

// ============================================================
// Cache size management
// ============================================================

void CacheManager::SetRamCacheSize(uint64_t size)
{
    std::lock_guard<std::mutex> lock(ramMutex_);
    ramSpace_.ChangeTotalSize(size);
    // Trigger eviction if needed
    ApplyRamSpace(0);
}

void CacheManager::SetFileCacheSize(uint64_t size)
{
    std::vector<std::string> evicted;
    {
        std::lock_guard<std::mutex> lock(fileMutex_);
        fileSpace_.ChangeTotalSize(size);
        ApplyFileSpace(0, evicted);
        if (size == 0) {
            // Invalidate every queued background write, including ones whose
            // task has no fileCache_ entry yet (write not yet started).
            writeEpochGlobal_++;
        }
    }
    // Remove evicted files from disk
    for (const auto& taskId : evicted) {
        RemoveFileWithQueue(taskId);
    }
}

uint64_t CacheManager::RamCacheSize() const
{
    return ramSpace_.TotalCapacity();
}

uint64_t CacheManager::FileCacheSize() const
{
    return fileSpace_.TotalCapacity();
}

// ============================================================
// Cache operations
// ============================================================

std::shared_ptr<RamCache> CacheManager::Fetch(const std::string& taskId)
{
    // 1. Check primary RAM cache
    {
        std::lock_guard<std::mutex> lock(ramMutex_);
        auto cached = ramCache_.Get(taskId);
        if (cached.has_value()) {
            return *cached;
        }
    }

    // 2. Check backup RAM cache (being written to file)
    {
        std::lock_guard<std::mutex> lock(fileMutex_);
        auto it = backupRams_.find(taskId);
        if (it != backupRams_.end()) {
            return it->second;
        }
    }

    // 3. Try to load from file cache
    return LoadRamFromFile(taskId);
}

bool CacheManager::Contains(const std::string& taskId) const
{
    // Check RAM
    {
        std::lock_guard<std::mutex> lock(ramMutex_);
        if (ramCache_.Contains(taskId)) {
            return true;
        }
    }

    // Check file
    {
        std::lock_guard<std::mutex> lock(fileMutex_);
        if (fileCache_.Contains(taskId) || backupRams_.count(taskId) > 0) {
            return true;
        }
    }

    return FileCache::Exists(taskId);
}

void CacheManager::Remove(const std::string& taskId)
{
    // Remove from RAM
    {
        std::lock_guard<std::mutex> lock(ramMutex_);
        ramCache_.Remove(taskId);
    }

    // Remove from file and backup
    {
        std::lock_guard<std::mutex> lock(fileMutex_);
        fileCache_.Remove(taskId);
        backupRams_.erase(taskId);
        fileLoadOnce_.erase(taskId);
        writeEpochs_[taskId]++;
        startupFileHashes_.erase(FileCache::HashKey(taskId));
    }

    // Fire-and-forget: remove from disk (queued)
    RemoveFileWithQueue(taskId);
}

std::unique_ptr<RamCache> CacheManager::CreateRamCache(const std::string& taskId, size_t sizeHint)
{
    return std::make_unique<RamCache>(taskId, ramSpace_, sizeHint);
}

void CacheManager::OnDownloadComplete(std::unique_ptr<RamCache> cache)
{
    if (!cache) {
        return;
    }

    bool isCached = cache->FinishWrite();

    auto sharedCache = std::shared_ptr<RamCache>(std::move(cache));

    if (isCached) {
        UpdateRamCache(sharedCache);
    }

    // Schedule background file write. Capture the invalidation epochs now so
    // the write can abort if the entry is evicted/removed before it starts.
    uint64_t globalEpoch = 0;
    uint64_t taskEpoch = 0;
    std::string taskId = sharedCache->TaskId();
    {
        std::lock_guard<std::mutex> lock(fileMutex_);
        globalEpoch = writeEpochGlobal_;
        auto it = writeEpochs_.find(taskId);
        taskEpoch = (it != writeEpochs_.end()) ? it->second : 0;
    }
    if (taskRunner_) {
        taskRunner_([this, taskId, sharedCache, globalEpoch, taskEpoch]() {
            WriteToFileInBackground(taskId, sharedCache, globalEpoch, taskEpoch);
        });
    }
}

std::unique_ptr<RamCache> CacheManager::ReadLocalFile(const std::string& taskId)
{
    return FileCache::ReadFromFile(taskId, ramSpace_);
}

// ============================================================
// Cache cleanup
// ============================================================

void CacheManager::ClearMemoryCache(const std::unordered_set<std::string>& runningTasks)
{
    std::vector<std::string> toRemove;

    {
        std::lock_guard<std::mutex> lock(ramMutex_);
        // Collect keys to remove (those not in runningTasks)
        ramCache_.ForEach([&](const std::string& key, const std::shared_ptr<RamCache>&) {
            if (runningTasks.find(key) == runningTasks.end()) {
                toRemove.push_back(key);
            }
        });
    }

    // Remove outside the ForEach lock to avoid deadlock
    for (const auto& key : toRemove) {
        std::lock_guard<std::mutex> lock(ramMutex_);
        ramCache_.Remove(key);
    }
}

void CacheManager::ClearFileCache(const std::unordered_set<std::string>& runningTasks)
{
    std::vector<std::string> toRemove;

    {
        std::lock_guard<std::mutex> lock(fileMutex_);
        fileCache_.ForEach([&](const std::string& key, const uint64_t& size) {
            if (runningTasks.find(key) == runningTasks.end()) {
                toRemove.push_back(key);
            }
        });
    }

    for (const auto& taskId : toRemove) {
        {
            std::lock_guard<std::mutex> lock(fileMutex_);
            fileCache_.Remove(taskId);
            backupRams_.erase(taskId);
            fileLoadOnce_.erase(taskId);
            writeEpochs_[taskId]++;
            startupFileHashes_.erase(FileCache::HashKey(taskId));
        }
        RemoveFileWithQueue(taskId);
    }
    {
        std::lock_guard<std::mutex> lock(fileMutex_);
        writeEpochGlobal_++;
    }
}

// ============================================================
// Index management
// ============================================================

void CacheManager::InitCacheDir(const std::string& cacheDir)
{
    FileCache::InitCacheDir(cacheDir);
}

void CacheManager::BuildCachedFilesIndex()
{
    auto files = FileCache::ListCachedFiles();

    // On restart, disk file names are sha256(url) hashes — we cannot reverse
    // them to URLs. Instead of putting hash keys into fileCache_ (which causes
    // double-hashing on eviction and double-counting with URL keys from lazy
    // loading), we only reserve space and record the hashes. Files are lazily
    // indexed by URL when LoadRamFromFile is called (FileCache::Exists(url)
    // correctly hashes the URL to find the file).
    std::lock_guard<std::mutex> lock(fileMutex_);
    for (const auto& [hash, fileSize] : files) {
        // Try to reserve space for this file.
        if (fileSpace_.ApplyCacheSize(fileSize)) {
            startupFileHashes_.insert(hash);
        } else {
            // Can't fit — delete the file from disk to stay within capacity.
            // Use RemoveFileByName because the hash is already the on-disk
            // file name; RemoveFile would re-hash it (double-hash bug).
            FileCache::RemoveFileByName(hash);
        }
    }
}

// ============================================================
// Background task runner
// ============================================================

void CacheManager::SetTaskRunner(TaskRunner runner)
{
    taskRunner_ = std::move(runner);
}

// ============================================================
// Internal helpers
// ============================================================

bool CacheManager::ApplyRamSpace(uint64_t size)
{
    // ramMutex_ must be held by caller
    if (size > MAX_CACHE_SIZE) {
        return false;
    }

    size_t guard = 0;
    size_t lastCount = SIZE_MAX;
    while (guard <= EVICTION_GUARD_LIMIT) {
        if (ramSpace_.ApplyCacheSize(size)) {
            return true;
        }
        // Try to evict the LRU entry from RAM
        std::string oldestKey;
        size_t count = 0;
        ramCache_.ForEach([&](const std::string& key, const std::shared_ptr<RamCache>&) {
            oldestKey = key; // Last item in ForEach is the least-recently-used
            count++;
        });

        if (count == 0 || oldestKey.empty()) {
            return false; // Nothing to evict
        }
        // If the entry count did not shrink since the last iteration, eviction
        // failed to reclaim space (another caller still holds a shared_ptr to the
        // RamCache, so its destructor did not release the bytes). Bail out
        // instead of spinning forever — the entry simply won't be RAM-cached.
        if (count >= lastCount) {
            return false;
        }
        lastCount = count;
        ramCache_.Remove(oldestKey);
        ++guard;
    }
    return false;
}

bool CacheManager::ApplyFileSpace(uint64_t size, std::vector<std::string>& evictedTasks)
{
    // fileMutex_ must be held by caller
    if (size > MAX_CACHE_SIZE) {
        return false;
    }

    size_t guard = 0;
    while (guard <= EVICTION_GUARD_LIMIT) {
        if (fileSpace_.ApplyCacheSize(size)) {
            return true;
        }
        // Try to evict the LRU entry from file cache
        std::string oldestKey;
        fileCache_.ForEach([&](const std::string& key, const uint64_t&) {
            oldestKey = key;
        });

        if (oldestKey.empty()) {
            return false; // Nothing to evict
        }

        uint64_t evictedSize = 0;
        auto val = fileCache_.Get(oldestKey);
        if (val.has_value()) {
            evictedSize = *val;
        }
        fileCache_.Remove(oldestKey);
        fileSpace_.Release(evictedSize);
        fileLoadOnce_.erase(oldestKey);
        // Drop any in-flight backup RAM and invalidate its queued background
        // write so Contains() stops seeing the entry immediately.
        backupRams_.erase(oldestKey);
        writeEpochs_[oldestKey]++;
        startupFileHashes_.erase(FileCache::HashKey(oldestKey));
        evictedTasks.push_back(oldestKey);
        ++guard;
    }
    return false;
}

void CacheManager::UpdateRamCache(std::shared_ptr<RamCache> cache)
{
    std::lock_guard<std::mutex> lock(ramMutex_);
    // Ensure space is available
    ApplyRamSpace(cache->Size());
    ramCache_.Put(cache->TaskId(), cache);
}

void CacheManager::WriteToFileInBackground(const std::string& taskId,
    std::shared_ptr<RamCache> cache,
    uint64_t globalEpoch, uint64_t taskEpoch)
{
    // 1. Check if we need to evict from file cache
    std::vector<std::string> evictedBefore;
    {
        std::lock_guard<std::mutex> lock(fileMutex_);

        // Abort if the entry was invalidated (evicted/removed/cleared) after
        // this write was queued — writing it now would resurrect a cache entry
        // the caller already cleared.
        auto epochIt = writeEpochs_.find(taskId);
        uint64_t curTaskEpoch = (epochIt != writeEpochs_.end()) ? epochIt->second : 0;
        if (globalEpoch != writeEpochGlobal_ || taskEpoch != curTaskEpoch) {
            return;
        }

        // Remove any existing file cache entry for this task
        auto existing = fileCache_.Get(taskId);
        if (existing.has_value()) {
            fileSpace_.Release(*existing);
        }
        fileCache_.Remove(taskId);
        fileLoadOnce_.erase(taskId);

        // Try to allocate space for the new file, evicting LRU entries if needed.
        // (ApplyFileSpace handles eviction + allocation; no spurious Release.)
        ApplyFileSpace(cache->Size(), evictedBefore);

        // Register the file cache entry
        fileCache_.Put(taskId, cache->Size());

        // Store backup RAM reference while writing
        backupRams_[taskId] = cache;
    }

    // Remove evicted files from disk (outside the fileMutex_ to avoid deadlock
    // with the per-task operation queue).
    for (const auto& evictedId : evictedBefore) {
        RemoveFileWithQueue(evictedId);
    }

    // 2. Serialized file write (wait for prior operations on this task)
    auto enq = [&]() -> std::pair<bool, std::shared_ptr<TaskOperationQueue::OpSlot>> {
        std::lock_guard<std::mutex> lock(opMutex_);
        auto& q = taskOps_[taskId];
        if (!q) {
            q = std::make_shared<TaskOperationQueue>();
        }
        return q->Enqueue();
    }();
    if (!enq.first) {
        // Wait for prior operations to complete (durable-flag wait, no lost wakeup).
        std::shared_ptr<TaskOperationQueue::OpSlot> slot = enq.second;
        std::unique_lock<std::mutex> waitLock(slot->mutex);
        slot->cv.wait(waitLock, [&slot] { return slot->ready; });
    }

    // 3. Perform the actual file write
    FileCache::WriteToFile(taskId, cache->Data(), cache->Size());

    // 4. Signal next operation
    {
        std::lock_guard<std::mutex> lock(opMutex_);
        auto it = taskOps_.find(taskId);
        if (it != taskOps_.end()) {
            it->second->NotifyNext();
        }
    }

    // 5. Clean up backup RAM reference
    {
        std::lock_guard<std::mutex> lock(fileMutex_);
        backupRams_.erase(taskId);
    }
}

std::shared_ptr<RamCache> CacheManager::FinalizeLoadedRam(const std::string& taskId,
                                                          std::unique_ptr<RamCache> ramCache)
{
    if (ramCache) {
        ramCache->FinishWrite();
        auto shared = std::shared_ptr<RamCache>(std::move(ramCache));
        if (shared->IsCached()) {
            UpdateRamCache(shared);
        }
        // Mark as loaded only after a successful read so a transient failure
        // remains recoverable on the next Fetch (OH OnceLock retry semantics).
        std::lock_guard<std::mutex> flock(fileMutex_);
        fileLoadOnce_[taskId] = true;
        return shared;
    }

    // Read failed: drop the file cache entry so Contains() stays consistent and
    // a future Fetch can retry instead of being permanently poisoned.
    {
        std::lock_guard<std::mutex> flock(fileMutex_);
        fileLoadOnce_.erase(taskId);
        fileCache_.Remove(taskId);
    }
    return nullptr;
}

CacheManager::FileLoadAction CacheManager::PrepareFileForLoad(
    const std::string& taskId,
    std::vector<std::string>& evictedOnPopulate,
    std::shared_ptr<RamCache>& backupRam)
{
    std::lock_guard<std::mutex> lock(fileMutex_);

    // Check backup rams first (file write in progress)
    auto it = backupRams_.find(taskId);
    if (it != backupRams_.end()) {
        backupRam = it->second;
        return FileLoadAction::kReturnBackup;
    }

    // Check if already loaded (Once semantics)
    if (fileLoadOnce_.count(taskId) > 0) {
        return FileLoadAction::kSkip;
    }

    if (!fileCache_.Contains(taskId)) {
        // File may exist on disk but not in the in-memory index (after
        // restart, BuildCachedFilesIndex reserves space without creating
        // fileCache_ entries — files are lazily indexed here by URL).
        if (!FileCache::Exists(taskId)) {
            return FileLoadAction::kSkip;
        }
        uint64_t fileSize = FileCache::FileSize(taskId);
        std::string hash = FileCache::HashKey(taskId);
        if (startupFileHashes_.count(hash) > 0) {
            // Space was already reserved at startup — just index by URL,
            // don't double-reserve.
            startupFileHashes_.erase(hash);
        } else {
            // New file (or missed startup index) — reserve space.
            if (!ApplyFileSpace(fileSize, evictedOnPopulate)) {
                return FileLoadAction::kSkip;
            }
        }
        fileCache_.Put(taskId, fileSize);
    }

    // Mark as loaded (Once semantics)
    fileLoadOnce_[taskId] = true;
    return FileLoadAction::kContinue;
}

void CacheManager::EnqueueAndWaitForFileRead(const std::string& taskId)
{
    // Serialized file read: enqueue and wait for prior operations.
    auto enq = [&]() -> std::pair<bool, std::shared_ptr<TaskOperationQueue::OpSlot>> {
        std::lock_guard<std::mutex> lock(opMutex_);
        auto& q = taskOps_[taskId];
        if (!q) {
            q = std::make_shared<TaskOperationQueue>();
        }
        return q->Enqueue();
    }();
    if (!enq.first) {
        std::shared_ptr<TaskOperationQueue::OpSlot> slot = enq.second;
        std::unique_lock<std::mutex> waitLock(slot->mutex);
        slot->cv.wait(waitLock, [&slot] { return slot->ready; });
    }
}

void CacheManager::SignalNextFileOp(const std::string& taskId)
{
    std::lock_guard<std::mutex> lock(opMutex_);
    auto it = taskOps_.find(taskId);
    if (it != taskOps_.end()) {
        it->second->NotifyNext();
    }
}

std::shared_ptr<RamCache> CacheManager::LoadRamFromFile(const std::string& taskId)
{
    std::vector<std::string> evictedOnPopulate;
    std::shared_ptr<RamCache> backupRam;
    auto action = PrepareFileForLoad(taskId, evictedOnPopulate, backupRam);
    if (action == FileLoadAction::kReturnBackup) {
        return backupRam;
    }
    if (action == FileLoadAction::kSkip) {
        return nullptr;
    }

    // Remove evicted files from disk (outside fileMutex_ to avoid deadlock
    // with the per-task operation queue).
    for (const auto& evictedId : evictedOnPopulate) {
        RemoveFileWithQueue(evictedId);
    }

    EnqueueAndWaitForFileRead(taskId);
    auto ramCache = FileCache::ReadFromFile(taskId, ramSpace_);
    SignalNextFileOp(taskId);

    return FinalizeLoadedRam(taskId, std::move(ramCache));
}

void CacheManager::RemoveFileWithQueue(const std::string& taskId)
{
    auto enq = [&]() -> std::pair<bool, std::shared_ptr<TaskOperationQueue::OpSlot>> {
        std::lock_guard<std::mutex> lock(opMutex_);
        auto& q = taskOps_[taskId];
        if (!q) {
            q = std::make_shared<TaskOperationQueue>();
        }
        return q->Enqueue();
    }();
    if (!enq.first) {
        std::shared_ptr<TaskOperationQueue::OpSlot> slot = enq.second;
        std::unique_lock<std::mutex> waitLock(slot->mutex);
        slot->cv.wait(waitLock, [&slot] { return slot->ready; });
    }

    FileCache::RemoveFile(taskId);

    {
        std::lock_guard<std::mutex> lock(opMutex_);
        auto it = taskOps_.find(taskId);
        if (it != taskOps_.end()) {
            it->second->NotifyNext();
        }
    }
}

} // namespace OHOS::Plugin::CacheDownload
