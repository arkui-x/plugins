/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <dirent.h>
#include <memory>

#include "cache_proxy.h"
#include "constant.h"
#include "event_list.h"
#include "http_async_work.h"
#include "http_time.h"
#include "napi_utils.h"
#include "netstack_common_utils.h"
#include "netstack_log.h"
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

#define NETSTACK_CURL_EASY_PERFORM(handle, asyncContext)                                                 \
    do {                                                                                                 \
        CURLcode result = curl_easy_perform(handle);                                                     \
        if (result != CURLE_OK) {                                                                        \
            (asyncContext)->SetErrorCode(result);                                                        \
            return false;                                                                                \
        }                                                                                                \
    } while (0)

#define NETSTACK_CURL_EASY_GET_INFO(handle, opt, data, asyncContext)                                   \
    do {                                                                                               \
        CURLcode result = curl_easy_getinfo(handle, opt, data);                                        \
        if (result != CURLE_OK) {                                                                      \
            const char *err = curl_easy_strerror(result);                                              \
            NETSTACK_LOGE("Failed to get info: %{public}s, %{public}s %{public}d", #opt, err, result); \
            (asyncContext)->SetErrorCode(result);                                                      \
            return false;                                                                              \
        }                                                                                              \
    } while (0)

namespace OHOS::NetStack {
namespace {
constexpr size_t MAX_LIMIT = 65536;
const char *const HTTP_SYSTEM_CA_PATH = "/system/etc/security/cacerts/";
} // namespace

std::mutex HttpExec::mutex_;
#ifndef MAC_PLATFORM
ThreadPool<HttpExec::Task, DEFAULT_THREAD_NUM, MAX_THREAD_NUM> HttpExec::threadPool_(DEFAULT_TIMEOUT);

HttpExec::Task::Task(RequestContext *context) : context_(context) {}

void HttpExec::Task::Execute()
{
    HttpAsyncWork::ExecRequest(context_->GetEnv(), context_);
    NapiUtils::CreateUvQueueWorkEnhanced(context_->GetEnv(), context_, HttpAsyncWork::RequestCallback);
}

bool HttpExec::Task::operator<(const Task &e) const
{
    return context_->options.GetPriority() < e.context_->options.GetPriority();
}

std::atomic_bool HttpExec::initialized_(false);
#else
bool HttpExec::initialized_ = false;
#endif

bool HttpExec::RequestWithoutCache(RequestContext *context, const std::string &cacert)
{
    if (!initialized_) {
        NETSTACK_LOGE("curl not init");
        return false;
    }

    std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> handle(curl_easy_init(), curl_easy_cleanup);

    if (!handle) {
        NETSTACK_LOGE("Failed to create fetch task");
        return false;
    }

    NETSTACK_LOGI("final url: ...");

    std::vector<std::string> vec;
    std::for_each(context->options.GetHeader().begin(), context->options.GetHeader().end(),
                  [&vec](const std::pair<std::string, std::string> &p) {
                      vec.emplace_back(p.first + HttpConstant::HTTP_HEADER_SEPARATOR + p.second);
                  });
    std::unique_ptr<struct curl_slist, decltype(&curl_slist_free_all)> header(MakeHeaders(vec), curl_slist_free_all);

    if (!SetOption(handle.get(), context, header.get(), cacert)) {
        NETSTACK_LOGE("set option failed");
        return false;
    }

    context->response.SetRequestTime(HttpTime::GetNowTimeGMT());
    NETSTACK_CURL_EASY_PERFORM(handle.get(), context);
    context->response.SetResponseTime(HttpTime::GetNowTimeGMT());

    int32_t responseCode;
    NETSTACK_CURL_EASY_GET_INFO(handle.get(), CURLINFO_RESPONSE_CODE, &responseCode, context);
    NETSTACK_LOGI("responseCode is %{public}d", responseCode);

    struct curl_slist *cookies = nullptr;
    NETSTACK_CURL_EASY_GET_INFO(handle.get(), CURLINFO_COOKIELIST, &cookies, context);
    std::unique_ptr<struct curl_slist, decltype(&curl_slist_free_all)> cookiesHandle(cookies, curl_slist_free_all);
    while (cookies) {
        context->response.AppendCookies(cookies->data, strlen(cookies->data));
        if (cookies->next != nullptr) {
            context->response.AppendCookies(HttpConstant::HTTP_LINE_SEPARATOR,
                                            strlen(HttpConstant::HTTP_LINE_SEPARATOR));
        }
        cookies = cookies->next;
    }

    context->response.SetResponseCode(responseCode);
    context->response.ParseHeaders();

    return true;
}

void HttpExec::GetCacertListFromSystem(std::vector<std::string> &cacertList)
{
    DIR* dir;
    struct dirent* ptr;
    std::vector<std::string> file_list;

    dir = opendir(HTTP_SYSTEM_CA_PATH);
    while ((ptr = readdir(dir)) != nullptr) {
        if (strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0) {
            continue;
        }
        if (strcmp(ptr->d_name, "pcd") == 0) {
            continue;
        }
        cacertList.push_back(ptr->d_name);
    }
    closedir(dir);

    NETSTACK_LOGI("cacertlist size: %{public}ld", cacertList.size());
}

bool HttpExec::ExecRequest(RequestContext *context)
{
    context->options.SetRequestTime(HttpTime::GetNowTimeGMT());
    CacheProxy proxy(context->options);
    if (context->IsUsingCache() && proxy.ReadResponseFromCache(context)) {
        return true;
    }

    std::vector<std::string> cacertList;
    GetCacertListFromSystem(cacertList);

    bool isFind = false;
    for (auto i = 0; i < cacertList.size(); i++) {
        if (RequestWithoutCache(context, cacertList[i])) {
            isFind = true;
            break;
        }
    }

    if (isFind) {
        proxy.WriteResponseToCache(context->response);
    }
    return isFind;
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

    napi_value header = MakeResponseHeader(context);
    if (NapiUtils::GetValueType(context->GetEnv(), header) == napi_object) {
        OnHeaderReceive(context, header);
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
            (void)memcpy_s(data, body.size(), body.c_str(), body.size());
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

napi_value HttpExec::Request2Callback(OHOS::NetStack::RequestContext *context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
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

bool HttpExec::Initialize()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return true;
    }
    NETSTACK_LOGI("call curl_global_init");
    if (curl_global_init(CURL_GLOBAL_ALL) != CURLE_OK) {
        NETSTACK_LOGE("Failed to initialize 'curl'");
        return false;
    }
    initialized_ = true;
    return initialized_;
}

bool HttpExec::SetCertification(CURL *curl, RequestContext *context, const std::string &cacert)
{
#if NO_SSL_CERTIFICATION
    // in real life, you should buy a ssl certification and rename it to /etc/ssl/cert.pem
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_SSL_VERIFYHOST, 0L, context);
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_SSL_VERIFYPEER, 0L, context);
#else
    std::string path;
    if (cacert.empty()) {
        path.append(HttpConstant::HTTP_DEFAULT_CA_PATH);
    } else {
        path.append(HTTP_SYSTEM_CA_PATH);
        path.append(cacert);
    }
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_CAINFO, path.c_str(), context);
#endif // NO_SSL_CERTIFICATION
    return true;
}

