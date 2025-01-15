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

#ifndef PLUGINS_WEB_WEBVIEW_GEOLOCATION_PERMISSIONS_CALLBACK_H
#define PLUGINS_WEB_WEBVIEW_GEOLOCATION_PERMISSIONS_CALLBACK_H

#include "plugins/web/webview/napi_parse_utils.h"
#include "web_errors.h"

namespace OHOS::Plugin {
enum class GeolocationPermissionsTaskType { NONE, GET_ACCESSIBLE_GEOLOCATION, GET_STORED_GEOLOCATION };

struct GeolocationPermissionsData {
    explicit GeolocationPermissionsData(napi_env napiEnv);
    virtual ~GeolocationPermissionsData();
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;
};

struct GeolocationPermissionsResultCallbackInfo : public GeolocationPermissionsData {
public:
    GeolocationPermissionsResultCallbackInfo(napi_env env, int32_t id): GeolocationPermissionsData(env), uniqueId_(id){}
    int errCode = 0;
    bool result = false;
    std::vector<std::string> originsArray;
    GeolocationPermissionsTaskType taskType = GeolocationPermissionsTaskType::NONE;
    int32_t GetUniqueId() const
    {
        return uniqueId_;
    }

private:
    int32_t uniqueId_;
};
}

#endif // PLUGINS_WEB_WEBVIEW_GEOLOCATION_PERMISSIONS_CALLBACK_H