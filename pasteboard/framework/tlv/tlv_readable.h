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

#ifndef DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_READABLE_H
#define DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_READABLE_H

#include "endian_converter.h"
#include "tlv_buffer.h"
#include "tlv_utils.h"
#include "uri.h"

namespace OHOS::MiscServices {

class ReadOnlyBuffer;

class TLVReadable {
public:
    virtual ~TLVReadable() = default;

    virtual bool DecodeTLV(ReadOnlyBuffer& buffer) = 0;

    API_EXPORT bool Decode(const std::vector<uint8_t>& buffer);
};

class ReadOnlyBuffer : public TLVBuffer {
public:
    explicit ReadOnlyBuffer(const std::vector<uint8_t>& data) : TLVBuffer(data.size()), data_(data) {}

    template<typename T>
    bool ReadValue(std::vector<T>& value, const TLVHead& head)
    {
        if (cursor_ > UINT32_MAX - head.len) {
            return false;
        }
        auto vectorEnd = cursor_ + head.len;
        if (vectorEnd > data_.size()) {
            return false;
        }
        for (; cursor_ < vectorEnd;) {
            // V: item value
            TLVHead valueHead {};
            bool ret = ReadHead(valueHead);
            T item {};
            ret = ret && ReadValue(item, valueHead);
            if (!ret) {
                return false;
            }
            value.push_back(item);
        }
        return true;
    }

    template<typename T>
    bool ReadValue(std::shared_ptr<T>& value, const TLVHead& head)
    {
        value = std::make_shared<T>();
        if (value == nullptr) {
            return false;
        }
        return ReadValue(*value, head);
    }

    bool ReadHead(TLVHead& head);
    bool ReadValue(std::monostate& value, const TLVHead& head);
    bool ReadValue(void* value, const TLVHead& head);
    bool ReadValue(bool& value, const TLVHead& head);
    bool ReadValue(int8_t& value, const TLVHead& head);
    bool ReadValue(int16_t& value, const TLVHead& head);
    bool ReadValue(int32_t& value, const TLVHead& head);
    bool ReadValue(int64_t& value, const TLVHead& head);
    bool ReadValue(double& value, const TLVHead& head);
    bool ReadValue(uint32_t& value, const TLVHead& head);
    bool ReadValue(std::string& value, const TLVHead& head);
    bool ReadValue(RawMem& rawMem, const TLVHead& head);
    bool ReadValue(TLVReadable& value, const TLVHead& head);
    bool ReadValue(std::vector<uint8_t>& value, const TLVHead& head);
    bool ReadValue(Object& value, const TLVHead& head);
    bool ReadValue(std::shared_ptr<OHOS::Uri>& value, const TLVHead& head);
    bool ReadValue(std::shared_ptr<AAFwk::Want>& value, const TLVHead& head);
    bool ReadValue(std::shared_ptr<Media::PixelMap>& value, const TLVHead& head);
    bool ReadValue(std::map<std::string, std::vector<uint8_t>>& value, const TLVHead& head);
    bool ReadValue(Details& value, const TLVHead& head);

    template<typename _InTp>
    bool ReadVariant(uint32_t step, uint32_t index, _InTp& input, const TLVHead& head);

    template<typename _InTp, typename _First, typename... _Rest>
    bool ReadVariant(uint32_t step, uint32_t index, _InTp& input, const TLVHead& head);

    template<typename... _Types>
    bool ReadValue(std::variant<_Types...>& value, const TLVHead& head);

    template<>
    bool ReadValue(EntryValue& value, const TLVHead& head);

private:
    bool ReadBasicValue(bool& value, const TLVHead& head)
    {
        if (head.len != sizeof(bool) || head.len == 0) {
            return false;
        }
        if (!HasExpectBuffer(head.len)) {
            return false;
        }
        uint8_t rawValue = 0;
        auto ret = memcpy_s(&rawValue, sizeof(bool), data_.data() + cursor_, sizeof(bool));
        if (ret != EOK) {
            return false;
        }
        if (rawValue > 1) {
            return false;
        }
        value = NetToHost(rawValue);
        cursor_ += sizeof(bool);
        return true;
    }

    template<typename T>
    bool ReadBasicValue(T& value, const TLVHead& head)
    {
        if (head.len != sizeof(T) || head.len == 0) {
            return false;
        }
        if (!HasExpectBuffer(head.len)) {
            return false;
        }
        auto ret = memcpy_s(&value, sizeof(T), data_.data() + cursor_, sizeof(T));
        if (ret != EOK) {
            return false;
        }
        value = NetToHost(value);
        cursor_ += sizeof(T);
        return true;
    }

    const std::vector<uint8_t> data_;
};
} // namespace OHOS::MiscServices
#endif // DISTRIBUTEDDATAMGR_PASTEBOARD_TLV_READABLE_H
