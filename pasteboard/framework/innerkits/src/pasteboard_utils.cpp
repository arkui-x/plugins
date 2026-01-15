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

#include "pasteboard_utils.h"

#include <unordered_set>

#include "application_defined_record.h"
#include "audio.h"
#include "folder.h"
#include "html.h"
#include "image.h"
#include "link.h"
#include "pasteboard_hilog.h"
#include "plain_text.h"
#include "system_defined_appitem.h"
#include "system_defined_form.h"
#include "system_defined_pixelmap.h"
#include "video.h"

namespace OHOS {
namespace MiscServices {
using UnifiedRecord = UDMF::UnifiedRecord;
using UnifiedData = UDMF::UnifiedData;
using UnifiedDataProperties = UDMF::UnifiedDataProperties;
using UDType = UDMF::UDType;
using ShareOptions = UDMF::ShareOptions;

std::shared_ptr<PasteData> PasteboardUtils::Convert(const UnifiedData& unifiedData)
{
    auto unifiedRecords = unifiedData.GetRecords();
    auto pasteData = std::make_shared<PasteData>(Convert(unifiedRecords));
    auto unifiedDataProperties = unifiedData.GetProperties();
    auto properties = Convert(*unifiedDataProperties);
    auto recordTypes = unifiedData.GetTypesLabels();
    properties.mimeTypes = Convert(recordTypes);
    pasteData->SetProperty(properties);
    return pasteData;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::ConvertToUnifiedRecord(const std::shared_ptr<PasteDataRecord>& record)
{
    auto type = Convert(record->GetUDType(), record->GetMimeType());
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "GCA to unified type is :%d", type);
    switch (type) {
        case UDMF::TEXT:
            return PasteRecord2Text(record);
        case UDMF::PLAIN_TEXT:
            return PasteRecord2PlaintText(record);
        case UDMF::OPENHARMONY_WANT:
            return PasteRecord2Want(record);
        case UDMF::HTML:
            return PasteRecord2Html(record);
        case UDMF::HYPERLINK:
            return PasteRecord2Link(record);
        case UDMF::FILE:
        case UDMF::FILE_URI:
            return PasteRecord2File(record);
        case UDMF::IMAGE:
            return PasteRecord2Image(record);
        case UDMF::VIDEO:
            return PasteRecord2Video(record);
        case UDMF::AUDIO:
            return PasteRecord2Audio(record);
        case UDMF::FOLDER:
            return PasteRecord2Folder(record);
        case UDMF::SYSTEM_DEFINED_PIXEL_MAP:
            return PasteRecord2PixelMap(record);
        case UDMF::SYSTEM_DEFINED_RECORD:
            return PasteRecord2SystemDefined(record);
        case UDMF::SYSTEM_DEFINED_FORM:
            return PasteRecord2Form(record);
        case UDMF::SYSTEM_DEFINED_APP_ITEM:
            return PasteRecord2AppItem(record);
        default:
            return nullptr;
    }
}

std::shared_ptr<UnifiedData> PasteboardUtils::Convert(const PasteData& pasteData)
{
    auto unifiedData = std::make_shared<UnifiedData>();
    for (std::size_t i = 0; i < pasteData.GetRecordCount(); ++i) {
        auto pasteboardRecord = pasteData.GetRecordAt(i);
        if (pasteboardRecord == nullptr) {
            continue;
        }
        std::shared_ptr<UnifiedRecord> convertedRecord = ConvertToUnifiedRecord(pasteboardRecord);
        if (convertedRecord) {
            unifiedData->AddRecord(convertedRecord);
        } else {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "not find type, go to customData");
            unifiedData->AddRecords(Custom2AppDefined(pasteboardRecord));
        }
    }
    auto pastedProp = pasteData.GetProperty();
    auto unifiedProp = Convert(pastedProp);
    unifiedData->SetProperties(unifiedProp);
    return unifiedData;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::ConvertToPasteDataRecord(const std::shared_ptr<UnifiedRecord>& record)
{
    auto type = record->GetType();
    PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "GCA to pasteData type is :%d", type);
    switch (type) {
        case UDMF::TEXT:
            return Text2PasteRecord(record);
        case UDMF::PLAIN_TEXT:
            return PlainText2PasteRecord(record);
        case UDMF::OPENHARMONY_WANT:
            return Want2PasteRecord(record);
        case UDMF::HTML:
            return Html2PasteRecord(record);
        case UDMF::HYPERLINK:
            return Link2PasteRecord(record);
        case UDMF::FILE:
        case UDMF::FILE_URI:
            return File2PasteRecord(record);
        case UDMF::IMAGE:
            return Image2PasteRecord(record);
        case UDMF::VIDEO:
            return Video2PasteRecord(record);
        case UDMF::AUDIO:
            return Audio2PasteRecord(record);
        case UDMF::FOLDER:
            return Folder2PasteRecord(record);
        case UDMF::SYSTEM_DEFINED_PIXEL_MAP:
            return PixelMap2PasteRecord(record);
        case UDMF::SYSTEM_DEFINED_RECORD:
            return SystemDefined2PasteRecord(record);
        case UDMF::SYSTEM_DEFINED_FORM:
            return Form2PasteRecord(record);
        case UDMF::SYSTEM_DEFINED_APP_ITEM:
            return AppItem2PasteRecord(record);
        case UDMF::APPLICATION_DEFINED_RECORD:
            return AppDefined2PasteRecord(record);
        default:
            return nullptr;
    }
}

std::vector<std::shared_ptr<PasteDataRecord>> PasteboardUtils::Convert(
    const std::vector<std::shared_ptr<UnifiedRecord>>& records)
{
    std::vector<std::shared_ptr<PasteDataRecord>> pasteboardRecords;
    for (const auto& record : records) {
        if (record == nullptr) {
            continue;
        }
        std::shared_ptr<PasteDataRecord> pasteboardRecord = ConvertToPasteDataRecord(record);

        if (pasteboardRecord) {
            pasteboardRecords.push_back(pasteboardRecord);
        } else {
            PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "not find type, convert to AppDefinedRecord");
            pasteboardRecords.push_back(AppDefined2PasteRecord(record));
        }
    }
    return pasteboardRecords;
}

