/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2026. All rights reserved.
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

#include "paste_data_record.h"

#include "pasteboard_hilog.h"

using namespace OHOS::Media;

namespace OHOS {
namespace MiscServices {
constexpr int MAX_TEXT_LEN = 100 * 1024 * 1024;

PasteDataRecord::Builder& PasteDataRecord::Builder::SetMimeType(std::string mimeType)
{ // LCOV_EXCL_START
    record_->mimeType_ = std::move(mimeType);
    return *this;
} // LCOV_EXCL_STOP

enum TAG_PASTEBOARD_RECORD : uint16_t {
    TAG_MIMETYPE = TAG_BUFF + 1,
    TAG_HTMLTEXT,
    TAG_WANT,
    TAG_PLAINTEXT,
    TAG_URI,
    TAG_PIXELMAP,
    TAG_CUSTOM_DATA,
    TAG_CONVERT_URI,
    TAG_URI_PERMISSION,
    TAG_UDC_UDTYPE,
    TAG_UDC_DETAILS,
    TAG_UDC_TEXTCONTENT,
    TAG_UDC_SYSTEMCONTENTS,
    TAG_UDC_UDMFVALUE,
    TAG_UDC_ENTRIES,
    TAG_DATA_ID,
    TAG_RECORD_ID,
    TAG_DELAY_RECORD_FLAG,
    TAG_FROM,
};

PasteDataRecord::Builder& PasteDataRecord::Builder::SetHtmlText(std::shared_ptr<std::string> htmlText)
{ // LCOV_EXCL_START
    if (htmlText == nullptr) {
        return *this;
    }
    auto entry = std::make_shared<PasteDataEntry>();
    entry->SetValue(*htmlText);
    record_->AddEntryByMimeType(MIMETYPE_TEXT_HTML, entry);
    return *this;
} // LCOV_EXCL_STOP

PasteDataRecord::Builder& PasteDataRecord::Builder::SetWant(std::shared_ptr<OHOS::AAFwk::Want> want)
{ // LCOV_EXCL_START
    if (want == nullptr) {
        return *this;
    }
    auto entry = std::make_shared<PasteDataEntry>();
    entry->SetValue(std::move(want));
    record_->AddEntryByMimeType(MIMETYPE_TEXT_WANT, entry);
    return *this;
} // LCOV_EXCL_STOP

PasteDataRecord::Builder& PasteDataRecord::Builder::SetPlainText(std::shared_ptr<std::string> plainText)
{ // LCOV_EXCL_START
    if (plainText == nullptr) {
        return *this;
    }
    auto entry = std::make_shared<PasteDataEntry>();
    entry->SetValue(*plainText);
    record_->AddEntryByMimeType(MIMETYPE_TEXT_PLAIN, entry);
    return *this;
} // LCOV_EXCL_STOP

PasteDataRecord::Builder& PasteDataRecord::Builder::SetUri(std::shared_ptr<OHOS::Uri> uri)
{ // LCOV_EXCL_START
    if (uri == nullptr) {
        return *this;
    }
    auto entry = std::make_shared<PasteDataEntry>();
    entry->SetValue(uri->ToString());
    record_->AddEntryByMimeType(MIMETYPE_TEXT_URI, entry);
    return *this;
} // LCOV_EXCL_STOP

PasteDataRecord::Builder& PasteDataRecord::Builder::SetPixelMap(std::shared_ptr<OHOS::Media::PixelMap> pixelMap)
{ // LCOV_EXCL_START
    if (pixelMap == nullptr) {
        return *this;
    }
    auto entry = std::make_shared<PasteDataEntry>();
    entry->SetValue(std::move(pixelMap));
    record_->AddEntryByMimeType(MIMETYPE_PIXELMAP, entry);
    return *this;
} // LCOV_EXCL_STOP

PasteDataRecord::Builder& PasteDataRecord::Builder::SetCustomData(std::shared_ptr<MineCustomData> customData)
{ // LCOV_EXCL_START
    record_->customData_ = std::move(customData);
    return *this;
} // LCOV_EXCL_STOP

std::shared_ptr<PasteDataRecord> PasteDataRecord::Builder::Build()
{ // LCOV_EXCL_START
    if (record_->mimeType_.empty()) {
        return record_;
    }
    auto entries = record_->GetEntries();
    auto record = std::make_shared<PasteDataRecord>();
    for (size_t i = 0; i < entries.size(); ++i) {
        if (record_->mimeType_ == entries[i]->GetMimeType()) {
            record->AddEntry(entries[i]->GetUtdId(), entries[i]);
        }
    }
    for (size_t i = 0; i < entries.size(); ++i) {
        if (record_->mimeType_ != entries[i]->GetMimeType()) {
            record->AddEntry(entries[i]->GetUtdId(), entries[i]);
        }
    }
    record->customData_ = std::move(record_->customData_);
    record->mimeType_ = record_->mimeType_;
    return record;
} // LCOV_EXCL_STOP

PasteDataRecord::Builder::Builder(const std::string& mimeType)
{ // LCOV_EXCL_START
    record_ = std::make_shared<PasteDataRecord>();
    if (record_ != nullptr) {
        record_->mimeType_ = mimeType;
        record_->htmlText_ = nullptr;
        record_->want_ = nullptr;
        record_->plainText_ = nullptr;
        record_->uri_ = nullptr;
        record_->convertUri_ = "";
        record_->pixelMap_ = nullptr;
        record_->customData_ = nullptr;
    }
} // LCOV_EXCL_STOP

void PasteDataRecord::AddUriEntry()
{ // LCOV_EXCL_START
    auto object = std::make_shared<Object>();
    object->value_[UDMF::UNIFORM_DATA_TYPE] = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FILE_URI);
    if (uri_ != nullptr) {
        object->value_[UDMF::FILE_URI_PARAM] = uri_->ToString();
    }
    auto utdId = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FILE_URI);
    AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, object));
} // LCOV_EXCL_STOP

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewHtmlRecord(const std::string& htmlText)
{ // LCOV_EXCL_START
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE((htmlText.length() < MAX_TEXT_LEN), nullptr, PASTEBOARD_MODULE_CLIENT,
        "record length not support, length=%{public}zu", htmlText.length());
    return Builder(MIMETYPE_TEXT_HTML).SetHtmlText(std::make_shared<std::string>(htmlText)).Build();
} // LCOV_EXCL_STOP

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewWantRecord(std::shared_ptr<OHOS::AAFwk::Want> want)
{ // LCOV_EXCL_START
    return Builder(MIMETYPE_TEXT_WANT).SetWant(std::move(want)).Build();
} // LCOV_EXCL_STOP

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewPlainTextRecord(const std::string& text)
{ // LCOV_EXCL_START
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE((text.length() < MAX_TEXT_LEN), nullptr, PASTEBOARD_MODULE_CLIENT,
        "PlainText length not support, length=%{public}zu", text.length());
    return Builder(MIMETYPE_TEXT_PLAIN).SetPlainText(std::make_shared<std::string>(text)).Build();
} // LCOV_EXCL_STOP

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewPixelMapRecord(std::shared_ptr<PixelMap> pixelMap)
{ // LCOV_EXCL_START
    return Builder(MIMETYPE_PIXELMAP).SetPixelMap(std::move(pixelMap)).Build();
} // LCOV_EXCL_STOP

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewUriRecord(const OHOS::Uri& uri)
{ // LCOV_EXCL_START
    return Builder(MIMETYPE_TEXT_URI).SetUri(std::make_shared<OHOS::Uri>(uri)).Build();
} // LCOV_EXCL_STOP

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewKvRecord(
    const std::string& mimeType, const std::vector<uint8_t>& arrayBuffer)
{ // LCOV_EXCL_START
    std::shared_ptr<MineCustomData> customData = std::make_shared<MineCustomData>();
    customData->AddItemData(mimeType, arrayBuffer);
    return Builder(mimeType).SetCustomData(std::move(customData)).Build();
} // LCOV_EXCL_STOP

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewMultiTypeRecord(
    std::shared_ptr<std::map<std::string, std::shared_ptr<EntryValue>>> values, const std::string& recordMimeType)
{ // LCOV_EXCL_START
    auto record = std::make_shared<PasteDataRecord>();
    if (values == nullptr) {
        return record;
    }
    if (!recordMimeType.empty()) {
        auto recordDefaultIter = values->find(recordMimeType);
        if (recordDefaultIter != values->end() && recordDefaultIter->second != nullptr) {
            auto utdId = CommonUtils::Convert2UtdId(UDMF::UDType::UD_BUTT, recordMimeType);
            record->AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, *(recordDefaultIter->second)));
        }
        record->mimeType_ = recordMimeType;
    }
    for (auto [mimeType, value] : *values) {
        if (mimeType == recordMimeType) {
            continue;
        }
        auto utdId = CommonUtils::Convert2UtdId(UDMF::UDType::UD_BUTT, mimeType);
        record->AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, *value));
    }
    return record;
} // LCOV_EXCL_STOP

