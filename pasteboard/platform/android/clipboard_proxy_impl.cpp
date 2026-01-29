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

#include "clipboard_proxy_impl.h"

#include <thread>

#include "clipboard_jni.h"
#include "log.h"
#include "pasteboard_error.h"

using namespace OHOS::MiscServices;

namespace OHOS {
namespace Plugin {

int32_t ClipboardProxyImpl::SetPasteData(const PasteData& pasteData)
{
    return ClipboardJni::SetData(pasteData);
}

int32_t ClipboardProxyImpl::GetPasteData(PasteData& pasteData, int32_t& realErrorCode)
{
    LOGI("ClipboardProxyImpl::GetPasteData() has been called");
    realErrorCode = ClipboardJni::GetData(pasteData);
    return 0;
}

int ClipboardProxyImpl::Clear()
{
    auto ret = ClipboardJni::Clear();
    if (!ret) {
        LOGE("ClipboardJni::Clear failed");
        return static_cast<int>(PasteboardError::INVALID_OPERATION_ERROR);
    }
    return ERR_OK;
}

bool ClipboardProxyImpl::Subscribe(PasteboardObserverType type, sptr<PasteboardObserverStub> callback)
{
    if (type != PasteboardObserverType::OBSERVER_LOCAL) {
        LOGE("Unsupported observer type: %{public}d", static_cast<int>(type));
        return false;
    }
    std::lock_guard<std::mutex> lock(observerMutex_);
    observerSet_.emplace(callback);

    auto ret = ClipboardJni::Subscribe();
    if (!ret) {
        observerSet_.erase(callback);
        LOGE("Failed to subscribe to clipboard events");
        return false;
    }
    return ret;
}

void ClipboardProxyImpl::NotifyObservers()
{
    std::thread thread([this]() {
        std::lock_guard<std::mutex> lock(observerMutex_);
        for (auto& observer : observerSet_) {
            if (observer == nullptr) {
                LOGE("observerSet_ contains nullptr");
                continue;
            }
            observer->OnPasteboardChanged();
        }
    });
    thread.detach();
}

void ClipboardProxyImpl::Unsubscribe(PasteboardObserverType type, sptr<PasteboardObserverStub> callback)
{
    if (type != PasteboardObserverType::OBSERVER_LOCAL) {
        LOGE("Unsupported observer type: %{public}d", static_cast<int>(type));
        return;
    }
    {
        std::lock_guard<std::mutex> lock(observerMutex_);
        observerSet_.erase(callback);
    }
    auto ret = ClipboardJni::Unsubscribe();
    if (!ret) {
        LOGE("Failed to unsubscribe from clipboard events");
        return;
    }
}

int ClipboardProxyImpl::DetectPatterns(const std::vector<Pattern>& patternsToCheck, std::vector<Pattern>& funcResult)
{
    return ClipboardJni::DetectPatterns(patternsToCheck, funcResult);
}

ErrCode ClipboardProxyImpl::HasPasteData(bool& funcResult)
{
    auto ret = ClipboardJni::HasPasteData();
    funcResult = ret;
    if (!ret) {
        LOGE("ClipboardJni::HasPasteData failed");
        return static_cast<int>(PasteboardError::INVALID_OPERATION_ERROR);
    }
    return ERR_OK;
}

ErrCode ClipboardProxyImpl::HasDataType(const std::string& mimeType, bool& funcResult)
{
    auto ret = ClipboardJni::HasDataType(mimeType);
    funcResult = ret;
    if (!ret) {
        LOGE("ClipboardJni::HasDataType failed");
        return static_cast<int>(PasteboardError::INVALID_OPERATION_ERROR);
    }
    return ERR_OK;
}

ErrCode ClipboardProxyImpl::GetMimeTypes(std::vector<std::string>& funcResult)
{
    auto ret = ClipboardJni::GetMimeTypes(funcResult);
    if (!ret) {
        LOGE("ClipboardJni::GetMimeTypes failed");
        return static_cast<int>(PasteboardError::INVALID_OPERATION_ERROR);
    }
    return ERR_OK;
}

ErrCode ClipboardProxyImpl::GetChangeCount(uint32_t& changeCount)
{
    LOGW("ClipboardProxyImpl::GetChangeCount Platform unsupported");
    return static_cast<int>(PasteboardError::NOT_SUPPORT);
}
} // namespace Plugin
} // namespace OHOS