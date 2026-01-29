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

#include "paste_data.h"

#include "int_wrapper.h"
#include "long_wrapper.h"
#include "pasteboard_hilog.h"
#include "want_params_wrapper.h"

using namespace std::chrono;
using namespace OHOS::Media;

namespace OHOS {
namespace MiscServices {
enum TAG_PASTEBOARD : uint16_t {
    TAG_PROPS = TAG_BUFF + 1,
    TAG_RECORDS,
    TAG_DRAGGED_DATA_FLAG,
    TAG_LOCAL_PASTE_FLAG,
    TAG_DELAY_DATA_FLAG,
    TAG_DEVICE_ID,
    TAG_PASTE_ID,
    TAG_DELAY_RECORD_FLAG,
    TAG_DATA_ID,
    TAG_RECORD_ID,
    TAG_USER_ID,
};
enum TAG_PROPERTY : uint16_t {
    TAG_ADDITIONS = TAG_BUFF + 1,
    TAG_MIMETYPES,
    TAG_TAG,
    TAG_LOCAL_ONLY,
    TAG_TIMESTAMP,
    TAG_SHAREOPTION,
    TAG_TOKENID,
    TAG_ISREMOTE,
    TAG_BUNDLENAME,
    TAG_SETTIME,
    TAG_SCREEN_STATUS,
};

std::string PasteData::WEBVIEW_PASTEDATA_TAG = "WebviewPasteDataTag";
const char* REMOTE_FILE_SIZE = "remoteFileSize";
const char* REMOTE_FILE_SIZE_LONG = "remoteFileSizeLong";
const char* ORIGIN_TOKEN_ID = "originTokenId";
const char* ORIGIN_INFO = "originInfo";
constexpr int32_t SUB_PASTEID_NUM = 3;
constexpr int32_t PASTEID_MAX_SIZE = 1024;
constexpr int32_t PARCEL_MAX_CAPACITY = 500 * 1024;

PasteData::PasteData()
{ // LCOV_EXCL_START
    props_.timestamp = steady_clock::now().time_since_epoch().count();
    props_.localOnly = false;
    props_.shareOption = ShareOption::CrossDevice;
} // LCOV_EXCL_STOP

PasteData::~PasteData() {}

PasteData::PasteData(const PasteData& data)
    : rawDataSize_(data.rawDataSize_), deviceId_(data.deviceId_), userId_(data.userId_), valid_(data.valid_),
      isDraggedData_(data.isDraggedData_), isLocalPaste_(data.isLocalPaste_), isDelayData_(data.isDelayData_),
      isDelayRecord_(data.isDelayRecord_), dataId_(data.dataId_), recordId_(data.recordId_), textSize_(data.textSize_),
      originAuthority_(data.originAuthority_), pasteId_(data.pasteId_)
{ // LCOV_EXCL_START
    this->props_ = data.props_;
    for (const auto& item : data.records_) {
        if (item == nullptr) {
            continue;
        }
        this->records_.emplace_back(std::make_shared<PasteDataRecord>(*item));
    }
} // LCOV_EXCL_STOP

PasteData::PasteData(std::vector<std::shared_ptr<PasteDataRecord>> records) : records_ { std::move(records) }
{ // LCOV_EXCL_START
    for (const auto& item : records_) {
        PASTEBOARD_CHECK_AND_RETURN_LOGE(item != nullptr, PASTEBOARD_MODULE_CLIENT, "record is null");
        item->SetRecordId(++recordId_);
    }
    props_.timestamp = steady_clock::now().time_since_epoch().count();
    props_.localOnly = false;
    props_.shareOption = ShareOption::CrossDevice;
} // LCOV_EXCL_STOP

PasteData& PasteData::operator=(const PasteData& data)
{ // LCOV_EXCL_START
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "PasteData copy");
    if (this == &data) {
        return *this;
    }
    this->originAuthority_ = data.originAuthority_;
    this->valid_ = data.valid_;
    this->isDraggedData_ = data.isDraggedData_;
    this->isLocalPaste_ = data.isLocalPaste_;
    this->isDelayData_ = data.isDelayData_;
    this->isDelayRecord_ = data.isDelayRecord_;
    this->dataId_ = data.dataId_;
    this->props_ = data.props_;
    this->records_.clear();
    this->deviceId_ = data.deviceId_;
    this->pasteId_ = data.pasteId_;
    for (const auto& item : data.records_) {
        if (item == nullptr) {
            continue;
        }
        this->records_.emplace_back(std::make_shared<PasteDataRecord>(*item));
    }
    this->recordId_ = data.GetRecordId();
    this->textSize_ = data.textSize_;
    this->rawDataSize_ = data.rawDataSize_;
    this->userId_ = data.userId_;
    return *this;
} // LCOV_EXCL_STOP

PasteDataProperty PasteData::GetProperty() const
{ // LCOV_EXCL_START
    return PasteDataProperty(props_);
} // LCOV_EXCL_STOP

void PasteData::SetProperty(const PasteDataProperty& property)
{ // LCOV_EXCL_START
    this->props_ = property;
} // LCOV_EXCL_STOP

void PasteData::AddHtmlRecord(const std::string& html)
{ // LCOV_EXCL_START
    this->AddRecord(PasteDataRecord::NewHtmlRecord(html));
} // LCOV_EXCL_STOP

void PasteData::AddPixelMapRecord(std::shared_ptr<PixelMap> pixelMap)
{ // LCOV_EXCL_START
    this->AddRecord(PasteDataRecord::NewPixelMapRecord(std::move(pixelMap)));
} // LCOV_EXCL_STOP

void PasteData::AddWantRecord(std::shared_ptr<OHOS::AAFwk::Want> want)
{ // LCOV_EXCL_START
    this->AddRecord(PasteDataRecord::NewWantRecord(std::move(want)));
} // LCOV_EXCL_STOP

void PasteData::AddTextRecord(const std::string& text)
{ // LCOV_EXCL_START
    this->AddRecord(PasteDataRecord::NewPlainTextRecord(text));
} // LCOV_EXCL_STOP

void PasteData::AddUriRecord(const OHOS::Uri& uri)
{ // LCOV_EXCL_START
    this->AddRecord(PasteDataRecord::NewUriRecord(uri));
} // LCOV_EXCL_STOP

void PasteData::AddKvRecord(const std::string& mimeType, const std::vector<uint8_t>& arrayBuffer)
{ // LCOV_EXCL_START
    AddRecord(PasteDataRecord::NewKvRecord(mimeType, arrayBuffer));
} // LCOV_EXCL_STOP

void PasteData::AddRecord(std::shared_ptr<PasteDataRecord> record)
{ // LCOV_EXCL_START
    PASTEBOARD_CHECK_AND_RETURN_LOGE(record != nullptr, PASTEBOARD_MODULE_CLIENT, "record is null");
    record->SetRecordId(++recordId_);

    props_.mimeTypes.insert(props_.mimeTypes.begin(), record->GetMimeType());
    records_.insert(records_.begin(), std::move(record));
} // LCOV_EXCL_STOP

void PasteData::AddRecord(const PasteDataRecord& record)
{ // LCOV_EXCL_START
    this->AddRecord(std::make_shared<PasteDataRecord>(record));
} // LCOV_EXCL_STOP

std::vector<std::string> PasteData::GetMimeTypes()
{ // LCOV_EXCL_START
    std::set<std::string> mimeTypes;
    for (const auto& item : records_) {
        if (item != nullptr && item->GetFrom() > 0 && item->GetRecordId() != item->GetFrom()) {
            continue;
        }
        auto itemTypes = item->GetMimeTypes();
        mimeTypes.insert(itemTypes.begin(), itemTypes.end());
    }
    return std::vector<std::string>(mimeTypes.begin(), mimeTypes.end());
} // LCOV_EXCL_STOP

std::vector<std::string> PasteData::GetReportMimeTypes()
{ // LCOV_EXCL_START
    std::vector<std::string> mimeTypes;
    uint32_t recordNum = records_.size();
    uint32_t maxReportNum = recordNum > MAX_REPORT_RECORD_NUM ? MAX_REPORT_RECORD_NUM : recordNum;
    for (uint32_t i = 0; i < maxReportNum; ++i) {
        auto& item = records_[i];
        if (item == nullptr) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "record is nullptr.");
            mimeTypes.emplace_back("NULL");
            continue;
        }
        if (item->GetFrom() > 0 && item->GetRecordId() != item->GetFrom()) {
            mimeTypes.emplace_back("web/uri");
            continue;
        }
        auto itemTypes = item->GetMimeTypes();
        mimeTypes.insert(mimeTypes.end(), itemTypes.begin(), itemTypes.end());
    }
    return mimeTypes;
} // LCOV_EXCL_STOP

