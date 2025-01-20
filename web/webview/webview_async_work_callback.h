/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef PLUGIN_WEB_WEBVIEW_WEBVIEW_ASYNC_WORK_CALLBACK_H
#define PLUGIN_WEB_WEBVIEW_WEBVIEW_ASYNC_WORK_CALLBACK_H

#include "napi_parse_utils.h"
#include "plugins/web/webview/web_message_ext.h"
#include "web_errors.h"

namespace OHOS::Plugin {
struct AsyncWorkData {
    explicit AsyncWorkData(napi_env napiEnv);
    virtual ~AsyncWorkData();
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;
};

struct NapiJsCallBackParm : public AsyncWorkData {
    explicit NapiJsCallBackParm(napi_env env) : AsyncWorkData(env) {}
    std::string result_ = "";
};

struct NapiJsCallBackParmExt : public AsyncWorkData {
    explicit NapiJsCallBackParmExt(napi_env env) : AsyncWorkData(env) {}
    std::shared_ptr<WebMessage> webMessage_ = std::make_shared<WebMessage>(WebValue::Type::STRING);
    std::shared_ptr<WebMessageExt> result_ = std::make_shared<WebMessageExt>(webMessage_);
};

struct AsyncEvaluteJSResultCallbackInfo : public AsyncWorkData {
public:
    explicit AsyncEvaluteJSResultCallbackInfo(napi_env env, int32_t id) : AsyncWorkData(env), uniqueId_(id) {}
    std::string result;
    int32_t GetUniqueId() const
    {
        return uniqueId_;
    }
private:
    int32_t uniqueId_;
};
}
#endif // PLUGIN_WEB_WEBVIEW_WEBVIEW_ASYNC_WORK_CALLBACK_H
