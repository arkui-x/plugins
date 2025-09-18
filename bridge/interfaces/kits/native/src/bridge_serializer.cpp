/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "bridge_serializer.h"

#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <vector>

#include "log.h"

namespace OHOS::Plugin::Bridge {
namespace {
enum class CodecableType {
    K_NULL = 0,
    K_TRUE,
    K_FALSE,
    K_INT32,
    K_INT64,
    K_DOUBLE,
    K_STRING,
    K_LIST_UINT8,
    K_LIST_BOOL,
    K_LIST_INT32,
    K_LIST_INT64,
    K_LIST_DOUBLE,
    K_LIST_STRING,
    K_MAP,
    K_COMPOSITE_LIST,
};

CodecableType CovertCodecableTypeByValue(const CodecableValue& value)
{
    switch (static_cast<CodecableValueType>(value.index())) {
        case CodecableValueType::T_NULL:
            return CodecableType::K_NULL;
        case CodecableValueType::T_BOOL:
            return std::get<bool>(value) ? CodecableType::K_TRUE : CodecableType::K_FALSE;
        default:
            return static_cast<CodecableType>(value.index() + 1);
    }
}
} // namespace

const BridgeSerializer& BridgeSerializer::GetInstance()
{
    static BridgeSerializer sInstance;
    return sInstance;
}

CodecableValue BridgeSerializer::ReadValue(BridgeStreamReader* stream) const
{
    if (!stream) {
        LOGE("stream is nullptr, will return null.");
        return CodecableValue();
    }
    uint8_t type = stream->ReadByte();
    switch (static_cast<CodecableType>(type)) {
        case CodecableType::K_NULL:
            return CodecableValue();
        case CodecableType::K_TRUE:
            return CodecableValue(true);
        case CodecableType::K_FALSE:
            return CodecableValue(false);
        case CodecableType::K_INT32:
            return CodecableValue(stream->ReadInt32());
        case CodecableType::K_INT64:
            return CodecableValue(stream->ReadInt64());
        case CodecableType::K_DOUBLE: {
            stream->ReadAlignment(8);
            return CodecableValue(stream->ReadDouble());
        }
        case CodecableType::K_STRING:
            return ReadString(stream);
        case CodecableType::K_LIST_UINT8:
            return ReadVector<uint8_t>(stream);
        case CodecableType::K_LIST_BOOL:
            return ReadListBool(stream);
        case CodecableType::K_LIST_INT32:
            return ReadVector<int32_t>(stream);
        case CodecableType::K_LIST_INT64:
            return ReadVector<int64_t>(stream);
        case CodecableType::K_LIST_DOUBLE:
            return ReadVector<double>(stream);
        case CodecableType::K_LIST_STRING:
            return ReadListString(stream);
        case CodecableType::K_MAP:
            return ReadMap(stream);
        case CodecableType::K_COMPOSITE_LIST: {
            size_t size = ReadSize(stream);
            CodecableList list;
            list.reserve(size);
            for (size_t i = 0; i < size; ++i) {
                list.push_back(ReadValue(stream));
            }
            return CodecableValue(list);
        }
        default:
            LOGW("invaild type, can not read value from stream.");
            break;
    }
    return CodecableValue();
}

void BridgeSerializer::WriteValue(const CodecableValue& value, BridgeStreamWriter* stream) const
{
    if (!stream) {
        LOGE("stream is nullptr, will return null.");
        return;
    }
    stream->WriteByte(static_cast<uint8_t>(CovertCodecableTypeByValue(value)));
    switch (static_cast<CodecableValueType>(value.index())) {
        case CodecableValueType::T_NULL:
        case CodecableValueType::T_BOOL:
            break;
        case CodecableValueType::T_INT32:
            stream->WriteInt32(std::get<int32_t>(value));
            break;
        case CodecableValueType::T_INT64:
            stream->WriteInt64(std::get<int64_t>(value));
            break;
        case CodecableValueType::T_DOUBLE:
            stream->WriteAlignment(8);
            stream->WriteDouble(std::get<double>(value));
            break;
        case CodecableValueType::T_STRING: {
            WriteString(std::get<std::string>(value), stream);
            break;
        }
        case CodecableValueType::T_LIST_UINT8:
            WriteVector(std::get<std::vector<uint8_t>>(value), stream);
            break;
        case CodecableValueType::T_LIST_BOOL: {
            WriteListBool(std::get<std::vector<bool>>(value), stream);
            break;
        }
        case CodecableValueType::T_LIST_INT32:
            WriteVector(std::get<std::vector<int32_t>>(value), stream);
            break;
        case CodecableValueType::T_LIST_INT64:
            WriteVector(std::get<std::vector<int64_t>>(value), stream);
            break;
        case CodecableValueType::T_LIST_DOUBLE:
            WriteVector(std::get<std::vector<double>>(value), stream);
            break;
        case CodecableValueType::T_LIST_STRING: {
            WriteListString(std::get<std::vector<std::string>>(value), stream);
            break;
        }
        case CodecableValueType::T_MAP:
            WriteMap(std::get<CodecableMap>(value), stream);
            break;
        case CodecableValueType::T_COMPOSITE_LIST: {
            const auto& list = std::get<CodecableList>(value);
            WriteSize(list.size(), stream);
            for (const auto& item : list) {
                WriteValue(item, stream);
            }
            break;
        }
        default:
            LOGW("invaild type, can not write value to stream.");
            break;
    }
}

void BridgeSerializer::WriteListBool(const std::vector<bool>& vector, BridgeStreamWriter* stream) const
{
    WriteSize(vector.size(), stream);
    for (const auto& item : vector) {
        auto boolValue = item ? CodecableType::K_TRUE : CodecableType::K_FALSE;
        stream->WriteByte(static_cast<uint8_t>(boolValue));
    }
}

void BridgeSerializer::WriteListString(const std::vector<std::string>& vector, BridgeStreamWriter* stream) const
{
    WriteSize(vector.size(), stream);
    for (const auto& item : vector) {
        WriteString(item, stream);
    }
}

CodecableValue BridgeSerializer::ReadListString(BridgeStreamReader* stream) const
{
    size_t size = ReadSize(stream);
    std::vector<std::string> vector;
    vector.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        vector.push_back(std::get<std::string>(ReadString(stream)));
    }
    return CodecableValue(vector);
}

