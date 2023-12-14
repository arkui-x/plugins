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

#ifndef PLUGINS_BRIDGE_BRIDGE_SERIALIZER_H
#define PLUGINS_BRIDGE_BRIDGE_SERIALIZER_H

#include "bridge_stream_reader.h"
#include "bridge_stream_writer.h"
#include "codecable_value.h"

namespace OHOS::Plugin::Bridge {
class BridgeSerializer {
public:
    static const BridgeSerializer& GetInstance();
    
    virtual ~BridgeSerializer() = default;

    BridgeSerializer(BridgeSerializer const&) = delete;
    BridgeSerializer& operator=(BridgeSerializer const&) = delete;

    CodecableValue ReadValue(BridgeStreamReader* stream) const;
    void WriteValue(const CodecableValue& value, BridgeStreamWriter* stream) const;
    
private:
    BridgeSerializer() = default;

    size_t ReadSize(BridgeStreamReader* stream) const;
    void WriteSize(size_t size, BridgeStreamWriter* stream) const;

    void WriteListBool(const std::vector<bool>& vector, BridgeStreamWriter* stream) const;
    CodecableValue ReadListBool(BridgeStreamReader* stream) const;

    CodecableValue ReadString(BridgeStreamReader* stream) const;
    void WriteString(const std::string& str, BridgeStreamWriter* stream) const;

    void WriteListString(const std::vector<std::string>& vector, BridgeStreamWriter* stream) const;
    CodecableValue ReadListString(BridgeStreamReader* stream) const;

    template <typename T>
    CodecableValue ReadVector(BridgeStreamReader* stream) const;
    template <typename T>
    void WriteVector(const std::vector<T>& vector, BridgeStreamWriter* stream) const;

    CodecableValue ReadMap(BridgeStreamReader* stream) const;
    void WriteMap(const CodecableMap& map, BridgeStreamWriter* stream) const;
};
} // OHOS::Plugin::Bridge
#endif