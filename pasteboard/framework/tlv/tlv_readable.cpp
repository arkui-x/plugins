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

#include "tlv_readable.h"

#include "pasteboard_hilog.h"

namespace OHOS::MiscServices {

bool TLVReadable::Decode(const std::vector<std::uint8_t>& buffer)
{
    ReadOnlyBuffer buff(buffer);
    return DecodeTLV(buff);
}

bool ReadOnlyBuffer::ReadHead(TLVHead& head)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        HasExpectBuffer(sizeof(TLVHead)), false, PASTEBOARD_MODULE_COMMON, "read head failed");
    const auto* pHead = reinterpret_cast<const TLVHead*>(data_.data() + cursor_);
    if (!HasExpectBuffer(NetToHost(pHead->len)) && !HasExpectBuffer(NetToHost(pHead->len) + sizeof(TLVHead))) {
        return false;
    }
    head.tag = NetToHost(pHead->tag);
    head.len = NetToHost(pHead->len);
    cursor_ += sizeof(TLVHead);
    return true;
}

bool ReadOnlyBuffer::ReadValue(std::monostate& value, const TLVHead& head)
{
    (void)value;
    (void)head;
    return true;
}

bool ReadOnlyBuffer::ReadValue(void* value, const TLVHead& head)
{
    (void)value;
    (void)head;
    return true;
}

bool ReadOnlyBuffer::ReadValue(bool& value, const TLVHead& head)
{
    return ReadBasicValue(value, head);
}

bool ReadOnlyBuffer::ReadValue(int8_t& value, const TLVHead& head)
{
    return ReadBasicValue(value, head);
}

bool ReadOnlyBuffer::ReadValue(int16_t& value, const TLVHead& head)
{
    return ReadBasicValue(value, head);
}

bool ReadOnlyBuffer::ReadValue(int32_t& value, const TLVHead& head)
{
    return ReadBasicValue(value, head);
}

bool ReadOnlyBuffer::ReadValue(int64_t& value, const TLVHead& head)
{
    return ReadBasicValue(value, head);
}

bool ReadOnlyBuffer::ReadValue(double& value, const TLVHead& head)
{
    return ReadBasicValue(value, head);
}

bool ReadOnlyBuffer::ReadValue(uint32_t& value, const TLVHead& head)
{
    return ReadBasicValue(value, head);
}

bool ReadOnlyBuffer::ReadValue(std::string& value, const TLVHead& head)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        HasExpectBuffer(head.len), false, PASTEBOARD_MODULE_COMMON, "read string failed, tag=%{public}hu", head.tag);
    value.append(reinterpret_cast<const char*>(data_.data() + cursor_), head.len);
    cursor_ += head.len;
    return true;
}

bool ReadOnlyBuffer::ReadValue(RawMem& rawMem, const TLVHead& head)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        HasExpectBuffer(head.len), false, PASTEBOARD_MODULE_COMMON, "read RawMem failed, tag=%{public}hu", head.tag);
    rawMem.buffer = (uintptr_t)(data_.data() + cursor_);
    rawMem.bufferLen = head.len;
    cursor_ += head.len;
    return true;
}

bool ReadOnlyBuffer::ReadValue(TLVReadable& value, const TLVHead& head)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(HasExpectBuffer(head.len), false, PASTEBOARD_MODULE_COMMON,
        "read TLVReadable failed, tag=%{public}hu", head.tag);
    auto tmp = total_;
    total_ = cursor_ + head.len;
    bool ret = value.DecodeTLV(*this);
    total_ = tmp;
    return ret;
}

bool ReadOnlyBuffer::ReadValue(std::vector<uint8_t>& value, const TLVHead& head)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        HasExpectBuffer(head.len), false, PASTEBOARD_MODULE_COMMON, "read vector failed, tag=%{public}hu", head.tag);
    std::vector<uint8_t> buff(data_.data() + cursor_, data_.data() + cursor_ + head.len);
    value = std::move(buff);
    cursor_ += head.len;
    return true;
}

bool ReadOnlyBuffer::ReadValue(std::map<std::string, std::vector<uint8_t>>& value, const TLVHead& head)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        HasExpectBuffer(head.len), false, PASTEBOARD_MODULE_COMMON, "read map failed, tag=%{public}hu", head.tag);
    auto mapEnd = cursor_ + head.len;
    for (; cursor_ < mapEnd;) {
        // item key
        TLVHead keyHead {};
        bool ret = ReadHead(keyHead);
        std::string itemKey;
        ret = ret && ReadValue(itemKey, keyHead);

        // item value
        TLVHead valueHead {};
        ret = ret && ReadHead(valueHead);
        std::vector<uint8_t> itemValue(0);
        ret = ret && ReadValue(itemValue, valueHead);
        if (!ret) {
            return false;
        }
        value.emplace(itemKey, itemValue);
    }
    return true;
}

