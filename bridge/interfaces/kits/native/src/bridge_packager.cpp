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

#include "bridge_packager.h"

#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "log.h"

namespace OHOS::Plugin::Bridge {
const BridgePackager::UnMarshallFuncMap BridgePackager::unMarshallFuncMap = {
    BridgePackager::UnMarshallFuncValueType(CodecableIndex::I_NULL,
        [](BridgeBinaryUnmarshaller*) { return CodecableValue(); }),
    BridgePackager::UnMarshallFuncValueType(CodecableIndex::I_TRUE,
        [](BridgeBinaryUnmarshaller*) { return CodecableValue(true); }),
    BridgePackager::UnMarshallFuncValueType(CodecableIndex::I_FALSE,
        [](BridgeBinaryUnmarshaller*) { return CodecableValue(false); }),
    BridgePackager::UnMarshallFuncValueType(CodecableIndex::I_INT32,
        [](BridgeBinaryUnmarshaller* buffer) { return CodecableValue(buffer->UnmarshallingInt32()); }),
    BridgePackager::UnMarshallFuncValueType(CodecableIndex::I_INT64,
        [](BridgeBinaryUnmarshaller* buffer) { return CodecableValue(buffer->UnmarshallingInt64()); }),
    BridgePackager::UnMarshallFuncValueType(CodecableIndex::I_DOUBLE,
        [](BridgeBinaryUnmarshaller* buffer) {
            buffer->UnmarshallingAlign(8);
            return CodecableValue(buffer->UnmarshallingDouble());
        }),
    BridgePackager::UnMarshallFuncValueType(CodecableIndex::I_STRING,
        [](BridgeBinaryUnmarshaller* buffer) { return BridgePackager::UnMarshallingString(buffer); }),
    BridgePackager::UnMarshallFuncValueType(CodecableIndex::I_LIST_UINT8,
        [](BridgeBinaryUnmarshaller* buffer) { return BridgePackager::UnMarshallingVector<uint8_t>(buffer); }),
    BridgePackager::UnMarshallFuncValueType(CodecableIndex::I_LIST_BOOL,
        [](BridgeBinaryUnmarshaller* buffer) { return BridgePackager::UnMarshallingListBool(buffer); }),
    BridgePackager::UnMarshallFuncValueType(CodecableIndex::I_LIST_INT32,
        [](BridgeBinaryUnmarshaller* buffer) { return BridgePackager::UnMarshallingVector<int32_t>(buffer); }),
    BridgePackager::UnMarshallFuncValueType(CodecableIndex::I_LIST_INT64,
        [](BridgeBinaryUnmarshaller* buffer) { return BridgePackager::UnMarshallingVector<int64_t>(buffer); }),
    BridgePackager::UnMarshallFuncValueType(CodecableIndex::I_LIST_DOUBLE,
        [](BridgeBinaryUnmarshaller* buffer) { return BridgePackager::UnMarshallingVector<double>(buffer); }),
    BridgePackager::UnMarshallFuncValueType(CodecableIndex::I_LIST_STRING,
        [](BridgeBinaryUnmarshaller* buffer) { return BridgePackager::UnMarshallingListString(buffer); }),
    BridgePackager::UnMarshallFuncValueType(CodecableIndex::I_MAP,
        [](BridgeBinaryUnmarshaller* buffer) { return BridgePackager::UnMarshallingMap(buffer); }),
    BridgePackager::UnMarshallFuncValueType(CodecableIndex::I_COMPOSITE_LIST,
        [](BridgeBinaryUnmarshaller* buffer) { return BridgePackager::UnMarshallingCompositeList(buffer); })
    };

const BridgePackager::MarshallFuncMap BridgePackager::marshallFuncMap = {
    BridgePackager::MarshallFuncValueType(CodecableType::T_NULL,
        [](const CodecableValue&, BridgeBinaryMarshaller*) { }),
    BridgePackager::MarshallFuncValueType(CodecableType::T_BOOL,
        [](const CodecableValue&, BridgeBinaryMarshaller*) { }),
    BridgePackager::MarshallFuncValueType(CodecableType::T_INT32,
        [](const CodecableValue& value, BridgeBinaryMarshaller* buffer) {
            buffer->MarshallingInt32(std::get<int32_t>(value)); }),
    BridgePackager::MarshallFuncValueType(CodecableType::T_INT64,
        [](const CodecableValue& value, BridgeBinaryMarshaller* buffer) {
            buffer->MarshallingInt64(std::get<int64_t>(value)); }),
    BridgePackager::MarshallFuncValueType(CodecableType::T_DOUBLE,
        [](const CodecableValue& value, BridgeBinaryMarshaller* buffer) {
            buffer->MarshallingAlign(8);
            buffer->MarshallingDouble(std::get<double>(value));}),
    BridgePackager::MarshallFuncValueType(CodecableType::T_STRING,
        [](const CodecableValue& value, BridgeBinaryMarshaller* buffer) {
            BridgePackager::MarshallingString(std::get<std::string>(value), buffer); }),
    BridgePackager::MarshallFuncValueType(CodecableType::T_LIST_UINT8,
        [](const CodecableValue& value, BridgeBinaryMarshaller* buffer) {
            BridgePackager::MarshallingVector(std::get<std::vector<uint8_t>>(value), buffer); }),
    BridgePackager::MarshallFuncValueType(CodecableType::T_LIST_BOOL,
        [](const CodecableValue& value, BridgeBinaryMarshaller* buffer) {
            BridgePackager::MarshallingListBool(std::get<std::vector<bool>>(value), buffer); }),
    BridgePackager::MarshallFuncValueType(CodecableType::T_LIST_INT32,
        [](const CodecableValue& value, BridgeBinaryMarshaller* buffer) {
            BridgePackager::MarshallingVector(std::get<std::vector<int32_t>>(value), buffer); }),
    BridgePackager::MarshallFuncValueType(CodecableType::T_LIST_INT64,
        [](const CodecableValue& value, BridgeBinaryMarshaller* buffer) {
            BridgePackager::MarshallingVector(std::get<std::vector<int64_t>>(value), buffer); }),
    BridgePackager::MarshallFuncValueType(CodecableType::T_LIST_DOUBLE,
        [](const CodecableValue& value, BridgeBinaryMarshaller* buffer) {
            BridgePackager::MarshallingVector(std::get<std::vector<double>>(value), buffer); }),
    BridgePackager::MarshallFuncValueType(CodecableType::T_LIST_STRING,
        [](const CodecableValue& value, BridgeBinaryMarshaller* buffer) {
            BridgePackager::MarshallingListString(std::get<std::vector<std::string>>(value), buffer); }),
    BridgePackager::MarshallFuncValueType(CodecableType::T_MAP,
        [](const CodecableValue& value, BridgeBinaryMarshaller* buffer) {
            BridgePackager::MarshallingMap(std::get<CodecableMap>(value), buffer); }),
    BridgePackager::MarshallFuncValueType(CodecableType::T_COMPOSITE_LIST,
        [](const CodecableValue& value, BridgeBinaryMarshaller* buffer) {
            BridgePackager::MarshallingCompositeList(std::get<CodecableList>(value), buffer); }),
    };

CodecableIndex BridgePackager::GetCodecableIndex(const CodecableValue& value)
{
    CodecableType type = static_cast<CodecableType>(value.index());
    if (type == CodecableType::T_NULL) {
        return CodecableIndex::I_NULL;
    } else if (type == CodecableType::T_BOOL) {
        return std::get<bool>(value) ? CodecableIndex::I_TRUE : CodecableIndex::I_FALSE;
    } else {
        return static_cast<CodecableIndex>(value.index() + 1);
    }
}

CodecableValue BridgePackager::UnMarshalling(BridgeBinaryUnmarshaller* pendingBuffer)
{
    if (!pendingBuffer) {
        LOGE("pendingBuffer is nullptr, will return null.");
        return CodecableValue();
    }

    CodecableIndex index = static_cast<CodecableIndex>(pendingBuffer->UnmarshallingByte());
    auto iter = unMarshallFuncMap.find(index);
    if (iter != unMarshallFuncMap.end()) {
        return (iter->second)(pendingBuffer);
    }

    LOGW("invaild type, can not unmarshalling value from buffer.");
    return CodecableValue();
}

void BridgePackager::Marshalling(const CodecableValue& value, BridgeBinaryMarshaller* pendingBuffer)
{
    if (!pendingBuffer) {
        LOGE("pendingBuffer is nullptr, will return null.");
        return;
    }

    pendingBuffer->MarshallingByte(static_cast<uint8_t>(GetCodecableIndex(value)));
    CodecableType type = static_cast<CodecableType>(value.index());
    auto iter = marshallFuncMap.find(type);
    if (iter != marshallFuncMap.end()) {
        (iter->second)(value, pendingBuffer);
        return;
    }

    LOGW("invaild type, can not marshalling value to buffer.");
    return;
}

void BridgePackager::MarshallingListBool(const std::vector<bool>& vector, BridgeBinaryMarshaller* pendingBuffer)
{
    MarshallingSize(vector.size(), pendingBuffer);
    for (const auto& item : vector) {
        auto boolValue = item ? CodecableIndex::I_TRUE : CodecableIndex::I_FALSE;
        pendingBuffer->MarshallingByte(static_cast<uint8_t>(boolValue));
    }
}

void BridgePackager::MarshallingListString(
    const std::vector<std::string>& vector, BridgeBinaryMarshaller* pendingBuffer)
{
    MarshallingSize(vector.size(), pendingBuffer);
    for (const auto& item : vector) {
        MarshallingString(item, pendingBuffer);
    }
}

CodecableValue BridgePackager::UnMarshallingListString(BridgeBinaryUnmarshaller* pendingBuffer)
{
    size_t size = UnMarshallingSize(pendingBuffer);
    std::vector<std::string> temp;
    temp.reserve(size);
    size_t i = 0;
    while(i < size) {
        temp.push_back(std::get<std::string>(UnMarshallingString(pendingBuffer)));
        ++i;
    }
    return CodecableValue(temp);
}

CodecableValue BridgePackager::UnMarshallingListBool(BridgeBinaryUnmarshaller* pendingBuffer)
{
    size_t size = UnMarshallingSize(pendingBuffer);
    std::vector<bool> temp;
    temp.reserve(size);
    size_t i = 0;
    while(i < size) {
        uint8_t item = pendingBuffer->UnmarshallingByte();
        bool rawItem = (static_cast<CodecableIndex>(item) == CodecableIndex::I_TRUE);
        temp.push_back(rawItem);
        ++i;
    }
    return CodecableValue(temp);
}

void BridgePackager::MarshallingString(const std::string& value, BridgeBinaryMarshaller* pendingBuffer)
{
    size_t size = value.size();
    MarshallingSize(size, pendingBuffer);
    if (size > BridgeBinaryMarshaller::MARSHALL_SIZE_0) {
        const uint8_t* head = reinterpret_cast<const uint8_t*>(value.data());
        pendingBuffer->MarshallingBytes(head, size);
    }
}

CodecableValue BridgePackager::UnMarshallingString(BridgeBinaryUnmarshaller* pendingBuffer)
{
    size_t size = UnMarshallingSize(pendingBuffer);
    std::string value;
    value.resize(size);
    uint8_t* head = reinterpret_cast<uint8_t*>(&value[0]);
    pendingBuffer->UnmarshallingBytes(head, size);
    return CodecableValue(value);
}

void BridgePackager::MarshallingSize(size_t size, BridgeBinaryMarshaller* pendingBuffer)
{
    if (size < 0xFE) {
        pendingBuffer->MarshallingByte(static_cast<uint8_t>(size));
        return;
    }
    if (size <= 0xFFFF) {
        pendingBuffer->MarshallingByte(0xFE);
        uint16_t value = static_cast<uint16_t>(size);
        pendingBuffer->MarshallingBytes(
            reinterpret_cast<uint8_t*>(&value), BridgeBinaryMarshaller::MARSHALL_SIZE_2);
        return;
    }
    pendingBuffer->MarshallingByte(0xFF);
    uint32_t value = static_cast<uint32_t>(size);
    pendingBuffer->MarshallingBytes(
        reinterpret_cast<uint8_t*>(&value), BridgeBinaryMarshaller::MARSHALL_SIZE_4);

    return;
}

size_t BridgePackager::UnMarshallingSize(BridgeBinaryUnmarshaller* pendingBuffer)
{
    if (pendingBuffer == nullptr) {
        LOGW("pendingBuffer is nullptr.");
        return BridgeBinaryUnmarshaller::UNMARSHALL_ERROR;
    }

    uint8_t size = pendingBuffer->UnmarshallingByte();
    if (size < 0xFE) {
        return size;
    }
    if (size == 0xFE) {
        uint32_t value = 0;
        pendingBuffer->UnmarshallingBytes(
            reinterpret_cast<uint8_t*>(&value), BridgeBinaryUnmarshaller::UNMARSHALL_SIZE_2);
        return value;
    }

    uint32_t value = 0;
    pendingBuffer->UnmarshallingBytes(
        reinterpret_cast<uint8_t*>(&value), BridgeBinaryUnmarshaller::UNMARSHALL_SIZE_4);

    return value;
}

CodecableValue BridgePackager::UnMarshallingMap(BridgeBinaryUnmarshaller* pendingBuffer)
{
    size_t size = UnMarshallingSize(pendingBuffer);
    CodecableMap tempMap;
    size_t i = 0;
    while(i < size) {
        auto first = UnMarshalling(pendingBuffer);
        auto second = UnMarshalling(pendingBuffer);
        tempMap.emplace(std::move(first), std::move(second));
        ++i;
    }
    return CodecableValue(tempMap);
}

void BridgePackager::MarshallingMap(const CodecableMap& map, BridgeBinaryMarshaller* pendingBuffer)
{
    MarshallingSize(map.size(), pendingBuffer);
    for (const auto& pair : map) {
        Marshalling(pair.first, pendingBuffer);
        Marshalling(pair.second, pendingBuffer);
    }
}

template<typename T>
void BridgePackager::MarshallingVector(const std::vector<T>& vector, BridgeBinaryMarshaller* pendingBuffer)
{
    size_t size = vector.size();
    MarshallingSize(size, pendingBuffer);
    if (size > BridgeBinaryMarshaller::MARSHALL_SIZE_0) {
        uint8_t tSize = static_cast<uint8_t>(sizeof(T));
        if (tSize > BridgeBinaryMarshaller::MARSHALL_SIZE_1) {
            pendingBuffer->MarshallingAlign(tSize);
        }
        size_t bytesSize = size * tSize;
        pendingBuffer->MarshallingBytes(reinterpret_cast<const uint8_t*>(vector.data()), bytesSize);
    }
}

template<typename T>
CodecableValue BridgePackager::UnMarshallingVector(BridgeBinaryUnmarshaller* pendingBuffer)
{
    size_t size = UnMarshallingSize(pendingBuffer);
    uint8_t tSize = static_cast<uint8_t>(sizeof(T));
    if (tSize > BridgeBinaryUnmarshaller::UNMARSHALL_SIZE_1) {
        pendingBuffer->UnmarshallingAlign(tSize);
    }
    std::vector<T> temp;
    temp.resize(size);
    size_t bytesSize = size * tSize;
    pendingBuffer->UnmarshallingBytes(reinterpret_cast<uint8_t*>(temp.data()), bytesSize);
    return CodecableValue(temp);
}

CodecableValue BridgePackager::UnMarshallingCompositeList(BridgeBinaryUnmarshaller* pendingBuffer)
{
    size_t size = UnMarshallingSize(pendingBuffer);
    CodecableList list;
    list.reserve(size);
    size_t i = 0; 
    while(i < size) {
        list.push_back(UnMarshalling(pendingBuffer));
        ++i;
    }
    return CodecableValue(list);
}

void BridgePackager::MarshallingCompositeList(const CodecableList& list, BridgeBinaryMarshaller* pendingBuffer)
{
    MarshallingSize(list.size(), pendingBuffer);
    for (const auto& item : list) {
        Marshalling(item, pendingBuffer);
    }
}
} // OHOS::Plugin::Bridge