ShareOption PasteboardUtils::UdmfOptions2PbOption(ShareOptions udmfOptions)
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

ShareOptions PasteboardUtils::PbOption2UdmfOptions(ShareOption pbOption)
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

PasteDataProperty PasteboardUtils::Convert(const UnifiedDataProperties& properties)
{
    PasteDataProperty pasteDataProperty;
    pasteDataProperty.shareOption = UdmfOptions2PbOption(properties.shareOptions);
    pasteDataProperty.additions = properties.extras;
    pasteDataProperty.timestamp = properties.timestamp;
    pasteDataProperty.tag = properties.tag;
    pasteDataProperty.isRemote = properties.isRemote;
    return PasteDataProperty(pasteDataProperty);
}

std::shared_ptr<UnifiedDataProperties> PasteboardUtils::Convert(const PasteDataProperty& properties)
{
    auto unifiedDataProperties = std::make_shared<UnifiedDataProperties>();
    unifiedDataProperties->shareOptions = PbOption2UdmfOptions(properties.shareOption);
    unifiedDataProperties->extras = properties.additions;
    unifiedDataProperties->timestamp = properties.timestamp;
    unifiedDataProperties->tag = properties.tag;
    unifiedDataProperties->isRemote = properties.isRemote;
    return unifiedDataProperties;
}

std::vector<std::string> PasteboardUtils::Convert(const std::vector<std::string>& utdIds)
{
    std::vector<std::string> types;
    for (const auto& utdId : utdIds) {
        types.push_back(CommonUtils::Convert2MimeType(utdId));
    }
    return types;
}

