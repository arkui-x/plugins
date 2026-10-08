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

#ifndef PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_LRU_H
#define PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_LRU_H

#include <cstdint>
#include <functional>
#include <list>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>

namespace OHOS::Plugin::CacheDownload {

/**
 * A thread-safe LRU (Least Recently Used) cache.
 *
 * @tparam V The value type stored in the cache.
 *
 * Usage:
 *   LRUCache<std::string> cache(100);
 *   cache.Put("key1", "value1");
 *   auto val = cache.Get("key1");  // returns optional<string>
 *   cache.Remove("key1");
 *   cache.Clear();
 */
template<typename V>
class LRUCache {
public:
    /** Callback invoked when an entry is evicted from the cache. */
    using EvictCallback = std::function<void(const std::string& key, const V& value)>;

    /**
     * Creates an LRU cache with the specified capacity.
     * @param maxSize Maximum number of entries. 0 = no limit.
     */
    explicit LRUCache(size_t maxSize = 0)
        : maxSize_(maxSize)
    {
    }

    ~LRUCache() = default;

    /**
     * Retrieves a value by key, promoting it to most-recently-used.
     * @return The value if found, std::nullopt otherwise.
     */
    std::optional<V> Get(const std::string& key)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);

        auto it = map_.find(key);
        if (it == map_.end()) {
            return std::nullopt;
        }

        // Move to front (most recently used)
        list_.splice(list_.begin(), list_, it->second);
        return it->second->second;
    }

    /**
     * Inserts or updates a key-value pair. If the key already exists,
     * the value is updated and the key is promoted to most-recently-used.
     * If the cache is at capacity, the least-recently-used entry is evicted.
     */
    void Put(const std::string& key, const V& value)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);

        auto it = map_.find(key);
        if (it != map_.end()) {
            // Update existing entry
            it->second->second = value;
            list_.splice(list_.begin(), list_, it->second);
            return;
        }

        // Evict if at capacity
        while (maxSize_ > 0 && list_.size() >= maxSize_) {
            auto last = list_.back();
            if (evictCallback_) {
                evictCallback_(last.first, last.second);
            }
            map_.erase(last.first);
            list_.pop_back();
        }

        // Insert new entry at front
        list_.emplace_front(key, value);
        map_[key] = list_.begin();
    }

    /**
     * Removes an entry by key.
     * @return true if the entry was found and removed.
     */
    bool Remove(const std::string& key)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);

        auto it = map_.find(key);
        if (it == map_.end()) {
            return false;
        }

        list_.erase(it->second);
        map_.erase(it);
        return true;
    }

    /** Returns true if the key exists in the cache (does not affect LRU order). */
    bool Contains(const std::string& key) const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return map_.find(key) != map_.end();
    }

    /** Removes all entries from the cache. */
    void Clear()
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        list_.clear();
        map_.clear();
    }

    /** Returns the current number of entries. */
    size_t Size() const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return list_.size();
    }

    /** Returns the maximum number of entries. */
    size_t MaxSize() const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return maxSize_;
    }

    /**
     * Changes the maximum cache size. If the new size is smaller than
     * the current number of entries, LRU entries are evicted.
     */
    void SetMaxSize(size_t maxSize)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        maxSize_ = maxSize;

        while (maxSize_ > 0 && list_.size() > maxSize_) {
            auto last = list_.back();
            if (evictCallback_) {
                evictCallback_(last.first, last.second);
            }
            map_.erase(last.first);
            list_.pop_back();
        }
    }

    /** Sets a callback invoked when entries are evicted. */
    void SetEvictCallback(EvictCallback callback)
    {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        evictCallback_ = std::move(callback);
    }

    /**
     * Iterates over all entries (from most-recent to least-recent).
     * The callback receives (key, value). Iteration is done under a shared lock.
     */
    void ForEach(std::function<void(const std::string&, const V&)> callback) const
    {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        for (const auto& entry : list_) {
            callback(entry.first, entry.second);
        }
    }

private:
    using ListIterator = typename std::list<std::pair<std::string, V>>::iterator;

    size_t maxSize_;
    std::list<std::pair<std::string, V>> list_;
    std::unordered_map<std::string, ListIterator> map_;
    mutable std::shared_mutex mutex_;
    EvictCallback evictCallback_;
};

} // namespace OHOS::Plugin::CacheDownload

#endif // PLUGINS_REQUEST_CACHE_DOWNLOAD_CACHE_LRU_H
