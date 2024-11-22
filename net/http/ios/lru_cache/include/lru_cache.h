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

#ifndef COMMUNICATIONNETSTACK_LRU_CACHE_IOS_H
#define COMMUNICATIONNETSTACK_LRU_CACHE_IOS_H

#include <list>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace OHOS::NetStack::Http {
class LRUCache {
public:
    LRUCache();

    explicit LRUCache(size_t capacity);

    std::unordered_map<std::string, std::string> Get(const std::string &key);

    void Put(const std::string &key, const std::unordered_map<std::string, std::string> &value);

    void MergeOtherCache(const LRUCache &other);

    std::string WriteCacheToJsonValue();

    void ReadCacheFromJsonValue(const std::string root);

    void Clear();

private:
    struct Node {
        std::string key;
        std::unordered_map<std::string, std::string> value;

        Node() = delete;

        Node(std::string key, std::unordered_map<std::string, std::string> value);
    };

    void AddNode(const Node &node);

    void MoveNodeToHead(const std::list<Node>::iterator &it);

    void EraseTailNode();

    std::mutex mutex_;
    std::unordered_map<std::string, std::list<Node>::iterator> cache_;
    std::list<Node> nodeList_;
    size_t capacity_;
    size_t size_;
};
} // namespace OHOS::NetStack::Http
#endif /* COMMUNICATIONNETSTACK_LRU_CACHE_IOS_H */