std::shared_ptr<PasteDataRecord> PasteDataRecord::NewMultiTypeDelayRecord(
    std::vector<std::string> mimeTypes, const std::shared_ptr<UDMF::EntryGetter> entryGetter)
{ // LCOV_EXCL_START
    auto record = std::make_shared<PasteDataRecord>();
    for (auto mimeType : mimeTypes) {
        auto utdId = CommonUtils::Convert2UtdId(UDMF::UDType::UD_BUTT, mimeType);
        auto entry = std::make_shared<PasteDataEntry>();
        entry->SetMimeType(mimeType);
        entry->SetUtdId(utdId);
        record->AddEntry(utdId, entry);
    }
    if (entryGetter != nullptr) {
        record->SetEntryGetter(entryGetter);
        record->SetDelayRecordFlag(true);
    }
    return record;
} // LCOV_EXCL_STOP

PasteDataRecord::PasteDataRecord(std::string mimeType, std::shared_ptr<std::string> htmlText,
    std::shared_ptr<OHOS::AAFwk::Want> want, std::shared_ptr<std::string> plainText, std::shared_ptr<OHOS::Uri> uri)
    : mimeType_ { std::move(mimeType) }, htmlText_ { std::move(htmlText) }, want_ { std::move(want) },
      plainText_ { std::move(plainText) }, uri_ { std::move(uri) }
{ // LCOV_EXCL_START
} // LCOV_EXCL_STOP

PasteDataRecord::PasteDataRecord()
{ // LCOV_EXCL_START
} // LCOV_EXCL_STOP

PasteDataRecord::~PasteDataRecord()
{ // LCOV_EXCL_START
    std::vector<std::shared_ptr<PasteDataEntry>>().swap(entries_);
} // LCOV_EXCL_STOP

