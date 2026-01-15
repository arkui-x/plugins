/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2026. All rights reserved.
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

#include "paste_data_entry.h"

#include "common/constant.h"
#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {

const char* PASTE_FILE_SIZE = "pasteFileSize";

enum TAG_CUSTOMDATA : uint16_t {
    TAG_ITEM_DATA = TAG_BUFF + 1,
};

enum TAG_ENTRY : uint16_t {
    TAG_ENTRY_UTDID = TAG_BUFF + 1,
    TAG_ENTRY_MIMETYPE,
    TAG_ENTRY_VALUE,
};

std::map<std::string, std::vector<uint8_t>> MineCustomData::GetItemData()
{ // LCOV_EXCL_START
    return this->itemData_;
} // LCOV_EXCL_STOP

void MineCustomData::AddItemData(const std::string& mimeType, const std::vector<uint8_t>& arrayBuffer)
{ // LCOV_EXCL_START
    itemData_.emplace(mimeType, arrayBuffer);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "itemData_.size = %{public}zu", itemData_.size());
} // LCOV_EXCL_STOP

bool MineCustomData::EncodeTLV(WriteOnlyBuffer& buffer) const
{
    return buffer.Write(TAG_ITEM_DATA, itemData_);
}

bool MineCustomData::DecodeTLV(ReadOnlyBuffer& buffer)
{
    for (; buffer.IsEnough();) {
        TLVHead head {};
        bool ret = buffer.ReadHead(head);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON, "read head failed");
        switch (head.tag) {
            case TAG_ITEM_DATA:
                ret = buffer.ReadValue(itemData_, head);
                break;
            default:
                ret = buffer.Skip(head.len);
                break;
        }
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON,
            "read value failed, tag=%{public}hu, len=%{public}u", head.tag, head.len);
    }
    return true;
}

size_t MineCustomData::CountTLV() const
{
    return TLVCountable::Count(itemData_);
}

PasteDataEntry::PasteDataEntry(const PasteDataEntry& entry)
    : rawDataSize_(entry.rawDataSize_), utdId_(entry.utdId_), mimeType_(entry.mimeType_), value_(entry.value_)
{ // LCOV_EXCL_START
} // LCOV_EXCL_STOP

PasteDataEntry& PasteDataEntry::operator=(const PasteDataEntry& entry)
{ // LCOV_EXCL_START
    if (this == &entry) {
        return *this;
    }
    this->utdId_ = entry.GetUtdId();
    this->mimeType_ = entry.GetMimeType();
    this->value_ = entry.GetValue();
    this->rawDataSize_ = entry.rawDataSize_;
    return *this;
} // LCOV_EXCL_STOP

PasteDataEntry::PasteDataEntry(const std::string& utdId, const EntryValue& value) : utdId_(utdId), value_(value)
{ // LCOV_EXCL_START
    mimeType_ = CommonUtils::Convert2MimeType(utdId_);
} // LCOV_EXCL_STOP

PasteDataEntry::PasteDataEntry(const std::string& utdId, const std::string& mimeType, const EntryValue& value)
    : utdId_(utdId), mimeType_(std::move(mimeType)), value_(std::move(value))
{ // LCOV_EXCL_START
} // LCOV_EXCL_STOP

void PasteDataEntry::SetUtdId(const std::string& utdId)
{ // LCOV_EXCL_START
    utdId_ = utdId;
} // LCOV_EXCL_STOP

std::string PasteDataEntry::GetUtdId() const
{ // LCOV_EXCL_START
    return utdId_;
} // LCOV_EXCL_STOP

void PasteDataEntry::SetMimeType(const std::string& mimeType)
{ // LCOV_EXCL_START
    mimeType_ = mimeType;
} // LCOV_EXCL_STOP

std::string PasteDataEntry::GetMimeType() const
{ // LCOV_EXCL_START
    return mimeType_;
} // LCOV_EXCL_STOP

EntryValue PasteDataEntry::GetValue() const
{ // LCOV_EXCL_START
    return value_;
} // LCOV_EXCL_STOP

void PasteDataEntry::SetValue(const EntryValue& value)
{ // LCOV_EXCL_START
    value_ = value;
} // LCOV_EXCL_STOP

bool PasteDataEntry::EncodeTLV(WriteOnlyBuffer& buffer) const
{
    bool ret = buffer.Write(TAG_ENTRY_UTDID, utdId_);
    ret = ret && buffer.Write(TAG_ENTRY_MIMETYPE, mimeType_);
    ret = ret && buffer.Write(TAG_ENTRY_VALUE, value_);
    return ret;
}

