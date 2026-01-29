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

#include "pasteboard_client.h"

#include <charconv>
#include <cstdint>
#include <memory>
#include <mutex>
#include <regex>
#include <string>
#include <thread>
#include <vector>

#include "clipboard_proxy.h"
#include "convert_utils.h"
#include "nlohmann/json.hpp"
#include "paste_data.h"
#include "paste_data_entry.h"
#include "paste_data_record.h"
#include "pasteboard_delay_getter.h"
#include "pasteboard_error.h"
#include "pasteboard_pattern.h"
#include "pasteboard_types.h"
#include "pasteboard_utils.h"
#include "unified_data.h"
#include "unified_meta.h"

using namespace OHOS::Media;
using namespace OHOS::Plugin;
using json = nlohmann::json;
namespace OHOS {
namespace MiscServices {
std::string g_progressKey;
std::mutex PasteboardClient::instanceLock_;
std::atomic<bool> PasteboardClient::remoteTask_(false);
std::atomic<bool> PasteboardClient::isPasting_(false);
std::atomic<uint64_t> PasteboardClient::progressStartTime_;

struct RadarReportIdentity {
    pid_t pid;
    int32_t errorCode;
};

bool operator==(const RadarReportIdentity& lhs, const RadarReportIdentity& rhs)
{
    return lhs.pid == rhs.pid && lhs.errorCode == rhs.errorCode;
}

PasteboardClient::PasteboardClient() {}

PasteboardClient::~PasteboardClient()
{
    observerSet_.clear();
}

PasteboardClient* PasteboardClient::GetInstance()
{
    static PasteboardClient instance;
    return &instance;
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreateHtmlTextRecord(const std::string& htmlText)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New text record");
    return PasteDataRecord::NewHtmlRecord(htmlText);
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreateWantRecord(std::shared_ptr<OHOS::AAFwk::Want> want)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New want record");
    return PasteDataRecord::NewWantRecord(std::move(want));
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreatePlainTextRecord(const std::string& text)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New text record");
    return PasteDataRecord::NewPlainTextRecord(text);
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreatePixelMapRecord(std::shared_ptr<PixelMap> pixelMap)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New pixelMap record");
    return PasteDataRecord::NewPixelMapRecord(std::move(pixelMap));
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreateUriRecord(const OHOS::Uri& uri)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New uri record");
    return PasteDataRecord::NewUriRecord(uri);
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreateKvRecord(
    const std::string& mimeType, const std::vector<uint8_t>& arrayBuffer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New kv record");
    return PasteDataRecord::NewKvRecord(mimeType, arrayBuffer);
}

std::shared_ptr<PasteDataRecord> PasteboardClient::CreateMultiDelayRecord(
    std::vector<std::string> mimeTypes, const std::shared_ptr<UDMF::EntryGetter> entryGetter)
{
    return PasteDataRecord::NewMultiTypeDelayRecord(mimeTypes, entryGetter);
}

std::shared_ptr<PasteData> PasteboardClient::CreateHtmlData(const std::string& htmlText)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New htmlText data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddHtmlRecord(htmlText);
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreateWantData(std::shared_ptr<OHOS::AAFwk::Want> want)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New want data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddWantRecord(std::move(want));
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreatePlainTextData(const std::string& text)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New plain data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddTextRecord(text);
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreatePixelMapData(std::shared_ptr<PixelMap> pixelMap)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New pixelMap data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddPixelMapRecord(std::move(pixelMap));
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreateUriData(const OHOS::Uri& uri)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New uri data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddUriRecord(uri);
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreateKvData(
    const std::string& mimeType, const std::vector<uint8_t>& arrayBuffer)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New Kv data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddKvRecord(mimeType, arrayBuffer);
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreateMultiTypeData(
    std::shared_ptr<std::map<std::string, std::shared_ptr<EntryValue>>> typeValueMap, const std::string& recordMimeType)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New multiType data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddRecord(PasteDataRecord::NewMultiTypeRecord(std::move(typeValueMap), recordMimeType));
    return pasteData;
}

std::shared_ptr<PasteData> PasteboardClient::CreateMultiTypeDelayData(
    std::vector<std::string> mimeTypes, std::shared_ptr<UDMF::EntryGetter> entryGetter)
{
    PASTEBOARD_HILOGD(PASTEBOARD_MODULE_CLIENT, "New multiTypeDelay data");
    auto pasteData = std::make_shared<PasteData>();
    pasteData->AddRecord(PasteDataRecord::NewMultiTypeDelayRecord(mimeTypes, entryGetter));
    return pasteData;
}

int32_t PasteboardClient::GetChangeCount(uint32_t& changeCount)
{
    auto clipboardProxy = ClipboardProxy::GetInstance();
    if (clipboardProxy == nullptr) {
        return static_cast<int32_t>(PasteboardError::UNKNOWN_ERROR);
    }
    int32_t ret = clipboardProxy->GetChangeCount(changeCount);
    return ConvertErrCode(ret);
}

int32_t PasteboardClient::SubscribeEntityObserver(
    EntityType entityType, uint32_t expectedDataLength, const sptr<EntityRecognitionObserver>& observer)
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

int32_t PasteboardClient::UnsubscribeEntityObserver(
    EntityType entityType, uint32_t expectedDataLength, const sptr<EntityRecognitionObserver>& observer)
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

int32_t PasteboardClient::GetRecordValueByType(uint32_t dataId, uint32_t recordId, PasteDataEntry& value)
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

void PasteboardClient::Clear()
{
    auto clipboardProxy = ClipboardProxy::GetInstance();
    PASTEBOARD_CHECK_AND_RETURN_LOGE(clipboardProxy != nullptr, PASTEBOARD_MODULE_CLIENT, "clipboardProxy is nullptr");
    clipboardProxy->Clear();
}

void PasteboardClient::ClearByUser(int32_t userId) {}

void PasteboardClient::CloseSharedMemFd(int fd) {}

int32_t PasteboardClient::GetPasteData(PasteData& pasteData)
{
    auto clipboardProxy = ClipboardProxy::GetInstance();
    if (clipboardProxy == nullptr) {
        return static_cast<int32_t>(PasteboardError::UNKNOWN_ERROR);
    }
    int32_t realErrCode = 0;
    clipboardProxy->GetPasteData(pasteData, realErrCode);
    int32_t ret = ConvertErrCode(realErrCode);
    if (ret != static_cast<int32_t>(PasteboardError::E_OK)) {
        return ret;
    }
    return static_cast<int32_t>(PasteboardError::E_OK);
}

void PasteboardClient::GetDataReport(
    PasteData& pasteData, int32_t syncTime, const std::string& currentId, const std::string& currentPid, int32_t ret)
{}

void PasteboardClient::GetProgressByProgressInfo(std::shared_ptr<GetDataParams> params) {}

int32_t PasteboardClient::SetProgressWithoutFile(std::string& progressKey, std::shared_ptr<GetDataParams> params)
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

void PasteboardClient::ProgressSmoothToTwentyPercent(
    PasteData& pasteData, std::string& progressKey, std::shared_ptr<GetDataParams> params)
{}

void PasteboardClient::UpdateProgress(std::shared_ptr<GetDataParams> params, int progressValue) {}

void PasteboardClient::OnProgressAbnormal(int32_t result) {}

int32_t PasteboardClient::GetPasteDataFromService(PasteData& pasteData,
    PasteDataFromServiceInfo& pasteDataFromServiceInfo, std::string progressKey, std::shared_ptr<GetDataParams> params)
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

template<typename T>
int32_t PasteboardClient::ProcessPasteData(T& data, int64_t rawDataSize, int fd, const std::vector<uint8_t>& recvTLV)
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

void PasteboardClient::ProcessRadarReport(
    int32_t ret, PasteData& pasteData, PasteDataFromServiceInfo& pasteDataFromServiceInfo, int32_t syncTime)
{}

void PasteboardClient::ProgressRadarReport(PasteData& pasteData, PasteDataFromServiceInfo& pasteDataFromServiceInfo) {}

int32_t PasteboardClient::ProgressAfterTwentyPercent(
    PasteData& pasteData, std::shared_ptr<GetDataParams> params, std::string progressKey)
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

int32_t PasteboardClient::CheckProgressParam(std::shared_ptr<GetDataParams> params)
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

int32_t PasteboardClient::GetDataWithProgress(PasteData& pasteData, std::shared_ptr<GetDataParams> params)
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

int32_t PasteboardClient::GetUnifiedDataWithProgress(
    UDMF::UnifiedData& unifiedData, std::shared_ptr<GetDataParams> params)
{
    PasteData pasteData;
    int32_t ret = GetDataWithProgress(pasteData, params);
    unifiedData = *(ConvertUtils::Convert(pasteData));
    return ret;
}

int32_t PasteboardClient::GetUnifiedData(UDMF::UnifiedData& unifiedData)
{
    PasteData pasteData;
    int32_t ret = GetPasteData(pasteData);
    unifiedData = *(PasteboardUtils::GetInstance().Convert(pasteData));
    return ret;
}

int32_t PasteboardClient::GetUdsdData(UDMF::UnifiedData& unifiedData)
{
    PasteData pasteData;
    int32_t ret = GetPasteData(pasteData);
    unifiedData = *(ConvertUtils::Convert(pasteData));
    return ret;
}

bool PasteboardClient::HasPasteData()
{
    auto clipboardProxy = Plugin::ClipboardProxy::GetInstance();
    if (clipboardProxy == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "clipboardProxy is nullptr");
        return false;
    }
    bool ret = false;
    int32_t errCode = clipboardProxy->HasPasteData(ret);
    if (errCode != ERR_OK) {
        return false;
    }
    return ret;
}

bool PasteboardClient::HasRemoteData()
{
    return false;
}

void PasteboardClient::CreateGetterAgent(sptr<PasteboardDelayGetterClient>& delayGetterAgent,
    std::shared_ptr<PasteboardDelayGetter>& delayGetter, sptr<PasteboardEntryGetterClient>& entryGetterAgent,
    std::map<uint32_t, std::shared_ptr<UDMF::EntryGetter>>& entryGetters, PasteData& pasteData)
{}

int32_t PasteboardClient::WritePasteData(PasteData& pasteData, std::vector<uint8_t>& buffer, int& fd, int64_t& tlvSize,
    MessageParcelWarp& messageData, MessageParcel& parcelPata)
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

int32_t PasteboardClient::SetPasteData(PasteData& pasteData, std::shared_ptr<PasteboardDelayGetter> delayGetter,
    std::map<uint32_t, std::shared_ptr<UDMF::EntryGetter>> entryGetters)
{
    auto clipboardProxy = Plugin::ClipboardProxy::GetInstance();
    if (clipboardProxy == nullptr) {
        return static_cast<int32_t>(PasteboardError::UNKNOWN_ERROR);
    }
    int32_t ret = clipboardProxy->SetPasteData(pasteData);
    PASTEBOARD_HILOGI(PASTEBOARD_MODULE_CLIENT, "leave, ret=%{public}d", ret);
    return ConvertErrCode(ret);
}

int32_t PasteboardClient::SetUnifiedData(
    const UDMF::UnifiedData& unifiedData, std::shared_ptr<PasteboardDelayGetter> delayGetter)
{
    auto pasteData = PasteboardUtils::GetInstance().Convert(unifiedData);
    return SetPasteData(*pasteData);
}

int32_t PasteboardClient::SetUdsdData(const UDMF::UnifiedData& unifiedData)
{
    auto pasteData = ConvertUtils::Convert(unifiedData);
    return SetPasteData(*pasteData);
}

void PasteboardClient::SubscribePasteboardSA() {}

void PasteboardClient::UnSubscribePasteboardSA() {}

void PasteboardClient::ReleaseSaListener() {}

int32_t PasteboardClient::DetachPasteboard()
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

void PasteboardClient::Resubscribe() {}

bool PasteboardClient::Subscribe(PasteboardObserverType type, sptr<PasteboardObserver> callback)
{
    auto clipboardProxy = Plugin::ClipboardProxy::GetInstance();
    if (clipboardProxy == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "clipboardProxy is nullptr");
        return false;
    }
    return clipboardProxy->Subscribe(type, callback);
}