uint8_t PasteData::GenerateDataType()
{ // LCOV_EXCL_START
    std::vector<std::string> mimeTypes = GetMimeTypes();
    if (mimeTypes.empty()) {
        return 0;
    }
    std::bitset<MAX_INDEX_LENGTH> dataType(0);
    for (size_t i = 0; i < mimeTypes.size(); i++) {
        auto it = typeMap_.find(mimeTypes[i]);
        if (it == typeMap_.end()) {
            continue;
        }
        auto index = it->second;
        PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "mimetype is exist index=%{public}d", index);
        if (it->second == HTML_INDEX && GetTag() == PasteData::WEBVIEW_PASTEDATA_TAG) {
            dataType.reset();
            dataType.set(index);
            break;
        }
        dataType.set(index);
    }
    auto types = dataType.to_ulong();
    uint8_t value = types & 0xff;
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_SERVICE, "value = %{public}d", value);
    return value;
} // LCOV_EXCL_STOP

DataDescription PasteData::GetReportDescription()
{ // LCOV_EXCL_START
    DataDescription description;
    description.recordNum = records_.size();
    description.mimeTypes = GetReportMimeTypes();
    for (uint32_t i = 0; i < description.recordNum; i++) {
        auto record = GetRecordAt(i);
        if (record == nullptr) {
            PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "GetRecordAt(%{public}u) failed.", i);
            description.entryNum.push_back(-1);
            continue;
        }
        auto entries = record->GetEntries();
        description.entryNum.push_back(entries.size());
    }
    return description;
} // LCOV_EXCL_STOP

