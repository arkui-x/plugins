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

#include "convert_utils.h"

#include "pasteboard_hilog.h"

namespace OHOS {
namespace MiscServices {
using UnifiedRecord = UDMF::UnifiedRecord;
using UnifiedData = UDMF::UnifiedData;
using UnifiedDataProperties = UDMF::UnifiedDataProperties;
using UDType = UDMF::UDType;
using ShareOptions = UDMF::ShareOptions;

std::shared_ptr<PasteData> ConvertUtils::Convert(const UnifiedData& unifiedData)
{
    auto pasteData = std::make_shared<PasteData>(Convert(unifiedData.GetRecords()));
    pasteData->SetProperty(ConvertProperty(unifiedData.GetProperties(), unifiedData));
    return pasteData;
}

std::shared_ptr<UnifiedData> ConvertUtils::Convert(const PasteData& pasteData)
{
    auto unifiedData = std::make_shared<UnifiedData>();
    unifiedData->SetRecords(Convert(pasteData.AllRecords()));
    unifiedData->SetProperties(ConvertProperty(pasteData.GetProperty()));
    unifiedData->SetDataId(pasteData.GetDataId());
    return unifiedData;
}

std::vector<std::shared_ptr<UnifiedRecord>> ConvertUtils::Convert(
    const std::vector<std::shared_ptr<PasteDataRecord>>& records)
{
    std::vector<std::shared_ptr<UnifiedRecord>> unifiedRecords;
    for (auto const& record : records) {
        unifiedRecords.emplace_back(Convert(record));
    }
    return unifiedRecords;
}

std::vector<std::shared_ptr<PasteDataRecord>> ConvertUtils::Convert(
    const std::vector<std::shared_ptr<UnifiedRecord>>& records)
{
    std::vector<std::shared_ptr<PasteDataRecord>> pasteboardRecords;
    for (auto const& record : records) {
        pasteboardRecords.emplace_back(Convert(record));
    }
    return pasteboardRecords;
}

std::shared_ptr<UnifiedRecord> ConvertUtils::Convert(std::shared_ptr<PasteDataRecord> record)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        record != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "paste record is nullptr");
    std::shared_ptr<UnifiedRecord> udmfRecord = std::make_shared<UnifiedRecord>();
    auto entries = Convert(record->GetEntries(), record);
    for (auto& udmfEntry : *entries) {
        udmfRecord->AddEntry(udmfEntry.first, std::move(udmfEntry.second));
    }
    udmfRecord->SetChannelName(CHANNEL_NAME);
    udmfRecord->SetDataId(record->GetDataId());
    udmfRecord->SetRecordId(record->GetRecordId());
    return udmfRecord;
}

std::shared_ptr<PasteDataRecord> ConvertUtils::Convert(std::shared_ptr<UnifiedRecord> record)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(record != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "udmfRecord is nullptr");
    std::shared_ptr<PasteDataRecord> pbRecord = std::make_shared<PasteDataRecord>();
    auto utdId = record->GetUtdId();
    pbRecord->AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, record->GetOriginValue()));
    for (auto const& entry : Convert(record->GetEntries())) {
        if (entry == nullptr) {
            PASTEBOARD_HILOGW(PASTEBOARD_MODULE_CLIENT, "entry is empty");
            continue;
        }
        if (utdId == entry->GetUtdId()) {
            continue;
        }
        pbRecord->AddEntry(entry->GetUtdId(), entry);
    }
    pbRecord->SetDataId(record->GetDataId());
    pbRecord->SetRecordId(record->GetRecordId());
    if (record->GetEntryGetter() != nullptr) {
        pbRecord->SetDelayRecordFlag(true);
    }
    return pbRecord;
}

std::vector<std::shared_ptr<PasteDataEntry>> ConvertUtils::Convert(
    const std::shared_ptr<std::map<std::string, UDMF::ValueType>>& entries)
{
    std::vector<std::shared_ptr<PasteDataEntry>> pbEntries;
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(entries != nullptr, pbEntries, PASTEBOARD_MODULE_CLIENT, "pbEntries is empty");
    for (auto const& [utdId, value] : *entries) {
        pbEntries.emplace_back(std::make_shared<PasteDataEntry>(utdId, value));
    }
    return pbEntries;
}

