/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#include <memory>
#include <thread>

#include "base64_utils.h"
#include "cache_proxy.h"
#include "constant.h"
#include "curl_slist.h"
#include "event_list.h"
#include "http_async_work.h"
#include "http_exec_ios_iml.h"
#include "http_time.h"
#include "napi_utils.h"
#include "netstack_common_utils.h"
#include "netstack_log.h"
#include "securec.h"
#include "secure_char.h"

namespace OHOS::NetStack::Http {
static constexpr int CURL_TIMEOUT_MS = 50;
static constexpr int CONDITION_TIMEOUT_S = 3600;
static constexpr const int EVENT_PARAM_ZERO = 0;
static constexpr const int EVENT_PARAM_ONE = 1;
static constexpr const int EVENT_PARAM_TWO = 2;
static constexpr size_t MAX_LIMIT = 5 * 1024 * 1024;
static constexpr const char *TLS12_SECURITY_CIPHER_SUITE = R"(DEFAULT:!eNULL:!EXPORT)";

HttpExec::StaticVariable HttpExec::staticVariable_;
HttpExec::StaticContextVec HttpExec::staticContextSet_;

napi_value HttpExec::RequestCallback(RequestContext *context)
{
    napi_value result = HttpExec::BuildRequestCallback(context);
    context->StopAndCacheNapiPerformanceTiming(HttpConstant::RESPONSE_TOTAL_TIMING);
    context->SetPerformanceTimingToResult(result);
    return result;
}

napi_value HttpExec::RequestInStreamCallback(RequestContext* context)
{
    napi_value number = NapiUtils::CreateUint32(context->GetEnv(), context->response.GetResponseCode());
    if (NapiUtils::GetValueType(context->GetEnv(), number) != napi_number) {
        return nullptr;
    }
    return number;
}

static void RequestContextDeleter(RequestContext *context)
{
    context->DeleteReference();
    delete context;
    context = nullptr;
}

static void AsyncWorkRequestInStreamCallback(napi_env env, napi_status status, void *data)
{
    if (status != napi_ok) {
        return;
    }
    std::unique_ptr<RequestContext, decltype(&RequestContextDeleter)> context(static_cast<RequestContext *>(data),
                                                                              RequestContextDeleter);
    napi_value argv[EVENT_PARAM_TWO] = {nullptr};
    if (context->IsParseOK() && context->IsExecOK()) {
        argv[EVENT_PARAM_ZERO] = NapiUtils::GetUndefined(env);
        argv[EVENT_PARAM_ONE] = HttpExec::RequestInStreamCallback(context.get());
        if (argv[EVENT_PARAM_ONE] == nullptr) {
            return;
        }
    } else {
        argv[EVENT_PARAM_ZERO] =
            NapiUtils::CreateErrorMessage(env, context->GetErrorCode(), context->GetErrorMessage());
        if (argv[EVENT_PARAM_ZERO] == nullptr) {
            return;
        }

        argv[EVENT_PARAM_ONE] = NapiUtils::GetUndefined(env);
    }

    napi_value undefined = NapiUtils::GetUndefined(env);
    if (context->GetDeferred() != nullptr) {
        if (context->IsExecOK()) {
            napi_resolve_deferred(env, context->GetDeferred(), argv[EVENT_PARAM_ONE]);
            context->EmitSharedManager(ON_DATA_END, std::make_pair(undefined, undefined));
        } else {
            napi_reject_deferred(env, context->GetDeferred(), argv[EVENT_PARAM_ZERO]);
        }
        return;
    }
    napi_value func = context->GetCallback();
    if (NapiUtils::GetValueType(env, func) == napi_function) {
        (void)NapiUtils::CallFunction(env, undefined, func, EVENT_PARAM_TWO, argv);
    }
    if (context->IsExecOK()) {
        context->EmitSharedManager(ON_DATA_END, std::make_pair(undefined, undefined));
    }
}

static void AsyncWorkRequestCallback(napi_env env, napi_status status, void *data)
{
    if (status != napi_ok) {
        return;
    }
    std::unique_ptr<RequestContext, decltype(&RequestContextDeleter)> context(static_cast<RequestContext *>(data),
                                                                              RequestContextDeleter);
    napi_value argv[EVENT_PARAM_TWO] = {nullptr};
    if (context->IsParseOK() && context->IsExecOK()) {
        argv[EVENT_PARAM_ZERO] = NapiUtils::GetUndefined(env);
        argv[EVENT_PARAM_ONE] = HttpExec::RequestCallback(context.get());
        if (argv[EVENT_PARAM_ONE] == nullptr) {
            return;
        }
    } else {
        argv[EVENT_PARAM_ZERO] =
            NapiUtils::CreateErrorMessage(env, context->GetErrorCode(), context->GetErrorMessage());
        if (argv[EVENT_PARAM_ZERO] == nullptr) {
            return;
        }

        argv[EVENT_PARAM_ONE] = NapiUtils::GetUndefined(env);
    }
    napi_value undefined = NapiUtils::GetUndefined(env);
    if (context->GetDeferred() != nullptr) {
        if (context->IsExecOK()) {
            napi_resolve_deferred(env, context->GetDeferred(), argv[EVENT_PARAM_ONE]);
        } else {
            napi_reject_deferred(env, context->GetDeferred(), argv[EVENT_PARAM_ZERO]);
        }
        return;
    }
    napi_value func = context->GetCallback();
    if (NapiUtils::GetValueType(env, func) == napi_function) {
        (void)NapiUtils::CallFunction(env, undefined, func, EVENT_PARAM_TWO, argv);
    }
}

bool HttpExec::RequestWithoutCache(RequestContext* context)
{
    std::vector<std::string> vec;
    std::for_each(context->options.GetHeader().begin(), context->options.GetHeader().end(),
                  [&vec](const std::pair<std::string, std::string> &p) {
                      if (!p.second.empty()) {
                          vec.emplace_back(p.first + HttpConstant::HTTP_HEADER_SEPARATOR + p.second);
                      } else {
                          vec.emplace_back(p.first + HttpConstant::HTTP_HEADER_BLANK_SEPARATOR);
                      }
                  });
    context->SetCurlHeaderList(MakeHeaders(vec));

    context->response.SetRequestTime(HttpTime::GetNowTimeGMT());

    std::lock_guard<std::mutex> lock(staticVariable_.mutex);
    std::shared_ptr<HttpExecInterface> httpExec = std::make_shared<HttpExecIosIml>();
    if (httpExec == nullptr) {
        NETSTACK_LOGE("Failed to new HttpExecIosIml");
        return false;
    }

    RequestInfo requestInfo(httpExec);
    staticVariable_.contextMap.insert(std::make_pair(context, requestInfo));
    staticVariable_.conditionVariable.notify_all();
    return true;
}

bool HttpExec::ExecRequest(RequestContext* context)
{
    if (!CommonUtils::HasInternetPermission()) {
        context->SetPermissionDenied(true);
        return false;
    }
    if (context->GetSharedManager()->IsEventDestroy()) {
        return false;
    }   
    context->options.SetRequestTime(HttpTime::GetNowTimeGMT());
    CacheProxy proxy(context->options);
    if (context->IsUsingCache() && proxy.ReadResponseFromCache(context)) {
        return true;
    }

    if (!RequestWithoutCache(context)) {
        context->SetErrorCode(NapiUtils::NETSTACK_NAPI_INTERNAL_ERROR);
        if (context->GetSharedManager()) {
            if (context->IsRequestInStream()) {
                NapiUtils::CreateUvQueueWorkByModuleId(
                    context->GetEnv(), std::bind(AsyncWorkRequestInStreamCallback, context->GetEnv(), napi_ok, context),
                    context->GetModuleId());
            } else {
                NapiUtils::CreateUvQueueWorkByModuleId(
                    context->GetEnv(), std::bind(AsyncWorkRequestCallback, context->GetEnv(), napi_ok, context),
                    context->GetModuleId());
            }
        }
        return false;
    }
    return true;
}

napi_value HttpExec::BuildRequestCallback(RequestContext *context)
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

std::string HttpExec::MakeUrl(const std::string& url, std::string param, const std::string& extraParam)
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

bool HttpExec::MethodForGet(const std::string& method)
{
    return (method == HttpConstant::HTTP_METHOD_HEAD || method == HttpConstant::HTTP_METHOD_OPTIONS ||
            method == HttpConstant::HTTP_METHOD_TRACE || method == HttpConstant::HTTP_METHOD_GET ||
            method == HttpConstant::HTTP_METHOD_CONNECT);
}

bool HttpExec::MethodForPost(const std::string& method)
{
    return (method == HttpConstant::HTTP_METHOD_POST || method == HttpConstant::HTTP_METHOD_PUT ||
            method == HttpConstant::HTTP_METHOD_DELETE);
}

bool HttpExec::EncodeUrlParam(std::string& str)
{
    static constexpr size_t ENCODE_LEN = 4;
    char encoded[ENCODE_LEN];
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

void HttpExec::RunThread()
{
    while (staticVariable_.runThread) {
        SendRequest();

        std::this_thread::sleep_for(std::chrono::milliseconds(CURL_TIMEOUT_MS));

        std::unique_lock l(staticVariable_.mutex);
        auto &contextMap = staticVariable_.contextMap;
        staticVariable_.conditionVariable.wait_for(l, std::chrono::seconds(CONDITION_TIMEOUT_S),
            [contextMap] { return !contextMap.empty(); });
    }
}

bool HttpExec::Initialize()
{
    std::lock_guard<std::mutex> lock(staticVariable_.mutexForInitialize);
    if (staticVariable_.initialized) {
        return true;
    }

    staticVariable_.workThread = std::thread(RunThread);

    staticVariable_.initialized = true;
    return staticVariable_.initialized;
}

size_t HttpExec::OnWritingMemoryBody(const void* data, size_t size, void* userData)
{
    auto context = static_cast<RequestContext*>(userData);
    if (context == nullptr) {
        return 0;
    }

    if (context->IsRequestInStream()) {
        context->SetTempData(data, size);
        NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, OnDataReceive);
        return size;
    }

    if (context->response.GetResult().size() > MAX_LIMIT) {
        return 0;
    }

    context->response.AppendResult(data, size);
    return size;
}

static void MakeSetCookieArray(napi_env env, napi_value header,
                               const std::pair<const std::basic_string<char>, std::basic_string<char>> &headerElement)
{
    std::vector<std::string> cookieVec =
        CommonUtils::Split(headerElement.second, HttpConstant::RESPONSE_KEY_SET_COOKIE_SEPARATOR);
    uint32_t index = 0;
    auto len = cookieVec.size();
    auto array = NapiUtils::CreateArray(env, len);
    for (const auto &setCookie : cookieVec) {
        auto str = NapiUtils::CreateStringUtf8(env, setCookie);
        NapiUtils::SetArrayElement(env, array, index, str);
        ++index;
    }
    NapiUtils::SetArrayProperty(env, header, HttpConstant::RESPONSE_KEY_SET_COOKIE, array);
}

static void MakeHeaderWithSetCookieArray(napi_env env, napi_value header, std::map<std::string, std::string> *headerMap)
{
    for (const auto &it : *headerMap) {
        if (!it.first.empty() && !it.second.empty()) {
            if (it.first == HttpConstant::RESPONSE_KEY_SET_COOKIE) {
                MakeSetCookieArray(env, header, it);
                continue;
            }
            NapiUtils::SetStringPropertyUtf8(env, header, it.first, it.second);
        }
    }
}

static void ResponseHeaderCallback(uv_work_t *work, int status)
{
    (void)status;

    auto workWrapper = static_cast<UvWorkWrapperShared*>(work->data);
    napi_env env = workWrapper->env;
    auto headerMap = static_cast<std::map<std::string, std::string> *>(workWrapper->data);
    auto closeScope = [env](napi_handle_scope scope) { NapiUtils::CloseScope(env, scope); };
    std::unique_ptr<napi_handle_scope__, decltype(closeScope)> scope(NapiUtils::OpenScope(env), closeScope);
    napi_value header = NapiUtils::CreateObject(env);
    if (NapiUtils::GetValueType(env, header) == napi_object) {
        MakeHeaderWithSetCookieArray(env, header, headerMap);
    }
    std::pair<napi_value, napi_value> arg = {NapiUtils::GetUndefined(env), header};
    workWrapper->manager->Emit(workWrapper->type, arg);
    delete headerMap;
    headerMap = nullptr;
    delete workWrapper;
    workWrapper = nullptr;
    delete work;
    work = nullptr;
}

static std::map<std::string, std::string> MakeHeaderWithSetCookie(RequestContext * context)
{
    std::map<std::string, std::string> tempMap = context->response.GetHeader();
    std::string setCookies;
    int loop = 0;
    for (const auto &setCookie : context->response.GetsetCookie()) {
        setCookies += setCookie;
        if (loop < context->response.GetsetCookie().size() - 1) {
            setCookies += HttpConstant::RESPONSE_KEY_SET_COOKIE_SEPARATOR;
        }
        ++loop;
    }
    tempMap[HttpConstant::RESPONSE_KEY_SET_COOKIE] = setCookies;
    return tempMap;
}

size_t HttpExec::OnWritingMemoryHeader(const void* data, size_t size, void* userData)
{
    auto context = static_cast<RequestContext*>(userData);
    if (context == nullptr) {
        context->StopAndCacheNapiPerformanceTiming(HttpConstant::RESPONSE_BODY_TIMING);
        return 0;
    }
    if (context->GetSharedManager()->IsEventDestroy()) {
        context->StopAndCacheNapiPerformanceTiming(HttpConstant::RESPONSE_HEADER_TIMING);
        return 0;
    }

    if (context->response.GetResult().size() > context->options.GetMaxLimit()) {
        NETSTACK_LOGE("response data exceeds the maximum limit");
        context->StopAndCacheNapiPerformanceTiming(HttpConstant::RESPONSE_HEADER_TIMING);
        return 0;
    }

    context->response.AppendRawHeader(data, size);

    NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, OnHeaderReceive);
    context->StopAndCacheNapiPerformanceTiming(HttpConstant::RESPONSE_HEADER_TIMING);
    return size;
}

