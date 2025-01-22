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

#ifndef PLUGINS_WEB_WEBVIEW_NAPI_WEB_MESSAGE_EXT_H
#define PLUGINS_WEB_WEBVIEW_NAPI_WEB_MESSAGE_EXT_H

#include <map>
#include <uv.h>

#include "plugins/web/webview/webview_async_work_callback.h"
#include "web_errors.h"

namespace OHOS::Plugin {
napi_valuetype GetArrayValueType(napi_env env, napi_value array, bool& isDouble);
class NapiWebMessageExt {
public:
    NapiWebMessageExt() = default;
    ~NapiWebMessageExt() = default;

    static thread_local napi_ref g_webMsgExtClassRef;

    static napi_value Init(napi_env env, napi_value exports);
    static napi_value JsConstructor(napi_env env, napi_callback_info info);
    static napi_value GetType(napi_env env, napi_callback_info info);
    static napi_value GetString(napi_env env, napi_callback_info info);
    static napi_value GetNumber(napi_env env, napi_callback_info info);
    static napi_value GetBoolean(napi_env env, napi_callback_info info);
    static napi_value GetArrayBuffer(napi_env env, napi_callback_info info);
    static napi_value GetArray(napi_env env, napi_callback_info info);
    static napi_value GetError(napi_env env, napi_callback_info info);
    static napi_value SetType(napi_env env, napi_callback_info info);
    static napi_value SetString(napi_env env, napi_callback_info info);
    static napi_value SetNumber(napi_env env, napi_callback_info info);
    static napi_value SetBoolean(napi_env env, napi_callback_info info);
    static napi_value SetArrayBuffer(napi_env env, napi_callback_info info);
    static napi_value SetArray(napi_env env, napi_callback_info info);
    static napi_value SetError(napi_env env, napi_callback_info info);
};
} // namespace OHOS::Plugin
#endif // PLUGINS_WEB_WEBVIEW_NAPI_WEB_MESSAGE_EXT_H