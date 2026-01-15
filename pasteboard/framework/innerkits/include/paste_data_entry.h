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

#ifndef PASTE_BOARD_ENTRY_H
#define PASTE_BOARD_ENTRY_H

#include "tlv_readable.h"
#include "tlv_writeable.h"

namespace OHOS {
namespace MiscServices {
class API_EXPORT MineCustomData : public TLVWriteable, public TLVReadable {
public:
    MineCustomData() = default;
    std::map<std::string, std::vector<uint8_t>> GetItemData();
    void AddItemData(const std::string& mimeType, const std::vector<uint8_t>& arrayBuffer);

    bool EncodeTLV(WriteOnlyBuffer& buffer) const override;
    bool DecodeTLV(ReadOnlyBuffer& buffer) override;
    size_t CountTLV() const override;

private:
    std::map<std::string, std::vector<uint8_t>> itemData_;
};

class API_EXPORT PasteDataEntry : public TLVWriteable, public TLVReadable {
public:
    using UDType = UDMF::UDType;
    PasteDataEntry() = default;
    PasteDataEntry(const PasteDataEntry& entry);
    PasteDataEntry& operator=(const PasteDataEntry& entry);
    PasteDataEntry(const std::string& utdId, const EntryValue& value);
    PasteDataEntry(const std::string& utdId, const std::string& mimeType, const EntryValue& value);

    std::shared_ptr<std::string> ConvertToPlainText() const;
    std::shared_ptr<std::string> ConvertToHtml() const;
    std::shared_ptr<Uri> ConvertToUri() const;
    std::shared_ptr<AAFwk::Want> ConvertToWant() const;
    std::shared_ptr<Media::PixelMap> ConvertToPixelMap() const;
    std::shared_ptr<MineCustomData> ConvertToCustomData() const;

    void SetValue(const EntryValue& value);
    EntryValue GetValue() const;
    void SetUtdId(const std::string& utdId);
    std::string GetUtdId() const;
    void SetMimeType(const std::string& mimeType);
    std::string GetMimeType() const;
    void SetFileSize(int64_t fileSize);
    int64_t GetFileSize() const;
    bool HasContent(const std::string& utdId) const;
    bool HasContentByMimeType(const std::string& mimeType) const;

    bool EncodeTLV(WriteOnlyBuffer& buffer) const override;
    bool DecodeTLV(ReadOnlyBuffer& buffer) override;
    size_t CountTLV() const override;

    int64_t rawDataSize_ = 0;

private:
    std::string utdId_;
    std::string mimeType_; // pasteboard mimeType
    EntryValue value_;
};

class API_EXPORT CommonUtils {
public:
    using UDType = UDMF::UDType;
    static std::string Convert(UDType uDType);
    static std::string Convert2MimeType(const std::string& utdId);
    static UDType Convert(int32_t uDType, const std::string& mimeType);
    static std::string Convert2UtdId(int32_t uDType, const std::string& mimeType);
    static bool IsFileUri(const std::string& utdId);
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_ENTRY_H