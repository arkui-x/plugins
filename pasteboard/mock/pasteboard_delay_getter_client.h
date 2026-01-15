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

#ifndef PLUGIN_PASTEBOARD_MOCK_PASTEBOARD_DELAY_GETTER_CLIENT_H
#define PLUGIN_PASTEBOARD_MOCK_PASTEBOARD_DELAY_GETTER_CLIENT_H
#include "pasteboard_types.h"
#include "pasteboard_delay_getter.h"
#include "ipasteboard_service.h"

namespace OHOS {
namespace MiscServices {
class PasteboardDelayGetterClient {
public:
    explicit PasteboardDelayGetterClient(std::shared_ptr<PasteboardDelayGetter> delayGetter) {}
    ~PasteboardDelayGetterClient() = default;

    void GetPasteData(const std::string &type, PasteData &data) {}
    void GetUnifiedData(const std::string &type, UDMF::UnifiedData &data) {}
};
} // namespace MiscServices
} // namespace OHOS
#endif // PLUGIN_PASTEBOARD_MOCK_PASTEBOARD_DELAY_GETTER_CLIENT_H