std::shared_ptr<std::string> PasteData::GetPrimaryHtml()
{ // LCOV_EXCL_START
    for (const auto& item : records_) {
        std::shared_ptr<std::string> primary = item->GetHtmlText();
        if (primary) {
            return primary;
        }
    }
    return nullptr;
} // LCOV_EXCL_STOP

std::shared_ptr<PixelMap> PasteData::GetPrimaryPixelMap()
{ // LCOV_EXCL_START
    for (const auto& item : records_) {
        std::shared_ptr<PixelMap> primary = item->GetPixelMap();
        if (primary) {
            return primary;
        }
    }
    return nullptr;
} // LCOV_EXCL_STOP

std::shared_ptr<OHOS::AAFwk::Want> PasteData::GetPrimaryWant()
{ // LCOV_EXCL_START
    for (const auto& item : records_) {
        std::shared_ptr<OHOS::AAFwk::Want> primary = item->GetWant();
        if (primary) {
            return primary;
        }
    }
    return nullptr;
} // LCOV_EXCL_STOP

std::shared_ptr<std::string> PasteData::GetPrimaryText()
{ // LCOV_EXCL_START
    for (const auto& item : records_) {
        std::shared_ptr<std::string> primary = item->GetPlainText();
        if (primary) {
            return primary;
        }
    }
    return nullptr;
} // LCOV_EXCL_STOP

std::shared_ptr<OHOS::Uri> PasteData::GetPrimaryUri()
{ // LCOV_EXCL_START
    for (const auto& item : records_) {
        std::shared_ptr<OHOS::Uri> primary = item->GetUri();
        if (primary) {
            return primary;
        }
    }
    return nullptr;
} // LCOV_EXCL_STOP

std::shared_ptr<std::string> PasteData::GetPrimaryMimeType()
{ // LCOV_EXCL_START
    if (records_.empty()) {
        return nullptr;
    }
    return std::make_shared<std::string>(records_.front()->GetMimeType());
} // LCOV_EXCL_STOP

std::shared_ptr<PasteDataRecord> PasteData::GetRecordAt(std::size_t index) const
{ // LCOV_EXCL_START
    if (records_.size() > index) {
        return records_[index];
    } else {
        return nullptr;
    }
} // LCOV_EXCL_STOP

std::shared_ptr<PasteDataRecord> PasteData::GetRecordById(uint32_t recordId) const
{ // LCOV_EXCL_START
    for (const auto& record : records_) {
        if (record != nullptr && record->GetRecordId() == recordId) {
            return record;
        }
    }
    return nullptr;
} // LCOV_EXCL_STOP

std::size_t PasteData::GetRecordCount() const
{ // LCOV_EXCL_START
    return records_.size();
} // LCOV_EXCL_STOP

