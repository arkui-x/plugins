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

#ifndef PASTE_BOARD_OBSERVER_H
#define PASTE_BOARD_OBSERVER_H

#include "pasteboard_observer_stub.h"

namespace OHOS {
namespace MiscServices {
class PasteboardObserver : public PasteboardObserverStub {
public:
    PasteboardObserver() {};
    ~PasteboardObserver() {};
    void OnPasteboardChanged() override {};
    void OnPasteboardEvent(const PasteboardChangedEvent& event) override {};
};
} // namespace MiscServices
} // namespace OHOS
#endif // PASTE_BOARD_OBSERVER_H
