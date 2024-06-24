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

#include "lru_cache_disk_handler.h"

#include <thread>

#include "netstack_log.h"

namespace OHOS::NetStack::Http {
LRUCacheDiskHandler::LRUCacheDiskHandler(std::string fileName, size_t capacity)
    : diskHandler_(std::move(fileName)),
      capacity_(std::max<size_t>(std::min<size_t>(MAX_DISK_CACHE_SIZE, capacity), MIN_DISK_CACHE_SIZE))
{
}

void LRUCacheDiskHandler::SetCapacity(size_t capacity)
{
    capacity_ = std::max<size_t>(std::min<size_t>(MAX_DISK_CACHE_SIZE, capacity), MIN_DISK_CACHE_SIZE);
    WriteCacheToJsonFile();
}

void LRUCacheDiskHandler::Delete()
{
    cache_.Clear();
    diskHandler_.Delete();
}

std::string LRUCacheDiskHandler::ReadJsonValueFromFile()
{
    std::string jsonStr = diskHandler_.Read();
    return jsonStr;
}

void LRUCacheDiskHandler::WriteJsonValueToFile(const std::string root)
{
    diskHandler_.Write(root);
}

void LRUCacheDiskHandler::WriteCacheToJsonFile()
{
    LRUCache oldCache(capacity_);
    std::string readRoot = ReadJsonValueFromFile();
    oldCache.ReadCacheFromJsonValue(readRoot);

    oldCache.MergeOtherCache(cache_);
    std::string writeRoot = oldCache.WriteCacheToJsonValue();
    WriteJsonValueToFile(writeRoot);
    cache_.Clear();
}

void LRUCacheDiskHandler::ReadCacheFromJsonFile()
{
    std::string root = ReadJsonValueFromFile();
    cache_.ReadCacheFromJsonValue(root);
}

std::unordered_map<std::string, std::string> LRUCacheDiskHandler::Get(const std::string &key)
{
    auto valueFromMemory = cache_.Get(key);
    if (!valueFromMemory.empty()) {
        return valueFromMemory;
    }

    LRUCache diskCache(capacity_);
    std::string root = ReadJsonValueFromFile();
    diskCache.ReadCacheFromJsonValue(root);
    auto valueFromDisk = diskCache.Get(key);
    cache_.Put(key, valueFromDisk);
    return valueFromDisk;
}

void LRUCacheDiskHandler::Put(const std::string &key, const std::unordered_map<std::string, std::string> &value)
{
    cache_.Put(key, value);
}
} // namespace OHOS::NetStack::Http
