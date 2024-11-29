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

#ifndef COMMUNICATIONNETSTACK_LRU_CACHE_DISK_IOS_HANDLER_H
#define COMMUNICATIONNETSTACK_LRU_CACHE_DISK_IOS_HANDLER_H

#include <atomic>

#include "disk_handler.h"
#include "lru_cache.h"

static constexpr const int MAX_DISK_CACHE_SIZE = 1024 * 1024 * 10;
static constexpr const int MIN_DISK_CACHE_SIZE = 1024 * 1024;

namespace OHOS::NetStack::Http {
class LRUCacheDiskHandler {
public:
    LRUCacheDiskHandler() = delete;

    LRUCacheDiskHandler(std::string fileName, size_t capacity);

    void WriteCacheToJsonFile();

    void ReadCacheFromJsonFile();

    void Delete();

    void SetCapacity(size_t capacity);

    std::unordered_map<std::string, std::string> Get(const std::string &key);

    void Put(const std::string &key, const std::unordered_map<std::string, std::string> &value);

private:
    LRUCache cache_;
    DiskHandler diskHandler_;
    std::atomic<size_t> capacity_;

    std::string ReadJsonValueFromFile();

    void WriteJsonValueToFile(const std::string root);
};
} // namespace OHOS::NetStack::Http
#endif /* COMMUNICATIONNETSTACK_LRU_CACHE_DISK_IOS_HANDLER_H */
