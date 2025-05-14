/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "http_module.h"

#include "cache_proxy.h"
#include "constant.h"
#include "event_list.h"
#include "http_async_work.h"
#include "http_exec.h"
#ifdef ANDROID_PLATFORM
#include "http_jni.h"
#endif
#include "module_template.h"
#include "netstack_log.h"
#ifdef ANDROID_PLATFORM
#include "plugin_utils.h"
#endif

#define DECLARE_RESPONSE_CODE(code) \
    DECLARE_NAPI_STATIC_PROPERTY(#code, NapiUtils::CreateUint32(env, static_cast<uint32_t>(ResponseCode::code)))

#define DECLARE_REQUEST_METHOD(method) \
    DECLARE_NAPI_STATIC_PROPERTY(HttpConstant::method, NapiUtils::CreateStringUtf8(env, HttpConstant::method))

#define DECLARE_HTTP_PROTOCOL(protocol) \
    DECLARE_NAPI_STATIC_PROPERTY(#protocol, \
        NapiUtils::CreateUint32(env, static_cast<uint32_t>(HttpProtocol::protocol)))

namespace OHOS::NetStack::Http {
static constexpr const char *FLUSH_ASYNC_WORK_NAME = "ExecFlush";

#ifdef MAC_PLATFORM
static constexpr const char *REQUEST_ASYNC_WORK_NAME = "ExecRequest";
#endif

static constexpr const char *DELETE_ASYNC_WORK_NAME = "ExecDelete";

static constexpr const char *HTTP_MODULE_NAME = "net.http";

napi_value HttpModuleExports::InitHttpModule(napi_env env, napi_value exports)
{
    DefineHttpRequestClass(env, exports);
    DefineHttpResponseCacheClass(env, exports);
    InitHttpProperties(env, exports);
    NapiUtils::SetEnvValid(env);
    napi_add_env_cleanup_hook(env, NapiUtils::HookForEnvCleanup, env);

    return exports;
}

napi_value HttpModuleExports::CreateHttp(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::NewInstanceWithSharedManager(
        env, info, INTERFACE_HTTP_REQUEST, [](napi_env, void *data, void *) {
            NETSTACK_LOGI("http request handle is finalized");
            auto sharedManager = reinterpret_cast<std::shared_ptr<EventManager>*>(data);
            if (sharedManager != nullptr && *sharedManager != nullptr) {
                delete sharedManager;
            }
        });
}

napi_value HttpModuleExports::CreateHttpResponseCache(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    size_t paramsCount = MAX_PARAM_NUM;
    napi_value params[MAX_PARAM_NUM] = {nullptr};
    NAPI_CALL(env, napi_get_cb_info(env, info, &paramsCount, params, &thisVal, nullptr));
    if (paramsCount != 1 || NapiUtils::GetValueType(env, params[0]) != napi_number) {
        CacheProxy::RunCache();
    } else {
        size_t size = NapiUtils::GetUint32FromValue(env, params[0]);
        CacheProxy::RunCacheWithSize(size);
    }

    return ModuleTemplate::NewInstanceNoManager(env, info,
        INTERFACE_HTTP_RESPONSE_CACHE, [](napi_env, void *, void *) {
        NETSTACK_LOGI("http response cache handle is finalized");
    });
}

void HttpModuleExports::DefineHttpRequestClass(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_FUNCTION(HttpRequest::FUNCTION_REQUEST, HttpRequest::Request),
        DECLARE_NAPI_FUNCTION(HttpRequest::FUNCTION_REQUEST_IN_STREAM, HttpRequest::RequestInStream),
        DECLARE_NAPI_FUNCTION(HttpRequest::FUNCTION_DESTROY, HttpRequest::Destroy),
        DECLARE_NAPI_FUNCTION(HttpRequest::FUNCTION_ON, HttpRequest::On),
        DECLARE_NAPI_FUNCTION(HttpRequest::FUNCTION_ONCE, HttpRequest::Once),
        DECLARE_NAPI_FUNCTION(HttpRequest::FUNCTION_OFF, HttpRequest::Off),
    };
    ModuleTemplate::DefineClass(env, exports, properties, INTERFACE_HTTP_REQUEST);
}

void HttpModuleExports::DefineHttpResponseCacheClass(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_FUNCTION(HttpResponseCache::FUNCTION_FLUSH, HttpResponseCache::Flush),
        DECLARE_NAPI_FUNCTION(HttpResponseCache::FUNCTION_DELETE, HttpResponseCache::Delete),
    };
    ModuleTemplate::DefineClass(env, exports, properties, INTERFACE_HTTP_RESPONSE_CACHE);
}