void PasteboardClient::AddPasteboardChangedObserver(sptr<PasteboardObserver> callback) {}

void PasteboardClient::AddPasteboardEventObserver(sptr<PasteboardObserver> callback) {}

void PasteboardClient::Unsubscribe(PasteboardObserverType type, sptr<PasteboardObserver> callback)
{
    auto clipboardProxy = Plugin::ClipboardProxy::GetInstance();
    if (clipboardProxy == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "clipboardProxy is nullptr");
        return;
    }
    clipboardProxy->Unsubscribe(type, callback);
}

void PasteboardClient::RemovePasteboardChangedObserver(sptr<PasteboardObserver> callback) {}

void PasteboardClient::RemovePasteboardEventObserver(sptr<PasteboardObserver> callback) {}

int32_t PasteboardClient::SubscribeDisposableObserver(
    const sptr<PasteboardDisposableObserver>& observer, int32_t targetWindowId, DisposableType type, uint32_t maxLength)
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

int32_t PasteboardClient::SetGlobalShareOption(const std::map<uint32_t, ShareOption>& globalShareOptions)
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

int32_t PasteboardClient::RemoveGlobalShareOption(const std::vector<uint32_t>& tokenIds)
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

std::map<uint32_t, ShareOption> PasteboardClient::GetGlobalShareOption(const std::vector<uint32_t>& tokenIds)
{
    std::map<uint32_t, ShareOption> result;
    return result;
}

