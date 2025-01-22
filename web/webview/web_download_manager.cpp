/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "web_download_manager.h"

#include <cstring>
#include <unordered_map>

#include "log.h"
#include "web_download_delegate.h"
#ifdef ANDROID_PLATFORM
#include "android/java/jni/web_download_item_jni.h"
#include "android/java/jni/web_download_manager_android.h"
#endif
#ifdef IOS_PLATFORM
#include "ios/web_download_manager_ios.h"
#endif

namespace OHOS {
namespace Plugin {
CallbackMap WebDownloadManager::g_callback;
std::unique_ptr<WebDownloadDelegate> WebDownloadManager::g_default_delegate;
std::unordered_map<int32_t, WebDownloadDelegate*> WebDownloadManager::g_web_download_delegate_map;

WebDownloadDelegate* WebDownloadManager::GetWebDownloadDelegate(int32_t nwebId)
{
    auto it = g_web_download_delegate_map.find(nwebId);
    if (it != g_web_download_delegate_map.end()) {
        return it->second;
    }
    return nullptr;
}

WebDownloadDelegate* WebDownloadManager::GetWebDownloadDefaultDelegate()
{
    return g_default_delegate.get();
}

void WebDownloadManager::OnBeforeDownload(int32_t webId, void* object)
{
    if (!g_callback.empty() && g_callback[ON_DOWNLOAD_BEFORE_START] != nullptr) {
        g_callback[ON_DOWNLOAD_BEFORE_START](webId, object);
    }
}

void WebDownloadManager::OnUpdatedDownload(int32_t webId, void* object)
{
    if (!g_callback.empty() && g_callback[ON_DOWNLOAD_UPDATED] != nullptr) {
        g_callback[ON_DOWNLOAD_UPDATED](webId, object);
    }
}

void WebDownloadManager::OnFailedDownload(int32_t webId, void* object)
{
    if (!g_callback.empty() && g_callback[ON_DOWNLOAD_FAILED] != nullptr) {
        g_callback[ON_DOWNLOAD_FAILED](webId, object);
    }
}

void WebDownloadManager::OnFinishDownload(int32_t webId, void* object)
{
    if (!g_callback.empty() && g_callback[ON_DOWNLOAD_FINISH] != nullptr) {
        g_callback[ON_DOWNLOAD_FINISH](webId, object);
    }
}

void WebDownloadManager::RegisterDownloadCallback(int32_t webId)
{
#ifdef ANDROID_PLATFORM
    if (g_callback.empty()) {
        WebDownloadManagerAndroid::RegisterDownloadCallback(g_callback);
    } else {
        LOGI("WebDownloadManager had Registered DownloadCallback");
    }
#endif
#ifdef IOS_PLATFORM
    WebDownloadManagerIOS::RegisterDownloadCallback(webId);
#endif
}

void WebDownloadManager::RemoveDownloadDelegate(WebDownloadDelegate* delegate)
{
    auto iterator = g_web_download_delegate_map.begin();
    while (iterator != g_web_download_delegate_map.end()) {
        if (iterator->second == delegate) {
            iterator = g_web_download_delegate_map.erase(iterator);
        } else {
            iterator++;
        }
    }
}

void WebDownloadManager::AddDownloadDelegateForWeb(int32_t nwebId, WebDownloadDelegate* delegate)
{
    g_web_download_delegate_map.insert_or_assign(nwebId, delegate);
    RegisterDownloadCallback(nwebId);
}

void WebDownloadManager::RemoveDownloadDelegateRef(int32_t nwebId)
{
    auto iter = g_web_download_delegate_map.find(nwebId);
    if (iter != g_web_download_delegate_map.end()) {
        iter->second->RemoveSelfRef();
    }
}

void WebDownloadManager::SetDownloadDelegate(WebDownloadDelegate* delegate)
{
    if (!g_default_delegate) {
        g_default_delegate = std::make_unique<WebDownloadDelegate>(*delegate);
        RegisterDownloadCallback();
    }
}
} // namespace Plugin
} // namespace OHOS