ShareOption PasteData::GetShareOption()
{ // LCOV_EXCL_START
    return props_.shareOption;
} // LCOV_EXCL_STOP

void PasteData::SetShareOption(ShareOption shareOption)
{ // LCOV_EXCL_START
    props_.shareOption = shareOption;
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "shareOption = %{public}d.", shareOption);
} // LCOV_EXCL_STOP

std::uint32_t PasteData::GetTokenId()
{ // LCOV_EXCL_START
    return props_.tokenId;
} // LCOV_EXCL_STOP

int32_t PasteData::GetOriginTokenId()
{ // LCOV_EXCL_START
    auto originInfo = props_.additions.GetWantParams(ORIGIN_INFO);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGD(
        !originInfo.IsEmpty(), PasteData::INVALID_TOKEN_ID, PASTEBOARD_MODULE_SERVICE, "originInfo invalid");
    return originInfo.GetIntParam(ORIGIN_TOKEN_ID, PasteData::INVALID_TOKEN_ID);
} // LCOV_EXCL_STOP

void PasteData::SetTokenId(uint32_t tokenId)
{ // LCOV_EXCL_START
    props_.tokenId = tokenId;
} // LCOV_EXCL_STOP

void PasteData::RemoveEmptyEntry()
{ // LCOV_EXCL_START
    for (auto& record : records_) {
        if (record != nullptr) {
            record->RemoveEmptyEntry();
        }
    }
    RefreshMimeProp();
} // LCOV_EXCL_STOP

bool PasteData::RemoveRecordAt(std::size_t number)
{ // LCOV_EXCL_START
    if (records_.size() > number) {
        records_.erase(records_.begin() + static_cast<std::int64_t>(number));
        RefreshMimeProp();
        return true;
    } else {
        return false;
    }
} // LCOV_EXCL_STOP

bool PasteData::ReplaceRecordAt(std::size_t number, std::shared_ptr<PasteDataRecord> record)
{ // LCOV_EXCL_START
    if (record == nullptr) {
        return false;
    }
    if (records_.size() > number) {
        records_[number] = std::move(record);
        RefreshMimeProp();
        return true;
    } else {
        return false;
    }
} // LCOV_EXCL_STOP

bool PasteData::HasMimeType(const std::string& mimeType)
{ // LCOV_EXCL_START
    for (const auto& item : records_) {
        auto itemTypes = item->GetMimeTypes();
        if (itemTypes.find(mimeType) != itemTypes.end()) {
            return true;
        }
    }
    return false;
} // LCOV_EXCL_STOP

bool PasteData::HasUtdType(const std::string& utdType)
{ // LCOV_EXCL_START
    for (const auto& item : records_) {
        auto itemTypes = item->GetUtdTypes();
        if (itemTypes.find(utdType) != itemTypes.end()) {
            return true;
        }
    }
    return false;
} // LCOV_EXCL_STOP

std::vector<std::shared_ptr<PasteDataRecord>> PasteData::AllRecords() const
{ // LCOV_EXCL_START
    return this->records_;
} // LCOV_EXCL_STOP

bool PasteData::IsDraggedData() const
{ // LCOV_EXCL_START
    return isDraggedData_;
} // LCOV_EXCL_STOP

bool PasteData::IsLocalPaste() const
{ // LCOV_EXCL_START
    return isLocalPaste_;
} // LCOV_EXCL_STOP

void PasteData::SetDraggedDataFlag(bool isDraggedData)
{ // LCOV_EXCL_START
    isDraggedData_ = isDraggedData;
} // LCOV_EXCL_STOP

void PasteData::SetLocalPasteFlag(bool isLocalPaste)
{ // LCOV_EXCL_START
    isLocalPaste_ = isLocalPaste;
} // LCOV_EXCL_STOP

void PasteData::SetRemote(bool isRemote)
{ // LCOV_EXCL_START
    props_.isRemote = isRemote;
} // LCOV_EXCL_STOP

bool PasteData::IsRemote() const
{ // LCOV_EXCL_START
    return props_.isRemote;
} // LCOV_EXCL_STOP

void PasteData::SetBundleInfo(const std::string& bundleName, int32_t appIndex)
{ // LCOV_EXCL_START
    props_.bundleName = bundleName;
    props_.appIndex = appIndex;
} // LCOV_EXCL_STOP

std::string PasteData::GetBundleName() const
{ // LCOV_EXCL_START
    return props_.bundleName;
} // LCOV_EXCL_STOP