bool PasteDataEntry::DecodeTLV(ReadOnlyBuffer& buffer)
{
    for (; buffer.IsEnough();) {
        TLVHead head {};
        bool ret = buffer.ReadHead(head);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_CLIENT, "read head failed");
        switch (head.tag) {
            case TAG_ENTRY_UTDID:
                ret = buffer.ReadValue(utdId_, head);
                break;
            case TAG_ENTRY_MIMETYPE:
                ret = buffer.ReadValue(mimeType_, head);
                break;
            case TAG_ENTRY_VALUE:
                ret = buffer.ReadValue(value_, head);
                break;
            default:
                ret = buffer.Skip(head.len);
                break;
        }
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON,
            "read value failed, tag=%{public}hu, len=%{public}u", head.tag, head.len);
    }
    return true;
}

size_t PasteDataEntry::CountTLV() const
{
    return TLVCountable::Count(utdId_) + TLVCountable::Count(mimeType_) + TLVCountable::Count(value_);
}

std::shared_ptr<std::string> PasteDataEntry::ConvertToPlainText() const
{ // LCOV_EXCL_START
    std::string res;
    auto utdId = GetUtdId();
    auto entry = GetValue();
    if (std::holds_alternative<std::string>(entry)) {
        res = std::get<std::string>(entry);
        return std::make_shared<std::string>(res);
    }
    if (!std::holds_alternative<std::shared_ptr<Object>>(entry)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "value error, no plaintext");
        return nullptr;
    }
    auto object = std::get<std::shared_ptr<Object>>(entry);
    if (utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::PLAIN_TEXT)) {
        object->GetValue(UDMF::CONTENT, res);
    } else if (utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::HTML)) {
        object->GetValue(UDMF::PLAIN_CONTENT, res);
    } else {
        object->GetValue(UDMF::URL, res);
    }
    return std::make_shared<std::string>(res);
} // LCOV_EXCL_STOP

std::shared_ptr<std::string> PasteDataEntry::ConvertToHtml() const
{ // LCOV_EXCL_START
    std::string res;
    if (GetUtdId() != UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::HTML)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "type error, utdId:%{public}s", GetUtdId().c_str());
        return nullptr;
    }
    auto entry = GetValue();
    if (std::holds_alternative<std::string>(entry)) {
        res = std::get<std::string>(entry);
        return std::make_shared<std::string>(res);
    }
    if (!std::holds_alternative<std::shared_ptr<Object>>(entry)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "value error, no html");
        return nullptr;
    }
    auto object = std::get<std::shared_ptr<Object>>(entry);
    object->GetValue(UDMF::HTML_CONTENT, res);
    return std::make_shared<std::string>(res);
} // LCOV_EXCL_STOP

std::shared_ptr<Uri> PasteDataEntry::ConvertToUri() const
{ // LCOV_EXCL_START
    std::string res;
    if (!CommonUtils::IsFileUri(GetUtdId())) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "type error, utdId:%{public}s", GetUtdId().c_str());
        return nullptr;
    }
    auto entry = GetValue();
    if (std::holds_alternative<std::string>(entry)) {
        res = std::get<std::string>(entry);
        return std::make_shared<Uri>(Uri(res));
    }
    if (!std::holds_alternative<std::shared_ptr<Object>>(entry)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "value error, no uri");
        return nullptr;
    }
    auto object = std::get<std::shared_ptr<Object>>(entry);
    object->GetValue(UDMF::FILE_URI_PARAM, res);
    return std::make_shared<Uri>(Uri(res));
} // LCOV_EXCL_STOP

std::shared_ptr<AAFwk::Want> PasteDataEntry::ConvertToWant() const
{ // LCOV_EXCL_START
    if (GetUtdId() != UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::OPENHARMONY_WANT)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "type error, utdId:%{public}s", GetUtdId().c_str());
        return nullptr;
    }
    auto entry = GetValue();
    if (!std::holds_alternative<std::shared_ptr<AAFwk::Want>>(entry)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "value error, no want");
        return nullptr;
    }
    // no uds want
    return std::get<std::shared_ptr<AAFwk::Want>>(entry);
} // LCOV_EXCL_STOP

