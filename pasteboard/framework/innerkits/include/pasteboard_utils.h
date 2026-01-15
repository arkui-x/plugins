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

#ifndef PASTE_BOARD_UTILS_H
#define PASTE_BOARD_UTILS_H

#include "paste_data.h"
#include "unified_data.h"
namespace OHOS {
namespace MiscServices {
class API_EXPORT PasteboardUtils {
public:
    using UnifiedRecord = UDMF::UnifiedRecord;
    using UnifiedData = UDMF::UnifiedData;
    using UnifiedDataProperties = UDMF::UnifiedDataProperties;
    using UDType = UDMF::UDType;
    using ShareOptions = UDMF::ShareOptions;
    static PasteboardUtils& GetInstance();
    std::shared_ptr<PasteData> Convert(const UnifiedData& unifiedData);
    std::shared_ptr<UnifiedData> Convert(const PasteData& pasteData);
    std::vector<std::string> DeduplicateVector(const std::vector<std::string>& vec);

private:
    PasteboardUtils();

    std::vector<std::shared_ptr<PasteDataRecord>> Convert(const std::vector<std::shared_ptr<UnifiedRecord>>& records);
    static PasteDataProperty Convert(const UnifiedDataProperties& properties);
    static std::shared_ptr<UnifiedDataProperties> Convert(const PasteDataProperty& properties);
    static std::vector<std::string> Convert(const std::vector<std::string>& utdIds);
    static std::string Convert(UDType uDType);
    static UDType Convert(int32_t uDType, const std::string& mimeType);

    static std::shared_ptr<UnifiedRecord> PasteRecord2Text(std::shared_ptr<PasteDataRecord> record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2PlaintText(std::shared_ptr<PasteDataRecord> record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2Want(std::shared_ptr<PasteDataRecord> record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2Html(std::shared_ptr<PasteDataRecord> record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2Link(std::shared_ptr<PasteDataRecord> record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2File(std::shared_ptr<PasteDataRecord> record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2Image(std::shared_ptr<PasteDataRecord> record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2Video(std::shared_ptr<PasteDataRecord> record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2Audio(std::shared_ptr<PasteDataRecord> record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2Folder(std::shared_ptr<PasteDataRecord> record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2PixelMap(std::shared_ptr<PasteDataRecord> record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2SystemDefined(std::shared_ptr<PasteDataRecord> record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2AppItem(std::shared_ptr<PasteDataRecord> record);
    static std::shared_ptr<UnifiedRecord> PasteRecord2Form(std::shared_ptr<PasteDataRecord> record);
    static std::vector<std::shared_ptr<UnifiedRecord>> Custom2AppDefined(std::shared_ptr<PasteDataRecord> record);

    static std::shared_ptr<PasteDataRecord> Text2PasteRecord(std::shared_ptr<UnifiedRecord> record);
    static std::shared_ptr<PasteDataRecord> PlainText2PasteRecord(std::shared_ptr<UnifiedRecord> record);
    static std::shared_ptr<PasteDataRecord> Want2PasteRecord(std::shared_ptr<UnifiedRecord> record);
    static std::shared_ptr<PasteDataRecord> Html2PasteRecord(std::shared_ptr<UnifiedRecord> record);
    static std::shared_ptr<PasteDataRecord> Link2PasteRecord(std::shared_ptr<UnifiedRecord> record);
    static std::shared_ptr<PasteDataRecord> File2PasteRecord(std::shared_ptr<UnifiedRecord> record);
    static std::shared_ptr<PasteDataRecord> Image2PasteRecord(std::shared_ptr<UnifiedRecord> record);
    static std::shared_ptr<PasteDataRecord> Video2PasteRecord(std::shared_ptr<UnifiedRecord> record);
    static std::shared_ptr<PasteDataRecord> Audio2PasteRecord(std::shared_ptr<UnifiedRecord> record);
    static std::shared_ptr<PasteDataRecord> Folder2PasteRecord(std::shared_ptr<UnifiedRecord> record);
    static std::shared_ptr<PasteDataRecord> PixelMap2PasteRecord(std::shared_ptr<UnifiedRecord> record);
    static std::shared_ptr<PasteDataRecord> AppItem2PasteRecord(std::shared_ptr<UnifiedRecord> record);
    static std::shared_ptr<PasteDataRecord> Form2PasteRecord(std::shared_ptr<UnifiedRecord> record);
    static std::shared_ptr<PasteDataRecord> SystemDefined2PasteRecord(std::shared_ptr<UnifiedRecord> record);
    static std::shared_ptr<PasteDataRecord> AppDefined2PasteRecord(std::shared_ptr<UnifiedRecord> record);

    static ShareOption UdmfOptions2PbOption(ShareOptions udmfOptions);
    static ShareOptions PbOption2UdmfOptions(ShareOption pbOption);

    std::shared_ptr<UnifiedRecord> ConvertToUnifiedRecord(const std::shared_ptr<PasteDataRecord>& record);
    std::shared_ptr<PasteDataRecord> ConvertToPasteDataRecord(const std::shared_ptr<UnifiedRecord>& record);
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_UTILS_H