PasteDataRecord::PasteDataRecord(const PasteDataRecord& record)
    : isDelay_(record.isDelay_), hasGrantUriPermission_(record.hasGrantUriPermission_), udType_(record.udType_),
      dataId_(record.dataId_), recordId_(record.recordId_), from_(record.from_), uriPermission_(record.uriPermission_),
      convertUri_(record.convertUri_), textContent_(record.textContent_), mimeType_(record.mimeType_),
      htmlText_(record.htmlText_), want_(record.want_), plainText_(record.plainText_), uri_(record.uri_),
      pixelMap_(record.pixelMap_), customData_(record.customData_), details_(record.details_),
      systemDefinedContents_(record.systemDefinedContents_), udmfValue_(record.udmfValue_), entries_(record.entries_),
      entryGetter_(record.entryGetter_)
{ // LCOV_EXCL_START
    this->isConvertUriFromRemote = record.isConvertUriFromRemote;
} // LCOV_EXCL_STOP

std::shared_ptr<std::string> PasteDataRecord::GetHtmlTextV0() const
{ // LCOV_EXCL_START
    for (const auto& entry : entries_) {
        if (entry && entry->GetMimeType() == MIMETYPE_TEXT_HTML) {
            return entry->ConvertToHtml();
        }
    }
    return htmlText_;
} // LCOV_EXCL_STOP

std::shared_ptr<std::string> PasteDataRecord::GetHtmlText()
{ // LCOV_EXCL_START
    auto htmlText = GetHtmlTextV0();
    if (htmlText) {
        return htmlText;
    }
    auto entry = GetEntryByMimeType(MIMETYPE_TEXT_HTML);
    if (entry == nullptr) {
        return htmlText_;
    }
    return entry->ConvertToHtml();
} // LCOV_EXCL_STOP

std::string PasteDataRecord::GetMimeType() const
{ // LCOV_EXCL_START
    if (!mimeType_.empty()) {
        return mimeType_;
    }
    if (!entries_.empty()) {
        return entries_.front()->GetMimeType();
    }
    return this->mimeType_;
} // LCOV_EXCL_STOP

std::shared_ptr<std::string> PasteDataRecord::GetPlainTextV0() const
{ // LCOV_EXCL_START
    for (const auto& entry : entries_) {
        if (entry && entry->GetMimeType() == MIMETYPE_TEXT_PLAIN) {
            return entry->ConvertToPlainText();
        }
    }
    return plainText_;
} // LCOV_EXCL_STOP

std::shared_ptr<std::string> PasteDataRecord::GetPlainText()
{ // LCOV_EXCL_START
    auto plainText = GetPlainTextV0();
    if (plainText) {
        return plainText;
    }
    auto entry = GetEntryByMimeType(MIMETYPE_TEXT_PLAIN);
    if (entry == nullptr) {
        return plainText_;
    }
    return entry->ConvertToPlainText();
} // LCOV_EXCL_STOP

std::shared_ptr<PixelMap> PasteDataRecord::GetPixelMapV0() const
{ // LCOV_EXCL_START
    for (const auto& entry : entries_) {
        if (entry && entry->GetMimeType() == MIMETYPE_PIXELMAP) {
            return entry->ConvertToPixelMap();
        }
    }
    return pixelMap_;
} // LCOV_EXCL_STOP

std::shared_ptr<PixelMap> PasteDataRecord::GetPixelMap()
{ // LCOV_EXCL_START
    auto pixelMap = GetPixelMapV0();
    if (pixelMap) {
        return pixelMap;
    }
    auto entry = GetEntryByMimeType(MIMETYPE_PIXELMAP);
    if (entry == nullptr) {
        return pixelMap_;
    }
    return entry->ConvertToPixelMap();
} // LCOV_EXCL_STOP

std::shared_ptr<OHOS::Uri> PasteDataRecord::GetUriV0() const
{ // LCOV_EXCL_START
    if (convertUri_.empty()) {
        return GetOriginUri();
    }
    return std::make_shared<OHOS::Uri>(convertUri_);
} // LCOV_EXCL_STOP

std::shared_ptr<OHOS::Uri> PasteDataRecord::GetUri()
{ // LCOV_EXCL_START
    auto uri = GetUriV0();
    if (uri) {
        return uri;
    }
    auto entry = GetEntryByMimeType(MIMETYPE_TEXT_URI);
    if (entry == nullptr) {
        return GetUriV0();
    }
    return entry->ConvertToUri();
} // LCOV_EXCL_STOP

void PasteDataRecord::ClearPixelMap()
{ // LCOV_EXCL_START
    this->pixelMap_ = nullptr;
    entries_.erase(std::remove_if(entries_.begin(), entries_.end(),
                       [](const auto& entry) { return entry != nullptr && entry->GetMimeType() == MIMETYPE_PIXELMAP; }),
        entries_.end());
} // LCOV_EXCL_STOP

void PasteDataRecord::SetUri(std::shared_ptr<OHOS::Uri> uri)
{ // LCOV_EXCL_START
    if (uri == nullptr) {
        return;
    }

    for (auto& entry : entries_) {
        if (entry != nullptr && entry->GetMimeType() == MIMETYPE_TEXT_URI) {
            auto entryValue = entry->GetValue();
            if (std::holds_alternative<std::shared_ptr<Object>>(entryValue)) {
                auto object = std::get<std::shared_ptr<Object>>(entryValue);
                object->value_[UDMF::FILE_URI_PARAM] = uri->ToString();
            } else {
                entry->SetValue(uri->ToString());
            }
            return;
        }
    }

    auto entry = std::make_shared<PasteDataEntry>();
    entry->SetValue(uri->ToString());
    AddEntryByMimeType(MIMETYPE_TEXT_URI, entry);
} // LCOV_EXCL_STOP