std::shared_ptr<Media::PixelMap> PasteDataEntry::ConvertToPixelMap() const
{ // LCOV_EXCL_START
    auto utdId = GetUtdId();
    if (utdId != UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::SYSTEM_DEFINED_PIXEL_MAP)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "type error, utdId:%{public}s", utdId.c_str());
        return nullptr;
    }
    auto entry = GetValue();
    if (std::holds_alternative<std::shared_ptr<Media::PixelMap>>(entry)) {
        return std::get<std::shared_ptr<Media::PixelMap>>(entry);
    }
    if (!std::holds_alternative<std::shared_ptr<Object>>(entry)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "value error, no pixelmap");
        return nullptr;
    }
    auto object = std::get<std::shared_ptr<Object>>(entry);
    std::string objType;
    if (!object->GetValue(UDMF::UNIFORM_DATA_TYPE, objType)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "type error, utdId:%{public}s", utdId.c_str());
        return nullptr;
    }
    if (objType != UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::SYSTEM_DEFINED_PIXEL_MAP)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "type error, objType:%{public}s", objType.c_str());
        return nullptr;
    }
    auto val = object->value_[UDMF::PIXEL_MAP];
    if (std::holds_alternative<std::shared_ptr<Media::PixelMap>>(val)) {
        return std::get<std::shared_ptr<Media::PixelMap>>(val);
    }
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "value error, no pixelmap");
    return nullptr;
} // LCOV_EXCL_STOP

std::shared_ptr<MineCustomData> PasteDataEntry::ConvertToCustomData() const
{ // LCOV_EXCL_START
    auto entry = GetValue();
    MineCustomData customdata;
    if (std::holds_alternative<std::vector<uint8_t>>(entry)) {
        customdata.AddItemData(GetMimeType(), std::get<std::vector<uint8_t>>(entry));
        return std::make_shared<MineCustomData>(customdata);
    }
    if (!std::holds_alternative<std::shared_ptr<Object>>(entry)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "value error, no custom data, utdId:%{public}s", utdId_.c_str());
        return nullptr;
    }
    auto object = std::get<std::shared_ptr<Object>>(entry);
    std::string objType;
    if (!object->GetValue(UDMF::UNIFORM_DATA_TYPE, objType)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "type error, utdId:%{public}s", utdId_.c_str());
        return nullptr;
    }
    if (objType != GetUtdId()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "type diff error, utdId:%{public}s, objType:%{public}s",
            utdId_.c_str(), objType.c_str());
    }
    std::vector<uint8_t> recordValue;
    if (!object->GetValue(UDMF::ARRAY_BUFFER, recordValue)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "get value error, utdId:%{public}s", utdId_.c_str());
        return nullptr;
    }
    customdata.AddItemData(utdId_, recordValue);
    return std::make_shared<MineCustomData>(customdata);
} // LCOV_EXCL_STOP

bool PasteDataEntry::HasContent(const std::string& utdId) const
{ // LCOV_EXCL_START
    auto mimeType = CommonUtils::Convert2MimeType(utdId);
    return HasContentByMimeType(mimeType);
} // LCOV_EXCL_STOP

bool PasteDataEntry::HasContentByMimeType(const std::string& mimeType) const
{ // LCOV_EXCL_START
    if (mimeType == MIMETYPE_PIXELMAP) {
        return ConvertToPixelMap() != nullptr;
    } else if (mimeType == MIMETYPE_TEXT_HTML) {
        auto html = ConvertToHtml();
        return html != nullptr && !html->empty();
    } else if (mimeType == MIMETYPE_TEXT_PLAIN) {
        auto plainText = ConvertToPlainText();
        return plainText != nullptr && !plainText->empty();
    } else if (mimeType == MIMETYPE_TEXT_URI) {
        auto uri = ConvertToUri();
        return uri != nullptr && !uri->ToString().empty();
    } else if (mimeType == MIMETYPE_TEXT_WANT) {
        return ConvertToWant() != nullptr;
    } else {
        return ConvertToCustomData() != nullptr;
    }
} // LCOV_EXCL_STOP

void PasteDataEntry::SetFileSize(int64_t fileSize)
{ // LCOV_EXCL_START
    auto entry = GetValue();
    PASTEBOARD_CHECK_AND_RETURN_LOGE(
        std::holds_alternative<std::shared_ptr<Object>>(entry), PASTEBOARD_MODULE_COMMON, "entry not contain object");

    auto object = std::get<std::shared_ptr<Object>>(entry);
    PASTEBOARD_CHECK_AND_RETURN_LOGE(object != nullptr, PASTEBOARD_MODULE_COMMON, "entry object is null");

    object->value_[PASTE_FILE_SIZE] = fileSize;
} // LCOV_EXCL_STOP

int64_t PasteDataEntry::GetFileSize() const
{ // LCOV_EXCL_START
    auto entry = GetValue();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(std::holds_alternative<std::shared_ptr<Object>>(entry), 0,
        PASTEBOARD_MODULE_COMMON, "entry not contain object");

    auto object = std::get<std::shared_ptr<Object>>(entry);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(object != nullptr, 0, PASTEBOARD_MODULE_COMMON, "entry object is null");

    int64_t fileSize = 0L;
    object->GetValue(PASTE_FILE_SIZE, fileSize);
    return fileSize;
} // LCOV_EXCL_STOP

