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
        return nullptr;
    }
    return webDownloadDelegate;
}

void AssignWebDownloadManagerByObject(int32_t webId, WebDownloadItem* webDownloadItem,
    const std::shared_ptr<AceWebDownloadImpl> webDownloadImpl)
{
    if (webDownloadItem == nullptr) {
        LOGE("webDownloadItem or object is nullptr can't assign");
        return;
    }
    std::string guid = webDownloadImpl->GetGuid();
    webDownloadItem->guid = guid;
    webDownloadItem->currentSpeed = webDownloadImpl->GetCurrentSpeed();
    webDownloadItem->percentComplete = webDownloadImpl->GetPercentComplete();
    webDownloadItem->totalBytes = webDownloadImpl->GetTotalBytes();
    webDownloadItem->state = static_cast<PluginWebDownloadItemState>(webDownloadImpl->GetState());
    webDownloadItem->lastErrorCode = webDownloadImpl->GetLastErrorCode();
    webDownloadItem->method = webDownloadImpl->GetMethod();
    webDownloadItem->mimeType = webDownloadImpl->GetMimeType();
    webDownloadItem->url = webDownloadImpl->GetUrl();
    webDownloadItem->suggestedFileName = webDownloadImpl->GetSuggestedFileName();
    webDownloadItem->receivedBytes = webDownloadImpl->GetReceivedBytes();
    webDownloadItem->fullPath = webDownloadImpl->GetFullPath();

    webDownloadItem->beforeCallback = [webId, guid](const std::string path)->void {
        webDownloadItemStartOC(webId, guid, path);
    };
    webDownloadItem->downloadCancelCallback = [webId, guid]()->void {
        webDownloadItemCancelOC(webId, guid);
    };
    webDownloadItem->downloadPauseCallback = [webId, guid]()->int {
        webDownloadItemPauseOC(webId, guid);
        return 0;
    };
    webDownloadItem->downloadResumeCallback = [webId, guid]()->int {
        webDownloadItemResumeOC(webId, guid);
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