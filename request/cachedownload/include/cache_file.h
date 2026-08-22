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

#ifndef PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_FILE_H
#define PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_FILE_H

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "cache_space.h"

namespace OHOS::Plugin::CacheDownload {

// Forward declarations
class RamCache;

/**
 * File-based cache implementation for task data.
 *
 * Manages disk-based storage for cached download data. Files are stored
 * in the configured cache directory with a "_F" suffix to indicate
 * completed (fully written) files.
 *
 * Ported from: request_request/common/cache_core/src/data/file.rs → FileCache
 */
class FileCache {
public:
    /**
     * Suffix appended to files that are fully written and finalized.
     * Files without this suffix are incomplete and considered invalid.
     */
    static constexpr const char* FINISH_SUFFIX = "_F";

    /**
     * Initializes the file cache with the given storage directory.
     * Creates the directory if it doesn't exist.
     * @param cacheDir Path to the cache directory.
     * @return true if directory exists and is writable.
     */
    static bool InitCacheDir(const std::string& cacheDir);

    /** Returns the current cache directory path. */
    static std::string CacheDir();

    /**
     * Creates a cache file and writes RamCache data to it.
     * Writes to a temp file first, then renames with finish suffix.
     * @param taskId Task identifier.
     * @param data Binary data to write.
     * @param size Size of data in bytes.
     * @return true on success.
     */
    static bool WriteToFile(const std::string& taskId,
                            const uint8_t* data,
                            size_t size);

    /**
     * Reads a cache file back into memory.
     * @param taskId Task identifier.
     * @param spaceMgr SpaceManager for memory accounting.
     * @param sizeHint Expected file size (0 = unknown).
     * @return A RamCache with file contents, or nullptr on failure.
     */
    static std::unique_ptr<RamCache> ReadFromFile(const std::string& taskId,
                                                   SpaceManager& spaceMgr,
                                                   size_t sizeHint = 0);

    /**
     * Removes a cache file from disk.
     * @param taskId Task identifier (URL — will be hashed to get the file name).
     */
    static void RemoveFile(const std::string& taskId);

    /**
     * Removes a cache file by its on-disk name (the sha256 hex, without re-hashing).
     * Used by BuildCachedFilesIndex eviction where the key is already the hash.
     * @param name The on-disk file name (sha256 hex of the URL).
     */
    static void RemoveFileByName(const std::string& name);

    /**
     * Returns the sha256 hex hash of the given taskId (URL).
     * This is the on-disk file name (without the _F suffix).
     */
    static std::string HashKey(const std::string& taskId);

    /**
     * Returns the full file path for a task ID.
     * @param taskId Task identifier (URL — will be hashed to get the file name).
     * @return Full path to the cache file (with finish suffix).
     */
    static std::string FilePath(const std::string& taskId);

    /**
     * Returns the size of a cached file on disk, or 0 if not found.
     */
    static uint64_t FileSize(const std::string& taskId);

    /**
     * Checks if a cache file exists for the task.
     */
    static bool Exists(const std::string& taskId);

    /**
     * Enumerates all finished cache files in the directory.
     * @return Vector of (taskId, fileSize) pairs.
     */
    static std::vector<std::pair<std::string, uint64_t>> ListCachedFiles();

private:
    static std::string s_cacheDir_;
    static std::mutex s_mutex_;
};

} // namespace OHOS::Plugin::CacheDownload

#endif // PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_FILE_H
