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

#ifndef PLUGIN_WEB_WEBVIEW_WEBVIEW_STORAGE_CALLBACK_H
#define PLUGIN_WEB_WEBVIEW_WEBVIEW_STORAGE_CALLBACK_H

#include "plugins/web/webview/napi_parse_utils.h"
#include "web_errors.h"
#include "plugins/web/webview/android/java/jni/web_storage_struct.h"

namespace OHOS::Plugin {
enum class StorageTaskType {
    NONE,
    GET_ORIGINQUOTA,
    GET_ORIGINUSAGE,
    GET_ORIGINS
};

struct AsyncWorkStorageData {
    explicit AsyncWorkStorageData(napi_env napiEnv);
    virtual ~AsyncWorkStorageData();
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;
};

struct AsyncWebStorageResultCallbackInfo : public AsyncWorkStorageData {
public:
    AsyncWebStorageResultCallbackInfo(napi_env env, int32_t id) : AsyncWorkStorageData(env), result(0), resultVector(), storageTaskType(StorageTaskType::NONE), uniqueId_(id) {}
    int64_t result;
    std::vector<WebStorageStruct> resultVector;
    StorageTaskType storageTaskType = StorageTaskType::NONE;
    int32_t GetUniqueId() const
    {
        return uniqueId_;
    }

private:
    int32_t uniqueId_;
};
}

#endif // PLUGIN_WEB_WEBVIEW_WEBVIEW_STORAGE_CALLBACK_H