bool HttpExec::OnSuccessResponse(HttpResponse& httpResponse, void* userData)
{
    auto context = static_cast<RequestContext*>(userData);
    if (context == nullptr) {
        return false;
    }

    context->response.SetResponseTime(HttpTime::GetNowTimeGMT());
    if (context->response.GetResponseCode() == static_cast<uint32_t>(ResponseCode::NOT_MODIFIED)) {
        context->SetResponseByCache();
    } else {
        context->response = httpResponse;
        context->response.ParseHeaders();
    }

    context->SetExecOK(true);
    if (context->IsExecOK()) {
        CacheProxy proxy(context->options);
        proxy.WriteResponseToCache(context->response);
    }

    ReleaseRequestInfo(context);

    if (context->GetSharedManager()) {
        if (context->IsRequestInStream()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(CURL_TIMEOUT_MS));
            NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, AsyncWorkRequestInStreamCallback);
        } else {
            NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, AsyncWorkRequestCallback);
        }
    }

    return true;
}

bool HttpExec::OnFailedResponse(int32_t errCode, std::string& errMessage, void* userData)
{
    auto context = static_cast<RequestContext*>(userData);
    if (context == nullptr) {
        return false;
    }

    context->SetError(errCode, errMessage);
    context->SetExecOK(false);
    ReleaseRequestInfo(context);

    if (context->GetSharedManager()) {
        if (context->IsRequestInStream()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(CURL_TIMEOUT_MS));
            NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, AsyncWorkRequestInStreamCallback);
        } else {
            NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, AsyncWorkRequestCallback);
        }
    }

    return true;
}

