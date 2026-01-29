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

#ifndef PASTE_BOARD_RECORD_H
#define PASTE_BOARD_RECORD_H

#include "common/constant.h"
#include "paste_data_entry.h"

namespace OHOS {
namespace UDMF {
class EntryGetter;
}
namespace MiscServices {
enum ResultCode : int32_t { OK = 0, IPC_NO_DATA, IPC_ERROR };

struct RemoteRecordValue {
    int32_t udType_ = UDMF::UD_BUTT;
    std::string mimeType_;
    std::shared_ptr<std::string> plainText_;
    std::shared_ptr<std::string> htmlText_;
    std::shared_ptr<OHOS::AAFwk::Want> want_;
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap_;
    std::shared_ptr<OHOS::Uri> uri_;
    EntryValue udmfValue_;
    std::vector<std::shared_ptr<PasteDataEntry>> entries_;
};

class API_EXPORT PasteDataRecord : public TLVWriteable, public TLVReadable {
public:
    static constexpr unsigned int NO_PERMISSION = 0;
    PasteDataRecord();
    ~PasteDataRecord();
    PasteDataRecord(const PasteDataRecord& record);
    PasteDataRecord(std::string mimeType, std::shared_ptr<std::string> htmlText,
        std::shared_ptr<OHOS::AAFwk::Want> want, std::shared_ptr<std::string> plainText,
        std::shared_ptr<OHOS::Uri> uri);

    bool EncodeTLV(WriteOnlyBuffer& buffer) const override;
    bool DecodeTLV(ReadOnlyBuffer& buffer) override;
    size_t CountTLV() const override;
    size_t CountTLVLocal() const;
    size_t CountTLVRemote() const;
    bool EncodeTLVLocal(WriteOnlyBuffer& buffer) const;
    bool EncodeTLVRemote(WriteOnlyBuffer& buffer) const;

    static std::shared_ptr<PasteDataRecord> NewHtmlRecord(const std::string& htmlText);
    static std::shared_ptr<PasteDataRecord> NewWantRecord(std::shared_ptr<OHOS::AAFwk::Want> want);
    static std::shared_ptr<PasteDataRecord> NewPlainTextRecord(const std::string& text);
    static std::shared_ptr<PasteDataRecord> NewPixelMapRecord(std::shared_ptr<OHOS::Media::PixelMap> pixelMap);
    static std::shared_ptr<PasteDataRecord> NewUriRecord(const OHOS::Uri& uri);
    static std::shared_ptr<PasteDataRecord> NewKvRecord(
        const std::string& mimeType, const std::vector<uint8_t>& arrayBuffer);
    static std::shared_ptr<PasteDataRecord> NewMultiTypeRecord(
        std::shared_ptr<std::map<std::string, std::shared_ptr<EntryValue>>> values,
        const std::string& recordMimeType = "");
    static std::shared_ptr<PasteDataRecord> NewMultiTypeDelayRecord(
        std::vector<std::string> mimeTypes, const std::shared_ptr<UDMF::EntryGetter> entryGetter);

    bool isConvertUriFromRemote = false;
    std::string GetMimeType() const;
    std::set<std::string> GetUtdTypes() const;
    std::set<std::string> GetMimeTypes() const;
    std::shared_ptr<std::string> GetHtmlTextV0() const;
    std::shared_ptr<std::string> GetHtmlText();
    std::shared_ptr<std::string> GetPlainTextV0() const;
    std::shared_ptr<std::string> GetPlainText();
    std::shared_ptr<OHOS::Media::PixelMap> GetPixelMapV0() const;
    std::shared_ptr<OHOS::Media::PixelMap> GetPixelMap();
    void ClearPixelMap();
    std::shared_ptr<OHOS::Uri> GetUriV0() const;
    std::shared_ptr<OHOS::Uri> GetUri();
    void SetUri(std::shared_ptr<OHOS::Uri> uri);
    std::shared_ptr<OHOS::Uri> GetOriginUri() const;
    std::shared_ptr<OHOS::AAFwk::Want> GetWant() const;
    std::shared_ptr<MineCustomData> GetCustomData() const;