std::shared_ptr<OHOS::Uri> PasteDataRecord::GetOriginUri() const
{ // LCOV_EXCL_START
    for (const auto& entry : entries_) {
        if (entry && entry->GetMimeType() == MIMETYPE_TEXT_URI) {
            return entry->ConvertToUri();
        }
    }
    return uri_;
} // LCOV_EXCL_STOP

std::shared_ptr<OHOS::AAFwk::Want> PasteDataRecord::GetWant() const
{ // LCOV_EXCL_START
    for (const auto& entry : entries_) {
        if (entry && entry->GetMimeType() == MIMETYPE_TEXT_WANT) {
            return entry->ConvertToWant();
        }
    }
    return want_;
} // LCOV_EXCL_STOP

std::shared_ptr<MineCustomData> PasteDataRecord::GetCustomData() const
{ // LCOV_EXCL_START
    std::shared_ptr<MineCustomData> customData = std::make_shared<MineCustomData>();
    if (customData_) {
        const std::map<std::string, std::vector<uint8_t>>& itemData = customData_->GetItemData();
        for (const auto& [key, value] : itemData) {
            customData->AddItemData(key, value);
        }
    }
    for (const auto& entry : entries_) {
        if (entry && entry->GetMimeType() == entry->GetUtdId()) {
            std::shared_ptr<MineCustomData> entryCustomData = entry->ConvertToCustomData();
            if (entryCustomData == nullptr) {
                continue;
            }
            const std::map<std::string, std::vector<uint8_t>>& itemData = entryCustomData->GetItemData();
            for (const auto& [key, value] : itemData) {
                customData->AddItemData(key, value);
            }
        }
    }
    return customData->GetItemData().empty() ? nullptr : customData;
} // LCOV_EXCL_STOP

std::string PasteDataRecord::ConvertToText() const
{ // LCOV_EXCL_START
    auto htmlText = GetHtmlTextV0();
    if (htmlText != nullptr) {
        return *htmlText;
    }
    auto plainText = GetPlainTextV0();
    if (plainText != nullptr) {
        return *plainText;
    }
    auto originUri = GetOriginUri();
    if (originUri != nullptr) {
        return originUri->ToString();
    }
    return "";
} // LCOV_EXCL_STOP

bool PasteDataRecord::EncodeTLVLocal(WriteOnlyBuffer& buffer) const
{
    bool ret = buffer.Write(TAG_MIMETYPE, mimeType_);
    ret = ret && buffer.Write(TAG_HTMLTEXT, htmlText_);
    ret = ret && buffer.Write(TAG_WANT, want_);
    ret = ret && buffer.Write(TAG_PLAINTEXT, plainText_);
    ret = ret && buffer.Write(TAG_URI, TLVUtils::Parcelable2Raw(uri_.get()));
    ret = ret && buffer.Write(TAG_CONVERT_URI, convertUri_);
    ret = ret && buffer.Write(TAG_PIXELMAP, pixelMap_);
    ret = ret && buffer.Write(TAG_CUSTOM_DATA, customData_);
    ret = ret && buffer.Write(TAG_URI_PERMISSION, hasGrantUriPermission_);
    ret = ret && buffer.Write(TAG_UDC_UDTYPE, udType_);
    ret = ret && buffer.Write(TAG_UDC_DETAILS, details_);
    ret = ret && buffer.Write(TAG_UDC_TEXTCONTENT, textContent_);
    ret = ret && buffer.Write(TAG_UDC_SYSTEMCONTENTS, systemDefinedContents_);
    ret = ret && buffer.Write(TAG_UDC_UDMFVALUE, udmfValue_);
    ret = ret && buffer.Write(TAG_UDC_ENTRIES, entries_);
    ret = ret && buffer.Write(TAG_DATA_ID, dataId_);
    ret = ret && buffer.Write(TAG_RECORD_ID, recordId_);
    ret = ret && buffer.Write(TAG_DELAY_RECORD_FLAG, isDelay_);
    ret = ret && buffer.Write(TAG_FROM, from_);
    return ret;
}

bool PasteDataRecord::EncodeTLVRemote(WriteOnlyBuffer& buffer) const
{
    bool ret = true;

    auto remoteValue = Local2Remote();
    if (remoteValue != nullptr) {
        ret = ret && buffer.Write(TAG_MIMETYPE, remoteValue->mimeType_);
        ret = ret && buffer.Write(TAG_UDC_UDTYPE, remoteValue->udType_);
        ret = ret && buffer.Write(TAG_HTMLTEXT, remoteValue->htmlText_);
        ret = ret && buffer.Write(TAG_PLAINTEXT, remoteValue->plainText_);
        ret = ret && buffer.Write(TAG_PIXELMAP, remoteValue->pixelMap_);
        ret = ret && buffer.Write(TAG_WANT, remoteValue->want_);
        ret = ret && buffer.Write(TAG_URI, TLVUtils::Parcelable2Raw(remoteValue->uri_.get()));
        ret = ret && buffer.Write(TAG_UDC_UDMFVALUE, remoteValue->udmfValue_);
        ret = ret && buffer.Write(TAG_UDC_ENTRIES, remoteValue->entries_);
    }

    ret = ret && buffer.Write(TAG_CONVERT_URI, convertUri_);
    ret = ret && buffer.Write(TAG_CUSTOM_DATA, customData_);
    ret = ret && buffer.Write(TAG_URI_PERMISSION, hasGrantUriPermission_);
    ret = ret && buffer.Write(TAG_UDC_DETAILS, details_);
    ret = ret && buffer.Write(TAG_UDC_TEXTCONTENT, textContent_);
    ret = ret && buffer.Write(TAG_UDC_SYSTEMCONTENTS, systemDefinedContents_);
    ret = ret && buffer.Write(TAG_DATA_ID, dataId_);
    ret = ret && buffer.Write(TAG_RECORD_ID, recordId_);
    ret = ret && buffer.Write(TAG_DELAY_RECORD_FLAG, isDelay_);
    ret = ret && buffer.Write(TAG_FROM, from_);
    return ret;
}