std::string PasteboardUtils::Convert(UDType uDType)
{
    switch (uDType) {
        case UDType::PLAIN_TEXT:
            // fall-through
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
}

UDType PasteboardUtils::Convert(int32_t uDType, const std::string& mimeType)
{
    if (uDType != UDMF::UD_BUTT) {
        return static_cast<UDType>(uDType);
    }
    if (mimeType == MIMETYPE_TEXT_URI) {
        return UDMF::FILE;
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
    return UDMF::UD_BUTT;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::PlainText2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto plainText = static_cast<UDMF::PlainText*>(record.get());
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        plainText != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get PLAIN_TEXT record failed.");
    auto pbRecord = std::make_shared<PasteDataRecord>();
    auto utdId = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::PLAIN_TEXT);
    auto value = record->GetOriginValue();
    if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
        pbRecord->AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, value));
        return pbRecord;
    }
    auto object = std::make_shared<Object>();
    object->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
    object->value_[UDMF::CONTENT] = plainText->GetContent();
    object->value_[UDMF::ABSTRACT] = plainText->GetAbstract();
    pbRecord->AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, object));
    pbRecord->SetDetails(plainText->GetDetails());
    pbRecord->SetUDType(UDMF::PLAIN_TEXT);
    return pbRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2PlaintText(const std::shared_ptr<PasteDataRecord> record)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        record != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "plain text record is null.");
    auto entry = record->GetEntryByMimeType(MIMETYPE_TEXT_PLAIN);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(entry != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "entry is null.");
    auto plainText = std::make_shared<UDMF::PlainText>(UDMF::PLAIN_TEXT, entry->GetValue());
    if (record->GetDetails()) {
        plainText->SetDetails(*record->GetDetails());
    }
    return plainText;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Want2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto want = static_cast<UDMF::UnifiedRecord*>(record.get());
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(want != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get want record failed.");
    auto recordValue = want->GetValue();
    auto wantValue = std::get_if<std::shared_ptr<OHOS::AAFwk::Want>>(&recordValue);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        wantValue != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get want from unified record failed.");
    auto wantRecord = PasteDataRecord::NewWantRecord(*(wantValue));
    wantRecord->SetUDType(UDMF::OPENHARMONY_WANT);
    return wantRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Want(const std::shared_ptr<PasteDataRecord> record)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        record != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "record2 want record is null.");
    auto wantRecord = record->GetWant();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        wantRecord != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get want from paste record failed.");
    return std::make_shared<UDMF::UnifiedRecord>(UDMF::OPENHARMONY_WANT, wantRecord);
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Html2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto html = static_cast<UDMF::Html*>(record.get());
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(html != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get HTML record failed.");
    auto pbRecord = std::make_shared<PasteDataRecord>();
    auto utdId = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::HTML);
    auto value = record->GetOriginValue();
    if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
        pbRecord->AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, value));
        return pbRecord;
    }
    auto object = std::make_shared<Object>();
    object->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
    object->value_[UDMF::HTML_CONTENT] = html->GetHtmlContent();
    object->value_[UDMF::PLAIN_CONTENT] = html->GetPlainContent();
    pbRecord->AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, object));
    pbRecord->SetDetails(html->GetDetails());
    pbRecord->SetUDType(UDMF::HTML);
    return pbRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Html(const std::shared_ptr<PasteDataRecord> record)
{
#ifdef ANDROID_PLATFORM
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        record != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "record2 html record is null.");
    auto htmlTextPtr = record->GetHtmlText();
    auto plainTextPtr = record->GetPlainText();
    std::string htmlContent = htmlTextPtr ? *htmlTextPtr : "";
    std::string plainContent = plainTextPtr ? *plainTextPtr : "";

    auto utdId = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::HTML);
    auto object = std::make_shared<Object>();
    object->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
    object->value_[UDMF::HTML_CONTENT] = htmlContent;
    object->value_[UDMF::PLAIN_CONTENT] = plainContent;
    auto html = std::make_shared<UDMF::Html>(UDMF::HTML, object);
    if (record->GetDetails()) {
        html->SetDetails(*record->GetDetails());
    }
    return html;
#else
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        record != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "record2 html record is null.");
    auto entry = record->GetEntryByMimeType(MIMETYPE_TEXT_HTML);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(entry != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "entry is null.");
    auto html = std::make_shared<UDMF::Html>(UDMF::HTML, entry->GetValue());
    if (record->GetDetails()) {
        html->SetDetails(*record->GetDetails());
    }
    return html;