    std::string ConvertToText() const;
    void SetConvertUri(const std::string& value);
    std::string GetConvertUri() const;
    void SetGrantUriPermission(bool hasPermission);
    bool HasGrantUriPermission();
    void SetUriPermission(uint32_t uriPermission);
    uint32_t GetUriPermission();

    void SetTextContent(const std::string& content);
    std::string GetTextContent() const;
    void SetDetails(const Details& details);
    std::shared_ptr<Details> GetDetails() const;
    void SetSystemDefinedContent(const Details& contents);
    std::shared_ptr<Details> GetSystemDefinedContent() const;
    int32_t GetUDType() const;
    void SetUDType(int32_t type);

    bool HasEmptyEntry() const;
    uint32_t RemoveEmptyEntry();
    void AddEntry(const std::string& utdType, std::shared_ptr<PasteDataEntry> value);
    void AddEntryByMimeType(const std::string& mimeType, std::shared_ptr<PasteDataEntry> value);
    std::shared_ptr<PasteDataEntry> GetEntry(const std::string& utdType);
    std::shared_ptr<PasteDataEntry> GetEntryByMimeType(const std::string& mimeType);
    std::vector<std::shared_ptr<PasteDataEntry>> GetEntries() const;
    std::vector<std::string> GetValidTypes(const std::vector<std::string>& types) const;
    std::vector<std::string> GetValidMimeTypes(const std::vector<std::string>& mimeTypes) const;

    void SetDelayRecordFlag(bool isDelay);
    bool IsDelayRecord() const;
    void SetDataId(uint32_t dataId);
    uint32_t GetDataId() const;
    void SetRecordId(uint32_t recordId);
    uint32_t GetRecordId() const;

    void SetEntryGetter(const std::shared_ptr<UDMF::EntryGetter> entryGetter);
    std::shared_ptr<UDMF::EntryGetter> GetEntryGetter();

    void SetFrom(uint32_t from);
    uint32_t GetFrom() const;

    class Builder {
    public:
        explicit Builder(const std::string& mimeType);
        Builder& SetHtmlText(std::shared_ptr<std::string> htmlText);
        Builder& SetWant(std::shared_ptr<OHOS::AAFwk::Want> want);
        Builder& SetPlainText(std::shared_ptr<std::string> plainText);
        Builder& SetUri(std::shared_ptr<OHOS::Uri> uri);
        Builder& SetPixelMap(std::shared_ptr<OHOS::Media::PixelMap> pixelMap);
        Builder& SetCustomData(std::shared_ptr<MineCustomData> customData);
        Builder& SetMimeType(std::string mimeType);
        std::shared_ptr<PasteDataRecord> Build();

    private:
        std::shared_ptr<PasteDataRecord> record_ = nullptr;
    };

private:
    std::string GetPassUri();
    void AddUriEntry();
    bool DecodeItem1(uint16_t tag, ReadOnlyBuffer& buffer, TLVHead& head);
    bool DecodeItem2(uint16_t tag, ReadOnlyBuffer& buffer, TLVHead& head);
    std::shared_ptr<PasteDataEntry> Remote2Local() const;
    std::shared_ptr<RemoteRecordValue> Local2Remote() const;

    bool isDelay_ = false;
    bool hasGrantUriPermission_ = false;
    int32_t udType_ = UDMF::UD_BUTT;
    uint32_t dataId_ = 0;
    uint32_t recordId_ = 0;
    uint32_t from_ = 0;
    uint32_t uriPermission_ = NO_PERMISSION;
    std::string convertUri_;
    std::string textContent_;
    std::string mimeType_;
    std::shared_ptr<std::string> htmlText_;
    std::shared_ptr<OHOS::AAFwk::Want> want_;
    std::shared_ptr<std::string> plainText_;
    std::shared_ptr<OHOS::Uri> uri_;
    std::shared_ptr<OHOS::Media::PixelMap> pixelMap_;
    std::shared_ptr<MineCustomData> customData_;

    std::shared_ptr<Details> details_;
    std::shared_ptr<Details> systemDefinedContents_;
    std::shared_ptr<EntryValue> udmfValue_;
    std::vector<std::shared_ptr<PasteDataEntry>> entries_;
    std::shared_ptr<UDMF::EntryGetter> entryGetter_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_RECORD_H
