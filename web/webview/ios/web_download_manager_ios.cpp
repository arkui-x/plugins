/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "web_download_manager_ios.h"
#include "AceWebControllerBridge.h"
#include "AceWebDownloadImpl.h"
#include "log.h"

namespace OHOS::Plugin {
WebDownloadDelegate* GetDownloadDelegate(int32_t webId) {
    WebDownloadDelegate* webDownloadDelegate = WebDownloadManager::GetWebDownloadDelegate(webId);
    if (!webDownloadDelegate) {
        LOGW("can't found delegate for nweb.");
        webDownloadDelegate = WebDownloadManager::GetWebDownloadDefaultDelegate();
        if (!webDownloadDelegate) {
            return nullptr;
        }
    }
    return webDownloadDelegate;
}

void UpdateDownloadItem(int32_t webId, WebDownloadItem* webDownloadItem) {
    WebDownloadDelegate* webDownloadDelegate = GetDownloadDelegate(webId);
    if (webDownloadDelegate) {
        webDownloadDelegate->DownloadDidUpdate(webDownloadItem);
    }
}

void AssignWebDownloadManagerByObject(int32_t webId, WebDownloadItem* webDownloadItem,
    const std::shared_ptr<AceWebDownloadImpl> webDownloadImpl)
{
    if(webDownloadItem == nullptr) {
        LOGE("webDownloadItem or object is nullptr can't assign");
        return;
    }
    std::string guid = webDownloadImpl->GetGuid();
    webDownloadItem->guid = guid;
    webDownloadItem->currentSpeed = webDownloadImpl->GetCurrentSpeed();
    webDownloadItem->percentComplete = webDownloadImpl->GetPercentComplete();
    webDownloadItem->totalBytes = webDownloadImpl->GetTotalBytes();
    webDownloadItem->state = static_cast<PluginWebDownloadItemState>(webDownloadImpl->GetState());
    webDownloadItem->lastErrorCode = 0;
    webDownloadItem->method = webDownloadImpl->GetMethod();
    webDownloadItem->mimeType = webDownloadImpl->GetMimeType();
    webDownloadItem->url = webDownloadImpl->GetUrl();
    webDownloadItem->suggestedFileName = webDownloadImpl->GetSuggestedFileName();
    webDownloadItem->receivedBytes = webDownloadImpl->GetReceivedBytes();
    webDownloadItem->fullPath = webDownloadImpl->GetFullPath();

    webDownloadItem->beforeCallback = [webId, guid, webDownloadItem](const std::string path)->void {
        webDownloadItemStartOC(webId, guid, path);
        webDownloadItem->state = PluginWebDownloadItemState::PENDING;
        UpdateDownloadItem(webId, webDownloadItem);
    };
    webDownloadItem->downloadCancelCallback = [webId, guid, webDownloadItem]()->void {
        webDownloadItemCancelOC(webId, guid);
        webDownloadItem->lastErrorCode = download::DOWNLOAD_INTERRUPT_REASON_USER_CANCELED;
        webDownloadItem->state = PluginWebDownloadItemState::CANCELED;
        UpdateDownloadItem(webId, webDownloadItem);
    };
    webDownloadItem->downloadPauseCallback = [webId, guid, webDownloadItem]()->int {
        webDownloadItemPauseOC(webId, guid);
        webDownloadItem->state = PluginWebDownloadItemState::PAUSED;
        UpdateDownloadItem(webId, webDownloadItem);
        return 0;
    };
    webDownloadItem->downloadResumeCallback = [webId, guid, webDownloadItem]()->int {
        webDownloadItemResumeOC(webId, guid);
        webDownloadItem->state = PluginWebDownloadItemState::PENDING;
        UpdateDownloadItem(webId, webDownloadItem);
        return 0;
    };
}

void OnDownloadBeforeStart(int32_t webId, const std::shared_ptr<AceWebDownloadImpl> webDownloadImpl)
{
    WebDownloadDelegate* webDownloadDelegate = GetDownloadDelegate(webId);
    if (webDownloadDelegate) {
        WebDownloadItem* webDownloadItem = new(std::nothrow) WebDownloadItem(webDownloadDelegate->GetEnv());
        if (!webDownloadItem) {
            LOGE("webDownloadItem is null");
            return;
        }
        AssignWebDownloadManagerByObject(webId, webDownloadItem, webDownloadImpl);
        webDownloadDelegate->DownloadBeforeStart(webDownloadItem);
    }
}

void OnDownloadUpdated(int32_t webId, const std::shared_ptr<AceWebDownloadImpl> webDownloadImpl)
{
    WebDownloadDelegate* webDownloadDelegate = GetDownloadDelegate(webId);
    if (webDownloadDelegate) {
        WebDownloadItem* webDownloadItem = new(std::nothrow) WebDownloadItem(webDownloadDelegate->GetEnv());
        if (!webDownloadItem) {
            LOGE("webDownloadItem is null");
            return;
        }
        AssignWebDownloadManagerByObject(webId, webDownloadItem, webDownloadImpl);
        webDownloadDelegate->DownloadDidUpdate(webDownloadItem);
    }
}

void OnDownloadFailed(int32_t webId, const std::shared_ptr<AceWebDownloadImpl> webDownloadImpl)
{
    WebDownloadDelegate* webDownloadDelegate = GetDownloadDelegate(webId);
    if (webDownloadDelegate) {
        WebDownloadItem* webDownloadItem = new(std::nothrow) WebDownloadItem(webDownloadDelegate->GetEnv());
        if (!webDownloadItem) {
            LOGE("webDownloadItem is null");
            return;
        }
        AssignWebDownloadManagerByObject(webId, webDownloadItem, webDownloadImpl);
        webDownloadDelegate->DownloadDidFail(webDownloadItem);
    }
}

void OnDownloadFinish(int32_t webId, const std::shared_ptr<AceWebDownloadImpl> webDownloadImpl)
{
    WebDownloadDelegate* webDownloadDelegate = GetDownloadDelegate(webId);
    if (webDownloadDelegate) {
        WebDownloadItem* webDownloadItem = new(std::nothrow) WebDownloadItem(webDownloadDelegate->GetEnv());
        if (!webDownloadItem) {
            LOGE("webDownloadItem is null");
            return;
        }
        AssignWebDownloadManagerByObject(webId, webDownloadItem, webDownloadImpl);
        webDownloadDelegate->DownloadDidFinish(webDownloadItem);
    }
}

void WebDownloadManagerIOS::RegisterDownloadCallback(int32_t webId)
{
    onDownloadBeforeStartOC(webId, OnDownloadBeforeStart);
    onDownloadUpdatedOC(webId, OnDownloadUpdated);
    onDownloadFailedOC(webId, OnDownloadFailed);
    onDownloadFinishOC(webId, OnDownloadFinish);
}
} // namespace OHOS::Plugin