int32_t PasteData::GetAppIndex() const
{ // LCOV_EXCL_START
    return props_.appIndex;
} // LCOV_EXCL_STOP

void PasteData::SetOriginAuthority(const std::pair<std::string, int32_t>& bundleIndex)
{ // LCOV_EXCL_START
    originAuthority_ = bundleIndex;
} // LCOV_EXCL_STOP

std::pair<std::string, int32_t> PasteData::GetOriginAuthority() const
{ // LCOV_EXCL_START
    return originAuthority_;
} // LCOV_EXCL_STOP

void PasteData::SetTime(const std::string& setTime)
{ // LCOV_EXCL_START
    props_.setTime = setTime;
} // LCOV_EXCL_STOP

std::string PasteData::GetTime()
{ // LCOV_EXCL_START
    return props_.setTime;
} // LCOV_EXCL_STOP

void PasteData::SetScreenStatus(ScreenEvent screenStatus)
{ // LCOV_EXCL_START
    props_.screenStatus = screenStatus;
} // LCOV_EXCL_STOP

ScreenEvent PasteData::GetScreenStatus()
{ // LCOV_EXCL_START
    return props_.screenStatus;
} // LCOV_EXCL_STOP

void PasteData::SetTag(const std::string& tag)
{ // LCOV_EXCL_START
    props_.tag = tag;
} // LCOV_EXCL_STOP

std::string PasteData::GetTag()
{ // LCOV_EXCL_START
    return props_.tag;
} // LCOV_EXCL_STOP

void PasteData::SetAdditions(const AAFwk::WantParams& additions)
{ // LCOV_EXCL_START
    props_.additions = additions;
} // LCOV_EXCL_STOP

void PasteData::SetAddition(const std::string& key, AAFwk::IInterface* value)
{ // LCOV_EXCL_START
    PASTEBOARD_CHECK_AND_RETURN_LOGE(value != nullptr, PASTEBOARD_MODULE_CLIENT, "value is null");
    props_.additions.SetParam(key, value);
} // LCOV_EXCL_STOP

void PasteData::SetFileSize(int64_t fileSize)
{ // LCOV_EXCL_START
    int32_t fileIntSize = (fileSize > INT_MAX) ? INT_MAX : static_cast<int32_t>(fileSize);
    SetAddition(REMOTE_FILE_SIZE, AAFwk::Integer::Box(fileIntSize));
    SetAddition(REMOTE_FILE_SIZE_LONG, AAFwk::Long::Box(fileSize));
} // LCOV_EXCL_STOP

int64_t PasteData::GetFileSize() const
{ // LCOV_EXCL_START
    int64_t fileSize = 0L;
    auto value = props_.additions.GetParam(REMOTE_FILE_SIZE_LONG);
    AAFwk::ILong* ao = AAFwk::ILong::Query(value);
    if (ao != nullptr) {
        fileSize = AAFwk::Long::Unbox(ao);
    } else {
        fileSize = props_.additions.GetIntParam(REMOTE_FILE_SIZE, -1);
    }
    return fileSize;
} // LCOV_EXCL_STOP

void PasteData::SetLocalOnly(bool localOnly)
{ // LCOV_EXCL_START
    props_.localOnly = localOnly;
} // LCOV_EXCL_STOP

bool PasteData::GetLocalOnly()
{ // LCOV_EXCL_START
    return props_.localOnly;
} // LCOV_EXCL_STOP

void PasteData::RefreshMimeProp()
{ // LCOV_EXCL_START
    std::vector<std::string> mimeTypes;
    for (const auto& record : records_) {
        if (record == nullptr) {
            continue;
        }
        mimeTypes.emplace_back(record->GetMimeType());
    }
    props_.mimeTypes = mimeTypes;
} // LCOV_EXCL_STOP

bool PasteData::EncodeTLV(WriteOnlyBuffer& buffer) const
{
    bool ret = buffer.Write(TAG_PROPS, props_);
    ret = ret && buffer.Write(TAG_RECORDS, records_);
    ret = ret && buffer.Write(TAG_DRAGGED_DATA_FLAG, isDraggedData_);
    ret = ret && buffer.Write(TAG_LOCAL_PASTE_FLAG, isLocalPaste_);
    ret = ret && buffer.Write(TAG_DELAY_DATA_FLAG, isDelayData_);
    ret = ret && buffer.Write(TAG_DEVICE_ID, deviceId_);
    ret = ret && buffer.Write(TAG_PASTE_ID, pasteId_);
    ret = ret && buffer.Write(TAG_DELAY_RECORD_FLAG, isDelayRecord_);
    ret = ret && buffer.Write(TAG_DATA_ID, dataId_);
    ret = ret && buffer.Write(TAG_RECORD_ID, recordId_);
    ret = ret && buffer.Write(TAG_USER_ID, userId_);
    return ret;
}

