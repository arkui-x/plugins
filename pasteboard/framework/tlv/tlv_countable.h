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

#ifndef DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_COUNTABLE_H
#define DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_COUNTABLE_H

#include "tlv_buffer.h"
#include "tlv_utils.h"

namespace OHOS::MiscServices {

class TLVCountable {
public:
    virtual size_t CountTLV() const = 0;

    static inline size_t Count(bool value)
    {
        return sizeof(value) + sizeof(TLVHead);
    }

    static inline size_t Count(int8_t value)
    {
        return sizeof(value) + sizeof(TLVHead);
    }

    static inline size_t Count(int16_t value)
    {
        return sizeof(value) + sizeof(TLVHead);
    }

    static inline size_t Count(int32_t value)
    {
        return sizeof(value) + sizeof(TLVHead);
    }

    static inline size_t Count(double value)
    {
        return sizeof(value) + sizeof(TLVHead);
    }

    static inline size_t Count(int64_t value)
    {
        return sizeof(value) + sizeof(TLVHead);
    }

    static inline size_t Count(uint32_t value)
    {
        return sizeof(value) + sizeof(TLVHead);
    }

    static inline size_t Count(const std::string& value)
    {
        return value.size() + sizeof(TLVHead);
    }

    static inline size_t Count(const RawMem& value)
    {
        return value.bufferLen + sizeof(TLVHead);
    }

    static inline size_t Count(const TLVCountable& value)
    {
        return value.CountTLV() + sizeof(TLVHead);
    }

    template<typename T>
    static inline size_t Count(const std::shared_ptr<T>& value)
    {
        if (value == nullptr) {
            return 0;
        }
        return Count(*value);
    }

    template<typename T>
    static inline size_t Count(const std::vector<T>& value)
    {
        size_t expectSize = sizeof(TLVHead);
        for (const auto& item : value) {
            expectSize += Count(item);
        }
        return expectSize;
    }

    static inline size_t Count(const std::vector<uint8_t>& value)
    {
        size_t expectSize = sizeof(TLVHead);
        expectSize += value.size();
        return expectSize;
    }

    static inline size_t Count(const std::map<std::string, std::vector<uint8_t>>& value)
    {
        size_t expectSize = sizeof(TLVHead);
        for (const auto& item : value) {
            expectSize += Count(item.first);
            expectSize += Count(item.second);
        }
        return expectSize;
    }

    static inline size_t Count(const Details& value)
    {
        size_t expectSize = sizeof(TLVHead);
        for (const auto& item : value) {
            expectSize += Count(item.first);
            expectSize += Count(item.second);
        }
        return expectSize;
    }

    static inline size_t Count(const std::shared_ptr<AAFwk::Want>& value)
    {
        if (value == nullptr) {
            return 0;
        }
        return Count(TLVUtils::Want2Json(*value));
    }

    static inline size_t Count(const std::shared_ptr<Media::PixelMap> value)
    {
        if (value == nullptr) {
            return 0;
        }
        size_t expectSize = sizeof(TLVHead);
        return expectSize + Count(TLVUtils::PixelMap2Vector(value));
    }

    static inline size_t Count(const std::shared_ptr<Object>& value)
    {
        if (value == nullptr) {
            return 0;
        }
        size_t expectSize = sizeof(TLVHead);
        for (auto& item : value->value_) {
            expectSize += Count(item.first);
            expectSize += Count(item.second);
        }
        return expectSize;
    }

    static inline size_t Count(const std::monostate& value)
    {
        (void)value;
        return sizeof(TLVHead);
    }

    static inline size_t Count(const void* value)
    {
        (void)value;
        return sizeof(TLVHead);
    }

    template<typename _InTp>
    static inline size_t CountVariant(uint32_t step, const _InTp& input)
    {
        (void)step;
        (void)input;
        return 0;
    }

    template<typename _InTp, typename _First, typename... _Rest>
    static inline size_t CountVariant(uint32_t step, const _InTp& input)
    {
        if (input.index() == std::variant_npos || input.index() < step) {
            return 0;
        }
        if (step == input.index()) {
            return Count(step) + Count(std::get<_First>(input));
        }
        return CountVariant<_InTp, _Rest...>(step + 1, input);
    }

    template<typename... _Types>
    static inline size_t Count(const std::variant<_Types...>& input)
    {
        size_t expectSize = sizeof(TLVHead);
        return expectSize + CountVariant<decltype(input), _Types...>(0, input);
    }
};
} // namespace OHOS::MiscServices
#endif // DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_COUNTABLE_H