#endif
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Link2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto link = static_cast<UDMF::Link*>(record.get());
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(link != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get Link record failed.");
    auto pbRecord = std::make_shared<PasteDataRecord>();
    auto utdId = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::HYPERLINK);
    auto value = record->GetOriginValue();
    if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
        pbRecord->AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, value));
        return pbRecord;
    }
    auto object = std::make_shared<Object>();
    object->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
    object->value_[UDMF::URL] = link->GetUrl();
    object->value_[UDMF::DESCRIPTION] = link->GetDescription();
    pbRecord->AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, object));
    pbRecord->SetDetails(link->GetDetails());
    pbRecord->SetUDType(UDMF::HYPERLINK);
    return pbRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Link(const std::shared_ptr<PasteDataRecord> record)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        record != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "record2 link record is null.");
    auto utdId = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::HYPERLINK);
    auto entry = record->GetEntry(utdId);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(entry != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "entry is null.");
    auto link = std::make_shared<UDMF::Link>(UDMF::HYPERLINK, entry->GetValue());
    if (record->GetDetails()) {
        link->SetDetails(*record->GetDetails());
    }
    return link;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::File2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto file = static_cast<UDMF::File*>(record.get());
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(file != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get file record failed.");
    auto uriRecord = PasteDataRecord::NewUriRecord(OHOS::Uri(file->GetUri()));
    uriRecord->SetDetails(file->GetDetails());
    uriRecord->SetUDType(UDMF::FILE);
    return uriRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2File(const std::shared_ptr<PasteDataRecord> record)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        record != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "record2 file record is null.");
    auto uri = record->GetUriV0();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        uri != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get uri from paste record to file failed.");
    auto unifiedRecord = std::make_shared<UDMF::File>(uri->ToString());
    auto details = record->GetDetails();
    if (details != nullptr) {
        unifiedRecord->SetDetails(*details);
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Image2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto image = static_cast<UDMF::Image*>(record.get());
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        image != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get image record failed.");
    auto uriRecord = PasteDataRecord::NewUriRecord(OHOS::Uri(image->GetUri()));
    uriRecord->SetDetails(image->GetDetails());
    uriRecord->SetUDType(UDMF::IMAGE);
    return uriRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Image(const std::shared_ptr<PasteDataRecord> record)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        record != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "record2 iamge record is null.");
    auto uri = record->GetUriV0();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        uri != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get uri from paste record to image failed.");
    auto unifiedRecord = std::make_shared<UDMF::Image>(uri->ToString());
    auto details = record->GetDetails();
    if (details != nullptr) {
        unifiedRecord->SetDetails(*details);
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Video2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto video = static_cast<UDMF::Video*>(record.get());
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        video != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get video record failed.");
    auto uriRecord = PasteDataRecord::NewUriRecord(OHOS::Uri(video->GetUri()));
    uriRecord->SetDetails(video->GetDetails());
    uriRecord->SetUDType(UDMF::VIDEO);
    return uriRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Video(const std::shared_ptr<PasteDataRecord> record)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        record != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "record2 video record is null.");
    auto uri = record->GetUriV0();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        uri != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get uri from paste record to video failed.");
    auto unifiedRecord = std::make_shared<UDMF::Video>(uri->ToString());
    auto details = record->GetDetails();
    if (details != nullptr) {
        unifiedRecord->SetDetails(*details);
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Audio2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto audio = static_cast<UDMF::Audio*>(record.get());
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        audio != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get audio record failed.");
    auto uriRecord = PasteDataRecord::NewUriRecord(OHOS::Uri(audio->GetUri()));
    uriRecord->SetDetails(audio->GetDetails());
    uriRecord->SetUDType(UDMF::AUDIO);
    return uriRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Audio(const std::shared_ptr<PasteDataRecord> record)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        record != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "record2 audio record is null.");
    auto uri = record->GetUriV0();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        uri != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get uri from paste record to audio failed.");
    auto unifiedRecord = std::make_shared<UDMF::Audio>(uri->ToString());
    auto details = record->GetDetails();
    if (details != nullptr) {
        unifiedRecord->SetDetails(*details);
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Folder2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto folder = static_cast<UDMF::Folder*>(record.get());
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        folder != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get folder record failed.");
    auto uriRecord = PasteDataRecord::NewUriRecord(OHOS::Uri(folder->GetUri()));
    uriRecord->SetDetails(folder->GetDetails());
    uriRecord->SetUDType(UDMF::FOLDER);
    return uriRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Folder(const std::shared_ptr<PasteDataRecord> record)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        record != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "record2 folder record is null.");
    auto uri = record->GetUriV0();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        uri != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get uri from paste record to folder failed.");
    auto unifiedRecord = std::make_shared<UDMF::Folder>(uri->ToString());
    auto details = record->GetDetails();
    if (details != nullptr) {
        unifiedRecord->SetDetails(*details);
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::PixelMap2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto pixelMap = static_cast<UDMF::SystemDefinedPixelMap*>(record.get());
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        pixelMap != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get pixelMap record failed.");
    auto recordValue = pixelMap->GetValue();
    auto pixelMapValue = std::get_if<std::shared_ptr<Media::PixelMap>>(&recordValue);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        pixelMapValue != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get pixelMap from unified record failed.");
    auto pixelMapRecord = PasteDataRecord::NewPixelMapRecord(*(pixelMapValue));
    pixelMapRecord->SetUDType(UDMF::SYSTEM_DEFINED_PIXEL_MAP);
    return pixelMapRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2PixelMap(const std::shared_ptr<PasteDataRecord> record)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        record != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "pixel map record is null.");
    auto pixelMapRecord = record->GetPixelMapV0();
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        pixelMapRecord != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get pixelMap from paste record failed.");
    return std::make_shared<UDMF::UnifiedRecord>(UDMF::SYSTEM_DEFINED_PIXEL_MAP, pixelMapRecord);
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::AppItem2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto appItem = static_cast<UDMF::SystemDefinedAppItem*>(record.get());
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        appItem != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get SystemDefined record failed.");
    auto pbRecord = std::make_shared<PasteDataRecord>();
    auto utdId = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::SYSTEM_DEFINED_APP_ITEM);
    auto value = record->GetOriginValue();
    if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
        pbRecord->AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, value));
        return pbRecord;
    }
    auto object = std::make_shared<Object>();
    object->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
    object->value_[UDMF::APP_ID] = appItem->GetAppId();
    object->value_[UDMF::APP_NAME] = appItem->GetAppName();
    object->value_[UDMF::APP_ICON_ID] = appItem->GetAppIconId();
    object->value_[UDMF::APP_LABEL_ID] = appItem->GetAppLabelId();
    object->value_[UDMF::BUNDLE_NAME] = appItem->GetBundleName();
    object->value_[UDMF::ABILITY_NAME] = appItem->GetAbilityName();
    pbRecord->AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, object));
    pbRecord->SetDetails(appItem->GetDetails());
    pbRecord->SetUDType(UDMF::SYSTEM_DEFINED_APP_ITEM);
    return pbRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2AppItem(const std::shared_ptr<PasteDataRecord> record)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        record != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "app Item record is null.");
    auto utdId = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDType::SYSTEM_DEFINED_APP_ITEM);
    auto entry = record->GetEntry(utdId);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(entry != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "entry is null.");
    auto unifiedRecord = std::make_shared<UDMF::SystemDefinedAppItem>(UDMF::SYSTEM_DEFINED_APP_ITEM, entry->GetValue());
    if (record->GetDetails() != nullptr) {
        unifiedRecord->SetDetails(*record->GetDetails());
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Form2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto form = static_cast<UDMF::SystemDefinedForm*>(record.get());
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        form != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "Form2 Paste Record is null.");
    std::vector<uint8_t> arrayBuffer;
    auto kvRecord = PasteDataRecord::NewKvRecord(Convert(UDType::SYSTEM_DEFINED_FORM), arrayBuffer);
    kvRecord->SetDetails(form->GetDetails());
    kvRecord->SetSystemDefinedContent(form->GetItems());
    kvRecord->SetUDType(UDType::SYSTEM_DEFINED_FORM);
    return kvRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Form(const std::shared_ptr<PasteDataRecord> record)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        record != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "record2 from record is null.");
    auto unifiedRecord = std::make_shared<UDMF::SystemDefinedForm>();
    if (record->GetSystemDefinedContent() != nullptr) {
        unifiedRecord->SetItems(*record->GetSystemDefinedContent());
    }
    if (record->GetDetails() != nullptr) {
        unifiedRecord->SetDetails(*record->GetDetails());
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::SystemDefined2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto systemDefined = static_cast<UDMF::SystemDefinedRecord*>(record.get());
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        systemDefined != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get systemRecord record failed.");
    std::vector<uint8_t> arrayBuffer;
    auto kvRecord = PasteDataRecord::NewKvRecord(Convert(UDType::SYSTEM_DEFINED_RECORD), arrayBuffer);
    kvRecord->SetDetails(systemDefined->GetDetails());
    kvRecord->SetUDType(UDType::SYSTEM_DEFINED_RECORD);
    return kvRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2SystemDefined(const std::shared_ptr<PasteDataRecord> record)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        record != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "system fined record is null.");
    auto unifiedRecord = std::make_shared<UDMF::SystemDefinedRecord>();
    if (record->GetDetails() != nullptr) {
        unifiedRecord->SetDetails(*record->GetDetails());
    }
    return unifiedRecord;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::Text2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto text = static_cast<UDMF::Text*>(record.get());
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(text != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get TEXT record failed.");
    std::vector<uint8_t> arrayBuffer;
    std::string type = UDMF::UtdUtils::GetUtdIdFromUtdEnum(UDMF::TEXT);
    auto kvRecord = PasteDataRecord::NewKvRecord(type, arrayBuffer);
    kvRecord->SetUDType(UDMF::TEXT);
    kvRecord->SetDetails(text->GetDetails());
    return kvRecord;
}

