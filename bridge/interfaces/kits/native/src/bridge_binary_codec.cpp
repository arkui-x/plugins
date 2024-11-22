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

#include "bridge_binary_codec.h"

#include "bridge_packager.h"
#include "log.h"

namespace OHOS::Plugin::Bridge {
const BridgeBinaryCodec& BridgeBinaryCodec::GetInstance()
{
    static BridgeBinaryCodec sInstance;
    return sInstance;
}

std::unique_ptr<std::vector<uint8_t>> BridgeBinaryCodec::EncodeInner(const CodecableValue& data) const
{
    auto coded = std::make_unique<std::vector<uint8_t>>();
    BridgeBinaryMarshaller marshaller(coded.get());
    BridgePackager::Marshalling(data, &marshaller);
    return coded;
}

std::vector<uint8_t>* BridgeBinaryCodec::EncodeBuffer(const CodecableValue& data) const
{
    std::vector<uint8_t>* coded = new (std::nothrow) std::vector<uint8_t>();
    BridgeBinaryMarshaller marshaller(coded);
    BridgePackager::Marshalling(data, &marshaller);
    return coded;
}

std::unique_ptr<CodecableValue> BridgeBinaryCodec::DecodeInner(const std::vector<uint8_t>& data) const
{
    size_t size = data.size();
    if (size == BridgeBinaryUnmarshaller::UNMARSHALL_SIZE_0) {
        LOGW("The decode data is size error.");
        return std::make_unique<CodecableValue>();
    }
    const uint8_t* dataPtr = &data[0];
    if (!dataPtr) {
        LOGW("The decode data is nullptr.");
        return std::make_unique<CodecableValue>();
    }
    BridgeBinaryUnmarshaller unMarshaller(dataPtr, size);
    return std::make_unique<CodecableValue>(BridgePackager::UnMarshalling(&unMarshaller));
}

std::unique_ptr<CodecableValue> BridgeBinaryCodec::DecodeBuffer(const uint8_t* dataPtr, size_t size) const
{
    if (!dataPtr || size == BridgeBinaryUnmarshaller::UNMARSHALL_SIZE_0) {
        LOGW("The decode data is error.");
        return std::make_unique<CodecableValue>();
    }
    BridgeBinaryUnmarshaller unMarshaller(dataPtr, size);
    return std::make_unique<CodecableValue>(BridgePackager::UnMarshalling(&unMarshaller));
}
} // OHOS::Plugin::Bridge