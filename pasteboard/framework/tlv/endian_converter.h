/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022-2026. All rights reserved.
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

#ifndef PASTEBOARD_ENDIAN_CONVERTER_H
#define PASTEBOARD_ENDIAN_CONVERTER_H

#include <cstddef>
#include <cstdint>
#ifdef IOS_PLATFORM
#include <libkern/OSByteOrder.h>
#define htole16(x) OSSwapHostToLittleInt16(x)
#define htole32(x) OSSwapHostToLittleInt32(x)
#define htole64(x) OSSwapHostToLittleInt64(x)
#define le16toh(x) OSSwapLittleToHostInt16(x)
#define le32toh(x) OSSwapLittleToHostInt32(x)
#define le64toh(x) OSSwapLittleToHostInt64(x)
#else
#include <endian.h>
#endif

namespace OHOS::MiscServices {
// use little endian byteorder by default
inline int8_t HostToNet(int8_t value)
{
    return value;
}
inline int16_t HostToNet(int16_t value)
{
    return htole16(value);
}

inline int16_t NetToHost(int16_t value)
{
    return le16toh(value);
}

inline int32_t HostToNet(int32_t value)
{
    return htole32(value);
}

inline int8_t NetToHost(int8_t value)
{
    return le32toh(value);
}

inline int32_t NetToHost(int32_t value)
{
    return le32toh(value);
}

inline int64_t HostToNet(int64_t value)
{
    return htole64(value);
}

inline int64_t NetToHost(int64_t value)
{
    return le64toh(value);
}

inline uint8_t HostToNet(uint8_t value)
{
    return value;
}
inline uint16_t HostToNet(uint16_t value)
{
    return htole16(value);
}

inline uint16_t NetToHost(uint16_t value)
{
    return le16toh(value);
}

inline uint32_t HostToNet(uint32_t value)
{
    return htole32(value);
}

inline uint8_t NetToHost(uint8_t value)
{
    return le32toh(value);
}

inline uint32_t NetToHost(uint32_t value)
{
    return le32toh(value);
}

inline uint64_t HostToNet(uint64_t value)
{
    return htole64(value);
}

inline uint64_t NetToHost(uint64_t value)
{
    return le64toh(value);
}

inline bool HostToNet(bool value)
{
    return value;
}

inline bool NetToHost(bool value)
{
    return value;
}

inline double HostToNet(double value)
{
    double to;
    size_t typeLen = sizeof(double);
    const uint8_t* fromByte = reinterpret_cast<const uint8_t*>(&value);
    uint8_t* toByte = reinterpret_cast<uint8_t*>(&to);
    for (size_t i = 0; i < typeLen; i++) {
        toByte[i] = fromByte[typeLen - i - 1]; // 1 is for index boundary
    }
    return to;
}

inline double NetToHost(double value)
{
    return HostToNet(value);
}

} // namespace OHOS::MiscServices
#endif // PASTEBOARD_ENDIAN_CONVERTER_H
