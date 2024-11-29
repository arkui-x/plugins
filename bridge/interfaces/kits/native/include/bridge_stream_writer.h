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

#ifndef PLUGINS_BRIDGE_STREAM_WRITER_H
#define PLUGINS_BRIDGE_STREAM_WRITER_H

#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#include "log.h"

namespace OHOS::Plugin::Bridge {
class BridgeStreamWriter {
public:
    explicit BridgeStreamWriter(std::vector<uint8_t>* bytebuffer) : bytes_(bytebuffer) {}
    virtual ~BridgeStreamWriter() = default;

    void WriteByte(uint8_t byte) { bytes_->push_back(byte); }

    void WriteBytes(const uint8_t* bytes, size_t length)
    {
        if (length <= 0) {
            LOGE("invalid write in writebyte function.");
            return;
        }
        bytes_->insert(bytes_->end(), bytes, bytes + length);
    }

    void WriteInt32(int32_t value)
    {
        WriteBytes(reinterpret_cast<const uint8_t*>(&value), 4);
    }

    void WriteInt64(int64_t value)
    {
        WriteBytes(reinterpret_cast<const uint8_t*>(&value), 8);
    }

    void WriteDouble(double value)
    {
        WriteBytes(reinterpret_cast<const uint8_t*>(&value), 8);
    }

    void WriteAlignment(uint8_t alignment)
    {
        uint8_t offset = bytes_->size() % alignment;
        if (offset) {
            for (int i = 0; i < alignment - offset; ++i) {
                WriteByte(0);
            }
        }
    }

private:
    std::vector<uint8_t>* bytes_;
};
} // OHOS::Plugin::Bridge 
#endif