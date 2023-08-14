/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_BRIDGE_STREAM_READER_H
#define PLUGINS_BRIDGE_STREAM_READER_H

#include <cstdint>
#include <iostream>

#include "log.h"
#include "securec.h"

namespace OHOS::Plugin::Bridge {
class BridgeStreamReader {
public:
    explicit BridgeStreamReader(const uint8_t* bytes, size_t size) : bytes_(bytes), size_(size) {}
    explicit BridgeStreamReader() = default;
    virtual ~BridgeStreamReader() = default;

    constexpr static uint8_t READER_ERROR = 0;

    uint8_t ReadByte()
    {
        if (currentPos_ >= size_) {
            LOGE("invalid read in readbyte function.");
            return READER_ERROR;
        }
        return bytes_[currentPos_++];
    }

    void ReadBytes(uint8_t* buffer, size_t length)
    {
        if (currentPos_ + length > size_) {
            LOGE("invalid read in readbyte function.");
            return;
        }
        if (memcpy_s(buffer, length, &bytes_[currentPos_], length) != EOK) {
            LOGE("invalid read in memcpy_s function.");
            return;
        };
        currentPos_ += length;
    }

    int32_t ReadInt32()
    {
        int32_t value { 0 };
        ReadBytes(reinterpret_cast<uint8_t*>(&value), 4);
        return value;
    }

    int64_t ReadInt64()
    {
        int64_t value { 0 };
        ReadBytes(reinterpret_cast<uint8_t*>(&value), 8);
        return value;
    }

    double ReadDouble()
    {
        double value { 0 };
        ReadBytes(reinterpret_cast<uint8_t*>(&value), 8);
        return value;
    }

    void ReadAlignment(uint8_t alignment)
    {
        uint8_t offset = currentPos_ % alignment;
        if (offset) {
            currentPos_ += alignment - offset;
        }
    }

private:
    const uint8_t* bytes_;
    size_t size_;
    size_t currentPos_ { 0 };
};
} // OHOS::Plugin::Bridge
#endif