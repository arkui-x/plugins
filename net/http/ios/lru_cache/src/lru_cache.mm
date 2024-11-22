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

#include "lru_cache.h"

#include <algorithm>
#include <mutex>

#include "netstack_log.h"
#import <Foundation/Foundation.h>

static constexpr const char *LRU_INDEX = "LRUIndex";
static constexpr const int DECIMAL_BASE = 10;
static constexpr const int MAX_SIZE = 1024 * 1024;
static constexpr const size_t INVALID_SIZE = SIZE_MAX;

namespace OHOS::NetStack::Http {
static size_t GetMapValueSize(const std::unordered_map<std::string, std::string> &m)
{
    size_t size = 0;
    for (const auto &p : m) {
        if (p.second.size() > MAX_SIZE) {
            return INVALID_SIZE;
        }
        if (size + p.second.size() > MAX_SIZE) {
            return INVALID_SIZE;
        }
        size += p.second.size();
    }
    if (size > MAX_SIZE || size == 0) {
        return INVALID_SIZE;
    }
    return size;
}

LRUCache::Node::Node(std::string key, std::unordered_map<std::string, std::string> value)
    : key(std::move(key)), value(std::move(value))
{
}

LRUCache::LRUCache() : capacity_(MAX_SIZE), size_(0) {}

LRUCache::LRUCache(size_t capacity) : capacity_(std::min<size_t>(MAX_SIZE, capacity)), size_(0) {}

void LRUCache::AddNode(const Node &node)
{
    nodeList_.emplace_front(node);
    cache_[node.key] = nodeList_.begin();
    size_ += GetMapValueSize(node.value);
}

void LRUCache::MoveNodeToHead(const std::list<Node>::iterator &it)
{
    std::string key = it->key;
    std::unordered_map<std::string, std::string> value = it->value;
    nodeList_.erase(it);
    nodeList_.emplace_front(key, value);
    cache_[key] = nodeList_.begin();
}

void LRUCache::EraseTailNode()
{
    if (nodeList_.empty()) {
        return;
    }
    Node node = nodeList_.back();
    nodeList_.pop_back();
    cache_.erase(node.key);
    size_ -= GetMapValueSize(node.value);
}

std::unordered_map<std::string, std::string> LRUCache::Get(const std::string &key)
{
    std::lock_guard<std::mutex> guard(mutex_);

    if (cache_.find(key) == cache_.end()) {
        return {};
    }
    auto it = cache_[key];
    auto value = it->value;
    MoveNodeToHead(it);
    return value;
}

void LRUCache::Put(const std::string &key, const std::unordered_map<std::string, std::string> &value)
{
    std::lock_guard<std::mutex> guard(mutex_);

    if (GetMapValueSize(value) == INVALID_SIZE) {
        NETSTACK_LOGE("value is invalid(0 or too long) can not insert to cache");
        return;
    }

    if (cache_.find(key) == cache_.end()) {
        AddNode(Node(key, value));
        while (size_ > capacity_) {
            EraseTailNode();
        }
        return;
    }

    auto it = cache_[key];

    size_ -= GetMapValueSize(it->value);
    it->value = value;
    size_ += GetMapValueSize(it->value);

    MoveNodeToHead(it);
    while (size_ > capacity_) {
        EraseTailNode();
    }
}

void LRUCache::MergeOtherCache(const LRUCache &other)
{
    std::list<Node> reverseList;
    {
        // set mutex in min scope
        std::lock_guard<std::mutex> guard(mutex_);
        if (other.nodeList_.empty()) {
            return;
        }
        reverseList = other.nodeList_;
    }
    reverseList.reverse();
    for (const auto &node : reverseList) {
        Put(node.key, node.value);
    }
}

std::string LRUCache::WriteCacheToJsonValue()
{
    NSMutableDictionary *root = [[NSMutableDictionary alloc] init];
    int index = 0;
    {
        // set mutex in min scope
        std::lock_guard<std::mutex> guard(mutex_);
        for (const auto &node : nodeList_) {
            NSMutableDictionary *nodeKey = [[NSMutableDictionary alloc] init];
            for (const auto &p : node.value) {
                NSData *rootData = [[NSData alloc] initWithBytes:p.second.c_str() length:p.second.length()];
                NSError *error;
                id rootObject = [NSJSONSerialization JSONObjectWithData:rootData options:0 error:&error];
                if (rootObject == nil) {
                    NETSTACK_LOGE("Error serializing JSON data.");
                    return "";
                }
                [nodeKey setObject:rootObject forKey:[NSString stringWithCString:p.first.c_str() encoding:NSUTF8StringEncoding]];
                
            }
            [nodeKey setObject:[NSString stringWithFormat:@"%d", index] forKey:[NSString stringWithUTF8String:LRU_INDEX]];
            index++;
            [root setObject:nodeKey forKey:[NSString stringWithUTF8String:node.key.c_str()]];
        }
    }
    NSError *error;
    NSData *jsonData = [NSJSONSerialization dataWithJSONObject:root options:0 error:&error];
    if (jsonData == nil) {
        return "";
    }
    NSString *jsonString = [[NSString alloc] initWithData:jsonData encoding:NSUTF8StringEncoding];
    std::string json([jsonString UTF8String]);
    return json;
}

void LRUCache::ReadCacheFromJsonValue(const std::string root)
{
    NSData *rootData = [[NSData alloc] initWithBytes:root.c_str() length:root.length()];
    NSError *error;
    id rootObject = [NSJSONSerialization JSONObjectWithData:rootData options:0 error:&error];
    if (rootObject == nil) {
        NETSTACK_LOGE("parse json not success, maybe file is broken.");
        return;
    }
    
    std::vector<Node> nodeVec;
    NSUInteger rootCount = [rootObject count];
    for (uint32_t i = 0; i < rootCount; i++) {
        id keyItem = [rootObject objectAtIndex:i];
        if (keyItem == nil || ![keyItem isKindOfClass:[NSDictionary class]]) {
            continue;
        }
        NSArray *allKeys = [keyItem allKeys];
        NSString *firstKey = allKeys[0];

        
        std::unordered_map<std::string, std::string> m;
        NSUInteger subCount = [keyItem count];
        for (uint32_t j = 0; j < subCount; j++) {
            id subItem = [keyItem objectAtIndex:j];
            if (subItem == nil) {
                continue;
            }
            NSArray *subAllKeys = [subItem allKeys];
            NSString *subFirstKey = subAllKeys[0];
            m[[subFirstKey cStringUsingEncoding:NSUTF8StringEncoding]] = [subItem[subAllKeys[0]] cStringUsingEncoding:NSUTF8StringEncoding];
        }
        if (m.find(LRU_INDEX) != m.end()) {
            nodeVec.emplace_back([firstKey cStringUsingEncoding:NSUTF8StringEncoding], m);
        }
    }
    std::sort(nodeVec.begin(), nodeVec.end(), [](Node &a, Node &b) {
        return std::strtol(a.value[LRU_INDEX].c_str(), nullptr, DECIMAL_BASE) >
               std::strtol(b.value[LRU_INDEX].c_str(), nullptr, DECIMAL_BASE);
    });

    for (auto &node : nodeVec) {
        node.value.erase(LRU_INDEX);
        if (!node.value.empty()) {
            Put(node.key, node.value);
        }
    }
}

void LRUCache::Clear()
{
    std::lock_guard<std::mutex> guard(mutex_);
    cache_.clear();
    nodeList_.clear();
}
} // namespace OHOS::NetStack::Http
