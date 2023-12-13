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

#include "http_exec.h"

#include <cstddef>
#include <cstring>
#include <unistd.h>

#ifdef HTTP_PROXY_ENABLE
#include "parameter.h"
#endif
#ifdef HAS_NETMANAGER_BASE
#include "http_proxy.h"
#include "net_conn_client.h"
#endif
#include "base64_utils.h"
#include "cache_proxy.h"
#include "constant.h"
#include "core/common/ace_application_info.h"
#include "event_list.h"
#include "http_async_work.h"
#include "http_jni.h"
#include "http_time.h"
#include "napi_utils.h"
#include "netstack_common_utils.h"
#include "netstack_log.h"
#include "plugin_utils.h"
#include "securec.h"

#define NETSTACK_CURL_EASY_SET_OPTION(handle, opt, data, asyncContext)                                   \
    do {                                                                                                 \
        CURLcode result = curl_easy_setopt(handle, opt, data);                                           \
        if (result != CURLE_OK) {                                                                        \
            const char *err = curl_easy_strerror(result);                                                \
            NETSTACK_LOGE("Failed to set option: %{public}s, %{public}s %{public}d", #opt, err, result); \
            (asyncContext)->SetErrorCode(result);                                                        \
            return false;                                                                                \
        }                                                                                                \
    } while (0)

namespace OHOS::NetStack::Http {
static constexpr size_t MAX_LIMIT = 5 * 1024 * 1024;
static constexpr int CURL_TIMEOUT_MS = 50;
static constexpr int CONDITION_TIMEOUT_S = 3600;
static constexpr int CURL_MAX_WAIT_MSECS = 10;
static constexpr int CURL_HANDLE_NUM = 10;
#ifdef HTTP_PROXY_ENABLE
static constexpr int32_t SYSPARA_MAX_SIZE = 128;
static constexpr const char *DEFAULT_HTTP_PROXY_HOST = "NONE";
static constexpr const char *DEFAULT_HTTP_PROXY_PORT = "0";
static constexpr const char *DEFAULT_HTTP_PROXY_EXCLUSION_LIST = "NONE";
static constexpr const char *HTTP_PROXY_HOST_KEY = "persist.netmanager_base.http_proxy.host";
static constexpr const char *HTTP_PROXY_PORT_KEY = "persist.netmanager_base.http_proxy.port";
static constexpr const char *HTTP_PROXY_EXCLUSIONS_KEY = "persist.netmanager_base.http_proxy.exclusion_list";
#endif
static constexpr const char *HTTP_SYSTEM_CA_PATH = "/storage/emulated/0/Android/data/";
static constexpr const char *HTTP_APPLICATION_CA_FILE = "/files/cacert.ca";

template <napi_value (*MakeJsValue)(napi_env, void *)> static void CallbackTemplate(uv_work_t *work, int status)
{
    (void)status;

    auto workWrapper = static_cast<UvWorkWrapper *>(work->data);
    napi_env env = workWrapper->env;
    auto closeScope = [env](napi_handle_scope scope) { NapiUtils::CloseScope(env, scope); };
    std::unique_ptr<napi_handle_scope__, decltype(closeScope)> scope(NapiUtils::OpenScope(env), closeScope);

    napi_value obj = MakeJsValue(env, workWrapper->data);

    std::pair<napi_value, napi_value> arg = {NapiUtils::GetUndefined(workWrapper->env), obj};
    workWrapper->manager->Emit(workWrapper->type, arg);

    delete workWrapper;
    delete work;
}

bool HttpExec::AddCurlHandle(CURL *handle, RequestContext *context)
{
    if (handle == nullptr || staticVariable_.curlMulti == nullptr) {
        NETSTACK_LOGE("handle nullptr");
        return false;
    }

    std::lock_guard guard(staticVariable_.curlMultiMutex);
    staticVariable_.infoQueue.emplace(context, handle);
    staticVariable_.conditionVariable.notify_all();
    return true;
}

HttpExec::StaticVariable HttpExec::staticVariable_; /* NOLINT */

bool HttpExec::RequestWithoutCache(RequestContext *context)
{
    if (!staticVariable_.initialized) {
        NETSTACK_LOGE("curl not init");
        return false;
    }

    auto handle = curl_easy_init();
    if (!handle) {
        NETSTACK_LOGE("Failed to create fetch task");
        return false;
    }

    std::vector<std::string> vec;
    std::for_each(context->options.GetHeader().begin(), context->options.GetHeader().end(),
                [&vec](const std::pair<std::string, std::string> &p) {
                    vec.emplace_back(p.first + HttpConstant::HTTP_HEADER_SEPARATOR + p.second);
                });
    context->SetCurlHeaderList(MakeHeaders(vec));

    if (!HttpExec::SetOption(handle, context, context->GetCurlHeaderList())) {
        NETSTACK_LOGE("set option failed");
        if (handle) {
            (void)curl_easy_cleanup(handle);
        }
        return false;
    }

    context->response.SetRequestTime(HttpTime::GetNowTimeGMT());
    if (!HttpExec::AddCurlHandle(handle, context)) {
        if (handle) {
            (void)curl_easy_cleanup(handle);
        }
        return false;
    }
    return true;
}

bool HttpExec::GetCurlDataFromHandle(CURL *handle, RequestContext *context, CURLMSG curlMsg, CURLcode result)
{
    if (curlMsg != CURLMSG_DONE) {
        NETSTACK_LOGE("CURLMSG %{public}s", std::to_string(curlMsg).c_str());
        context->SetErrorCode(NapiUtils::NETSTACK_NAPI_INTERNAL_ERROR);
        return false;
    }

    if (result != CURLE_OK) {
        context->SetErrorCode(result);
        NETSTACK_LOGE("CURLcode result %{public}s", std::to_string(result).c_str());
        return false;
    }

    context->response.SetResponseTime(HttpTime::GetNowTimeGMT());

    int64_t responseCode;
    CURLcode code = curl_easy_getinfo(handle, CURLINFO_RESPONSE_CODE, &responseCode);
    if (code != CURLE_OK) {
        context->SetErrorCode(code);
        return false;
    }
    context->response.SetResponseCode(responseCode);
    if (context->response.GetResponseCode() == static_cast<uint32_t>(ResponseCode::NOT_MODIFIED)) {
        NETSTACK_LOGI("cache is NOT_MODIFIED, we use the cache");
        context->SetResponseByCache();
        return true;
    }
    NETSTACK_LOGD("responseCode is %{public}s", std::to_string(responseCode).c_str());

    struct curl_slist *cookies = nullptr;
    code = curl_easy_getinfo(handle, CURLINFO_COOKIELIST, &cookies);
    if (code != CURLE_OK) {
        context->SetErrorCode(code);
        return false;
    }

    std::unique_ptr<struct curl_slist, decltype(&curl_slist_free_all)> cookiesHandle(cookies, curl_slist_free_all);
    while (cookies) {
        context->response.AppendCookies(cookies->data, strlen(cookies->data));
        if (cookies->next != nullptr) {
            context->response.AppendCookies(HttpConstant::HTTP_LINE_SEPARATOR,
                                            strlen(HttpConstant::HTTP_LINE_SEPARATOR));
        }
        cookies = cookies->next;
    }
    return true;
}

void HttpExec::HandleCurlData(CURLMsg *msg)
{
    if (msg == nullptr) {
        return;
    }

    auto handle = msg->easy_handle;
    if (handle == nullptr) {
        return;
    }

    auto it = staticVariable_.contextMap.find(handle);
    if (it == staticVariable_.contextMap.end()) {
        NETSTACK_LOGE("can not find context");
        return;
    }

    auto context = it->second;
    staticVariable_.contextMap.erase(it);
    if (context == nullptr) {
        NETSTACK_LOGE("can not find context");
        return;
    }

    NETSTACK_LOGI("priority = %{public}d", context->options.GetPriority());
    context->SetExecOK(GetCurlDataFromHandle(handle, context, msg->msg, msg->data.result));
    if (context->IsExecOK()) {
        CacheProxy proxy(context->options);
        proxy.WriteResponseToCache(context->response);
    }

    if (context->GetManager() == nullptr) {
        NETSTACK_LOGE("can not find context manager");
        return;
    }

    if (EventManager::IsManagerValid(context->GetManager())) {
        if (context->IsRequestInStream()) {
            if (context->IsExecOK()) {
                NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, OnDataEnd);
            }
            NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, HttpAsyncWork::RequestInStreamCallback);
        } else {
            NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, HttpAsyncWork::RequestCallback);
        }
    }
}

