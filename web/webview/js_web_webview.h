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

#include <uv.h>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"
#include "web_cookie_manager_callback.h"
#include "web_cookie_manager.h"
#include "webview_controller.h"
#include "web_storage_callback.h"
#include "web_storage.h"

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
const std::string WEB_HISTORY_LIST_CLASS_NAME = "WebHistoryList";
const std::string WEB_MESSAGE_PORT_CLASS_NAME = "WebMessagePort";

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

    static napi_value ScrollTo(napi_env env, napi_callback_info info);

    static napi_value ScrollBy(napi_env env, napi_callback_info info);

    static napi_value Zoom(napi_env env, napi_callback_info info);

    static napi_value Stop(napi_env env, napi_callback_info info);

    static napi_value SetCustomUserAgent(napi_env env, napi_callback_info info);

    static napi_value GetCustomUserAgent(napi_env env, napi_callback_info info);

    static napi_value AccessStep(napi_env env, napi_callback_info info);

    static napi_value ClearHistory(napi_env env, napi_callback_info info);

    static napi_value GetBackForwardEntries(napi_env env, napi_callback_info info);

    static napi_value RemoveCache(napi_env env, napi_callback_info info);

    static napi_value BackOrForward(napi_env env, napi_callback_info info);

    static napi_value GetTitle(napi_env env, napi_callback_info info);

    static napi_value GetPageHeight(napi_env env, napi_callback_info info);

    static napi_value CreateWebMessagePorts(napi_env env, napi_callback_info info);

    static napi_value PostMessage(napi_env env, napi_callback_info info);
};

class NapiWebDataBase {
public:
    NapiWebDataBase() {}
    ~NapiWebDataBase() = default;

    static napi_value Init(napi_env env, napi_value exports);

private:
    static napi_value JsConstructor(napi_env env, napi_callback_info info);

    static napi_value ExistHttpAuthCredentials(napi_env env, napi_callback_info info);

    static napi_value DeleteHttpAuthCredentials(napi_env env, napi_callback_info info);

    static napi_value SaveHttpAuthCredentials(napi_env env, napi_callback_info info);

    static napi_value GetHttpAuthCredentials(napi_env env, napi_callback_info info);
};

const std::string WEB_COOKIE_MANAGER_CLASS_NAME = "WebCookieManager";
class NapiWebCookieManager {
public:
    NapiWebCookieManager() {}

    ~NapiWebCookieManager() = default;

    struct WebCookieManagerParam {
        napi_env env_;
        napi_ref callback_;
        napi_deferred deferred_;
    };

    struct WebFetchCookieManagerParam {
        napi_env env_;
        napi_ref callback_;
        napi_deferred deferred_;
        std::string result_;
    };

    static napi_value Init(napi_env env, napi_value exports);

private:
    static napi_value JsConstructor(napi_env env, napi_callback_info info);

    static napi_value JsFetchCookieAsync(napi_env env, napi_callback_info info);

    static napi_value JsConfigCookieAsync(napi_env env, napi_callback_info info);

    static napi_value JsClearAllCookiesAsync(napi_env env, napi_callback_info info);

    static void CreateCookieAsyncWork(napi_env env, const std::string& taskName,
        const std::shared_ptr<AsyncCookieManagerResultCallbackInfo>& callbackInfo);

    static void CreateFetchCookieAsyncWork(napi_env env,
        const std::shared_ptr<AsyncCookieManagerResultCallbackInfo>& callbackInfo);
};

const std::string WEB_STORAGE_CLASS_NAME = "WebStorage";
class NapiWebStorage {
public:
    NapiWebStorage() {}

    ~NapiWebStorage() = default;

    struct WebStorageParam {
        napi_env env_;
        napi_ref callback_;
        napi_deferred deferred_;
    };


    static napi_value Init(napi_env env, napi_value exports);

private:
    static napi_value JsConstructor(napi_env env, napi_callback_info info);

    static napi_value JsGetOriginQuotaAsync(napi_env env, napi_callback_info info);

    static napi_value JsGetOriginUsageAsync(napi_env env, napi_callback_info info);

    static napi_value JsGetOriginsAsync(napi_env env, napi_callback_info info);

    static napi_value DeleteAllData(napi_env env, napi_callback_info info);

    static napi_value DeleteOrigin(napi_env env, napi_callback_info info);

    static void CreateGetOriginQuotaAsyncWork(napi_env env,
        const std::shared_ptr<AsyncWebStorageResultCallbackInfo>& callbackInfo);

    static void CreateGetOriginUsageAsyncWork(napi_env env,
        const std::shared_ptr<AsyncWebStorageResultCallbackInfo>& callbackInfo);

    static void CreateGetOriginsAsyncWork(napi_env env,
        const std::shared_ptr<AsyncWebStorageResultCallbackInfo>& callbackInfo);
    
    static void GetNapiWebStorageOriginForResult(napi_env env,
        const std::vector<WebStorageStruct> &info, napi_value result);

};

class NapiWebHistoryList {
public:
    NapiWebHistoryList() = default;
    virtual ~NapiWebHistoryList() = default;

    static napi_value JsConstructor(napi_env env, napi_callback_info info);

    static napi_value GetItem(napi_env env, napi_callback_info info);
};

class NapiWebMessagePort {
public:
    NapiWebMessagePort() = default;
    virtual ~NapiWebMessagePort() = default;

    static napi_value JsConstructor(napi_env env, napi_callback_info info);

    static napi_value Close(napi_env env, napi_callback_info info);

    static napi_value PostMessageEvent(napi_env env, napi_callback_info info);

    static napi_value OnMessageEvent(napi_env env, napi_callback_info info);
};
}
#endif