int32_t PasteboardClient::SetAppShareOptions(const ShareOption& shareOptions)
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

int32_t PasteboardClient::RemoveAppShareOptions()
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

bool PasteboardClient::IsRemoteData()
{
    return false;
}

int32_t PasteboardClient::GetDataSource(std::string& bundleName)
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

std::vector<std::string> PasteboardClient::GetMimeTypes()
{
    auto clipboardProxy = Plugin::ClipboardProxy::GetInstance();
    if (clipboardProxy == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "clipboardProxy is nullptr");
        return {};
    }
    std::vector<std::string> mimeTypes = {};
    int32_t ret = clipboardProxy->GetMimeTypes(mimeTypes);
    if (ret != ERR_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "GetMimeTypes failed, ret=%{public}d", ret);
        return {};
    }
    return mimeTypes;
}

bool PasteboardClient::HasDataType(const std::string& mimeType)
{
    auto clipboardProxy = Plugin::ClipboardProxy::GetInstance();
    if (clipboardProxy == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "clipboardProxy is nullptr");
        return false;
    }
    bool ret = false;
    int32_t retCode = clipboardProxy->HasDataType(mimeType, ret);
    if (retCode != ERR_OK) {
        return false;
    }
    return ret;
}

bool PasteboardClient::HasUtdType(const std::string& utdType)
{
    return false;
}

