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

#ifndef PLUGINS_WEB_WEBVIEW_WEB_STORAGE_H
#define PLUGINS_WEB_WEBVIEW_WEB_STORAGE_H

#include <map>
#include <any>
#include <memory>
#include <vector>

#include "napi_parse_utils.h"
#include "web_errors.h"
#include "web_storage_callback.h"
#include "plugins/web/webview/android/java/jni/web_storage_struct.h"

namespace OHOS::Plugin {

class WebStorage {
public:
    WebStorage() = default;
    
    virtual ~WebStorage() = default;
    static void GetOriginQuota(const std::string& url, int32_t asyncCallbackInfoId);
    static void GetOriginUsage(const std::string& url, int32_t asyncCallbackInfoId);
    static void GetOrigins(int32_t asyncCallbackInfoId);
    static void DeleteAllData();
    static void DeleteOrigin(const std::string& origin);
    static void OnGetOriginQuotaReceiveValue(const int64_t& result, int32_t asyncCallbackInfoId);
    static void OnGetOriginUsageReceiveValue(const int32_t& result, int32_t asyncCallbackInfoId);
    static void OnGetOriginsReceiveValue(const std::vector<WebStorageStruct>& result, int32_t asyncCallbackInfoId);

    static void InsertCallbackInfo(
        const std::shared_ptr<AsyncWebStorageResultCallbackInfo>& asyncCallbackInfo, StorageTaskType storageTaskType);
    static bool EraseCallbackInfo(
        const AsyncWebStorageResultCallbackInfo* asyncCallbackInfo, StorageTaskType storageTaskType);
private:
    static bool ExcuteAsyncCallbackInfo(const int64_t& result, int32_t asyncCallbackInfoId);
    static bool ExcuteAsyncCallbackInfo(const int32_t& result, int32_t asyncCallbackInfoId);
    static bool ExcuteAsyncCallbackInfo(const std::vector<WebStorageStruct>& result, int32_t asyncCallbackInfoId);
    static thread_local std::vector<std::shared_ptr<AsyncWebStorageResultCallbackInfo>>
        getOriginQuotaCallbackInfoContainer_;
    static thread_local std::vector<std::shared_ptr<AsyncWebStorageResultCallbackInfo>>
        getOriginUsageCallbackInfoContainer_;
    static thread_local std::vector<std::shared_ptr<AsyncWebStorageResultCallbackInfo>>
        getOriginsCallbackInfoContainer_;
  
};
} // namespace OHOS::Plugin

#endif // PLUGINS_WEB_WEBVIEW_WEB_WEBVIEW_STORAGE_H
