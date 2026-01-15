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

#ifndef OHOS_MISCSERVICES_PASTEBOARDTYPES_H
#define OHOS_MISCSERVICES_PASTEBOARDTYPES_H

#include <cstdbool>
#include <cstdint>
#include <iremote_proxy.h>
#include <message_parcel.h>
#include <string_ex.h>

namespace OHOS {
namespace MiscServices {

enum class PasteboardEventStatus : uint8_t {
    PASTEBOARD_CLEAR = 1,
    PASTEBOARD_READ = 2,
    PASTEBOARD_WRITE = 3,
};

enum class PasteboardObserverType : uint8_t {
    OBSERVER_LOCAL = 1,
    OBSERVER_REMOTE = 2,
    OBSERVER_ALL = 3,
    OBSERVER_EVENT = 4,
};

enum class Pattern : uint32_t {
    URL = 0,
    NUMBER,
    EMAIL_ADDRESS,
    COUNT,
};

enum class EntityType : uint32_t {
    ADDRESS = 0,
    MAX,
};

enum class DisposableType : uint32_t {
    PLAIN_TEXT = 0,
    MAX,
};

} // namespace MiscServices
} // namespace OHOS

#endif // OHOS_MISCSERVICES_PASTEBOARDTYPES_H