std::set<Pattern> PasteboardClient::DetectPatterns(const std::set<Pattern>& patternsToCheck)
{
    if (!PatternDetection::IsValid(patternsToCheck)) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "Invalid number in Pattern set!");
        return {};
    }
    auto clipboardProxy = Plugin::ClipboardProxy::GetInstance();
    if (clipboardProxy == nullptr) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "clipboardProxy is nullptr");
        return {};
    }
    std::vector<Pattern> patterns(patternsToCheck.begin(), patternsToCheck.end());
    std::vector<Pattern> funcResult = {};
    int32_t ret = clipboardProxy->DetectPatterns(patterns, funcResult);
    if (ret != ERR_OK) {
        PASTEBOARD_HILOGE(PASTEBOARD_MODULE_CLIENT, "DetectPatterns failed, ret=%{public}d", ret);
        return {};
    }
    std::set<Pattern> result(funcResult.begin(), funcResult.end());
    return result;
}

sptr<IPasteboardService> PasteboardClient::GetPasteboardService()
{
    sptr<IPasteboardService> pasteboardService;
    return pasteboardService;
}

void PasteboardClient::PasteStart(const std::string& pasteId) {}

void PasteboardClient::PasteComplete(const std::string& deviceId, const std::string& pasteId) {}

int32_t PasteboardClient::HandleSignalValue(const std::string& signalValue)
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

void PasteboardClient::ShowProgress(const std::string& progressKey) {}

int32_t PasteboardClient::SyncDelayedData()
{
    return static_cast<int32_t>(PasteboardError::NOT_SUPPORT);
}

std::string PasteboardClient::GetPasteDataInfoSummary(const PasteData& pasteData)
{
    return std::string("");
}

int32_t PasteboardClient::ConvertErrCode(int32_t errCode)
{
    switch (errCode) {
        case ERR_INVALID_VALUE: // fall-through
        case ERR_INVALID_DATA:
            return static_cast<int32_t>(PasteboardError::SERIALIZATION_ERROR);
        case ERR_OK:
            return static_cast<int32_t>(PasteboardError::E_OK);
        default:
            return errCode;
    }
}
} // namespace MiscServices
} // namespace OHOS
