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

#ifndef PLUGINS_BRIDGE_BINARY_UNMARSHALLER_H
#define PLUGINS_BRIDGE_BINARY_UNMARSHALLER_H

#include <cstdint>
#include <iostream>

#include "log.h"
#include "securec.h"

namespace OHOS::Plugin::Bridge {
class BridgeBinaryUnmarshaller {
public:
    BridgeBinaryUnmarshaller(const uint8_t* byteBuffer, size_t size) : byteBuffer_(byteBuffer), size_(size) {}
    BridgeBinaryUnmarshaller() = default;
    virtual ~BridgeBinaryUnmarshaller() = default;

    constexpr static uint8_t UNMARSHALL_ERROR = 0;
    constexpr static uint8_t UNMARSHALL_SIZE_0 = 0;
    constexpr static uint8_t UNMARSHALL_SIZE_1 = 1;
    constexpr static uint8_t UNMARSHALL_SIZE_2 = 2;
    constexpr static uint8_t UNMARSHALL_SIZE_4 = 4;
    constexpr static uint8_t UNMARSHALL_SIZE_8 = 8;

    uint8_t UnmarshallingByte()
    {
        if (currentPos_ < size_) {
            return byteBuffer_[currentPos_++];
        }
        LOGE("UnmarshallingByte fail.");
        return UNMARSHALL_ERROR;
    }

    void UnmarshallingBytes(uint8_t* buffer, size_t size)
    {
        if (currentPos_ + size <= size_) {
            if (memcpy_s(buffer, size, &byteBuffer_[currentPos_], size) != EOK) {
                LOGE("UnmarshallingBytes memcpy_s fail.");
                return;
            };
            currentPos_ += size;
            return;
        }

        LOGE("UnmarshallingBytes fail.");
        return;
    }

    int32_t UnmarshallingInt32()
    {
        int32_t int32Value = 0;
        UnmarshallingBytes(reinterpret_cast<uint8_t*>(&int32Value), UNMARSHALL_SIZE_4);
        return int32Value;
    }

    int64_t UnmarshallingInt64()
    {
        int64_t int64Value = 0;
        UnmarshallingBytes(reinterpret_cast<uint8_t*>(&int64Value), UNMARSHALL_SIZE_8);
        return int64Value;
    }

    double UnmarshallingDouble()
    {
        double doubleValue = 0;
        UnmarshallingBytes(reinterpret_cast<uint8_t*>(&doubleValue), UNMARSHALL_SIZE_8);
        return doubleValue;
    }

    void UnmarshallingAlign(uint8_t align)
    {
        if (uint8_t offset = currentPos_ % align; offset) {
            currentPos_ += align - offset;
        }
    }

private:
    const uint8_t* byteBuffer_ = nullptr;
    size_t size_ = 0;
    size_t currentPos_ = 0;
};
} // OHOS::Plugin::Bridge
#endif