bool HttpExec::ExecRequest(RequestContext *context)
{
    context->options.SetRequestTime(HttpTime::GetNowTimeGMT());
    CacheProxy proxy(context->options);
    if (context->IsUsingCache() && proxy.ReadResponseFromCache(context)) {
        return true;
    }

    if (!RequestWithoutCache(context)) {
        context->SetErrorCode(NapiUtils::NETSTACK_NAPI_INTERNAL_ERROR);
        if (EventManager::IsManagerValid(context->GetManager())) {
            if (context->IsRequestInStream()) {
                NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, HttpAsyncWork::RequestInStreamCallback);
            } else {
                NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, HttpAsyncWork::RequestCallback);
            }
        }
        return false;
    }

    return true;
}

napi_value HttpExec::RequestCallback(RequestContext *context)
{
    napi_value object = NapiUtils::CreateObject(context->GetEnv());

    if (NapiUtils::GetValueType(context->GetEnv(), object) != napi_object) {
        return nullptr;
    }

    NapiUtils::SetUint32Property(context->GetEnv(), object, HttpConstant::RESPONSE_KEY_RESPONSE_CODE,
                                context->response.GetResponseCode());
    NapiUtils::SetStringPropertyUtf8(context->GetEnv(), object, HttpConstant::RESPONSE_KEY_COOKIES,
                                context->response.GetCookies());

    napi_value header = MakeResponseHeader(context->GetEnv(), context);
    if (NapiUtils::GetValueType(context->GetEnv(), header) == napi_object) {
        NapiUtils::SetNamedProperty(context->GetEnv(), object, HttpConstant::RESPONSE_KEY_HEADER, header);
    }

    if (context->options.GetHttpDataType() != HttpDataType::NO_DATA_TYPE && ProcByExpectDataType(object, context)) {
        return object;
    }

    auto contentType = CommonUtils::ToLower(const_cast<std::map<std::string, std::string> &>(
        context->response.GetHeader())[HttpConstant::HTTP_CONTENT_TYPE]);

    if (contentType.find(HttpConstant::HTTP_CONTENT_TYPE_OCTET_STREAM) != std::string::npos ||
        contentType.find(HttpConstant::HTTP_CONTENT_TYPE_IMAGE) != std::string::npos) {
        void *data = nullptr;
        auto body = context->response.GetResult();
        napi_value arrayBuffer = NapiUtils::CreateArrayBuffer(context->GetEnv(), body.size(), &data);
        if (data != nullptr && arrayBuffer != nullptr) {
            if (memcpy_s(data, body.size(), body.c_str(), body.size()) != EOK) {
                NETSTACK_LOGE("memcpy_s failed!");
                return object;
            }
            NapiUtils::SetNamedProperty(context->GetEnv(), object, HttpConstant::RESPONSE_KEY_RESULT, arrayBuffer);
        }
        NapiUtils::SetUint32Property(context->GetEnv(), object, HttpConstant::RESPONSE_KEY_RESULT_TYPE,
                                    static_cast<uint32_t>(HttpDataType::ARRAY_BUFFER));
        return object;
    }

    /* now just support utf8 */
    NapiUtils::SetStringPropertyUtf8(context->GetEnv(), object, HttpConstant::RESPONSE_KEY_RESULT,
                                    context->response.GetResult());
    NapiUtils::SetUint32Property(context->GetEnv(), object, HttpConstant::RESPONSE_KEY_RESULT_TYPE,
                                 static_cast<uint32_t>(HttpDataType::STRING));
    return object;
}

