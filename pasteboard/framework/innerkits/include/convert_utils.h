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

#ifndef PASTE_BOARD_CONVERT_UTILS_H
#define PASTE_BOARD_CONVERT_UTILS_H

#include "paste_data.h"
#include "unified_data.h"
namespace OHOS {
namespace MiscServices {
class API_EXPORT ConvertUtils {
public:
    using UnifiedRecord = UDMF::UnifiedRecord;
    using UnifiedData = UDMF::UnifiedData;
    using UnifiedDataProperties = UDMF::UnifiedDataProperties;
    using UDType = UDMF::UDType;
    using ShareOptions = UDMF::ShareOptions;

    static std::shared_ptr<PasteData> Convert(const UnifiedData& unifiedData);
    static std::shared_ptr<UnifiedData> Convert(const PasteData& pasteData);

    static std::vector<std::shared_ptr<PasteDataRecord>> Convert(
        const std::vector<std::shared_ptr<UnifiedRecord>>& records);
    static std::vector<std::shared_ptr<UnifiedRecord>> Convert(
        const std::vector<std::shared_ptr<PasteDataRecord>>& records);

    static std::shared_ptr<PasteDataRecord> Convert(std::shared_ptr<UnifiedRecord> record);
    static std::shared_ptr<UnifiedRecord> Convert(std::shared_ptr<PasteDataRecord> record);

    static PasteDataProperty ConvertProperty(
        const std::shared_ptr<UnifiedDataProperties>& properties, const UnifiedData& unifiedData);
    static std::shared_ptr<UnifiedDataProperties> ConvertProperty(const PasteDataProperty& properties);

    static std::vector<std::shared_ptr<PasteDataEntry>> Convert(
        const std::shared_ptr<std::map<std::string, UDMF::ValueType>>& entries);
    static std::shared_ptr<std::vector<std::pair<std::string, UDMF::ValueType>>> Convert(
        const std::vector<std::shared_ptr<PasteDataEntry>>& entries, std::shared_ptr<PasteDataRecord> record);

    static UDMF::ValueType Convert(
        const std::shared_ptr<PasteDataEntry>& entry, std::shared_ptr<PasteDataRecord> record);

    static std::vector<std::string> Convert(const std::vector<std::string>& utdIds);

private:
    static constexpr const char* CHANNEL_NAME = "pasteboard";
    static ShareOption UdmfOptions2PbOption(ShareOptions udmfOptions);
    static ShareOptions PbOption2UdmfOptions(ShareOption pbOption);
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_CONVERT_UTILS_H