void HttpModuleExports::InitHttpProperties(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_FUNCTION(FUNCTION_CREATE_HTTP, CreateHttp),
        DECLARE_NAPI_FUNCTION(FUNCTION_CREATE_HTTP_RESPONSE_CACHE, CreateHttpResponseCache),
    };
    NapiUtils::DefineProperties(env, exports, properties);

    InitRequestMethod(env, exports);
    InitResponseCode(env, exports);
    InitCertType(env, exports);
    InitHttpProtocol(env, exports);
    InitHttpDataType(env, exports);
}

void HttpModuleExports::InitRequestMethod(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_REQUEST_METHOD(HTTP_METHOD_OPTIONS), DECLARE_REQUEST_METHOD(HTTP_METHOD_GET),
        DECLARE_REQUEST_METHOD(HTTP_METHOD_HEAD),    DECLARE_REQUEST_METHOD(HTTP_METHOD_POST),
        DECLARE_REQUEST_METHOD(HTTP_METHOD_PUT),     DECLARE_REQUEST_METHOD(HTTP_METHOD_DELETE),
        DECLARE_REQUEST_METHOD(HTTP_METHOD_TRACE),   DECLARE_REQUEST_METHOD(HTTP_METHOD_CONNECT),
    };

    napi_value requestMethod = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, requestMethod, properties);

    NapiUtils::SetNamedProperty(env, exports, INTERFACE_REQUEST_METHOD, requestMethod);
}

void HttpModuleExports::InitResponseCode(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_RESPONSE_CODE(OK),
        DECLARE_RESPONSE_CODE(CREATED),
        DECLARE_RESPONSE_CODE(ACCEPTED),
        DECLARE_RESPONSE_CODE(NOT_AUTHORITATIVE),
        DECLARE_RESPONSE_CODE(NO_CONTENT),
        DECLARE_RESPONSE_CODE(RESET),
        DECLARE_RESPONSE_CODE(PARTIAL),
        DECLARE_RESPONSE_CODE(MULT_CHOICE),
        DECLARE_RESPONSE_CODE(MOVED_PERM),
        DECLARE_RESPONSE_CODE(MOVED_TEMP),
        DECLARE_RESPONSE_CODE(SEE_OTHER),
        DECLARE_RESPONSE_CODE(NOT_MODIFIED),
        DECLARE_RESPONSE_CODE(USE_PROXY),
        DECLARE_RESPONSE_CODE(BAD_REQUEST),
        DECLARE_RESPONSE_CODE(UNAUTHORIZED),
        DECLARE_RESPONSE_CODE(PAYMENT_REQUIRED),
        DECLARE_RESPONSE_CODE(FORBIDDEN),
        DECLARE_RESPONSE_CODE(NOT_FOUND),
        DECLARE_RESPONSE_CODE(BAD_METHOD),
        DECLARE_RESPONSE_CODE(NOT_ACCEPTABLE),
        DECLARE_RESPONSE_CODE(PROXY_AUTH),
        DECLARE_RESPONSE_CODE(CLIENT_TIMEOUT),
        DECLARE_RESPONSE_CODE(CONFLICT),
        DECLARE_RESPONSE_CODE(GONE),
        DECLARE_RESPONSE_CODE(LENGTH_REQUIRED),
        DECLARE_RESPONSE_CODE(PRECON_FAILED),
        DECLARE_RESPONSE_CODE(ENTITY_TOO_LARGE),
        DECLARE_RESPONSE_CODE(REQ_TOO_LONG),
        DECLARE_RESPONSE_CODE(UNSUPPORTED_TYPE),
        DECLARE_RESPONSE_CODE(RANGE_NOT_SATISFIABLE),
        DECLARE_RESPONSE_CODE(INTERNAL_ERROR),
        DECLARE_RESPONSE_CODE(NOT_IMPLEMENTED),
        DECLARE_RESPONSE_CODE(BAD_GATEWAY),
        DECLARE_RESPONSE_CODE(UNAVAILABLE),
        DECLARE_RESPONSE_CODE(GATEWAY_TIMEOUT),
        DECLARE_RESPONSE_CODE(VERSION),
    };

    napi_value responseCode = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, responseCode, properties);

    NapiUtils::SetNamedProperty(env, exports, INTERFACE_RESPONSE_CODE, responseCode);
}