napi_value HttpExec::RequestInStreamCallback(OHOS::NetStack::Http::RequestContext* context)
{
    napi_value number = NapiUtils::CreateUint32(context->GetEnv(), context->response.GetResponseCode());
    if (NapiUtils::GetValueType(context->GetEnv(), number) != napi_number) {
        return nullptr;
    }
    return number;
}

std::string HttpExec::MakeUrl(const std::string &url, std::string param, const std::string &extraParam)
{
    if (param.empty()) {
        param += extraParam;
    } else {
        param += HttpConstant::HTTP_URL_PARAM_SEPARATOR;
        param += extraParam;
    }

    if (param.empty()) {
        return url;
    }

    return url + HttpConstant::HTTP_URL_PARAM_START + param;
}

bool HttpExec::MethodForGet(const std::string &method)
{
    return (method == HttpConstant::HTTP_METHOD_HEAD || method == HttpConstant::HTTP_METHOD_OPTIONS ||
            method == HttpConstant::HTTP_METHOD_DELETE || method == HttpConstant::HTTP_METHOD_TRACE ||
            method == HttpConstant::HTTP_METHOD_GET || method == HttpConstant::HTTP_METHOD_CONNECT);
}

bool HttpExec::MethodForPost(const std::string &method)
{
    return (method == HttpConstant::HTTP_METHOD_POST || method == HttpConstant::HTTP_METHOD_PUT);
}