bool PasteDataRecord::EncodeTLV(WriteOnlyBuffer& buffer) const
{
    return IsRemoteEncode() ? EncodeTLVRemote(buffer) : EncodeTLVLocal(buffer);
}

bool PasteDataRecord::DecodeItem1(uint16_t tag, ReadOnlyBuffer& buffer, TLVHead& head)
{
    switch (tag) {
        case TAG_MIMETYPE:
            return buffer.ReadValue(mimeType_, head);
        case TAG_HTMLTEXT:
            return buffer.ReadValue(htmlText_, head);
        case TAG_WANT:
            return buffer.ReadValue(want_, head);
        case TAG_PLAINTEXT:
            return buffer.ReadValue(plainText_, head);
        case TAG_URI:
            return buffer.ReadValue(uri_, head);
        case TAG_CONVERT_URI:
            return buffer.ReadValue(convertUri_, head);
        case TAG_PIXELMAP:
            return buffer.ReadValue(pixelMap_, head);
        case TAG_CUSTOM_DATA:
            return buffer.ReadValue(customData_, head);
        case TAG_URI_PERMISSION:
            return buffer.ReadValue(hasGrantUriPermission_, head);
        default:
            return DecodeItem2(tag, buffer, head);
    }
}

bool PasteDataRecord::DecodeItem2(uint16_t tag, ReadOnlyBuffer& buffer, TLVHead& head)
{
    switch (tag) {
        case TAG_UDC_UDTYPE:
            return buffer.ReadValue(udType_, head);
        case TAG_UDC_DETAILS:
            return buffer.ReadValue(details_, head);
        case TAG_UDC_TEXTCONTENT:
            return buffer.ReadValue(textContent_, head);
        case TAG_UDC_SYSTEMCONTENTS:
            return buffer.ReadValue(systemDefinedContents_, head);
        case TAG_UDC_UDMFVALUE:
            return buffer.ReadValue(udmfValue_, head);
        case TAG_UDC_ENTRIES:
            return buffer.ReadValue(entries_, head);
        case TAG_DATA_ID:
            return buffer.ReadValue(dataId_, head);
        case TAG_RECORD_ID:
            return buffer.ReadValue(recordId_, head);
        case TAG_DELAY_RECORD_FLAG:
            return buffer.ReadValue(isDelay_, head);
        case TAG_FROM:
            return buffer.ReadValue(from_, head);
        default:
            return buffer.Skip(head.len);
    }
}

bool PasteDataRecord::DecodeTLV(ReadOnlyBuffer& buffer)
{
    for (; buffer.IsEnough();) {
        TLVHead head {};
        bool ret = buffer.ReadHead(head);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON, "read head failed");
        ret = DecodeItem1(head.tag, buffer, head);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON,
            "read value failed, tag=%{public}hu, len=%{public}u", head.tag, head.len);
    }

    auto entry = Remote2Local();
    if (entry != nullptr) {
        entries_.insert(entries_.begin(), std::move(entry));
    }
    udmfValue_ = nullptr;
    plainText_ = nullptr;
    htmlText_ = nullptr;
    uri_ = nullptr;
    pixelMap_ = nullptr;
    want_ = nullptr;
    if (mimeType_.empty()) {
        mimeType_ = GetMimeType();
    }
    return true;
}

size_t PasteDataRecord::CountTLVLocal() const
{
    size_t expectedSize = 0;
    expectedSize += TLVCountable::Count(mimeType_);
    expectedSize += TLVCountable::Count(htmlText_);
    expectedSize += TLVCountable::Count(want_);
    expectedSize += TLVCountable::Count(plainText_);
    expectedSize += TLVCountable::Count(TLVUtils::Parcelable2Raw(uri_.get()));
    expectedSize += TLVCountable::Count(convertUri_);
    expectedSize += TLVCountable::Count(pixelMap_);
    expectedSize += TLVCountable::Count(customData_);
    expectedSize += TLVCountable::Count(hasGrantUriPermission_);
    expectedSize += TLVCountable::Count(udType_);
    expectedSize += TLVCountable::Count(details_);
    expectedSize += TLVCountable::Count(textContent_);
    expectedSize += TLVCountable::Count(systemDefinedContents_);
    expectedSize += TLVCountable::Count(udmfValue_);
    expectedSize += TLVCountable::Count(entries_);
    expectedSize += TLVCountable::Count(dataId_);
    expectedSize += TLVCountable::Count(recordId_);
    expectedSize += TLVCountable::Count(isDelay_);
    expectedSize += TLVCountable::Count(from_);
    return expectedSize;
}