bool PasteData::DecodeTLV(ReadOnlyBuffer& buffer)
{
    for (; buffer.IsEnough();) {
        TLVHead head {};
        bool ret = buffer.ReadHead(head);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON, "read head failed");
        if (head.tag == TAG_PROPS) {
            ret = buffer.ReadValue(props_, head);
        } else if (head.tag == TAG_RECORDS) {
            ret = buffer.ReadValue(records_, head);
        } else if (head.tag == TAG_DRAGGED_DATA_FLAG) {
            ret = buffer.ReadValue(isDraggedData_, head);
        } else if (head.tag == TAG_LOCAL_PASTE_FLAG) {
            ret = buffer.ReadValue(isLocalPaste_, head);
        } else if (head.tag == TAG_DELAY_DATA_FLAG) {
            ret = buffer.ReadValue(isDelayData_, head);
        } else if (head.tag == TAG_DEVICE_ID) {
            ret = buffer.ReadValue(deviceId_, head);
        } else if (head.tag == TAG_PASTE_ID) {
            ret = buffer.ReadValue(pasteId_, head);
        } else if (head.tag == TAG_DELAY_RECORD_FLAG) {
            ret = buffer.ReadValue(isDelayRecord_, head);
        } else if (head.tag == TAG_DATA_ID) {
            ret = buffer.ReadValue(dataId_, head);
        } else if (head.tag == TAG_RECORD_ID) {
            ret = buffer.ReadValue(recordId_, head);
        } else if (head.tag == TAG_USER_ID) {
            ret = buffer.ReadValue(userId_, head);
        } else {
            ret = buffer.Skip(head.len);
        }
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON,
            "read value failed, tag=%{public}hu, len=%{public}u", head.tag, head.len);
    }
    return true;
}

size_t PasteData::CountTLV() const
{
    size_t expectSize = 0;
    expectSize += TLVCountable::Count(props_);
    expectSize += TLVCountable::Count(records_);
    expectSize += TLVCountable::Count(isDraggedData_);
    expectSize += TLVCountable::Count(isLocalPaste_);
    expectSize += TLVCountable::Count(isDelayData_);
    expectSize += TLVCountable::Count(deviceId_);
    expectSize += TLVCountable::Count(pasteId_);
    expectSize += TLVCountable::Count(isDelayRecord_);
    expectSize += TLVCountable::Count(dataId_);
    expectSize += TLVCountable::Count(recordId_);
    expectSize += TLVCountable::Count(userId_);
    return expectSize;
}

bool PasteData::IsValid() const
{ // LCOV_EXCL_START
    return valid_;
} // LCOV_EXCL_STOP

void PasteData::SetInvalid()
{ // LCOV_EXCL_START
    valid_ = false;
} // LCOV_EXCL_STOP

void PasteData::SetDelayData(bool isDelay)
{ // LCOV_EXCL_START
    isDelayData_ = isDelay;
} // LCOV_EXCL_STOP

bool PasteData::IsDelayData() const
{ // LCOV_EXCL_START
    return isDelayData_;
} // LCOV_EXCL_STOP

void PasteData::SetDelayRecord(bool isDelay)
{ // LCOV_EXCL_START
    isDelayRecord_ = isDelay;
} // LCOV_EXCL_STOP

bool PasteData::IsDelayRecord() const
{ // LCOV_EXCL_START
    return isDelayRecord_;
} // LCOV_EXCL_STOP

void PasteData::SetDataId(uint32_t dataId)
{ // LCOV_EXCL_START
    dataId_ = dataId;
} // LCOV_EXCL_STOP

uint32_t PasteData::GetDataId() const
{ // LCOV_EXCL_START
    return dataId_;
} // LCOV_EXCL_STOP

uint32_t PasteData::GetRecordId() const
{ // LCOV_EXCL_START
    return recordId_;
} // LCOV_EXCL_STOP

