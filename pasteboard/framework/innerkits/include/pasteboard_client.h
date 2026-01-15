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

#ifndef PASTE_BOARD_CLIENT_H
#define PASTE_BOARD_CLIENT_H
#include <memory>
#include <mutex>
#include <singleton.h>
#include <string>
#include <vector>

#include "message_parcel_warp.h"
#include "paste_data.h"
#include "paste_data_entry.h"
#include "paste_data_record.h"
#include "pasteboard_delay_getter.h"
#include "pasteboard_disposable_observer.h"
#include "pasteboard_hilog.h"
#include "pasteboard_observer.h"
#include "pasteboard_progress_signal.h"
#include "pasteboard_types.h"
#include "unified_data.h"
#include "unified_meta.h"

namespace OHOS {
namespace MiscServices {

enum ProgressStatus {
    NORMAL_PASTE = 0,
    CANCEL_PASTE = 1,
    PASTE_TIME_OUT = 2,
};

enum FileConflictOption { FILE_OVERWRITE = 0, FILE_SKIP = 1, FILE_RENAME = 2 };

enum ProgressIndicator { NONE_PROGRESS_INDICATOR = 0, DEFAULT_PROGRESS_INDICATOR = 1 };

struct PasteDataFromServiceInfo {
    pid_t pid;
    std::string currentPid;
    std::string currentId;
};

struct ProgressReportListener {
    void (*OnProgressFail)(int32_t result);
};

struct ProgressInfo {
    int percentage;
};

struct GetDataParams;
struct ProgressListener {
    void (*ProgressNotify)(std::shared_ptr<GetDataParams> params);
};

struct GetDataParams {
    std::string destUri;
    enum FileConflictOption fileConflictOption;
    enum ProgressIndicator progressIndicator;
    struct ProgressListener listener;
    std::shared_ptr<ProgressSignalClient> progressSignal;
    ProgressInfo* info;
};

class API_EXPORT PasteboardClient {
public:
    DISALLOW_COPY_AND_MOVE(PasteboardClient);
    static PasteboardClient* GetInstance();
    static void ClipboardJniRegister();
    /**
     * CreateHtmlTextRecord
     * @description Create Html Text Record.
     * @param std::string text.
     * @return PasteDataRecord.
     */
    std::shared_ptr<PasteDataRecord> CreateHtmlTextRecord(const std::string& text);

    /**
     * CreatePlainTextRecord
     * @description Create Plaint Text Record.
     * @param std::string text.
     * @return PasteDataRecord.
     */
    std::shared_ptr<PasteDataRecord> CreatePlainTextRecord(const std::string& text);

    /**
     * CreatePixelMapRecord
     * @description Create PixelMap Record.
     * @param OHOS::Media::PixelMap pixelMap.
     * @return PasteDataRecord.
     */
    std::shared_ptr<PasteDataRecord> CreatePixelMapRecord(std::shared_ptr<OHOS::Media::PixelMap> pixelMap);

    /**
     * CreateUriRecord
     * @description Create Uri Text Record.
     * @param OHOS::Uri uri.
     * @return PasteDataRecord.
     */
    std::shared_ptr<PasteDataRecord> CreateUriRecord(const OHOS::Uri& uri);

    /**
     * CreateWantRecord
     * @description Create Plaint Want Record.
     * @param OHOS::AAFwk::Want want.
     * @return PasteDataRecord.
     */
    std::shared_ptr<PasteDataRecord> CreateWantRecord(std::shared_ptr<OHOS::AAFwk::Want> want);

    /**
     * CreateKvRecord
     * @description Create Kv Record.
     * @param std::string mimeType
     * @param std::vector<uint8_t> arrayBuffer
     * @return PasteDataRecord.
     */
    std::shared_ptr<PasteDataRecord> CreateKvRecord(
        const std::string& mimeType, const std::vector<uint8_t>& arrayBuffer);