size_t PasteDataRecord::CountTLVRemote() const
{
    size_t expectedSize = 0;
    auto remoteValue = Local2Remote();
    if (remoteValue != nullptr) {
        expectedSize += TLVCountable::Count(remoteValue->mimeType_);
        expectedSize += TLVCountable::Count(remoteValue->udType_);
        expectedSize += TLVCountable::Count(remoteValue->htmlText_);
        expectedSize += TLVCountable::Count(remoteValue->plainText_);
        expectedSize += TLVCountable::Count(remoteValue->pixelMap_);
        expectedSize += TLVCountable::Count(remoteValue->want_);
        expectedSize += TLVCountable::Count(TLVUtils::Parcelable2Raw(remoteValue->uri_.get()));
        expectedSize += TLVCountable::Count(remoteValue->udmfValue_);
        expectedSize += TLVCountable::Count(remoteValue->entries_);
    }

    expectedSize += TLVCountable::Count(convertUri_);
    expectedSize += TLVCountable::Count(customData_);
    expectedSize += TLVCountable::Count(hasGrantUriPermission_);
    expectedSize += TLVCountable::Count(details_);
    expectedSize += TLVCountable::Count(textContent_);
    expectedSize += TLVCountable::Count(systemDefinedContents_);
    expectedSize += TLVCountable::Count(dataId_);
    expectedSize += TLVCountable::Count(recordId_);
    expectedSize += TLVCountable::Count(isDelay_);
    expectedSize += TLVCountable::Count(from_);
    return expectedSize;
}

size_t PasteDataRecord::CountTLV() const
{
    return IsRemoteEncode() ? CountTLVRemote() : CountTLVLocal();
}

std::shared_ptr<PasteDataEntry> PasteDataRecord::Remote2Local() const
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGD(!mimeType_.empty(), nullptr, PASTEBOARD_MODULE_COMMON, "mimeType empty");

    auto entry = std::make_shared<PasteDataEntry>();
    auto utdId = CommonUtils::Convert2UtdId(udType_, mimeType_);
    entry->SetUtdId(utdId);
    entry->SetMimeType(mimeType_);

    if (udmfValue_ != nullptr) {
        if (std::holds_alternative<std::shared_ptr<Object>>(*udmfValue_)) {
            auto object = std::get<std::shared_ptr<Object>>(*udmfValue_);
            if (object != nullptr && !object->value_.empty()) {
                entry->SetValue(object);
                return entry;
            }
        } else if (std::holds_alternative<std::vector<uint8_t>>(*udmfValue_)) {
            auto array = std::get<std::vector<uint8_t>>(*udmfValue_);
            entry->SetValue(array);
            return entry;
        }
    }

    auto object = std::make_shared<Object>();
    if (mimeType_ == MIMETYPE_TEXT_PLAIN && plainText_ != nullptr) {
        object->value_[UDMF::UNIFORM_DATA_TYPE] = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::PLAIN_TEXT);
        object->value_[UDMF::CONTENT] = *plainText_;
    } else if (mimeType_ == MIMETYPE_TEXT_HTML && htmlText_ != nullptr) {
        object->value_[UDMF::UNIFORM_DATA_TYPE] = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::HTML);
        object->value_[UDMF::HTML_CONTENT] = *htmlText_;
        if (plainText_ != nullptr) {
            object->value_[UDMF::PLAIN_CONTENT] = *plainText_;
        }
    } else if (mimeType_ == MIMETYPE_TEXT_URI && uri_ != nullptr) {
        object->value_[UDMF::UNIFORM_DATA_TYPE] = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::FILE_URI);
        object->value_[UDMF::FILE_URI_PARAM] = uri_->ToString();
    } else if (mimeType_ == MIMETYPE_PIXELMAP && pixelMap_ != nullptr) {
        object->value_[UDMF::UNIFORM_DATA_TYPE] = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::SYSTEM_DEFINED_PIXEL_MAP);
        object->value_[UDMF::PIXEL_MAP] = std::move(pixelMap_);
    } else if (mimeType_ == MIMETYPE_TEXT_WANT && want_ != nullptr) {
        entry->SetValue(std::move(want_));
        return entry;
    } else {
        return nullptr;
    }

    entry->SetValue(object);
    return entry;
}

std::shared_ptr<RemoteRecordValue> PasteDataRecord::Local2Remote() const
{
    auto value = std::make_shared<RemoteRecordValue>();
    value->mimeType_ = mimeType_;
    if (entries_.empty()) {
        return value;
    }

    auto firstEntry = entries_.front();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(firstEntry != nullptr, value, PASTEBOARD_MODULE_COMMON, "firstEntry is null");

    auto entryValue = firstEntry->GetValue();
    std::string mimeType = firstEntry->GetMimeType();
    std::string utdId = firstEntry->GetUtdId();
    value->udType_ = UDMF::UtdUtils::GetUtdEnumFromUtdId(utdId);
    value->mimeType_ = mimeType;

    if (mimeType == MIMETYPE_TEXT_PLAIN) {
        value->plainText_ = firstEntry->ConvertToPlainText();
    } else if (mimeType == MIMETYPE_TEXT_WANT) {
        value->want_ = firstEntry->ConvertToWant();
    } else if (mimeType == MIMETYPE_TEXT_URI) {
        value->uri_ = firstEntry->ConvertToUri();
    } else if (mimeType == MIMETYPE_PIXELMAP) {
        value->pixelMap_ = firstEntry->ConvertToPixelMap();
    } else if (mimeType == MIMETYPE_TEXT_HTML) {
        value->htmlText_ = firstEntry->ConvertToHtml();
        if (std::holds_alternative<std::shared_ptr<Object>>(entryValue)) {
            auto object = std::get<std::shared_ptr<Object>>(entryValue);
            std::string plainText;
            object->GetValue(UDMF::PLAIN_CONTENT, plainText);
            if (!plainText.empty()) {
                value->plainText_ = std::make_shared<std::string>(plainText);
            }
        }
    }

    value->udmfValue_ = entryValue;
    value->entries_.assign(entries_.begin() + 1, entries_.end());
    return value;
}