bool HttpExec::EncodeUrlParam(std::string &str)
{
    char encoded[4];
    std::string encodeOut;
    size_t length = strlen(str.c_str());
    for (size_t i = 0; i < length; ++i) {
        auto c = static_cast<uint8_t>(str.c_str()[i]);
        if (IsUnReserved(c)) {
            encodeOut += static_cast<char>(c);
        } else {
            if (sprintf_s(encoded, sizeof(encoded), "%%%02X", c) < 0) {
                return false;
            }
            encodeOut += encoded;
        }
    }

    if (str == encodeOut) {
        return false;
    }
    str = encodeOut;
    return true;
}

void HttpExec::AddRequestInfo()
{
    std::lock_guard guard(staticVariable_.curlMultiMutex);
    int num = 0;
    while (!staticVariable_.infoQueue.empty()) {
        if (!staticVariable_.runThread || staticVariable_.curlMulti == nullptr) {
            break;
        }

        auto info = staticVariable_.infoQueue.top();
        staticVariable_.infoQueue.pop();

        auto ret = curl_multi_add_handle(staticVariable_.curlMulti, info.handle);
        if (ret == CURLM_OK) {
            staticVariable_.contextMap[info.handle] = info.context;
        }
        ++num;
        if (num >= CURL_HANDLE_NUM) {
            break;
        }
    }
}

void HttpExec::RunThread()
{
    while (staticVariable_.runThread && staticVariable_.curlMulti != nullptr) {
        AddRequestInfo();
        SendRequest();
        ReadResponse();
        std::this_thread::sleep_for(std::chrono::milliseconds(CURL_TIMEOUT_MS));
        std::unique_lock l(staticVariable_.curlMultiMutex);
        staticVariable_.conditionVariable.wait_for(l, std::chrono::seconds(CONDITION_TIMEOUT_S), [] {
            return !staticVariable_.infoQueue.empty() || !staticVariable_.contextMap.empty();
        });
    }
}

void HttpExec::SendRequest()
{
    std::lock_guard guard(staticVariable_.curlMultiMutex);

    int runningHandle = 0;
    int num = 0;
    do {
        if (!staticVariable_.runThread || staticVariable_.curlMulti == nullptr) {
            break;
        }

        auto ret = curl_multi_perform(staticVariable_.curlMulti, &runningHandle);

        if (runningHandle > 0) {
            ret = curl_multi_poll(staticVariable_.curlMulti, nullptr, 0, CURL_MAX_WAIT_MSECS, nullptr);
        }

        if (ret != CURLM_OK) {
            return;
        }

        ++num;
        if (num >= CURL_HANDLE_NUM) {
            break;
        }
    } while (runningHandle > 0);
}

