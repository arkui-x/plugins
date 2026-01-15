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

#ifndef CLIP_BOARD_PROXY_H
#define CLIP_BOARD_PROXY_H

#include <string>
#include <vector>

#include "errors.h"
#include "paste_data.h"
#include "pasteboard_observer_stub.h"
#include "pasteboard_types.h"

namespace OHOS {
namespace Plugin {
using namespace OHOS::MiscServices;
#define EXPORT __attribute__((visibility("default")))
class ClipboardProxy {
public:
    EXPORT ClipboardProxy() {}
    EXPORT virtual ~ClipboardProxy() {}

    EXPORT static ClipboardProxy* GetInstance();

    virtual int32_t SetPasteData(const PasteData& pasteData) = 0;
    virtual int32_t GetPasteData(PasteData& pasteData, int32_t& realErrorCode) = 0;
    virtual int Clear() = 0;

    virtual bool Subscribe(PasteboardObserverType type, sptr<PasteboardObserverStub> callback) = 0;
    virtual void Unsubscribe(PasteboardObserverType type, sptr<PasteboardObserverStub> callback) = 0;
    virtual void NotifyObservers() = 0;

    virtual int DetectPatterns(const std::vector<Pattern>& patternsToCheck, std::vector<Pattern>& funcResult) = 0;
    virtual ErrCode HasPasteData(bool& funcResult) = 0;
    virtual ErrCode HasDataType(const std::string& mimeType, bool& funcResult) = 0;
    virtual ErrCode GetMimeTypes(std::vector<std::string>& funcResult) = 0;
    virtual ErrCode GetChangeCount(uint32_t& changeCount) = 0;
};
} // namespace Plugin
} // namespace OHOS
#endif // CLIP_BOARD_PROXY_H