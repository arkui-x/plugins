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

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <memory>
#include <thread>

#include "base64_utils.h"
#include "cache_proxy.h"
#include "constant.h"
#include "event_list.h"
#include "http_async_work.h"
#include "http_exec_ios_iml.h"
#include "http_time.h"
#include "napi_utils.h"
#include "netstack_common_utils.h"
#include "netstack_log.h"
#include "securec.h"

namespace OHOS::NetStack::Http {
static constexpr size_t MAX_LIMIT = 5 * 1024 * 1024;
static constexpr int CURL_TIMEOUT_MS = 50;
static constexpr int CONDITION_TIMEOUT_S = 3600;

HttpExec::StaticVariable HttpExec::staticVariable_;

bool HttpExec::RequestWithoutCache(RequestContext* context)
{
    std::vector<std::string> vec;
    std::for_each(context->options.GetHeader().begin(), context->options.GetHeader().end(),
        [&vec](const std::pair<std::string, std::string>& p) {
            vec.emplace_back(p.first + HttpConstant::HTTP_HEADER_SEPARATOR + p.second);
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
    context->options.SetRequestTime(HttpTime::GetNowTimeGMT());
    CacheProxy proxy(context->options);
    if (context->IsUsingCache() && proxy.ReadResponseFromCache(context)) {
        return true;
    }

    if (!RequestWithoutCache(context)) {
        context->SetErrorCode(NapiUtils::NETSTACK_NAPI_INTERNAL_ERROR);
        if (context->GetManager()->IsManagerValid()) {
            if (context->IsRequest2()) {
                NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, HttpAsyncWork::Request2Callback);
            } else {
                NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, HttpAsyncWork::RequestCallback);
            }
        }
        return false;
    }
    return true;
}

napi_value HttpExec::RequestCallback(RequestContext* context)
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

    auto contentType = CommonUtils::ToLower(const_cast<std::map<std::string, std::string>&>(
        context->response.GetHeader())[HttpConstant::HTTP_CONTENT_TYPE]);
    if (contentType.find(HttpConstant::HTTP_CONTENT_TYPE_OCTET_STREAM) != std::string::npos ||
        contentType.find(HttpConstant::HTTP_CONTENT_TYPE_IMAGE) != std::string::npos) {
        void* data = nullptr;
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

napi_value HttpExec::Request2Callback(RequestContext* context)
{
    return nullptr;
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
            method == HttpConstant::HTTP_METHOD_DELETE || method == HttpConstant::HTTP_METHOD_TRACE ||
            method == HttpConstant::HTTP_METHOD_GET || method == HttpConstant::HTTP_METHOD_CONNECT);
}

bool HttpExec::MethodForPost(const std::string& method)
{
    return (method == HttpConstant::HTTP_METHOD_POST || method == HttpConstant::HTTP_METHOD_PUT);
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

        std::mutex m;
        std::unique_lock l(m);
        auto &contextMap = staticVariable_.contextMap;
        staticVariable_.conditionVariable.wait_for(l, std::chrono::seconds(CONDITION_TIMEOUT_S),
            [contextMap] { return !contextMap.empty(); });
    }
}

bool HttpExec::Initialize()
{
    std::lock_guard<std::mutex> lock(staticVariable_.mutex);
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

    if (context->IsRequest2()) {
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

size_t HttpExec::OnWritingMemoryHeader(const void* data, size_t size, void* userData)
{
    auto context = static_cast<RequestContext*>(userData);
    if (context == nullptr) {
        return 0;
    }

    if (context->response.GetResult().size() > MAX_LIMIT) {
        return 0;
    }

    context->response.AppendRawHeader(data, size);

    if (context->IsRequest2()) {
        NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, OnHeaderReceive);
        return size;
    }

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

    if (context->GetManager()->IsManagerValid()) {
        if (context->IsRequest2()) {
            if (context->IsExecOK()) {
                NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, OnDataEnd);
            }
            NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, HttpAsyncWork::Request2Callback);
        } else {
            NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, HttpAsyncWork::RequestCallback);
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

    if (context->GetManager()->IsManagerValid()) {
        if (context->IsRequest2()) {
            NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, HttpAsyncWork::Request2Callback);
        } else {
            NapiUtils::CreateUvQueueWorkEnhanced(context->GetEnv(), context, HttpAsyncWork::RequestCallback);
        }
    }

    return true;
}

void HttpExec::OnDataReceive(napi_env env, napi_status status, void* data)
{
    auto context = static_cast<RequestContext*>(data);
    if (context == nullptr) {
        return;
    }

    void* buffer = nullptr;
    const auto& tempData = context->GetTempData();
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

void HttpExec::OnHeaderReceive(napi_env env, napi_status status, void* data)
{
    auto context = static_cast<RequestContext*>(data);
    if (context == nullptr) {
        return;
    }

    napi_value header = MakeResponseHeader(context);

    napi_value undefined = NapiUtils::GetUndefined(context->GetEnv());
    context->Emit(ON_HEADER_RECEIVE, std::make_pair(undefined, header));
    context->Emit(ON_HEADERS_RECEIVE, std::make_pair(undefined, header));
}

void HttpExec::OnDataProgress(napi_env env, napi_status status, void* data)
{
    auto context = static_cast<RequestContext*>(data);
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
    context->Emit(ON_DATA_PROGRESS, std::make_pair(NapiUtils::GetUndefined(context->GetEnv()), progress));
}

void HttpExec::OnDataEnd(napi_env env, napi_status status, void* data)
{
    auto context = static_cast<RequestContext*>(data);
    if (context == nullptr) {
        return;
    }
    auto undefined = NapiUtils::GetUndefined(context->GetEnv());
    context->Emit(ON_DATA_END, std::make_pair(undefined, undefined));
}

int HttpExec::ProgressCallback(long dltotal, long dlnow, long ultotal, long ulnow, void* userData)
{
    (void)ultotal;
    (void)ulnow;
    auto context = static_cast<RequestContext*>(userData);
    if (context == nullptr || !context->IsRequest2()) {
        return 0;
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
        RequestContext* context = nullptr;
        context = it->first;
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

napi_value HttpExec::MakeResponseHeader(RequestContext* context)
{
    napi_value header = NapiUtils::CreateObject(context->GetEnv());
    if (NapiUtils::GetValueType(context->GetEnv(), header) == napi_object) {
        std::for_each(context->response.GetHeader().begin(), context->response.GetHeader().end(),
            [context, header](const std::pair<std::string, std::string>& p) {
                if (!p.first.empty() && !p.second.empty()) {
                    NapiUtils::SetStringPropertyUtf8(context->GetEnv(), header, p.first, p.second);
                }
            });
    }
    return header;
}

void HttpExec::OnHeaderReceive(RequestContext* context, napi_value header)
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
    staticVariable_.runThread = false;
    if (staticVariable_.workThread.joinable()) {
        staticVariable_.workThread.join();
    }
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
