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

#ifndef PLUGINS_BRIDGE_BINARY_MARSHALLER_H
#define PLUGINS_BRIDGE_BINARY_MARSHALLER_H

#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#include "log.h"

namespace OHOS::Plugin::Bridge {
class BridgeBinaryMarshaller {
public:
    explicit BridgeBinaryMarshaller(std::vector<uint8_t>* bytebuffer) : byteBuffer_(bytebuffer) {}
    virtual ~BridgeBinaryMarshaller() = default;

    constexpr static uint8_t MARSHALL_SIZE_0 = 0;
    constexpr static uint8_t MARSHALL_SIZE_1 = 1;
    constexpr static uint8_t MARSHALL_SIZE_2 = 2;
    constexpr static uint8_t MARSHALL_SIZE_4 = 4;
    constexpr static uint8_t MARSHALL_SIZE_8 = 8;

    void MarshallingByte(uint8_t byte) { byteBuffer_->push_back(byte); }

    void MarshallingBytes(const uint8_t* buffer, size_t size)
    {
        if (size <= 0) {
            LOGE("invalid write in writebyte function.");
            return;
        }
        byteBuffer_->insert(byteBuffer_->end(), buffer, buffer + size);
    }

    void MarshallingInt32(int32_t int32Value)
    {
        MarshallingBytes(reinterpret_cast<const uint8_t*>(&int32Value), MARSHALL_SIZE_4);
    }

    void MarshallingInt64(int64_t int64Value)
    {
        MarshallingBytes(reinterpret_cast<const uint8_t*>(&int64Value), MARSHALL_SIZE_8);
    }

    void MarshallingDouble(double doubleValue)
    {
        MarshallingBytes(reinterpret_cast<const uint8_t*>(&doubleValue), MARSHALL_SIZE_8);
    }

    void MarshallingAlign(uint8_t align)
    {
        if (uint8_t offset = byteBuffer_->size() % align; offset) {
            int i = 0; 
            while (i < align - offset) {
                MarshallingByte(0);
                ++i;
            }
        }
    }

private:
    std::vector<uint8_t>* byteBuffer_;
};
} // OHOS::Plugin::Bridge 
#endif