std::string CommonUtils::Convert(UDType uDType)
{ // LCOV_EXCL_START
    switch (uDType) {
        // fall-through
        case UDType::PLAIN_TEXT:
        // fall-through
        case UDType::HYPERLINK:
            return MIMETYPE_TEXT_PLAIN;
        // fall-through
        case UDType::HTML:
            return MIMETYPE_TEXT_HTML;
        // fall-through
        case UDType::FILE:
        // fall-through
        case UDType::IMAGE:
        // fall-through
        case UDType::VIDEO:
        // fall-through
        case UDType::AUDIO:
        // fall-through
        case UDType::FOLDER:
        // fall-through
        case UDType::FILE_URI:
            return MIMETYPE_TEXT_URI;
        // fall-through
        case UDType::SYSTEM_DEFINED_PIXEL_MAP:
            return MIMETYPE_PIXELMAP;
        // fall-through
        case UDType::OPENHARMONY_WANT:
            return MIMETYPE_TEXT_WANT;
        default:
            return UDMF::UtdUtils::GetUtdIdFromUtdEnum(uDType);
    }
} // LCOV_EXCL_STOP

std::string CommonUtils::Convert2MimeType(const std::string& utdId)
{ // LCOV_EXCL_START
    if (utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::PLAIN_TEXT) ||
        utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::HYPERLINK)) {
        return MIMETYPE_TEXT_PLAIN;
    }
    if (utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::HTML)) {
        return MIMETYPE_TEXT_HTML;
    }
    if (IsFileUri(utdId)) {
        return MIMETYPE_TEXT_URI;
    }
    if (utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::SYSTEM_DEFINED_PIXEL_MAP)) {
        return MIMETYPE_PIXELMAP;
    }
    if (utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::OPENHARMONY_WANT)) {
        return MIMETYPE_TEXT_WANT;
    }
    return utdId;
} // LCOV_EXCL_STOP

// other is appdefined-types
std::string CommonUtils::Convert2UtdId(int32_t uDType, const std::string& mimeType)
{ // LCOV_EXCL_START
    if (mimeType == MIMETYPE_TEXT_PLAIN && uDType == UDMF::HYPERLINK) {
        return UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::HYPERLINK);
    }
    if (mimeType == MIMETYPE_TEXT_PLAIN) {
        return UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::PLAIN_TEXT);
    }
    if (mimeType == MIMETYPE_TEXT_URI) {
        return UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::FILE_URI);
    }
    if (mimeType == MIMETYPE_TEXT_HTML) {
        return UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::HTML);
    }
    if (mimeType == MIMETYPE_TEXT_WANT) {
        return UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::OPENHARMONY_WANT);
    }
    if (mimeType == MIMETYPE_PIXELMAP) {
        return UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::SYSTEM_DEFINED_PIXEL_MAP);
    }
    return mimeType;
} // LCOV_EXCL_STOP

UDMF::UDType CommonUtils::Convert(int32_t uDType, const std::string& mimeType)
{ // LCOV_EXCL_START
    if (uDType != UDMF::UD_BUTT) {
        return static_cast<UDType>(uDType);
    }
    if (mimeType == MIMETYPE_TEXT_URI) {
        return UDMF::FILE_URI;
    }
    if (mimeType == MIMETYPE_TEXT_PLAIN) {
        return UDMF::PLAIN_TEXT;
    }
    if (mimeType == MIMETYPE_TEXT_HTML) {
        return UDMF::HTML;
    }
    if (mimeType == MIMETYPE_TEXT_WANT) {
        return UDMF::OPENHARMONY_WANT;
    }
    if (mimeType == MIMETYPE_PIXELMAP) {
        return UDMF::SYSTEM_DEFINED_PIXEL_MAP;
    }
    auto type = UDMF::UtdUtils::GetUtdEnumFromUtdId(mimeType);
    if (type != UDMF::UD_BUTT) {
        return static_cast<UDType>(type);
    }
    return UDMF::APPLICATION_DEFINED_RECORD;
} // LCOV_EXCL_STOP

bool CommonUtils::IsFileUri(const std::string& utdId)
{ // LCOV_EXCL_START
    return utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::FILE_URI) ||
           utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::FILE) ||
           utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::AUDIO) ||
           utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::IMAGE) ||
           utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::FOLDER) ||
           utdId == UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::VIDEO);
} // LCOV_EXCL_STOP
} // namespace MiscServices
} // namespace OHOS
