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

#ifndef FFRT_API_FFRT_INNER_H
#define FFRT_API_FFRT_INNER_H

#include <functional>
#include <string>
#include <vector>

namespace ffrt {
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
};

class task_handle {};

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
#endif
