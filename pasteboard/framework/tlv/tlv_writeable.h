/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2026-2026. All rights reserved.
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

#ifndef DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_WRITEABLE_H
#define DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_WRITEABLE_H

#include "endian_converter.h"
#include "tlv_countable.h"
#include "uri.h"

namespace OHOS::MiscServices {

bool IsRemoteEncode();

class WriteOnlyBuffer;

class TLVWriteable : public TLVCountable {
public:
    virtual ~TLVWriteable() = default;

    virtual bool EncodeTLV(WriteOnlyBuffer& buffer) const = 0;

    API_EXPORT bool Encode(std::vector<uint8_t>& buffer, bool isRemote = false) const;

    API_EXPORT bool Encode(size_t len, std::vector<uint8_t>& buffer, bool isRemote = false) const;

    API_EXPORT size_t Count(bool isRemote = false) const;
};

class WriteOnlyBuffer : public TLVBuffer {
public:
    explicit WriteOnlyBuffer(size_t len) : TLVBuffer(len), data_(len) {}

    const uint8_t* Data() const
    {
        return data_.data();
    }

    size_t Size() const
    {
        return cursor_;
    }

    template<typename T>
    bool Write(uint16_t type, const std::vector<T>& value)
    {
        if (!HasExpectBuffer(sizeof(TLVHead))) {
            return false;
        }
        auto tagCursor = cursor_;
        cursor_ += sizeof(TLVHead); // placeholder
        auto valueCursor = cursor_;
        bool ret = WriteValue(value);
        WriteHead(type, tagCursor, cursor_ - valueCursor);
        return ret;
    }

    template<typename T>
    bool Write(uint16_t type, const std::shared_ptr<T>& value)
    {
        if (value == nullptr) {
            return true;
        }
        return Write(type, *value);
    }

    bool Write(uint16_t type, std::monostate value);
    bool Write(uint16_t type, const void* value);
    bool Write(uint16_t type, bool value);
    bool Write(uint16_t type, double value);
    bool Write(uint16_t type, int8_t value);
    bool Write(uint16_t type, int16_t value);
    bool Write(uint16_t type, int32_t value);
    bool Write(uint16_t type, int64_t value);
    bool Write(uint16_t type, uint32_t value);
    bool Write(uint16_t type, const std::string& value);
    bool Write(uint16_t type, const Object& value);
    bool Write(uint16_t type, const AAFwk::Want& value);
    bool Write(uint16_t type, const Media::PixelMap& value);
    bool Write(uint16_t type, const RawMem& value);
    bool Write(uint16_t type, const TLVWriteable& value);
    bool Write(uint16_t type, const std::vector<uint8_t>& value);
    bool Write(uint16_t type, const std::map<std::string, std::vector<uint8_t>>& value);
    bool Write(uint16_t type, const Details& value);

    template<typename _InTp>
    bool WriteVariant(uint16_t type, uint32_t step, const _InTp& input);

    template<typename _InTp, typename _First, typename... _Rest>
    bool WriteVariant(uint16_t type, uint32_t step, const _InTp& input);

    template<typename... _Types>
    bool Write(uint16_t type, const std::variant<_Types...>& input);

    template<>
    bool Write(uint16_t type, const EntryValue& input);

private:
    void WriteHead(uint16_t type, size_t tagCursor, uint32_t len)
    {
        if (tagCursor + sizeof(TLVHead) > data_.size()) {
            return;
        }
        auto* tlvHead = reinterpret_cast<TLVHead*>(data_.data() + tagCursor);
        tlvHead->tag = HostToNet(type);
        tlvHead->len = HostToNet(len);
    }

    template<typename T>
    bool WriteBasic(uint16_t type, T value)
    {
        if (!HasExpectBuffer(sizeof(TLVHead) + sizeof(value))) {
            return false;
        }
        auto* tlvHead = reinterpret_cast<TLVHead*>(data_.data() + cursor_);
        tlvHead->tag = HostToNet(type);
        tlvHead->len = HostToNet((uint32_t)sizeof(value));
        auto valueBuff = HostToNet(value);
        auto ret = memcpy_s(tlvHead->value, total_ - cursor_ - sizeof(TLVHead), &valueBuff, sizeof(value));
        if (ret != EOK) {
            return false;
        }
        cursor_ += sizeof(TLVHead) + sizeof(value);
        return true;
    }

    template<typename T>
    bool WriteValue(const std::vector<T>& value)
    {
        // items iterator
        bool ret = true;
        for (const T& item : value) {
            // V:item value
            ret = ret && Write(TAG_VECTOR_ITEM, item);
        }
        return ret;
    }

    friend class TLVWriteable;
    std::vector<uint8_t> data_;
};
} // namespace OHOS::MiscServices
#endif // DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_WRITEABLE_H