void HttpModuleExports::InitHttpProtocol(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_HTTP_PROTOCOL(HTTP1_1),
        DECLARE_HTTP_PROTOCOL(HTTP2),
        DECLARE_HTTP_PROTOCOL(HTTP3),
    };

    napi_value httpProtocol = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, httpProtocol, properties);

    NapiUtils::SetNamedProperty(env, exports, INTERFACE_HTTP_PROTOCOL, httpProtocol);
}

void HttpModuleExports::InitCertType(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_STATIC_PROPERTY(HttpConstant::HTTP_CERT_TYPE_PEM,
                                     NapiUtils::CreateStringUtf8(env, HttpConstant::HTTP_CERT_TYPE_PEM)),
        DECLARE_NAPI_STATIC_PROPERTY(HttpConstant::HTTP_CERT_TYPE_DER,
                                     NapiUtils::CreateStringUtf8(env, HttpConstant::HTTP_CERT_TYPE_DER)),
        DECLARE_NAPI_STATIC_PROPERTY(HttpConstant::HTTP_CERT_TYPE_P12,
                                     NapiUtils::CreateStringUtf8(env, HttpConstant::HTTP_CERT_TYPE_P12)),
    };
    napi_value httpCertType = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, httpCertType, properties);
    NapiUtils::SetNamedProperty(env, exports, INTERFACE_CERT_TYPE, httpCertType);
}

void HttpModuleExports::InitHttpDataType(napi_env env, napi_value exports)
{
    std::initializer_list<napi_property_descriptor> properties = {
        DECLARE_NAPI_STATIC_PROPERTY(
            "STRING", NapiUtils::CreateUint32(env, static_cast<uint32_t>(HttpDataType::STRING))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "OBJECT", NapiUtils::CreateUint32(env, static_cast<uint32_t>(HttpDataType::OBJECT))),
        DECLARE_NAPI_STATIC_PROPERTY(
            "ARRAY_BUFFER", NapiUtils::CreateUint32(env, static_cast<uint32_t>(HttpDataType::ARRAY_BUFFER))),
    };
    napi_value httpDataType = NapiUtils::CreateObject(env);
    NapiUtils::DefineProperties(env, httpDataType, properties);
    NapiUtils::SetNamedProperty(env, exports, INTERFACE_HTTP_DATA_TYPE, httpDataType);
}

napi_value HttpModuleExports::HttpRequest::Request(napi_env env, napi_callback_info info)
{
#ifndef MAC_PLATFORM
    return ModuleTemplate::InterfaceWithOutAsyncWorkWithSharedManager<RequestContext>(
        env, info,
        [](napi_env, napi_value, RequestContext *context) -> bool {
            if (!HttpExec::Initialize()) {
                return false;
            }

            HttpExec::AsyncRunRequest(context);
            return context->IsExecOK();
        },
        "Request", HttpAsyncWork::ExecRequest, HttpAsyncWork::RequestCallback);
#else
    return ModuleTemplate::InterfaceWithSharedManager<RequestContext>(
        env, info, REQUEST_ASYNC_WORK_NAME,
        [](napi_env, napi_value, RequestContext*) -> bool { return HttpExec::Initialize(); },
        HttpAsyncWork::ExecRequest, HttpAsyncWork::RequestCallback);
#endif
}

