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

#include "web_storage_android.h"

#include "log.h"
#include "plugins/web/webview/android/java/jni/web_storage_jni.h"

using namespace OHOS::NWebError;

namespace OHOS::Plugin {

void WebStorageAndroid::GetOriginQuota(const std::string& origin, int32_t asyncCallbackInfoId)
{
    WebStorageJni::GetOriginQuota(origin, asyncCallbackInfoId);
}

void WebStorageAndroid::GetOriginUsage(const std::string& origin, int32_t asyncCallbackInfoId)
{
    WebStorageJni::GetOriginUsage(origin, asyncCallbackInfoId);
}

void WebStorageAndroid::GetOrigins(int32_t asyncCallbackInfoId)
{
    WebStorageJni::GetOrigins(asyncCallbackInfoId);
}

void WebStorageAndroid::DeleteAllData()
{
    WebStorageJni::DeleteAllData();
}

void WebStorageAndroid::DeleteOrigin(const std::string& origin)
{
    WebStorageJni::DeleteOrigin(origin);
}

} // namespace OHOS::Plugin