std::string PasteDataRecord::GetPassUri()
{ // LCOV_EXCL_START
    std::string tempUri;
    if (uri_ != nullptr) {
        tempUri = uri_->ToString();
    }
    if (!convertUri_.empty()) {
        tempUri = convertUri_;
    }
    return tempUri;
} // LCOV_EXCL_STOP

void PasteDataRecord::SetConvertUri(const std::string& value)
{ // LCOV_EXCL_START
    convertUri_ = value;
} // LCOV_EXCL_STOP

std::string PasteDataRecord::GetConvertUri() const
{ // LCOV_EXCL_START
    return convertUri_;
} // LCOV_EXCL_STOP

void PasteDataRecord::SetGrantUriPermission(bool hasPermission)
{ // LCOV_EXCL_START
    hasGrantUriPermission_ = hasPermission;
} // LCOV_EXCL_STOP

bool PasteDataRecord::HasGrantUriPermission()
{ // LCOV_EXCL_START
    return hasGrantUriPermission_;
} // LCOV_EXCL_STOP

void PasteDataRecord::SetUriPermission(uint32_t uriPermission)
{ // LCOV_EXCL_START
    uriPermission_ = uriPermission;
} // LCOV_EXCL_STOP

uint32_t PasteDataRecord::GetUriPermission()
{ // LCOV_EXCL_START
    return uriPermission_;
} // LCOV_EXCL_STOP

void PasteDataRecord::SetTextContent(const std::string& content)
{ // LCOV_EXCL_START
    this->textContent_ = content;
} // LCOV_EXCL_STOP

std::string PasteDataRecord::GetTextContent() const
{ // LCOV_EXCL_START
    return this->textContent_;
} // LCOV_EXCL_STOP

void PasteDataRecord::SetDetails(const Details& details)
{ // LCOV_EXCL_START
    this->details_ = std::make_shared<Details>(details);
} // LCOV_EXCL_STOP

std::shared_ptr<Details> PasteDataRecord::GetDetails() const
{ // LCOV_EXCL_START
    return this->details_;
} // LCOV_EXCL_STOP

void PasteDataRecord::SetSystemDefinedContent(const Details& contents)
{ // LCOV_EXCL_START
    this->systemDefinedContents_ = std::make_shared<Details>(contents);
} // LCOV_EXCL_STOP

std::shared_ptr<Details> PasteDataRecord::GetSystemDefinedContent() const
{ // LCOV_EXCL_START
    return this->systemDefinedContents_;
} // LCOV_EXCL_STOP

int32_t PasteDataRecord::GetUDType() const
{ // LCOV_EXCL_START
    return this->udType_;
} // LCOV_EXCL_STOP

void PasteDataRecord::SetUDType(int32_t type)
{ // LCOV_EXCL_START
    this->udType_ = type;
} // LCOV_EXCL_STOP

std::vector<std::string> PasteDataRecord::GetValidMimeTypes(const std::vector<std::string>& mimeTypes) const
{ // LCOV_EXCL_START
    std::vector<std::string> res;
    auto allTypes = GetMimeTypes();
    for (auto const& type : mimeTypes) {
        if (allTypes.find(type) != allTypes.end()) {
            res.emplace_back(type);
        }
    }
    return res;
} // LCOV_EXCL_STOP

std::vector<std::string> PasteDataRecord::GetValidTypes(const std::vector<std::string>& types) const
{ // LCOV_EXCL_START
    std::vector<std::string> res;
    auto allTypes = GetUtdTypes();
    for (auto const& type : types) {
        if (allTypes.find(type) != allTypes.end()) {
            res.emplace_back(type);
        }
    }
    return res;
} // LCOV_EXCL_STOP

bool PasteDataRecord::HasEmptyEntry() const
{ // LCOV_EXCL_START
    for (auto const& entry : GetEntries()) {
        if (std::holds_alternative<std::monostate>(entry->GetValue())) {
            return true;
        }
    }
    return false;
} // LCOV_EXCL_STOP

uint32_t PasteDataRecord::RemoveEmptyEntry()
{ // LCOV_EXCL_START
    uint32_t removeCnt = 0;
    for (auto iter = entries_.begin(); iter != entries_.end();) {
        auto entry = *iter;
        if (entry == nullptr || std::holds_alternative<std::monostate>(entry->GetValue())) {
            PASTEBOARD_HILOGI(PASTEBOARD_MODULE_SERVICE, "recordId=%{public}u, type=%{public}s", GetRecordId(),
                entry ? entry->GetUtdId().c_str() : "null");
            iter = entries_.erase(iter);
            ++removeCnt;
        } else {
            ++iter;
        }
    }
    return removeCnt;
} // LCOV_EXCL_STOP

std::set<std::string> PasteDataRecord::GetUtdTypes() const
{ // LCOV_EXCL_START
    std::set<std::string> types;
    if (!mimeType_.empty()) {
        types.emplace(CommonUtils::Convert2UtdId(udType_, mimeType_));
    }
    for (auto const& entry : entries_) {
        types.emplace(entry->GetUtdId());
    }
    return types;
} // LCOV_EXCL_STOP

std::set<std::string> PasteDataRecord::GetMimeTypes() const
{ // LCOV_EXCL_START
    std::set<std::string> types;
    if (!mimeType_.empty()) {
        types.emplace(mimeType_);
    }
    for (auto const& entry : entries_) {
        types.emplace(entry->GetMimeType());
    }
    return types;
} // LCOV_EXCL_STOP