napi_value HttpModuleExports::HttpRequest::RequestInStream(napi_env env, napi_callback_info info)
{
#ifndef MAC_PLATFORM
    return ModuleTemplate::InterfaceWithOutAsyncWorkWithSharedManager<RequestContext>(
        env, info,
        [](napi_env, napi_value, RequestContext *context) -> bool {
            if (!HttpExec::Initialize()) {
                return false;
            }

            context->EnableRequestInStream();
            HttpExec::AsyncRunRequest(context);
            return true;
        },
        "RequestInStream", HttpAsyncWork::ExecRequest, HttpAsyncWork::RequestCallback);
#else
    return ModuleTemplate::InterfaceWithSharedManager<RequestContext>(
        env, info, REQUEST_ASYNC_WORK_NAME,
        [](napi_env, napi_value, RequestContext*) -> bool { return HttpExec::Initialize(); },
        HttpAsyncWork::ExecRequest, HttpAsyncWork::RequestCallback);
#endif
}

napi_value HttpModuleExports::HttpRequest::Destroy(napi_env env, napi_callback_info info)
{
    napi_value thisVal = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVal, nullptr));
    std::shared_ptr<EventManager> *sharedManager = nullptr;
    auto napi_ret = napi_unwrap(env, thisVal, reinterpret_cast<void**>(&sharedManager));
    if (napi_ret != napi_ok) {
        NETSTACK_LOGE("get event manager in napi_unwrap failed, napi_ret is %{public}d", napi_ret);
        return NapiUtils::GetUndefined(env);
    }
    if (sharedManager != nullptr && *sharedManager != nullptr) {
        auto manager = *sharedManager;
        if (manager->IsEventDestroy()) {
            NETSTACK_LOGD("js object has been destroyed");
            return NapiUtils::GetUndefined(env);
        }
        manager->SetEventDestroy(true);
        manager->DeleteEventReference(env);
    }
    return NapiUtils::GetUndefined(env);
}

napi_value HttpModuleExports::HttpRequest::On(napi_env env, napi_callback_info info)
{
    ModuleTemplate::OnSharedManager(env, info,
        { ON_HEADERS_RECEIVE, ON_DATA_RECEIVE, ON_DATA_END, ON_DATA_RECEIVE_PROGRESS, ON_DATA_SEND_PROGRESS }, false);
    return ModuleTemplate::OnSharedManager(env, info, { ON_HEADER_RECEIVE }, true);
}

napi_value HttpModuleExports::HttpRequest::Once(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::OnceSharedManager(env, info, { ON_HEADER_RECEIVE, ON_HEADERS_RECEIVE }, false);
}

napi_value HttpModuleExports::HttpRequest::Off(napi_env env, napi_callback_info info)
{
    ModuleTemplate::OffSharedManager(env, info,
        { ON_HEADERS_RECEIVE, ON_DATA_RECEIVE, ON_DATA_END, ON_DATA_RECEIVE_PROGRESS, ON_DATA_SEND_PROGRESS });
    return ModuleTemplate::OffSharedManager(env, info, { ON_HEADER_RECEIVE });
}

napi_value HttpModuleExports::HttpResponseCache::Flush(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::InterfaceWithSharedManager<BaseContext>(
        env, info, FLUSH_ASYNC_WORK_NAME, nullptr, HttpAsyncWork::ExecFlush, HttpAsyncWork::FlushCallback);
}

napi_value HttpModuleExports::HttpResponseCache::Delete(napi_env env, napi_callback_info info)
{
    return ModuleTemplate::InterfaceWithSharedManager<BaseContext>(
        env, info, DELETE_ASYNC_WORK_NAME, nullptr, HttpAsyncWork::ExecDelete, HttpAsyncWork::DeleteCallback);
}

static napi_module g_httpModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = HttpModuleExports::InitHttpModule,
    .nm_modname = HTTP_MODULE_NAME,
    .nm_priv = nullptr,
    .reserved = {nullptr},
};

#ifdef ANDROID_PLATFORM
static void HttpJniRegister()
{
    const char className[] = "ohos.ace.plugin.httpplugin.HttpPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&HttpJni::Register, className);
}
#endif

extern "C" __attribute__((constructor)) void RegisterHttpModule(void)
{
#ifdef ANDROID_PLATFORM
    HttpJniRegister();
#endif
    napi_module_register(&g_httpModule);
}
} // namespace OHOS::NetStack::Http