CodecableValue BridgeSerializer::ReadListBool(BridgeStreamReader* stream) const
{
    size_t size = ReadSize(stream);
    std::vector<bool> vector;
    vector.reserve(size);
    for (size_t i = 0; i < size; ++i) {
        uint8_t item = stream->ReadByte();
        bool rawItem = (static_cast<CodecableType>(item) == CodecableType::K_TRUE);
        vector.push_back(rawItem);
    }
    return CodecableValue(vector);
}

void BridgeSerializer::WriteString(const std::string& stringValue, BridgeStreamWriter* stream) const
{
    size_t size = stringValue.size();
    WriteSize(size, stream);
    if (size > 0) {
        stream->WriteBytes(reinterpret_cast<const uint8_t*>(stringValue.data()), size);
    }
}

CodecableValue BridgeSerializer::ReadString(BridgeStreamReader* stream) const
{
    size_t size = ReadSize(stream);
    std::string stringValue;
    stringValue.resize(size);
    stream->ReadBytes(reinterpret_cast<uint8_t*>(&stringValue[0]), size);
    return CodecableValue(stringValue);
}

size_t BridgeSerializer::ReadSize(BridgeStreamReader* stream) const
{
    uint8_t byte = stream->ReadByte();
    if (byte < 0xFE) {
        return byte;
    } else if (byte == 0xFE) {
        uint16_t value = 0;
        stream->ReadBytes(reinterpret_cast<uint8_t*>(&value), 2);
        return value;
    } else {
        uint32_t value = 0;
        stream->ReadBytes(reinterpret_cast<uint8_t*>(&value), 4);
        return value;
    }
}

void BridgeSerializer::WriteSize(size_t size, BridgeStreamWriter* stream) const
{
    if (size < 0xFE) {
        stream->WriteByte(static_cast<uint8_t>(size));
    } else if (size <= 0xFFFF) {
        stream->WriteByte(0xFE);
        uint16_t value = static_cast<uint16_t>(size);
        stream->WriteBytes(reinterpret_cast<uint8_t*>(&value), 2);
    } else {
        stream->WriteByte(0xFF);
        uint32_t value = static_cast<uint32_t>(size);
        stream->WriteBytes(reinterpret_cast<uint8_t*>(&value), 4);
    }
}

template<typename T>
CodecableValue BridgeSerializer::ReadVector(BridgeStreamReader* stream) const
{
    size_t size = ReadSize(stream);
    std::vector<T> vector;
    vector.resize(size);
    uint8_t type_size = static_cast<uint8_t>(sizeof(T));
    if (type_size > 1) {
        stream->ReadAlignment(type_size);
    }
    stream->ReadBytes(reinterpret_cast<uint8_t*>(vector.data()), size * type_size);
    return CodecableValue(vector);
}

template<typename T>
void BridgeSerializer::WriteVector(const std::vector<T>& vector, BridgeStreamWriter* stream) const
{
    size_t count = vector.size();
    WriteSize(count, stream);
    if (count == 0) {
        return;
    }
    uint8_t type_size = static_cast<uint8_t>(sizeof(T));
    if (type_size > 1) {
        stream->WriteAlignment(type_size);
    }
    stream->WriteBytes(reinterpret_cast<const uint8_t*>(vector.data()), count * type_size);
}

CodecableValue BridgeSerializer::ReadMap(BridgeStreamReader* stream) const
{
    size_t size = ReadSize(stream);
    CodecableMap mapValue;
    for (size_t i = 0; i < size; ++i) {
        CodecableValue key = ReadValue(stream);
        CodecableValue value = ReadValue(stream);
        mapValue.emplace(std::move(key), std::move(value));
    }
    return CodecableValue(mapValue);
}

void BridgeSerializer::WriteMap(const CodecableMap& map, BridgeStreamWriter* stream) const
{
    size_t count = map.size();
    WriteSize(count, stream);
    for (const auto& pair : map) {
        WriteValue(pair.first, stream);
        WriteValue(pair.second, stream);
    }
}
} // OHOS::Plugin::Bridge