std::shared_ptr<UnifiedRecord> PasteboardUtils::PasteRecord2Text(std::shared_ptr<PasteDataRecord> record)
{
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        record != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "cord2 text record is null.");
    auto unifiedRecord = std::make_shared<UDMF::Text>();
    if (record->GetDetails() != nullptr) {
        unifiedRecord->SetDetails(*record->GetDetails());
    }
    return unifiedRecord;
}

std::vector<std::shared_ptr<UnifiedRecord>> PasteboardUtils::Custom2AppDefined(
    const std::shared_ptr<PasteDataRecord> record)
{
    std::vector<std::shared_ptr<UnifiedRecord>> unifiedRecords;
    if (record == nullptr) {
        return unifiedRecords;
    }
    if (record->GetCustomData() == nullptr) {
        return unifiedRecords;
    }
    auto customData = record->GetCustomData();
    if (customData == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "customData is null");
        return unifiedRecords;
    }
    for (auto& [type, rawData] : customData->GetItemData()) {
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "app defied type:%{public}s.", type.c_str());
        unifiedRecords.push_back(std::make_shared<UDMF::ApplicationDefinedRecord>(type, rawData));
    }
    return unifiedRecords;
}

std::shared_ptr<PasteDataRecord> PasteboardUtils::AppDefined2PasteRecord(const std::shared_ptr<UnifiedRecord> record)
{
    auto appRecord = static_cast<UDMF::ApplicationDefinedRecord*>(record.get());
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(
        appRecord != nullptr, nullptr, PASTEBOARD_MODULE_CLIENT, "get ApplicationDefinedRecord record failed.");
    auto utdId = appRecord->GetApplicationDefinedType();
    auto pbRecord = std::make_shared<PasteDataRecord>();
    auto value = record->GetOriginValue();
    if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
        pbRecord->AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, value));
        return pbRecord;
    }
    auto object = std::make_shared<Object>();
    object->value_[UDMF::UNIFORM_DATA_TYPE] = utdId;
    object->value_[UDMF::ARRAY_BUFFER] = appRecord->GetRawData();
    object->value_[UDMF::ARRAY_BUFFER_LENGTH] = static_cast<int64_t>(appRecord->GetRawData().size());
    pbRecord->AddEntry(utdId, std::make_shared<PasteDataEntry>(utdId, object));
    pbRecord->SetUDType(UDMF::APPLICATION_DEFINED_RECORD);
    return pbRecord;
}

std::vector<std::string> PasteboardUtils::DeduplicateVector(const std::vector<std::string>& vec)
{
    std::unordered_set<std::string> tmp(vec.begin(), vec.end());
    std::vector<std::string> result(tmp.begin(), tmp.end());
    return result;
}

PasteboardUtils::PasteboardUtils() {}

PasteboardUtils& PasteboardUtils::GetInstance()
{
    static PasteboardUtils instance;
    return instance;
}
} // namespace MiscServices
} // namespace OHOS