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

#ifndef PLUGIN_WEB_WEBVIEW_JS_WEB_WEBVIEW_H
#define PLUGIN_WEB_WEBVIEW_JS_WEB_WEBVIEW_H

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "webview_controller.h"

namespace OHOS::Plugin {
enum class ResourceType : uint32_t {
    COLOR = 10001,
    FLOAT,
    STRING,
    PLURAL,
    BOOLEAN,
    INTARRAY,
    INTEGER,
    PATTERN,
    STRARRAY,
    MEDIA = 20000,
    RAWFILE = 30000
};

const std::string WEBVIEW_CONTROLLER_CLASS_NAME = "WebviewController";

class NapiWebviewController {
public:
    NapiWebviewController() {}
    ~NapiWebviewController() = default;

    static napi_value Init(napi_env env, napi_value exports);
    
    static std::string appHapPath_;
private:
    static napi_value JsConstructor(napi_env env, napi_callback_info info);

    static napi_value SetWebId(napi_env env, napi_callback_info info);

    static napi_value InnerSetHapPath(napi_env env, napi_callback_info info);

    static napi_value LoadUrl(napi_env env, napi_callback_info info);

    static napi_value LoadUrlWithHttpHeaders(napi_env env, napi_callback_info info, const std::string& url,
        const napi_value* argv, WebviewController* webviewController);

    static napi_value LoadData(napi_env env, napi_callback_info info);

    static napi_value GetUrl(napi_env env, napi_callback_info info);

    static napi_value AccessForward(napi_env env, napi_callback_info info);

    static napi_value AccessBackward(napi_env env, napi_callback_info info);

    static napi_value Forward(napi_env env, napi_callback_info info);

    static napi_value Backward(napi_env env, napi_callback_info info);

    static napi_value Refresh(napi_env env, napi_callback_info info);

    static napi_value RunJavaScript(napi_env env, napi_callback_info info);
};
}
#endif