    /**
     * CreateMultiDelayRecord
     * @description Create Multi DelayRecord.
     * @param std::vector<std::string> mimeTypes
     * @param std::shared_ptr<UDMF::EntryGetter> entryGetter
     * @return PasteDataRecord.
     */
    std::shared_ptr<PasteDataRecord> CreateMultiDelayRecord(
        std::vector<std::string> mimeTypes, const std::shared_ptr<UDMF::EntryGetter> entryGetter);

    /**
     * CreateHtmlData
     * @description Create Html Paste Data.
     * @param std::string text  .
     * @return PasteData.
     */
    std::shared_ptr<PasteData> CreateHtmlData(const std::string& htmlText);

    /**
     * CreatePlainTextData
     * @description Create Plain Text Paste Data.
     * @param std::string text .
     * @return PasteData.
     */
    std::shared_ptr<PasteData> CreatePlainTextData(const std::string& text);

    /**
     * CreatePixelMapData
     * @description Create PixelMap Paste Data.
     * @param OHOS::Media::PixelMap pixelMap .
     * @return PasteData.
     */
    std::shared_ptr<PasteData> CreatePixelMapData(std::shared_ptr<OHOS::Media::PixelMap> pixelMap);

    /**
     * CreateUriData
     * @description Create Uri Paste Data.
     * @param OHOS::Uri uri .
     * @return PasteData.
     */
    std::shared_ptr<PasteData> CreateUriData(const OHOS::Uri& uri);

    /**
     * CreateWantData
     * @description Create Want Paste Data.
     * @param OHOS::AAFwk::Want want .
     * @return PasteData.
     */
    std::shared_ptr<PasteData> CreateWantData(std::shared_ptr<OHOS::AAFwk::Want> want);

    /**
     * CreateKvData
     * @description Create Kv Paste Data.
     * @param std::string mimeType
     * @param std::vector<uint8_t> arrayBuffer
     * @return PasteData.
     */
    std::shared_ptr<PasteData> CreateKvData(const std::string& mimeType, const std::vector<uint8_t>& arrayBuffer);

    /**
     * CreateMultiTypeData
     * @description Create multi-type Data.
     * @param std::map<std::string, EntryValue> typeValueMap
     * @param recordMimeType record's default mimeType
     * @return PasteData.
     */
    std::shared_ptr<PasteData> CreateMultiTypeData(
        std::shared_ptr<std::map<std::string, std::shared_ptr<EntryValue>>> typeValueMap,
        const std::string& recordMimeType = "");

    /**
     * CreateMultiTypeDelayData
     * @description Create delayed multi-type Data.
     * @param std::vector<std::string> utdTypes
     * @param std::shared_ptr<UDMF::EntryGetter> entryGetter
     * @return PasteData.
     */
    std::shared_ptr<PasteData> CreateMultiTypeDelayData(
        std::vector<std::string> mimeTypes, std::shared_ptr<UDMF::EntryGetter> entryGetter);

    /**
     * GetChangeCount
     * @description get clip changed count from the pasteboard.
     * @param changeCount the changeCount of the PasteData.
     * @return int32_t.
     */
    int32_t GetChangeCount(uint32_t& changeCount);

    /**
     * GetRecordValueByType
     * @description get entry value from the pasteboard.
     * @param dataId the dataId of the PasteData.
     * @param recordId the recordId of the PasteRecord.
     * @param value the value of the PasteDataEntry.
     * @return int32_t.
     */
    int32_t GetRecordValueByType(uint32_t dataId, uint32_t recordId, PasteDataEntry& value);

    /**
     * GetPasteData
     * @description get paste data from the pasteboard.
     * @param pasteData the object of the PasteDate.
     * @return int32_t.
     */
    int32_t GetPasteData(PasteData& pasteData);

    /**
     * GetMimeTypes
     * @description get mime types from the pasteboard.
     * @return Returns MimeTypes
     */
    std::vector<std::string> GetMimeTypes();

    /**
     * HasPasteData
     * @description check paste data exist in the pasteboard.
     * @return bool. True exists, false does not exist
     */
    bool HasPasteData();

