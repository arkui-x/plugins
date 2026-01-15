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

#ifndef DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_BUFFER_H
#define DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_BUFFER_H

#include <vector>

#include "unified_meta.h"

namespace OHOS::MiscServices {
#pragma pack(1)
struct TLVHead {
    uint16_t tag;
    uint32_t len;
    std::uint8_t value[0];
};
#pragma pack()

/*
 * Common tag definitions.
 * Product should use after TAG_BUFF
 **/
enum COMMON_TAG : uint16_t {
    TAG_VECTOR_ITEM = 0x0000,
    TAG_MAP_KEY,
    TAG_MAP_VALUE, // std::vector<uint8_t>
    TAG_MAP_VALUE_TYPE,
    TAG_VARIANT_INDEX,
    TAG_VARIANT_VALUE,
    TAG_BUFF = 0x0100,
};

using ValueType = std::variant<int32_t, int64_t, bool, double, std::string, std::vector<uint8_t>>;
using Details = std::map<std::string, ValueType>;

using EntryValue = UDMF::ValueType;
using Object = UDMF::Object;

struct TLVBuffer {
public:
    TLVBuffer() : total_(0), cursor_(0) {}
    explicit TLVBuffer(size_t total) : total_(total), cursor_(0) {}
    virtual ~TLVBuffer() = default;

    inline bool Skip(size_t len)
    {
        if (len > total_ - cursor_) {
            return false;
        }
        cursor_ += len;
        return true;
    }

    inline bool IsEnough() const
    {
        return cursor_ < total_;
    }

    inline bool HasExpectBuffer(uint32_t expectLen) const
    {
        return total_ >= cursor_ && total_ - cursor_ >= expectLen;
    }

protected:
    size_t total_ = 0;
    size_t cursor_ = 0;
};
} // namespace OHOS::MiscServices
#endif // DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_BUFFER_H
