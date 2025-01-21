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

#ifndef PLUGIN_WEB_WEBVIEW_WEBVIEW_JAVASCRIPT_ASYNC_WORK_CALLBACK_H
#define PLUGIN_WEB_WEBVIEW_WEBVIEW_JAVASCRIPT_ASYNC_WORK_CALLBACK_H

#include "napi_parse_utils.h"
#include "web_errors.h"

namespace OHOS::Plugin {

enum class JsMessageType : int {
    NOTSUPPORT = 0,
    STRING,
    NUMBER,
    BOOLEAN,
    ARRAYBUFFER,
    ARRAY
};

class NapiJsMessageExt {
public:
    NapiJsMessageExt() = default;
    ~NapiJsMessageExt() = default;

    static napi_value JsConstructor(napi_env env, napi_callback_info info);
    static napi_value GetType(napi_env env, napi_callback_info info);
    static napi_value GetString(napi_env env, napi_callback_info info);
    static napi_value GetNumber(napi_env env, napi_callback_info info);
    static napi_value GetBoolean(napi_env env, napi_callback_info info);
    static napi_value GetArrayBuffer(napi_env env, napi_callback_info info);
    static napi_value GetArray(napi_env env, napi_callback_info info);
};

class WebJsMessageExt {
public:
    explicit WebJsMessageExt(std::shared_ptr<WebMessage> value) : value_(value) {};
    ~WebJsMessageExt() = default;

    int32_t ConvertToJsType(WebValue::Type type);
    int GetType();
    std::string GetString();
    double GetNumber();
    bool GetBoolean();
    std::shared_ptr<WebMessage> GetJsMsgResult()
    {
        return value_;
    }

private:
    std::shared_ptr<WebMessage> value_ = nullptr;
};

struct AsyncJavaScriptExtWorkData {
    explicit AsyncJavaScriptExtWorkData(napi_env napiEnv);
    virtual ~AsyncJavaScriptExtWorkData();
    napi_env env = nullptr;
    napi_async_work asyncWork = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callback = nullptr;
};

struct NapiJsRunJavaScriptExtCallBackParam : public AsyncJavaScriptExtWorkData {
    explicit NapiJsRunJavaScriptExtCallBackParam(napi_env env) : AsyncJavaScriptExtWorkData(env) {}
    std::shared_ptr<WebMessage> result_;
};

struct AsyncJavaScriptExtEvaluteJSResultCallbackInfo : public AsyncJavaScriptExtWorkData {
public:
    explicit AsyncJavaScriptExtEvaluteJSResultCallbackInfo(napi_env env, int32_t id)
        : AsyncJavaScriptExtWorkData(env), uniqueId_(id) {}
    std::shared_ptr<WebMessage> result_;
    int32_t GetUniqueId() const
    {
        return uniqueId_;
    }

private:
    int32_t uniqueId_;

public:
    static void InitJSExcute(napi_env env, napi_value exports, napi_ref& jsMsgExtClassRef);
};
} // namespace OHOS::Plugin
#endif // PLUGIN_WEB_WEBVIEW_WEBVIEW_JAVASCRIPT_ASYNC_WORK_CALLBACK_H