bool HttpExec::SetOptionParam(CURL *curl, RequestContext *context, struct curl_slist *requestHeader,
    const std::string &method)
{
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_TIMEOUT_MS, context->options.GetReadTimeout(), context);
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_CONNECTTIMEOUT_MS, context->options.GetConnectTimeout(), context);
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_ACCEPT_ENCODING, HttpConstant::HTTP_CONTENT_ENCODING_GZIP, context);

    if (MethodForPost(method) && !context->options.GetBody().empty()) {
        NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_POST, 1L, context);
        NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_POSTFIELDS, context->options.GetBody().c_str(), context);
        NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_POSTFIELDSIZE, context->options.GetBody().size(), context);
    }

    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_HTTP_VERSION, context->options.GetHttpVersion(), context);

    return true;
}

bool HttpExec::SetOption(CURL *curl, RequestContext *context, struct curl_slist *requestHeader,
    const std::string &cacert)
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

#if NETSTACK_USE_PROXY
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_PROXY, NETSTACK_PROXY_URL_PORT, context);
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_PROXYTYPE, NETSTACK_PROXY_TYPE, context);
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_HTTPPROXYTUNNEL, 1L, context);
#ifdef NETSTACK_PROXY_PASS
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_PROXYUSERPWD, NETSTACK_PROXY_PASS, context);
#endif // NETSTACK_PROXY_PASS
#endif // NETSTACK_USE_PROXY
    if (!SetCertification(curl, context, cacert)) {
        return false;
    }
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_NOPROGRESS, 1L, context);
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_NOSIGNAL, 1L, context);
#if HTTP_CURL_PRINT_VERBOSE
    NETSTACK_CURL_EASY_SET_OPTION(curl, CURLOPT_VERBOSE, 1L, context);
#endif

    return SetOptionParam(curl, context, requestHeader, method);
}

size_t HttpExec::OnWritingMemoryBody(const void *data, size_t size, size_t memBytes, void *userData)
{
    auto context = static_cast<RequestContext *>(userData);
    context->response.AppendResult(data, size * memBytes);
    return size * memBytes;
}

size_t HttpExec::OnWritingMemoryHeader(const void *data, size_t size, size_t memBytes, void *userData)
{
    auto context = static_cast<RequestContext *>(userData);
    if (context->response.GetResult().size() > MAX_LIMIT) {
        return 0;
    }
    context->response.AppendRawHeader(data, size * memBytes);
    return size * memBytes;
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

napi_value HttpExec::MakeResponseHeader(RequestContext *context)
{
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

void HttpExec::OnHeaderReceive(RequestContext *context, napi_value header)
{
    napi_value undefined = NapiUtils::GetUndefined(context->GetEnv());
    context->Emit(ON_HEADER_RECEIVE, std::make_pair(undefined, header));
    context->Emit(ON_HEADERS_RECEIVE, std::make_pair(undefined, header));
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

            napi_value obj = NapiUtils::JsonParse(
                context->GetEnv(), NapiUtils::CreateStringUtf8(context->GetEnv(), context->response.GetResult()));
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
    threadPool_.Push(Task(context));
}
#endif

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
} // namespace OHOS::NetStack