void HttpExec::OnDataReceive(napi_env env, napi_status status, void* data)
{
    auto context = static_cast<RequestContext*>(data);
    if (context == nullptr) {
        NETSTACK_LOGE("context is nullptr");
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
    context->EmitSharedManager(
        ON_DATA_RECEIVE, std::make_pair(NapiUtils::GetUndefined(context->GetEnv()), arrayBuffer));
}

void HttpExec::OnHeaderReceive(napi_env env, napi_status status, void* data)
{
    auto context = static_cast<RequestContext*>(data);
    if (context == nullptr) {
        return;
    }

    napi_value header = MakeResponseHeader(context->GetEnv(), context);

    napi_value undefined = NapiUtils::GetUndefined(context->GetEnv());
    context->EmitSharedManager(ON_HEADER_RECEIVE, std::make_pair(undefined, header));
    context->EmitSharedManager(ON_HEADERS_RECEIVE, std::make_pair(undefined, header));
}

void HttpExec::OnDataProgress(napi_env env, napi_status status, void* data)
{
    auto context = static_cast<RequestContext*>(data);
    if (context == nullptr) {
        NETSTACK_LOGD("OnDataProgress context is null");
        return;
    }

    auto progress = NapiUtils::CreateObject(context->GetEnv());
    if (NapiUtils::GetValueType(context->GetEnv(), progress) == napi_undefined) {
        return;
    }
    auto dlLen = context->GetDlLen();
    if (dlLen.tLen && dlLen.nLen) {
        NapiUtils::SetUint32Property(context->GetEnv(), progress, "receiveSize", static_cast<uint32_t>(dlLen.nLen));
        NapiUtils::SetUint32Property(context->GetEnv(), progress, "totalSize", static_cast<uint32_t>(dlLen.tLen));

        context->EmitSharedManager(
            ON_DATA_RECEIVE_PROGRESS, std::make_pair(NapiUtils::GetUndefined(context->GetEnv()), progress));
    }
}

void HttpExec::OnDataUploadProgress(napi_env env, napi_status status, void *data)
{
    auto context = static_cast<RequestContext *>(data);
    if (context == nullptr) {
        NETSTACK_LOGD("OnDataUploadProgress context is null");
        return;
    }
    auto progress = NapiUtils::CreateObject(context->GetEnv());
    if (NapiUtils::GetValueType(context->GetEnv(), progress) == napi_undefined) {
        NETSTACK_LOGD("OnDataUploadProgress napi_undefined");
        return;
    }
    NapiUtils::SetUint32Property(context->GetEnv(), progress, "sendSize",
                                 static_cast<uint32_t>(context->GetUlLen().nLen));
    NapiUtils::SetUint32Property(context->GetEnv(), progress, "totalSize",
                                 static_cast<uint32_t>(context->GetUlLen().tLen));
    context->EmitSharedManager(
        ON_DATA_SEND_PROGRESS, std::make_pair(NapiUtils::GetUndefined(context->GetEnv()), progress));
}

int HttpExec::ProgressCallback(void *userData, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal,
                               curl_off_t ulnow)
{
    (void)ultotal;
    (void)ulnow;
    auto context = static_cast<RequestContext*>(userData);
    if (context == nullptr || !context->IsRequestInStream()) {
        return 0;
    }
    if (ultotal != 0 && ultotal >= ulnow && !context->CompareWithLastElement(ulnow, ultotal)) {
        context->SetUlLen(ulnow, ultotal);
        NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, OnDataUploadProgress);
    }
    if (dltotal != 0) {
        context->SetDlLen(dlnow, dltotal);
        NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, OnDataProgress);
    }
    return 0;
}