bool PasteData::Marshalling(Parcel& parcel) const
{
    std::vector<uint8_t> pasteDataTlv(0);
    bool ret = Encode(pasteDataTlv);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON, "encode failed");

    parcel.SetMaxCapacity(PARCEL_MAX_CAPACITY);
    ret = parcel.WriteUInt8Vector(pasteDataTlv);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON, "write vector failed");
    return true;
}

PasteData* PasteData::Unmarshalling(Parcel& parcel)
{
    std::vector<uint8_t> pasteDataTlv(0);
    bool ret = parcel.ReadUInt8Vector(&pasteDataTlv);
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, nullptr, PASTEBOARD_MODULE_COMMON, "read vector failed");
    PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(!pasteDataTlv.empty(), nullptr, PASTEBOARD_MODULE_COMMON, "vector empty");

    PasteData* pasteData = new (std::nothrow) PasteData();
    if (!pasteData->Decode(pasteDataTlv)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_COMMON, "decode failed");
        delete pasteData;
        pasteData = nullptr;
    }
    return pasteData;
}

PasteDataProperty::PasteDataProperty(const PasteDataProperty& property)
    : tag(property.tag), timestamp(property.timestamp), localOnly(property.localOnly),
      shareOption(property.shareOption), tokenId(property.tokenId), isRemote(property.isRemote),
      bundleName(property.bundleName), setTime(property.setTime), screenStatus(property.screenStatus)
{ // LCOV_EXCL_START
    this->additions = property.additions;
    std::copy(property.mimeTypes.begin(), property.mimeTypes.end(), std::back_inserter(this->mimeTypes));
} // LCOV_EXCL_STOP

PasteDataProperty::~PasteDataProperty()
{ // LCOV_EXCL_START
    std::vector<std::string>().swap(mimeTypes);
} // LCOV_EXCL_STOP

PasteDataProperty& PasteDataProperty::operator=(const PasteDataProperty& property)
{ // LCOV_EXCL_START
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "PasteDataProperty copy");
    if (this == &property) {
        return *this;
    }
    this->tag = property.tag;
    this->timestamp = property.timestamp;
    this->localOnly = property.localOnly;
    this->shareOption = property.shareOption;
    this->tokenId = property.tokenId;
    this->isRemote = property.isRemote;
    this->bundleName = property.bundleName;
    this->setTime = property.setTime;
    this->screenStatus = property.screenStatus;
    this->additions = property.additions;
    std::vector<std::string>().swap(this->mimeTypes);
    std::copy(property.mimeTypes.begin(), property.mimeTypes.end(), std::back_inserter(this->mimeTypes));
    return *this;
} // LCOV_EXCL_STOP

std::string WantParamsToJson(const AAFwk::WantParams& wantParams)
{
    AAFwk::WantParamWrapper wrapper(wantParams);
    std::string wantJson = "{\"" + AAFwk::JSON_WANTPARAMS_PARAM + "\":" + wrapper.ToString() + "}";
    return wantJson;
}

bool PasteDataProperty::EncodeTLV(WriteOnlyBuffer& buffer) const
{
    std::string wantJson = WantParamsToJson(additions);
    bool ret = buffer.Write(TAG_ADDITIONS, wantJson);
    ret = ret && buffer.Write(TAG_MIMETYPES, mimeTypes);
    ret = ret && buffer.Write(TAG_TAG, tag);
    ret = ret && buffer.Write(TAG_LOCAL_ONLY, localOnly);
    ret = ret && buffer.Write(TAG_TIMESTAMP, timestamp);
    ret = ret && buffer.Write(TAG_SHAREOPTION, static_cast<int32_t>(shareOption));
    ret = ret && buffer.Write(TAG_TOKENID, tokenId);
    ret = ret && buffer.Write(TAG_ISREMOTE, isRemote);
    ret = ret && buffer.Write(TAG_BUNDLENAME, bundleName);
    ret = ret && buffer.Write(TAG_SETTIME, setTime);
    ret = ret && buffer.Write(TAG_SCREEN_STATUS, static_cast<int32_t>(screenStatus));
    return ret;
}

