/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef PLUGIN_MOCK_AV_SHARED_MEMORY_H
#define PLUGIN_MOCK_AV_SHARED_MEMORY_H

#include <string>
#include "buffer/avsharedmemory.h"
#include "nocopyable.h"

namespace OHOS {
namespace Media {
class __attribute__((visibility("default"))) MockAVSharedMemory : public AVSharedMemory, public NoCopyable {
public:
    ~MockAVSharedMemory()
    {
        if (base_ != nullptr) {
            free(base_);
            base_ = nullptr;
        }
    }

    /**
     * @brief Construct a new AVSharedMemoryBase object. This function should only be used in the
     * local process.
     *
     * @param size the memory's size, bytes.
     * @param flags the memory's accessible flags, refer to {@AVSharedMemory::Flags}.
     * @param name the debug string
     */
    MockAVSharedMemory(int32_t size, int flag)
    {
        base_ = (uint8_t *)malloc(size);
        capacity_ = size;
        flags_ = flag;
    }

    /**
     * @brief Get the memory's virtual address
     * @return the memory's virtual address if the memory is valid, otherwise nullptr.
     */
    virtual uint8_t *GetBase() const override
    {
        return base_;
    }

    /**
     * @brief Get the memory's size
     * @return the memory's size if the memory is valid, otherwise -1.
     */
    virtual int32_t GetSize() const override
    {
        return (base_ != nullptr) ? capacity_ : -1;
    }

    /**
     * @brief Get the memory's flags set by the creator, refer to {@Flags}
     * @return the memory's flags if the memory is valid, otherwise 0.
     */
    virtual uint32_t GetFlags() const final
    {
        return (base_ != nullptr) ? flags_ : 0;
    }

private:
    uint8_t *base_;
    int32_t capacity_;
    uint32_t flags_;
};
} // namespace Media
} // namespace OHOS
#endif
