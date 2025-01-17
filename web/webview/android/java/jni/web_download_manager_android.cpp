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

#include "web_download_manager_android.h"

#include "log.h"
#include "web_download_item_jni.h"

namespace OHOS::Plugin {
void AssignWebDownloadManagerByObject(int32_t webId, WebDownloadItem* webDownloadItem, void* object)
{
    if (object == nullptr || webDownloadItem == nullptr) {
        LOGE("webDownloadItem or object is nullptr can't assign");
        return;
    }

    std::string guid = WebDownloadItemJni::GetGuid(object);
    webDownloadItem->guid = guid;
    webDownloadItem->currentSpeed = WebDownloadItemJni::GetCurrentSpeed(object);
    webDownloadItem->percentComplete = WebDownloadItemJni::GetPercentComplete(object);
    webDownloadItem->totalBytes = WebDownloadItemJni::GetTotalBytes(object);
    webDownloadItem->state = static_cast<PluginWebDownloadItemState>(WebDownloadItemJni::GetState(object));
    webDownloadItem->lastErrorCode = WebDownloadItemJni::GetLastErrorCode(object);
    webDownloadItem->method = WebDownloadItemJni::GetMethod(object);
    webDownloadItem->mimeType = WebDownloadItemJni::GetMimeType(object);
    webDownloadItem->url = WebDownloadItemJni::GetUrl(object);
    webDownloadItem->suggestedFileName = WebDownloadItemJni::GetSuggestedFileName(object);
    webDownloadItem->receivedBytes = WebDownloadItemJni::GetReceivedBytes(object);
    webDownloadItem->fullPath = WebDownloadItemJni::GetFullPath(object);

    webDownloadItem->beforeCallback = [webId, guid](const std::string path)->void {
        WebDownloadItemJni::Start(webId, guid, path);
    };
    webDownloadItem->downloadCancelCallback = [webId, guid]()->void {
        WebDownloadItemJni::Cancel(webId, guid);
    };
}

WebDownloadDelegate* GetDownloadDelegate(int32_t webId) 
{
    WebDownloadDelegate* webDownloadDelegate = WebDownloadManager::GetWebDownloadDelegate(webId);
    if (!webDownloadDelegate) {
        LOGW("can't found delegate for nweb.");
        webDownloadDelegate = WebDownloadManager::GetWebDownloadDefaultDelegate();
    } 
    return webDownloadDelegate;
}

void OnDownloadBeforeStart(int32_t webId, void* object)
{
    if (object == nullptr) {
        LOGE("onDownloadBeforeStart. object is nullptr");
        return;
    }
    WebDownloadDelegate* webDownloadDelegate = GetDownloadDelegate(webId);
    if (!webDownloadDelegate) {
        LOGE("webDownloadDelegate is null");
        return;
    }

    WebDownloadItem* webDownloadItem = new (std::nothrow) WebDownloadItem(webDownloadDelegate->GetEnv());
    if (!webDownloadItem) {
        LOGE("webDownloadItem is null");
        return;
    }
    AssignWebDownloadManagerByObject(webId, webDownloadItem, object);
    webDownloadDelegate->DownloadBeforeStart(webDownloadItem);
}

void OnDownloadUpdated(int32_t webId, void* object)
{
    if (object == nullptr) {
        LOGE("OnDownloadUpdated. object is nullptr");
        return;
    }
    WebDownloadDelegate* webDownloadDelegate = GetDownloadDelegate(webId);
    if (!webDownloadDelegate) {
        LOGE("webDownloadDelegate is null");
        return;
    }

    WebDownloadItem* webDownloadItem = new (std::nothrow) WebDownloadItem(webDownloadDelegate->GetEnv());
    if (!webDownloadItem) {
        LOGE("webDownloadItem is null");
        return;
    }
    AssignWebDownloadManagerByObject(webId, webDownloadItem, object);
    webDownloadDelegate->DownloadDidUpdate(webDownloadItem);
}

void OnDownloadFailed(int32_t webId, void* object)
{
    if (object == nullptr) {
        LOGE("OnDownloadFailed. object is nullptr");
        return;
    }
    WebDownloadDelegate* webDownloadDelegate = GetDownloadDelegate(webId);
    if (!webDownloadDelegate) {
        LOGE("webDownloadDelegate is null");
        return;
    }

    WebDownloadItem* webDownloadItem = new (std::nothrow) WebDownloadItem(webDownloadDelegate->GetEnv());
    if (!webDownloadItem) {
        LOGE("webDownloadItem is null");
        return;
    }
    AssignWebDownloadManagerByObject(webId, webDownloadItem, object);
    webDownloadDelegate->DownloadDidFail(webDownloadItem);
}

void OnDownloadFinish(int32_t webId, void* object)
{
    if (object == nullptr) {
        LOGE("OnDownloadFinish. object is nullptr");
        return;
    }
    WebDownloadDelegate* webDownloadDelegate = GetDownloadDelegate(webId);
    if (!webDownloadDelegate) {
        LOGE("webDownloadDelegate is null");
        return;
    }

    WebDownloadItem* webDownloadItem = new (std::nothrow) WebDownloadItem(webDownloadDelegate->GetEnv());
    if (!webDownloadItem) {
        LOGE("webDownloadItem is null");
        return;
    }
    AssignWebDownloadManagerByObject(webId, webDownloadItem, object);
    webDownloadDelegate->DownloadDidFinish(webDownloadItem);
}

void WebDownloadManagerAndroid::RegisterDownloadCallback(CallbackMap& callbacks)
{
    callbacks.emplace(ON_DOWNLOAD_BEFORE_START, OnDownloadBeforeStart);
    callbacks.emplace(ON_DOWNLOAD_UPDATED, OnDownloadUpdated);
    callbacks.emplace(ON_DOWNLOAD_FAILED, OnDownloadFailed);
    callbacks.emplace(ON_DOWNLOAD_FINISH, OnDownloadFinish);
}
}