void PasteDataRecord::AddEntryByMimeType(const std::string& mimeType, std::shared_ptr<PasteDataEntry> value)
{ // LCOV_EXCL_START
    PASTEBOARD_CHECK_AND_RETURN_LOGE(value != nullptr, PASTEBOARD_MODULE_CLIENT, "value is null");
    auto utdId = CommonUtils::Convert2UtdId(UDMF::UDType::UD_BUTT, mimeType);
    value->SetUtdId(utdId);
    value->SetMimeType(mimeType);
    AddEntry(utdId, value);
} // LCOV_EXCL_STOP

void PasteDataRecord::AddEntry(const std::string& utdType, std::shared_ptr<PasteDataEntry> value)
{ // LCOV_EXCL_START
    PASTEBOARD_CHECK_AND_RETURN_LOGE(value != nullptr, PASTEBOARD_MODULE_CLIENT, "Entry value is null");
    if (utdType != value->GetUtdId()) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Type is diff, UtdType:%{public}s, UtdId:%{public}s",
            utdType.c_str(), value->GetUtdId().c_str());
        return;
    }

    bool has = false;
    for (auto& entry : entries_) {
        if (entry->GetUtdId() == utdType ||
            (entry->GetMimeType() == MIMETYPE_TEXT_URI && value->GetMimeType() == MIMETYPE_TEXT_URI)) {
            entry = value;
            has = true;
            break;
        }
    }

    PASTEBOARD_CHECK_AND_RETURN_LOGD(!has, PASTEBOARD_MODULE_COMMON, "replace entry, type=%{public}s", utdType.c_str());
    if (entries_.empty()) {
        auto udType = UDMF::UtdUtils::GetUtdEnumFromUtdId(utdType);
        udType_ = udType == UDMF::UDType::UD_BUTT ? UDMF::UDType::APPLICATION_DEFINED_RECORD : udType;
    }
    entries_.emplace_back(value);
} // LCOV_EXCL_STOP

std::shared_ptr<PasteDataEntry> PasteDataRecord::GetEntryByMimeType(const std::string& mimeType)
{ // LCOV_EXCL_START
    auto utdId = CommonUtils::Convert2UtdId(UDMF::UDType::UD_BUTT, mimeType);
    std::shared_ptr<PasteDataEntry> entry = GetEntry(utdId);
    if (entry == nullptr && customData_ != nullptr) {
        const std::map<std::string, std::vector<uint8_t>>& itemData = customData_->GetItemData();
        for (const auto& [key, value] : itemData) {
            if (mimeType == key) {
                entry = std::make_shared<PasteDataEntry>(utdId, mimeType, value);
                return entry;
            }
        }
    }
    if (entry == nullptr && mimeType == MIMETYPE_TEXT_PLAIN) {
        utdId = CommonUtils::Convert2UtdId(UDMF::UDType::HYPERLINK, mimeType);
        entry = GetEntry(utdId);
    }
    return entry;
} // LCOV_EXCL_STOP

std::shared_ptr<PasteDataEntry> PasteDataRecord::GetEntry(const std::string& utdType)
{ // LCOV_EXCL_START
    for (auto const& entry : entries_) {
        if (entry->GetUtdId() == utdType ||
            (CommonUtils::IsFileUri(utdType) && CommonUtils::IsFileUri(entry->GetUtdId()))) {
            if (CommonUtils::IsFileUri(utdType) && GetUriV0() != nullptr) {
                return std::make_shared<PasteDataEntry>(utdType, GetUriV0()->ToString());
            }
            return entry;
        }
    }
    return nullptr;
} // LCOV_EXCL_STOP

std::vector<std::shared_ptr<PasteDataEntry>> PasteDataRecord::GetEntries() const
{ // LCOV_EXCL_START
    return entries_;
} // LCOV_EXCL_STOP

void PasteDataRecord::SetDataId(uint32_t dataId)
{ // LCOV_EXCL_START
    dataId_ = dataId;
} // LCOV_EXCL_STOP

uint32_t PasteDataRecord::GetDataId() const
{ // LCOV_EXCL_START
    return dataId_;
} // LCOV_EXCL_STOP

void PasteDataRecord::SetRecordId(uint32_t recordId)
{ // LCOV_EXCL_START
    recordId_ = recordId;
} // LCOV_EXCL_STOP

uint32_t PasteDataRecord::GetRecordId() const
{ // LCOV_EXCL_START
    return recordId_;
} // LCOV_EXCL_STOP

void PasteDataRecord::SetDelayRecordFlag(bool isDelay)
{ // LCOV_EXCL_START
    isDelay_ = isDelay;
} // LCOV_EXCL_STOP

bool PasteDataRecord::IsDelayRecord() const
{ // LCOV_EXCL_START
    return isDelay_;
} // LCOV_EXCL_STOP

void PasteDataRecord::SetEntryGetter(const std::shared_ptr<UDMF::EntryGetter> entryGetter)
{ // LCOV_EXCL_START
    entryGetter_ = std::move(entryGetter);
} // LCOV_EXCL_STOP

void PasteDataRecord::SetFrom(uint32_t from)
{ // LCOV_EXCL_START
    from_ = from;
} // LCOV_EXCL_STOP

uint32_t PasteDataRecord::GetFrom() const
{ // LCOV_EXCL_START
    return from_;
} // LCOV_EXCL_STOP

std::shared_ptr<UDMF::EntryGetter> PasteDataRecord::GetEntryGetter()
{ // LCOV_EXCL_START
    return entryGetter_;
} // LCOV_EXCL_STOP
} // namespace MiscServices
} // namespace OHOS
