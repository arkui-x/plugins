/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGIN_WEB_WEBVIEW_WEBVIEW_COOKIE_MANAGER_CALLBACK_H
#define PLUGIN_WEB_WEBVIEW_WEBVIEW_COOKIE_MANAGER_CALLBACK_H

#include "plugins/web/webview/napi_parse_utils.h"
#include "web_errors.h"

namespace OHOS::Plugin {
enum class TaskType {
    NONE,
    CONFIG_COOKIE,
    FETCH_COOKIE,
    CLEAR_ALL_COOKIES,
    CLEAR_SESSION_COOKIE
};

struct AsyncWorkCookieData {
    explicit AsyncWorkCookieData(napi_env napiEnv);
    virtual ~AsyncWorkCookieData();
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;
};

struct AsyncCookieManagerResultCallbackInfo : public AsyncWorkCookieData {
public:
    AsyncCookieManagerResultCallbackInfo(napi_env env, int32_t id) : AsyncWorkCookieData(env), uniqueId_(id) {}
    std::string result;
    TaskType taskType = TaskType::NONE;
    int32_t GetUniqueId() const
    {
        return uniqueId_;
    }

private:
    int32_t uniqueId_;
};
}

#endif // PLUGIN_WEB_WEBVIEW_WEBVIEW_COOKIE_MANAGER_CALLBACK_H