bool PasteDataProperty::DecodeTLV(ReadOnlyBuffer& buffer)
{
    for (; buffer.IsEnough();) {
        TLVHead head {};
        bool ret = buffer.ReadHead(head);
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON, "read head failed");
        if (head.tag == TAG_ADDITIONS) {
            std::string wantJson;
            ret = buffer.ReadValue(wantJson, head);
            AAFwk::Want want;
            want.ParseJson(wantJson);
            auto wantParams = std::static_pointer_cast<AAFwk::WantParams>(want.GetParams());
            additions = (wantParams != nullptr) ? *wantParams : additions;
        } else if (head.tag == TAG_MIMETYPES) {
            ret = buffer.ReadValue(mimeTypes, head);
        } else if (head.tag == TAG_TAG) {
            ret = buffer.ReadValue(tag, head);
        } else if (head.tag == TAG_LOCAL_ONLY) {
            ret = buffer.ReadValue(localOnly, head);
        } else if (head.tag == TAG_TIMESTAMP) {
            ret = buffer.ReadValue(timestamp, head);
        } else if (head.tag == TAG_SHAREOPTION) {
            ret = buffer.ReadValue((int32_t&)shareOption, head);
        } else if (head.tag == TAG_TOKENID) {
            ret = buffer.ReadValue(tokenId, head);
        } else if (head.tag == TAG_ISREMOTE) {
            ret = buffer.ReadValue(isRemote, head);
        } else if (head.tag == TAG_BUNDLENAME) {
            ret = buffer.ReadValue(bundleName, head);
        } else if (head.tag == TAG_SETTIME) {
            ret = buffer.ReadValue(setTime, head);
        } else if (head.tag == TAG_SCREEN_STATUS) {
            ret = buffer.ReadValue((int32_t&)screenStatus, head);
        } else {
            ret = buffer.Skip(head.len);
        }
        PASTEBOARD_CHECK_AND_RETURN_RET_LOGE(ret, false, PASTEBOARD_MODULE_COMMON,
            "read value failed, tag=%{public}hu, len=%{public}u", head.tag, head.len);
    }
    return true;
}

size_t PasteDataProperty::CountTLV() const
{
    size_t expectedSize = 0;
    expectedSize += TLVCountable::Count(WantParamsToJson(additions));
    expectedSize += TLVCountable::Count(mimeTypes);
    expectedSize += TLVCountable::Count(tag);
    expectedSize += TLVCountable::Count(localOnly);
    expectedSize += TLVCountable::Count(timestamp);
    expectedSize += TLVCountable::Count(shareOption);
    expectedSize += TLVCountable::Count(tokenId);
    expectedSize += TLVCountable::Count(isRemote);
    expectedSize += TLVCountable::Count(bundleName);
    expectedSize += TLVCountable::Count(setTime);
    expectedSize += TLVCountable::Count(screenStatus);
    return expectedSize;
}

void PasteData::ShareOptionToString(ShareOption shareOption, std::string& out)
{ // LCOV_EXCL_START
    if (shareOption == ShareOption::InApp) {
        out = "InAPP";
    } else if (shareOption == ShareOption::LocalDevice) {
        out = "LocalDevice";
    } else {
        out = "CrossDevice";
    }
} // LCOV_EXCL_STOP

std::string PasteData::CreatePasteId(const std::string& name, uint32_t sequence)
{ // LCOV_EXCL_START
    std::string currentId = name + "_" + std::to_string(sequence);
    return currentId;
} // LCOV_EXCL_STOP

bool PasteData::IsValidShareOption(int32_t shareOption)
{ // LCOV_EXCL_START
    switch (static_cast<ShareOption>(shareOption)) {
        case ShareOption::InApp:
            [[fallthrough]];
        case ShareOption::LocalDevice:
            [[fallthrough]];
        case ShareOption::CrossDevice:
            return true;
        default:
            return false;
    }
} // LCOV_EXCL_STOP

bool PasteData::IsValidPasteId(const std::string& pasteId)
{
    return false;
}

std::string PasteData::GetDeviceId() const
{ // LCOV_EXCL_START
    return deviceId_;
} // LCOV_EXCL_STOP

void PasteData::SetPasteId(const std::string& pasteId)
{ // LCOV_EXCL_START
    pasteId_ = pasteId;
} // LCOV_EXCL_STOP

std::string PasteData::GetPasteId() const
{ // LCOV_EXCL_START
    return pasteId_;
} // LCOV_EXCL_STOP

void PasteData::SetTextSize(size_t size)
{ // LCOV_EXCL_START
    textSize_ = size;
} // LCOV_EXCL_STOP

size_t PasteData::GetTextSize() const
{ // LCOV_EXCL_START
    return textSize_;
} // LCOV_EXCL_STOP
} // namespace MiscServices
} // namespace OHOS
