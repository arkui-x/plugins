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

#ifndef WRITE_CONTROLLER_H
#define WRITE_CONTROLLER_H

#include <list>
#include <mutex>
#include <sys/time.h>
#include <unordered_map>

namespace OHOS {
namespace HiviewDFX {
static constexpr uint64_t INVALID_TIME_STAMP = 0;
static constexpr size_t DEFAULT_CACHE_CAPACITY = 30;
static constexpr size_t HISYSEVENT_DEFAULT_PERIOD = 5;
static constexpr size_t HISYSEVENT_DEFAULT_THRESHOLD = 100;

using ControlParam = struct {
    size_t period;
    size_t threshold;
};

using CallerInfo = struct {
    const char* func;
    int64_t line;
    uint64_t timeStamp;
};

template<typename K, typename V, size_t capacity = DEFAULT_CACHE_CAPACITY>
class LruCache {
public:
    V Get(K key)
    {
        V v;
        if (key2Index.count(key) == 0) {
            return v;
        }
        Modify(key);
        return key2Index[key].value;
    }

    void Put(K key, V value)
    {
        if (key2Index.count(key) > 0) {
            key2Index[key].value = value;
            Modify(key);
            return;
        }
        if (keyCache.size() == capacity) {
            key2Index.erase(keyCache.back());
            keyCache.pop_back();
        }
        keyCache.push_front(key);
        key2Index[key] = {
            .iter = keyCache.cbegin(),
            .value = value
        };
    }

private:
    template<typename K_, typename V_>
    struct CacheNode {
        typename std::list<K_>::const_iterator iter;
        V_ value;
    };

private:
    void Modify(K key)
    {
        keyCache.splice(keyCache.begin(), keyCache, key2Index[key].iter);
        key2Index[key].iter = keyCache.cbegin();
    }

private:
    std::unordered_map<K, CacheNode<K, V>> key2Index;
    std::list<K> keyCache;
};

class WriteController {
public:
    uint64_t GetCurrentTimeMills();
    uint64_t CheckLimitWritingEvent(const ControlParam& param, const char* domain, const char* eventName,
        const CallerInfo& callerInfo);
    uint64_t CheckLimitWritingEvent(const ControlParam& param, const char* domain, const char* eventName,
        const char* func, int64_t line);

private:
    struct EventLimitStat {
        size_t count;
        uint64_t begin;

    public:
        EventLimitStat()
        {
            count = 0;
            begin = INVALID_TIME_STAMP;
        }

    public:
        bool IsValid()
        {
            return count > 0;
        }
    };

private:
    uint64_t ConcatenateInfoAsKey(const char* eventName, const char* func, int64_t line) const;

private:
    std::mutex lmtMutex;
    LruCache<uint64_t, EventLimitStat> lruCache;
};
} // HiviewDFX
} // OHOS
#endif // WRITE_CONTROLLER_H
