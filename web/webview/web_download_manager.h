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

#ifndef PLUGIN_WEB_DOWNLOAD_MANAGER_H
#define PLUGIN_WEB_DOWNLOAD_MANAGER_H

#include <map>
#include <string>
#include <unordered_map>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "web_download_delegate.h"
#include "web_download_item.h"

namespace OHOS {
namespace Plugin {

using CallbackMap = std::map<std::string, std::function<void(int32_t, void*)>>;
const std::string ON_DOWNLOAD_BEFORE_START = "onDownloadBeforeStart";
const std::string ON_DOWNLOAD_UPDATED = "onDownloadUpdated";
const std::string ON_DOWNLOAD_FAILED = "onDownloadFailed";
const std::string ON_DOWNLOAD_FINISH = "onDownloadFinish";

class WebDownloadDelegate;

class WebDownloadManager {
public:
    static void SetDownloadDelegate(WebDownloadDelegate* delegate);
    static void AddDownloadDelegateForWeb(int32_t nwebId, WebDownloadDelegate* delegate);
    static void RemoveDownloadDelegate(WebDownloadDelegate* delegate);
    static void RemoveDownloadDelegateRef(int32_t nwebId);

    WebDownloadManager() = default;
    ~WebDownloadManager() = default;

    static void OnBeforeDownload(int32_t webId, void* object);
    static void OnUpdatedDownload(int32_t webId, void* object);
    static void OnFailedDownload(int32_t webId, void* object);
    static void OnFinishDownload(int32_t webId, void* object);

    static WebDownloadDelegate* GetWebDownloadDelegate(int32_t nwebId);
    static WebDownloadDelegate* GetWebDownloadDefaultDelegate();

private:
    static void RegisterDownloadCallback(int32_t webId = -1);
    static CallbackMap g_callback;
    static std::unique_ptr<WebDownloadDelegate> g_default_delegate;
    static std::unordered_map<int32_t, WebDownloadDelegate*> g_web_download_delegate_map;
};
} // namespace Plugin
} // namespace OHOS
#endif // PLUGIN_WEB_DOWNLOAD_MANAGER_H
