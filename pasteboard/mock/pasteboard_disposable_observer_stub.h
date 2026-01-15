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

#ifndef PASTEBOARD_DISPOSABLE_OBSERVER_STUB_H
#define PASTEBOARD_DISPOSABLE_OBSERVER_STUB_H

#include <cstdint>
#include <string>
#include <unordered_map>

#include "message_parcel.h"

namespace OHOS {
class MessageOption;
namespace MiscServices {
class PasteboardDisposableObserverStub {
public:
    PasteboardDisposableObserverStub() {};
    virtual ~PasteboardDisposableObserverStub() = default;
    virtual int32_t OnRemoteRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        return 0;
    };

private:
    using PasteboardDisposableObserverFunc = int32_t (PasteboardDisposableObserverStub::*)(
        MessageParcel& data, MessageParcel& reply);

    virtual int32_t OnTextReceivedStub(MessageParcel& data, MessageParcel& reply)
    {
        return 0;
    };
    virtual void OnTextReceived(const std::string& text, int32_t errCode) {};

    std::unordered_map<uint32_t, PasteboardDisposableObserverFunc> memberFuncMap_;
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTEBOARD_DISPOSABLE_OBSERVER_STUB_H