void HttpExec::ReadResponse()
{
    std::lock_guard guard(staticVariable_.curlMultiMutex);
    CURLMsg *msg = nullptr; /* NOLINT */
    do {
        if (!staticVariable_.runThread || staticVariable_.curlMulti == nullptr) {
            break;
        }

        int leftMsg;
        msg = curl_multi_info_read(staticVariable_.curlMulti, &leftMsg);
        if (msg) {
            if (msg->msg == CURLMSG_DONE) {
                HandleCurlData(msg);
            }
            if (msg->easy_handle) {
                (void)curl_multi_remove_handle(staticVariable_.curlMulti, msg->easy_handle);
                (void)curl_easy_cleanup(msg->easy_handle);
            }
        }
    } while (msg);
}

void HttpExec::GetGlobalHttpProxyInfo(std::string &host, int32_t &port, std::string &exclusions)
{
#ifdef HTTP_PROXY_ENABLE
    char httpProxyHost[SYSPARA_MAX_SIZE] = {0};
    char httpProxyPort[SYSPARA_MAX_SIZE] = {0};
    char httpProxyExclusions[SYSPARA_MAX_SIZE] = {0};
    GetParameter(HTTP_PROXY_HOST_KEY, DEFAULT_HTTP_PROXY_HOST, httpProxyHost, sizeof(httpProxyHost));
    GetParameter(HTTP_PROXY_PORT_KEY, DEFAULT_HTTP_PROXY_PORT, httpProxyPort, sizeof(httpProxyPort));
    GetParameter(HTTP_PROXY_EXCLUSIONS_KEY, DEFAULT_HTTP_PROXY_EXCLUSION_LIST, httpProxyExclusions,
                 sizeof(httpProxyExclusions));

    host = Base64::Decode(httpProxyHost);
    if (host == DEFAULT_HTTP_PROXY_HOST) {
        host = std::string();
    }
    exclusions = httpProxyExclusions;
    if (exclusions == DEFAULT_HTTP_PROXY_EXCLUSION_LIST) {
        exclusions = std::string();
    }

    port = std::atoi(httpProxyPort);
#endif
}

void HttpExec::GetHttpProxyInfo(RequestContext *context, std::string &host, int32_t &port, std::string &exclusions)
{
    if (context->options.GetUsingHttpProxyType() == UsingHttpProxyType::USE_DEFAULT) {
#ifdef HAS_NETMANAGER_BASE
        using namespace NetManagerStandard;
        HttpProxy httpProxy;
        NetConnClient::GetInstance().GetDefaultHttpProxy(httpProxy);
        host = httpProxy.GetHost();
        port = httpProxy.GetPort();
        exclusions = CommonUtils::ToString(httpProxy.GetExclusionList());
#else
        GetGlobalHttpProxyInfo(host, port, exclusions);
#endif
    } else if (context->options.GetUsingHttpProxyType() == UsingHttpProxyType::USE_SPECIFIED) {
        context->options.GetSpecifiedHttpProxy(host, port, exclusions);
    }
}

bool HttpExec::Initialize()
{
    std::lock_guard<std::mutex> lock(staticVariable_.curlMultiMutex);
    if (staticVariable_.initialized) {
        return true;
    }

    NETSTACK_LOGI("call curl_global_init");
    if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
        NETSTACK_LOGE("Failed to initialize 'curl'");
        return false;
    }

    staticVariable_.curlMulti = curl_multi_init();
    if (staticVariable_.curlMulti == nullptr) {
        NETSTACK_LOGE("Failed to initialize 'curl_multi'");
        return false;
    }

    staticVariable_.workThread = std::thread(RunThread);
    staticVariable_.initialized = true;
    return staticVariable_.initialized;
}

bool HttpExec::SetOtherOption(CURL *curl, OHOS::NetStack::Http::RequestContext *context)
{
    std::string host, exclusions;
    int32_t port = 0;
    GetHttpProxyInfo(context, host, port, exclusions);
    if (!host.empty()) {
        NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_PROXY, host.c_str(), context);
        NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_PROXYPORT, port, context);
        if (!exclusions.empty()) {
            NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_NOPROXY, exclusions.c_str(), context);
        }
        NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP, context);
        NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_HTTPPROXYTUNNEL, 1L, context);
    }

