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


#ifndef PLUGIN_WEB_WEBVIEW_NAPI_WEB_SCHEME_HANDLER_REQUEST_H
#define PLUGIN_WEB_WEBVIEW_NAPI_WEB_SCHEME_HANDLER_REQUEST_H

#include <string>
#include <node_api.h>
#include "napi/native_common.h"

namespace OHOS::Plugin {
const std::string WEB_SCHEME_HANDLER = "WebSchemeHandler";
const std::string WEB_SCHEME_HANDLER_REQUEST = "WebSchemeHandlerRequest";
const std::string WEB_SCHEME_HANDLER_RESPONSE = "WebSchemeHandlerResponse";
const std::string WEB_RESOURCE_HANDLER = "WebResourceHandler";
const std::string WEB_HTTP_BODY_STREAM = "WebHttpBodyStream";
const std::string WEB_NET_ERROR_CODE = "WebNetErrorCode";
class NapiWebSchemeHandlerRequest {
public:
    NapiWebSchemeHandlerRequest() = default;
    ~NapiWebSchemeHandlerRequest() = default;

    static napi_value Init(napi_env env, napi_value exports);
    static void ExportWebSchemeHandlerRequestClass(napi_env, napi_value* exportsPointer);
    static napi_status DefineProperties(napi_env env, napi_value* object);
    
    static napi_value JS_Constructor(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_GetHeader(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_GetRequestUrl(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_GetRequestMethod(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_GetReferrer(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_IsRedirect(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_IsMainFrame(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_HasGesture(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_HttpBodyStream(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_GetRequestResourceType(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_GetFrameUrl(napi_env env, napi_callback_info cbinfo);

private:
    static napi_status ExportEnumWebResourceType(napi_env env, napi_value* value);
};

class NapiWebSchemeHandlerResponse {
public:
    NapiWebSchemeHandlerResponse() = default;
    ~NapiWebSchemeHandlerResponse() = default;

    static napi_value Init(napi_env env, napi_value exports);
    static void ExportWebSchemeHandlerResponseClass(napi_env, napi_value* exportsPointer);

    static napi_value JS_Constructor(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_GetUrl(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_SetUrl(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_GetStatus(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_SetStatus(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_GetStatusText(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_SetStatusText(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_GetMimeType(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_SetMimeType(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_GetEncoding(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_SetEncoding(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_GetHeaderByName(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_SetHeaderByName(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_GetNetErrorCode(napi_env env, napi_callback_info cbinfo);
    static napi_value JS_SetNetErrorCode(napi_env env, napi_callback_info cbinfo);
};

class NapiWebResourceHandler {
public:
    NapiWebResourceHandler() = default;
    ~NapiWebResourceHandler() = default;

    static napi_value Init(napi_env env, napi_value exports);
    static void ExportWebResourceHandlerClass(napi_env, napi_value* exportsPointer);
    static napi_status DefineProperties(napi_env env, napi_value* object);

    static napi_value JS_Constructor(napi_env env, napi_callback_info info);
    static napi_value JS_DidReceiveResponse(napi_env env, napi_callback_info info);
    static napi_value JS_DidReceiveResponseBody(napi_env env, napi_callback_info info);
    static napi_value JS_DidFinish(napi_env env, napi_callback_info info);
    static napi_value JS_DidFailWithError(napi_env env, napi_callback_info info);
};
}

#endif // PLUGIN_WEB_WEBVIEW_NAPI_WEB_SCHEME_HANDLER_REQUEST_H