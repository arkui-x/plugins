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

#ifndef PLUGIN_DATA_PASTEBOARD_MOCK_PASTE_BOARD_OBSERVER_STUB_H
#define PLUGIN_DATA_PASTEBOARD_MOCK_PASTE_BOARD_OBSERVER_STUB_H

#include <cstdint>
#include <map>

#include "iremote_object.h"
#include "message_parcel.h"

namespace OHOS {
namespace MiscServices {
struct PasteboardChangedEvent {
    uint32_t eventType;
    uint32_t eventId;
};
class PasteboardObserverStub : public RefBase {
public:
    PasteboardObserverStub() {};
    ~PasteboardObserverStub() {};
    virtual int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        return 0;
    };
    virtual void OnPasteboardChanged() = 0;
    virtual void OnPasteboardEvent(const PasteboardChangedEvent& event) = 0;
    virtual sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }
};
} // namespace MiscServices
} // namespace OHOS
#endif // PLUGIN_DATA_PASTEBOARD_MOCK_PASTE_BOARD_OBSERVER_STUB_H