#ifdef NETSTACK_PROXY_PASS
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_PROXYUSERPWD, NETSTACK_PROXY_PASS, context);
#endif // NETSTACK_PROXY_PASS

#if NO_SSL_CERTIFICATION
    // in real life, you should buy a ssl certification and rename it to /etc/ssl/cert.pem
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_SSL_VERIFYHOST, 0L, context);
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_SSL_VERIFYPEER, 0L, context);
#else
#ifndef WINDOWS_PLATFORM
    std::string cacertFile { HTTP_SYSTEM_CA_PATH };
    cacertFile.append(Ace::AceApplicationInfo::GetInstance().GetPackageName());
    cacertFile.append(HTTP_APPLICATION_CA_FILE);
    if (access(cacertFile.c_str(), F_OK) == 0) {
        NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_CAINFO, cacertFile.c_str(), context);
    } else {
        NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_SSL_VERIFYHOST, 0L, context);
        NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_SSL_VERIFYPEER, 0L, context);
    }
#endif // WINDOWS_PLATFORM
#endif // NO_SSL_CERTIFICATION

#if HTTP_CURL_PRINT_VERBOSE
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_VERBOSE, 1L, context);
#endif

#ifndef WINDOWS_PLATFORM
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_ACCEPT_ENCODING, HttpConstant::HTTP_CONTENT_ENCODING_GZIP, context);
#endif
    return true;
}

bool HttpExec::SetOption(CURL *curl, RequestContext *context, struct curl_slist *requestHeader)
{
    const std::string &method = context->options.GetMethod();
    if (!MethodForGet(method) && !MethodForPost(method)) {
        NETSTACK_LOGE("method %{public}s not supported", method.c_str());
        return false;
    }

    if (context->options.GetMethod() == HttpConstant::HTTP_METHOD_HEAD) {
        NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_NOBODY, 1L, context);
    }

    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_URL, context->options.GetUrl().c_str(), context);
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_CUSTOMREQUEST, method.c_str(), context);

    if (MethodForPost(method) && !context->options.GetBody().empty()) {
        NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_POST, 1L, context);
        NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_POSTFIELDS, context->options.GetBody().c_str(), context);
        NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_POSTFIELDSIZE, context->options.GetBody().size(), context);
    }

    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_XFERINFOFUNCTION, ProgressCallback, context);
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_XFERINFODATA, context, context);
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_NOPROGRESS, 0L, context);

    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_WRITEFUNCTION, OnWritingMemoryBody, context);
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_WRITEDATA, context, context);

    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_HEADERFUNCTION, OnWritingMemoryHeader, context);
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_HEADERDATA, context, context);

    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_HTTPHEADER, requestHeader, context);

    // Some servers don't like requests that are made without a user-agent field, so we provide one
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_USERAGENT, HttpConstant::HTTP_DEFAULT_USER_AGENT, context);

    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_FOLLOWLOCATION, 1L, context);

    /* first #undef CURL_DISABLE_COOKIES in curl config */
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_COOKIEFILE, "", context);

    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_NOSIGNAL, 1L, context);

    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_TIMEOUT_MS, context->options.GetReadTimeout(), context);
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_CONNECTTIMEOUT_MS, context->options.GetConnectTimeout(), context);

    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_HTTP_VERSION, context->options.GetHttpVersion(), context);
    if (!SetOtherOption(curl, context)) {
        return false;
    }

    return true;
}

size_t HttpExec::OnWritingMemoryBody(const void *data, size_t size, size_t memBytes, void *userData)
{
    auto context = static_cast<RequestContext *>(userData);
    if (context == nullptr) {
        return 0;
    }
    if (context->IsRequestInStream()) {
        context->SetTempData(data, size * memBytes);
        NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, OnDataReceive);
        return size * memBytes;
    }
    if (context->response.GetResult().size() > MAX_LIMIT) {
        return 0;
    }
    context->response.AppendResult(data, size * memBytes);
    return size * memBytes;
}

