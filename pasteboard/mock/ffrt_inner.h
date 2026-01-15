/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

#ifndef PLUGIN_PASTEBOARD_MOCK_FFRT_INNER_H
#define PLUGIN_PASTEBOARD_MOCK_FFRT_INNER_H

#include <cstdint>
#include <functional>
#include <initializer_list>
#include <string>
#include <utility>
#include <vector>

namespace ffrt {

// Common QoS placeholders (values are not meaningful in mock).
inline constexpr int32_t qos_default = 0;
inline constexpr int32_t qos_user_interactive = 1;
inline constexpr int32_t qos_user_initiated = 2;
inline constexpr int32_t qos_utility = 3;
inline constexpr int32_t qos_background = 4;
inline constexpr int32_t qos_deadline_request = 5;
inline constexpr int32_t qos_inherit = -1;

class mutex {
public:
    mutex() = default;
    ~mutex() = default;

    mutex(const mutex&) = delete;
    mutex& operator=(const mutex&) = delete;

    void lock() {}
    bool try_lock()
    {
        return true;
    }
    void unlock() {}
};

class task_attr_private {
public:
    std::string name_;
};

class task_attr {
public:
    inline task_attr& name(const char* name)
    {
        return *this;
    }

    inline task_attr& delay(uint64_t delay_us)
    {
        return *this;
    }

    inline task_attr& qos(int32_t qos)
    {
        return *this;
    }

    inline task_attr& qos(uint32_t qos)
    {
        return *this;
    }
};

class task_handle {};

// Global submit APIs (mock: run synchronously).
template<typename F>
inline void submit(F&& func)
{
    std::forward<F>(func)();
}

template<typename F>
inline void submit(F&& func, std::initializer_list<const void*> inDeps, std::initializer_list<const void*> outDeps,
    const task_attr& attr = {})
{
    std::thread t(std::forward<F>(func));
    t.detach();
}

template<typename F, typename... Args>
inline void submit(F&& func, Args&&... unused)
{
    std::thread t(std::forward<F>(func));
    t.detach();
}

template<typename F>
inline task_handle submit_h(F&& func)
{
    std::thread t(std::forward<F>(func));
    t.detach();
    return task_handle();
}

template<typename F>
inline task_handle submit_h(F&& func, std::initializer_list<const void*> inDeps,
    std::initializer_list<const void*> outDeps, const task_attr& attr = {})
{
    std::thread t(std::forward<F>(func));
    t.detach();
    return task_handle();
}

template<typename F, typename... Args>
inline task_handle submit_h(F&& func, Args&&... unused)
{
    std::thread t(std::forward<F>(func));
    t.detach();
    return task_handle();
}

// Global wait APIs (mock: tasks are synchronous, so wait is no-op).
inline void wait() {}

inline void wait(const task_handle& handle) {}

inline void wait(std::initializer_list<task_handle> handles) {}

inline void wait(const std::vector<task_handle>& handles) {}

class queue {
public:
    queue(const char* name) {}

    inline void submit(const std::function<void()>& func, const task_attr& attr = {}) {}

    inline void submit(std::function<void()>&& func, const task_attr& attr = {}) {}

    inline task_handle submit_h(const std::function<void()>& func, const task_attr& attr = {})
    {
        return task_handle();
    }

    inline task_handle submit_h(std::function<void()>&& func, const task_attr& attr = {})
    {
        return task_handle();
    }

    inline int cancel(const task_handle& handle)
    {
        return 0;
    }

    inline void wait(const task_handle& handle) {}
};
} // namespace ffrt
#endif // PLUGIN_PASTEBOARD_MOCK_FFRT_INNER_H
