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

#ifndef PLUGIN_PASTEBOARD_MOCK_PASTEBOARD_ENTRY_GETTER_CLIENT_H
#define PLUGIN_PASTEBOARD_MOCK_PASTEBOARD_ENTRY_GETTER_CLIENT_H
#include "pasteboard_delay_getter.h"

namespace OHOS {
namespace UDMF {
class EntryGetter;
}
namespace MiscServices {
class PasteboardEntryGetterClient {
public:
    explicit PasteboardEntryGetterClient(const std::map<uint32_t, std::shared_ptr<UDMF::EntryGetter>> entryGetters) {}
    ~PasteboardEntryGetterClient() = default;
    int32_t GetRecordValueByType(uint32_t recordId, PasteDataEntry& value)
    {
        return 0;
    }
};
} // namespace MiscServices
} // namespace OHOS
#endif // PLUGIN_PASTEBOARD_MOCK_PASTEBOARD_ENTRY_GETTER_CLIENT_H