template<typename _OutTp>
bool ReadOnlyBuffer::ReadVariant(uint32_t step, uint32_t index, _OutTp& output, const TLVHead& head)
{
    (void)step;
    (void)index;
    (void)output;
    (void)head;
    return true;
}

template<typename _OutTp, typename _First, typename... _Rest>
bool ReadOnlyBuffer::ReadVariant(uint32_t step, uint32_t index, _OutTp& value, const TLVHead& head)
{
    if (step == index) {
        TLVHead valueHead {};
        ReadHead(valueHead);
        _First output {};
        auto success = ReadValue(output, valueHead);
        value = output;
        return success;
    }
    return ReadVariant<_OutTp, _Rest...>(step + 1, index, value, head);
}

template<typename... _Types>
bool ReadOnlyBuffer::ReadValue(std::variant<_Types...>& value, const TLVHead& head)
{
    TLVHead valueHead {};
    ReadHead(valueHead);
    uint32_t index = 0;
    if (!ReadValue(index, valueHead)) {
        return false;
    }
    return ReadVariant<decltype(value), _Types...>(0, index, value, valueHead);
}

template<>
bool ReadOnlyBuffer::ReadValue(EntryValue& value, const TLVHead& head)
{
    TLVHead valueHead {};
    ReadHead(valueHead);
    uint32_t index = 0;
    if (!ReadValue(index, valueHead)) {
        return false;
    }
    return ReadVariant<decltype(value), std::monostate, int32_t, int64_t, double, bool, std::string,
        std::vector<uint8_t>, std::shared_ptr<OHOS::AAFwk::Want>, std::shared_ptr<OHOS::Media::PixelMap>,
        std::shared_ptr<Object>, nullptr_t>(0, index, value, valueHead);
}

bool ReadOnlyBuffer::ReadValue(Details& value, const TLVHead& head)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        HasExpectBuffer(head.len), false, PASTEBOARD_MODULE_COMMON, "read mapEnd failed, tag=%{public}hu", head.tag);
    auto mapEnd = cursor_ + head.len;
    while (cursor_ < mapEnd) {
        TLVHead keyHead {};
        if (!ReadHead(keyHead)) {
            return false;
        }
        std::string itemKey = "";
        if (!ReadValue(itemKey, keyHead)) {
            return false;
        }
        TLVHead variantHead {};
        if (!ReadHead(variantHead)) {
            return false;
        }
        ValueType itemValue;
        if (!ReadValue(itemValue, variantHead)) {
            return false;
        }
        value.emplace(itemKey, itemValue);
    }
    return true;
}

bool ReadOnlyBuffer::ReadValue(Object& value, const TLVHead& head)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        HasExpectBuffer(head.len), false, PASTEBOARD_MODULE_COMMON, "read mapEnd failed, tag=%{public}hu", head.tag);
    auto mapEnd = cursor_ + head.len;
    while (cursor_ < mapEnd) {
        TLVHead keyHead {};
        if (!ReadHead(keyHead)) {
            return false;
        }
        std::string itemKey = "";
        if (!ReadValue(itemKey, keyHead)) {
            return false;
        }
        TLVHead valueHead {};
        if (!ReadHead(valueHead)) {
            return false;
        }
        EntryValue itemValue;
        if (!ReadValue(itemValue, head)) {
            return false;
        }
        value.value_.emplace(itemKey, itemValue);
    }
    return true;
}

bool ReadOnlyBuffer::ReadValue(std::shared_ptr<OHOS::Uri>& value, const TLVHead& head)
{
    RawMem rawMem {};
    bool ret = ReadValue(rawMem, head);
    value = TLVUtils::Raw2Parcelable<OHOS::Uri>(rawMem);
    return ret;
}

bool ReadOnlyBuffer::ReadValue(std::shared_ptr<AAFwk::Want>& value, const TLVHead& head)
{
    std::string json;
    bool ret = ReadValue(json, head);
    value = TLVUtils::Json2Want(json);
    return ret;
}

bool ReadOnlyBuffer::ReadValue(std::shared_ptr<Media::PixelMap>& value, const TLVHead& head)
{
    std::vector<std::uint8_t> rawData;
    bool ret = ReadValue(rawData, head);
    value = TLVUtils::Vector2PixelMap(rawData);
    return ret;
}

} // namespace OHOS::MiscServices