size_t HttpExec::OnWritingMemoryHeader(const void *data, size_t size, size_t memBytes, void *userData)
{
    auto context = static_cast<RequestContext *>(userData);
    if (context == nullptr) {
        return 0;
    }
    if (context->response.GetResult().size() > MAX_LIMIT) {
        return 0;
    }
    context->response.AppendRawHeader(data, size * memBytes);
    if (CommonUtils::EndsWith(context->response.GetRawHeader(), HttpConstant::HTTP_RESPONSE_HEADER_SEPARATOR)) {
        context->response.ParseHeaders();
        if (context->GetManager() && EventManager::IsManagerValid(context->GetManager())) {
            context->GetManager()->EmitByUv(ON_HEADER_RECEIVE, context, CallbackTemplate<MakeResponseHeader>);
            context->GetManager()->EmitByUv(ON_HEADERS_RECEIVE, context, CallbackTemplate<MakeResponseHeader>);
        }
    }
    return size * memBytes;
}

void HttpExec::OnDataReceive(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<RequestContext *>(data);
    if (context == nullptr) {
        return;
    }

    void *buffer = nullptr;
    auto tempData = context->GetTempData();
    context->PopTempData();
    if (tempData.empty()) {
        NETSTACK_LOGI("[GetTempData] tempDate is empty!");
        return;
    }
    napi_value arrayBuffer = NapiUtils::CreateArrayBuffer(context->GetEnv(), tempData.size(), &buffer);
    if (buffer == nullptr || arrayBuffer == nullptr) {
        return;
    }
    if (memcpy_s(buffer, tempData.size(), tempData.data(), tempData.size()) != EOK) {
        NETSTACK_LOGE("[CreateArrayBuffer] memory copy failed");
        return;
    }
    context->Emit(ON_DATA_RECEIVE, std::make_pair(NapiUtils::GetUndefined(context->GetEnv()), arrayBuffer));
}

void HttpExec::OnDataProgress(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<RequestContext *>(data);
    if (context == nullptr) {
        return;
    }
    auto progress = NapiUtils::CreateObject(context->GetEnv());
    if (NapiUtils::GetValueType(context->GetEnv(), progress) == napi_undefined) {
        return;
    }
    NapiUtils::SetUint32Property(context->GetEnv(), progress, "receiveSize",
                                static_cast<uint32_t>(context->GetDlLen().nLen));
    NapiUtils::SetUint32Property(context->GetEnv(), progress, "totalSize",
                                static_cast<uint32_t>(context->GetDlLen().tLen));
    context->PopDlLen();
    context->Emit(ON_DATA_RECEIVE_PROGRESS, std::make_pair(NapiUtils::GetUndefined(context->GetEnv()), progress));
}

void HttpExec::OnDataEnd(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<RequestContext *>(data);
    if (context == nullptr) {
        return;
    }
    auto undefined = NapiUtils::GetUndefined(context->GetEnv());
    context->Emit(ON_DATA_END, std::make_pair(undefined, undefined));
}

int HttpExec::ProgressCallback(void *userData, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal,
                                curl_off_t ulnow)
{
    (void)ultotal;
    (void)ulnow;
    auto context = static_cast<RequestContext *>(userData);
    if (context == nullptr || !context->IsRequestInStream()) {
        return 0;
    }
    if (dltotal != 0) {
        context->SetDlLen(dlnow, dltotal);
        NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, OnDataProgress);
    }
    return 0;
}

struct curl_slist *HttpExec::MakeHeaders(const std::vector<std::string> &vec)
{
    struct curl_slist *header = nullptr;
    std::for_each(vec.begin(), vec.end(), [&header](const std::string &s) {
        if (!s.empty()) {
            header = curl_slist_append(header, s.c_str());
        }
    });
    return header;
}

napi_value HttpExec::MakeResponseHeader(napi_env env, void *ctx)
{
    auto context = reinterpret_cast<RequestContext *>(ctx);
    (void)env;
    napi_value header = NapiUtils::CreateObject(context->GetEnv());
    if (NapiUtils::GetValueType(context->GetEnv(), header) == napi_object) {
        std::for_each(context->response.GetHeader().begin(), context->response.GetHeader().end(),
                    [context, header](const std::pair<std::string, std::string> &p) {
                        if (!p.first.empty() && !p.second.empty()) {
                            NapiUtils::SetStringPropertyUtf8(context->GetEnv(), header, p.first, p.second);
                        }
                    });
    }
    return header;
}