UDMF::ValueType ConvertUtils::Convert(
    const std::shared_ptr<PasteDataEntry>& entry, std::shared_ptr<PasteDataRecord> record)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        entry != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "entry is null, convert failed.");
    auto utdId = entry->GetUtdId();
    auto value = entry->GetValue();
    if (std::holds_alternative<std::monostate>(value) || std::holds_alternative<std::shared_ptr<Object>>(value)) {
        if (std::holds_alternative<std::shared_ptr<Object>>(value) && CommonUtils::IsFileUri(utdId) &&
            record->GetUriV0() != nullptr) {
            auto object = std::get<std::shared_ptr<Object>>(value);
            object->value_[UDMF::FILE_URI_PARAM] = record->GetUriV0()->ToString();
        }
        return value;
    }
    auto mimeType = entry->GetMimeType();
    auto object = std::make_shared<UDMF::Object>();
    if (mimeType == MIMETYPE_TEXT_PLAIN) {
        object->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
        if (std::holds_alternative<std::string>(value)) {
            object->value_[UDMF::CONTENT] = std::get<std::string>(value);
        }
    } else if (mimeType == MIMETYPE_TEXT_HTML) {
        object->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
        if (std::holds_alternative<std::string>(value)) {
            object->value_[UDMF::HTML_CONTENT] = std::get<std::string>(value);
        }
    } else if (mimeType == MIMETYPE_TEXT_URI) {
        object->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
        if (std::holds_alternative<std::string>(value) && record->GetUriV0() != nullptr) {
            object->value_[UDMF::FILE_URI_PARAM] = record->GetUriV0()->ToString();
        }
    } else if (mimeType == MIMETYPE_PIXELMAP) {
        object->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
        if (std::holds_alternative<std::shared_ptr<OHOS::Media::PixelMap>>(value)) {
            object->value_[UDMF::PIXEL_MAP] = std::get<std::shared_ptr<OHOS::Media::PixelMap>>(value);
        }
    } else if (mimeType == MIMETYPE_TEXT_WANT) {
        PASTEBOARD_HILOGW(PASTEBOARD_MODULE_CLIENT, "mimeType is want, udmf not support");
    } else {
        object->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
        if (std::holds_alternative<std::vector<uint8_t>>(value)) {
            auto arrayBuffer = std::get<std::vector<uint8_t>>(value);
            object->value_[UDMF::ARRAY_BUFFER] = arrayBuffer;
            object->value_[UDMF::ARRAY_BUFFER_LENGTH] = static_cast<int64_t>(arrayBuffer.size());
        }
    }
    return object;
}

std::shared_ptr<std::vector<std::pair<std::string, UDMF::ValueType>>> ConvertUtils::Convert(
    const std::vector<std::shared_ptr<PasteDataEntry>>& entries, std::shared_ptr<PasteDataRecord> record)
{
    std::map<std::string, UDMF::ValueType> udmfEntryMap;
    std::vector<std::pair<std::string, UDMF::ValueType>> udmfEntries;
    std::vector<std::string> entryUtdIds;
    for (auto const& entry : entries) {
        if (entry == nullptr) {
            continue;
        }
        if (udmfEntryMap.find(entry->GetUtdId()) == udmfEntryMap.end()) {
            entryUtdIds.emplace_back(entry->GetUtdId());
        }
        auto udmfEntry = Convert(entry, record);
        if (std::holds_alternative<nullptr_t>(udmfEntry)) {
            continue;
        }
        udmfEntryMap.insert_or_assign(entry->GetUtdId(), udmfEntry);
    }
    for (auto const& utdId : entryUtdIds) {
        auto item = udmfEntryMap.find(utdId);
        if (item != udmfEntryMap.end()) {
            udmfEntries.emplace_back(std::pair<std::string, UDMF::ValueType>(item->first, item->second));
        }
    }
    return std::make_shared<std::vector<std::pair<std::string, UDMF::ValueType>>>(udmfEntries);
}

ShareOption ConvertUtils::UdmfOptions2PbOption(ShareOptions udmfOptions)
{
    ShareOption pbOption = CrossDevice;
    switch (udmfOptions) {
        case UDMF::IN_APP:
            pbOption = InApp;
            break;
        case UDMF::CROSS_APP:
            pbOption = LocalDevice;
            break;
        case UDMF::CROSS_DEVICE:
            pbOption = CrossDevice;
            break;
        default:
            break;
    }
    return pbOption;
}

ShareOptions ConvertUtils::PbOption2UdmfOptions(ShareOption pbOption)
{
    ShareOptions udmfOptions = UDMF::CROSS_DEVICE;
    switch (pbOption) {
        case InApp:
            udmfOptions = UDMF::IN_APP;
            break;
        case LocalDevice:
            udmfOptions = UDMF::CROSS_APP;
            break;
        case CrossDevice:
            udmfOptions = UDMF::CROSS_DEVICE;
            break;
        default:
            break;
    }
    return udmfOptions;
}

PasteDataProperty ConvertUtils::ConvertProperty(
    const std::shared_ptr<UnifiedDataProperties>& properties, const UnifiedData& unifiedData)
{
    if (!properties) {
        return {};
    }
    PasteDataProperty pasteDataProperty;
    pasteDataProperty.shareOption = UdmfOptions2PbOption(properties->shareOptions);
    pasteDataProperty.additions = properties->extras;
    pasteDataProperty.timestamp = properties->timestamp;
    pasteDataProperty.tag = properties->tag;
    auto utdIds = unifiedData.GetTypesLabels();
    pasteDataProperty.mimeTypes = Convert(utdIds);
    pasteDataProperty.isRemote = properties->isRemote;
    return PasteDataProperty(pasteDataProperty);
}

std::shared_ptr<UnifiedDataProperties> ConvertUtils::ConvertProperty(const PasteDataProperty& properties)
{
    auto unifiedDataProperties = std::make_shared<UnifiedDataProperties>();
    unifiedDataProperties->shareOptions = PbOption2UdmfOptions(properties.shareOption);
    unifiedDataProperties->extras = properties.additions;
    unifiedDataProperties->timestamp = properties.timestamp;
    unifiedDataProperties->tag = properties.tag;
    unifiedDataProperties->isRemote = properties.isRemote;
    return unifiedDataProperties;
}

std::vector<std::string> ConvertUtils::Convert(const std::vector<std::string>& utdIds)
{
    std::vector<std::string> types;
    for (const auto& utdId : utdIds) {
        types.push_back(CommonUtils::Convert2MimeType(utdId));
    }
    return types;
}
} // namespace MiscServices
} // namespace OHOS