void HttpExec::SendRequest()
{
    std::lock_guard<std::mutex> lock(staticVariable_.mutex);

    auto it = std::find_if(staticVariable_.contextMap.begin(), staticVariable_.contextMap.end(),
        [](const std::pair<RequestContext*, RequestInfo>& p) {
            if (p.second.requestStatus == RequestStatus::REQUEST_NOTSTART) {
                return true;
            }
            return false;
        });

    if (it != staticVariable_.contextMap.end()) {
        std::shared_ptr<HttpExecInterface> httpExec = nullptr;
        RequestContext* context = it->first;
        httpExec = it->second.httpExec;
        it->second.requestStatus = RequestStatus::REQUEST_STARTED;

        httpExec->SetSuccessCallback(HttpExec::OnSuccessResponse);
        httpExec->SetFailedCallback(HttpExec::OnFailedResponse);
        httpExec->SetProgressCallback(HttpExec::ProgressCallback);
        httpExec->SetWritingBodyCallback(HttpExec::OnWritingMemoryBody);
        httpExec->SetWritingHeaderCallback(HttpExec::OnWritingMemoryHeader);
        httpExec->SendRequest(context->options, context);
    }
}

struct curl_slist* HttpExec::MakeHeaders(const std::vector<std::string>& vec)
{
    struct curl_slist* header = nullptr;
    std::for_each(vec.begin(), vec.end(), [&header](const std::string& s) {
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
    if (!context) {
        NETSTACK_LOGE("context is nullptr");
        return header;
    }
    if (NapiUtils::GetValueType(context->GetEnv(), header) == napi_object) {
        for (const auto &it : context->response.GetHeader()) {
            if (!it.first.empty() && !it.second.empty()) {
                NapiUtils::SetStringPropertyUtf8(context->GetEnv(), header, it.first, it.second);
            }
        }
        if (!context->response.GetsetCookie().empty()) {
            uint32_t index = 0;
            auto len = context->response.GetsetCookie().size();
            auto array = NapiUtils::CreateArray(context->GetEnv(), len);
            for (const auto &setCookie : context->response.GetsetCookie()) {
                auto str = NapiUtils::CreateStringUtf8(context->GetEnv(), setCookie);
                NapiUtils::SetArrayElement(context->GetEnv(), array, index, str);
                ++index;
            }
            NapiUtils::SetArrayProperty(context->GetEnv(), header, HttpConstant::RESPONSE_KEY_SET_COOKIE, array);
        }
    }
    return header;
}

void HttpExec::OnHeaderReceive(RequestContext* context, napi_value header)
{
    napi_value undefined = NapiUtils::GetUndefined(context->GetEnv());
    context->EmitSharedManager(ON_HEADER_RECEIVE, std::make_pair(undefined, header));
    context->EmitSharedManager(ON_HEADERS_RECEIVE, std::make_pair(undefined, header));
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

bool HttpExec::ProcByExpectDataType(napi_value object, RequestContext* context)
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

void HttpExec::AsyncRunRequest(RequestContext* context)
{
    HttpAsyncWork::ExecRequest(context->GetEnv(), context);
}

bool HttpExec::IsInitialized()
{
    return staticVariable_.initialized;
}

void HttpExec::DeInitialize()
{
    std::lock_guard<std::mutex> lock(staticVariable_.mutex);
    staticVariable_.runThread = false;
    if (staticVariable_.workThread.joinable()) {
        staticVariable_.workThread.join();
    }
    staticVariable_.initialized = false;
}

void HttpExec::ReleaseRequestInfo(RequestContext* context)
{
    std::lock_guard<std::mutex> lock(staticVariable_.mutex);
    auto it = staticVariable_.contextMap.find(context);
    if (it == staticVariable_.contextMap.end()) {
        NETSTACK_LOGE("can not find context");
        return;
    }

    staticVariable_.contextMap.erase(it);
}

std::string HttpExec::GetCacheFileName()
{
    std::shared_ptr<HttpExecInterface> httpExec = std::make_shared<HttpExecIosIml>();
    return httpExec->GetCacheFileName();
}

bool HttpResponseCacheExec::ExecFlush(BaseContext* context)
{
    (void)context;
    CacheProxy::FlushCache();
    return true;
}

napi_value HttpResponseCacheExec::FlushCallback(BaseContext* context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}

bool HttpResponseCacheExec::ExecDelete(BaseContext* context)
{
    (void)context;
    CacheProxy::StopCacheAndDelete();
    return true;
}

napi_value HttpResponseCacheExec::DeleteCallback(BaseContext* context)
{
    return NapiUtils::GetUndefined(context->GetEnv());
}
} // namespace OHOS::NetStack::Http