bool HttpExec::IsUnReserved(unsigned char in)
{
    if ((in >= '0' && in <= '9') || (in >= 'a' && in <= 'z') || (in >= 'A' && in <= 'Z')) {
        return true;
    }
    switch (in) {
        case '-':
        case '.':
        case '_':
        case '~':
            return true;
        default:
            break;
    }
    return false;
}

bool HttpExec::ProcByExpectDataType(napi_value object, RequestContext *context)
{
    switch (context->options.GetHttpDataType()) {
        case HttpDataType::STRING: {
            NapiUtils::SetStringPropertyUtf8(context->GetEnv(), object, HttpConstant::RESPONSE_KEY_RESULT,
                                            context->response.GetResult());
            NapiUtils::SetUint32Property(context->GetEnv(), object, HttpConstant::RESPONSE_KEY_RESULT_TYPE,
                                        static_cast<uint32_t>(HttpDataType::STRING));
            return true;
        }
        case HttpDataType::OBJECT: {
            if (context->response.GetResult().size() > HttpConstant::MAX_JSON_PARSE_SIZE) {
                return false;
            }

            napi_value obj = NapiUtils::JsonParse(context->GetEnv(), context->response.GetResult());
            if (obj) {
                NapiUtils::SetNamedProperty(context->GetEnv(), object, HttpConstant::RESPONSE_KEY_RESULT, obj);
                NapiUtils::SetUint32Property(context->GetEnv(), object, HttpConstant::RESPONSE_KEY_RESULT_TYPE,
                                            static_cast<uint32_t>(HttpDataType::OBJECT));
                return true;
            }

            // parse maybe failed
            return false;
        }
        case HttpDataType::ARRAY_BUFFER: {
            void *data = nullptr;
            auto body = context->response.GetResult();
            napi_value arrayBuffer = NapiUtils::CreateArrayBuffer(context->GetEnv(), body.size(), &data);
            if (data != nullptr && arrayBuffer != nullptr) {
                if (memcpy_s(data, body.size(), body.c_str(), body.size()) < 0) {
                    NETSTACK_LOGE("[ProcByExpectDataType] memory copy failed");
                    return true;
                }
                NapiUtils::SetNamedProperty(context->GetEnv(), object, HttpConstant::RESPONSE_KEY_RESULT, arrayBuffer);
                NapiUtils::SetUint32Property(context->GetEnv(), object, HttpConstant::RESPONSE_KEY_RESULT_TYPE,
                                            static_cast<uint32_t>(HttpDataType::ARRAY_BUFFER));
            }
            return true;
        }
        default:
            break;
    }
    return false;
}

#ifndef MAC_PLATFORM
void HttpExec::AsyncRunRequest(RequestContext *context)
{
    HttpAsyncWork::ExecRequest(context->GetEnv(), context);
}
#endif

bool HttpExec::IsInitialized()
{
    return staticVariable_.initialized;
}

void HttpExec::DeInitialize()
{
    std::lock_guard<std::mutex> lock(staticVariable_.curlMultiMutex);
    staticVariable_.runThread = false;
    staticVariable_.conditionVariable.notify_all();
    if (staticVariable_.workThread.joinable()) {
        staticVariable_.workThread.join();
    }
    if (staticVariable_.curlMulti) {
        curl_multi_cleanup(staticVariable_.curlMulti);
    }
    staticVariable_.initialized = false;
}

std::string HttpExec::GetCacheFileName()
{
    return HttpJni::GetCacheDirJni();
}

bool HttpResponseCacheExec::ExecFlush(BaseContext *context)
{
    (void)context;
    CacheProxy::FlushCache();
    return true;
}

napi_value HttpResponseCacheExec::FlushCallback(BaseContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool HttpResponseCacheExec::ExecDelete(BaseContext *context)
{
    (void)context;
    CacheProxy::StopCacheAndDelete();
    return true;
}

napi_value HttpResponseCacheExec::DeleteCallback(BaseContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}
} // namespace OHOS::NetStack::Http