    /**
     * Clear
     * @description Clear Current pasteboard data.
     * @return void.
     */
    void Clear();

    /**
     * ClearByUser
     * @description Clear Current pasteboard data by user.
     * @return void.
     */
    void ClearByUser(int32_t uesrId) {}

    /**
     * SetPasteData
     * @description set paste data to the pasteboard.
     * @param pasteData the object of the PasteData.
     * @param pasteData the object of the PasteboardDelayGetter.
     * @param pasteData the map of the EntryGetter.
     * @return int32_t.
     */
    int32_t SetPasteData(PasteData& pasteData);

    int32_t SetUnifiedData(
        const UDMF::UnifiedData& unifiedData, std::shared_ptr<PasteboardDelayGetter> delayGetter = nullptr);

    /**
     *
     * SetPasteData
     * @description set paste data to the pasteboard.
     * @param unifiedData the object of the PasteDate.
     * @return int32_t.
     */
    int32_t GetUnifiedData(UDMF::UnifiedData& unifiedData);

    /**
     * SetUdsdData
     * @description set unified data with uds entries to the pasteboard.
     * @param unifiedData the object of the PasteDate.
     * @return int32_t.
     */
    int32_t SetUdsdData(const UDMF::UnifiedData& unifiedData);

    /**
     * GetUnifiedDataWithEntry
     * @description get unified data with uds entries from the pasteboard.
     * @param unifiedData the object of the PasteDate.
     * @return int32_t.
     */
    int32_t GetUdsdData(UDMF::UnifiedData& unifiedData);

    /**
     * IsRemoteData
     * @description check if remote data.
     * @return bool. True is remote data, else false.
     */
    bool IsRemoteData();

    /**
     * GetDataSource
     * @description Obtain the package name of the data source application.
     * @param std::string bundleName The package name of the application.
     * @return int32_t.
     */
    int32_t GetDataSource(std::string& bundleName);

    /**
     * HasDataType
     * @description Check if there is data of the specified type in the pasteboard.
     * @param std::string mimeType Specified mimetype.
     * @return bool. True exists, false does not exist
     */
    bool HasDataType(const std::string& mimeType);

    /**
     * HasUtdType
     * @description Check if there is data of the specified type in the pasteboard.
     * @param std::string utdType Specified mimetype.
     * @return bool. True exists, false does not exist
     */
    bool HasUtdType(const std::string& utdType);

    /**
     * DetectPatterns
     * @description Checks the specified patterns contained in clipboard, and removes if not found.
     * @param patternsToCheck A reference to an set of Pattern to check against the clipboard.
     * @return Returns DetectPatterns.
     */
    std::set<Pattern> DetectPatterns(const std::set<Pattern>& patternsToCheck);

    /**
     * SetGlobalShareOption
     * @description Set globalShareOptions.
     * @param globalShareOption globalShareOptions
     * @return int32_t
     */
    int32_t SetGlobalShareOption(const std::map<uint32_t, ShareOption>& globalShareOptions);

    /**
     * RemoveGlobalShareOption
     * @description Remove globalShareOptions.
     * @param tokenId tokenIds
     * @return int32_t
     */
    int32_t RemoveGlobalShareOption(const std::vector<uint32_t>& tokenIds);

    /**
     * GetGlobalShareOption
     * @description Get globalShareOptions.
     * @param tokenId tokenIds
     * @return globalShareOptions
     */
    std::map<uint32_t, ShareOption> GetGlobalShareOption(const std::vector<uint32_t>& tokenIds);
    /**
     * Subscribe
     * @description
     * @param type observer type
     * @param observer pasteboard change callback.
     * @return bool. True: subscribe success, false: subscribe failed.
     */
    bool Subscribe(PasteboardObserverType type, sptr<PasteboardObserver> callback);

    /**
     * Unsubscribe
     * @description
     * @param type observer type
     * @param observer pasteboard change callback.
     * @return void.
     */
    void Unsubscribe(PasteboardObserverType type, sptr<PasteboardObserver> callback);
    /**
     * SetAppShareOptions
     * @description Sets a unified ShareOptions for the application.
     * @param shareOptions shareOptions
     * @return result
     */
    int32_t SetAppShareOptions(const ShareOption& shareOptions);

