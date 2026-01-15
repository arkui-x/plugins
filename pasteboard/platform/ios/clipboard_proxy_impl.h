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

#ifndef PLUGINS_PASTEBOARD_PLATFORM_IOS_CLIPBOARD_PROXY_IMPL_H
#define PLUGINS_PASTEBOARD_PLATFORM_IOS_CLIPBOARD_PROXY_IMPL_H

#include <mutex>

#include "clipboard_proxy.h"
#include "paste_data.h"

namespace OHOS {
namespace Plugin {
class ClipboardProxyImpl final : public ClipboardProxy {
public:
    ClipboardProxyImpl() = default;
    ~ClipboardProxyImpl();

    int32_t SetPasteData(const PasteData& pasteData) override;
    int32_t GetPasteData(PasteData& pasteData, int32_t& realErrorCode) override;
    int Clear() override;

    bool Subscribe(PasteboardObserverType type, sptr<PasteboardObserverStub> callback) override;
    void Unsubscribe(PasteboardObserverType type, sptr<PasteboardObserverStub> callback) override;
    void NotifyObservers() override;

    int DetectPatterns(const std::vector<Pattern>& patternsToCheck, std::vector<Pattern>& funcResult) override;
    ErrCode HasPasteData(bool& funcResult) override;
    ErrCode HasDataType(const std::string& mimeType, bool& funcResult) override;
    ErrCode GetMimeTypes(std::vector<std::string>& funcResult) override;
    ErrCode GetChangeCount(uint32_t& changeCount) override;

private:
    std::mutex mutex_;
};
} // namespace Plugin
} // namespace OHOS
#endif // PLUGINS_PASTEBOARD_PLATFORM_IOS_CLIPBOARD_PROXY_IMPL_H