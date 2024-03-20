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

#ifndef HISYSEVENT_INTERFACE_ENCODE_INCLUDE_RAW_DATA_ENCODER_H
#define HISYSEVENT_INTERFACE_ENCODE_INCLUDE_RAW_DATA_ENCODER_H

#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "raw_data_base_def.h"
#include "raw_data.h"

namespace OHOS {
namespace HiviewDFX {
namespace Encoded {
enum DataCodedType: uint8_t {
    UNSIGNED_VARINT,
    UNSIGNED_VARINT_ARRAY,
    SIGNED_VARINT,
    SIGNED_VARINT_ARRAY,
    FLOATING,
    FLOATING_ARRAY,
    DSTRING,
    DSTRING_ARRAY,
};

class RawDataEncoder {
public:
    static bool ValueTypeEncoded(RawData& data, bool isArray, ValueType valueType,
        uint8_t count);
    static bool StringValueEncoded(RawData& data, const std::string& val);

public:
    // uintx_t -> uint64_t
    template<typename T>
    static bool UnsignedVarintEncoded(RawData& data, const EncodeType type, T val)
    {
        uint8_t cpyVal = EncodedTag(static_cast<uint8_t>(type)) | ((val < TAG_BYTE_BOUND) ? 0 : TAG_BYTE_BOUND) |
            static_cast<uint8_t>(val & TAG_BYTE_MASK);
        if (!data.Append(reinterpret_cast<uint8_t*>(&cpyVal), 1)) {
            return false;
        }
        val >>= TAG_BYTE_OFFSET;
        while (val > 0) {
            cpyVal = ((val < NON_TAG_BYTE_BOUND) ? 0 : NON_TAG_BYTE_BOUND) |
                static_cast<uint8_t>(val & NON_TAG_BYTE_MASK);
            if (!data.Append(reinterpret_cast<uint8_t*>(&cpyVal), 1)) {
                return false;
            }
            val >>= NON_TAG_BYTE_OFFSET;
        }
        return true;
    }

    // bool, intx_t -> int64_t
    template<typename T>
    static bool SignedVarintEncoded(RawData& data, const EncodeType type, T val)
    {
        int64_t valInt64 = static_cast<int64_t>(val);
        // zigzag encode
        uint64_t uValInt64 = static_cast<uint64_t>((valInt64 << 1) ^ (valInt64 >> ((sizeof(valInt64) << 3) - 1)));
        return UnsignedVarintEncoded(data, type, uValInt64);
    }

    // float, double => double
    template<typename T>
    static bool FloatingNumberEncoded(RawData& data, T val)
    {
        T valFdl = static_cast<T>(val);
        if (!UnsignedVarintEncoded(data, EncodeType::LENGTH_DELIMITED, sizeof(T))) {
            return false;
        }
        if (!data.Append(reinterpret_cast<uint8_t*>(&valFdl), sizeof(T))) {
            return false;
        }
        return true;
    }

private:
    static uint8_t EncodedTag(uint8_t type);

private:
    static constexpr int TAG_BYTE_OFFSET = 5;
    static constexpr int TAG_BYTE_BOUND  = (1 << TAG_BYTE_OFFSET);
    static constexpr int TAG_BYTE_MASK = (TAG_BYTE_BOUND - 1);

    static constexpr int NON_TAG_BYTE_OFFSET = 7;
    static constexpr int NON_TAG_BYTE_BOUND = (1 << NON_TAG_BYTE_OFFSET);
    static constexpr int NON_TAG_BYTE_MASK = (NON_TAG_BYTE_BOUND - 1);
};
} // namespace Encoded
} // namespace HiviewDFX
} // namespace OHOS
#endif // HISYSEVENT_INTERFACE_ENCODE_INCLUDE_RAW_DATA_ENCODER_H