    /**
     * RemoveAppShareOptions
     * @description Removes the ShareOptions for the application.
     * @return result
     */
    int32_t RemoveAppShareOptions();

    /**
     * PasteStart
     * @description Utilized to notify pasteboard service while reading PasteData, in this case, the service will help
     *     to preserve the context and resources
     * @return void.
     */
    void PasteStart(const std::string& pasteId) {}

    /**
     * PasteComplete
     * @description Invoked to notify pasteboard service the utilization of PasteData has completed and occupied
     *     resources can be released for further usage
     * @return void.
     */
    void PasteComplete(const std::string& deviceId, const std::string& pasteId) {}

    /**
     * GetDataWithProgress
     * @description Get paste data from the system pasteboard with system progress indicator.
     * @param pasteData the object of the PasteData.
     * @param params - Indicates the {@link GetDataParams}.
     * @returns int32_t
     */
    int32_t GetDataWithProgress(PasteData& pasteData, std::shared_ptr<GetDataParams> params);

    /**
     * GetUnifiedDataWithProgress
     * @description Get paste data from the system pasteboard with system progress indicator.
     * @param unifiedData - the object of the PasteData.
     * @param params - Indicates the {@link GetDataParams}.
     * @returns int32_t
     */
    int32_t GetUnifiedDataWithProgress(UDMF::UnifiedData& unifiedData, std::shared_ptr<GetDataParams> params);

    /**
     * HandleSignalValue
     * @description Handle hap signal value.
     * @param std::string signalValue - the value of hap ipc proxy.
     * @returns int32_t
     */
    int32_t HandleSignalValue(const std::string& signalValue);

    /**
     * ReleaseSaListener
     * @description Release Sa Listener.
     * @returns void
     */
    void ReleaseSaListener() {}

    /**
     * DetachPasteboard
     * @description Detach client from service, service will remove client death recipient.
     * @returns int32_t
     */
    int32_t DetachPasteboard();

    /**
     * SyncDelayedDataAsync
     * @description Notifies the system pasteboard to synchronize all time-lapse paste data from application.
     * @returns int32_t
     */
    int32_t SyncDelayedData();

protected:
    friend class PasteboardSaMgrListener;
    void Resubscribe() {}

private:
    PasteboardClient() {}
    ~PasteboardClient() {}
    static int32_t SetProgressWithoutFile(std::string& progressKey, std::shared_ptr<GetDataParams> params);
    int32_t GetPasteDataFromService(PasteData& pasteData, PasteDataFromServiceInfo& pasteDataFromServiceInfo,
        std::string progressKey, std::shared_ptr<GetDataParams> params);
    static std::atomic<uint64_t> progressStartTime_;
    static int32_t ProgressAfterTwentyPercent(
        PasteData& pasteData, std::shared_ptr<GetDataParams> params, std::string progressKey);
    static int32_t CheckProgressParam(std::shared_ptr<GetDataParams> params);
    int32_t ConvertErrCode(int32_t errCode);
    int32_t WritePasteData(PasteData& pasteData, std::vector<uint8_t>& pasteDataTlv, int& fd, int64_t& tlvSize,
        MessageParcelWarp& messageData, MessageParcel& parcelPata);
    template<typename T>
    int32_t ProcessPasteData(T& data, int64_t rawDataSize, int fd, const std::vector<uint8_t>& recvTLV);
    int32_t ProcessPasteDataFromService(
        PasteData& pasteData, int64_t rawDataSize, int fd, const std::vector<uint8_t>& recvTLV);

    static std::mutex instanceLock_;
    std::atomic<uint32_t> getSequenceId_ = 0;
    static std::atomic<bool> remoteTask_;
    static std::atomic<bool> isPasting_;
    std::mutex observerSetMutex_;
    std::mutex saListenerMutex_;
    bool isSubscribeSa_ = false;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_CLIENT_H
