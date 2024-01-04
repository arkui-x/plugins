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

#ifndef COMMUNICATIONNETSTACK_HTTP_REQUEST_EXEC_H
#define COMMUNICATIONNETSTACK_HTTP_REQUEST_EXEC_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>
#include <set>

#include "curl/curl.h"
#include "http_exec_interface.h"
#include "napi/native_api.h"
#include "request_context.h"

namespace OHOS::NetStack::Http {
class HttpResponseCacheExec final {
public:
    HttpResponseCacheExec() = default;

    ~HttpResponseCacheExec() = default;

    static bool ExecFlush(BaseContext* context);

    static napi_value FlushCallback(BaseContext* context);

    static bool ExecDelete(BaseContext* context);

    static napi_value DeleteCallback(BaseContext* context);
};

class HttpExec final {
public:
    HttpExec() = default;

    ~HttpExec() = default;

    static bool RequestWithoutCache(RequestContext* context);

    static bool ExecRequest(RequestContext* context);

    static napi_value BuildRequestCallback(RequestContext *context);

    static napi_value RequestCallback(RequestContext* context);

    static napi_value RequestInStreamCallback(RequestContext* context);

    static std::string MakeUrl(const std::string& url, std::string param, const std::string& extraParam);

    static bool MethodForGet(const std::string& method);

    static bool MethodForPost(const std::string& method);

    static bool EncodeUrlParam(std::string& str);

    static bool Initialize();

    static bool IsInitialized();

    static void DeInitialize();

    static void AsyncRunRequest(RequestContext* context);

    struct StaticContextVec {
        StaticContextVec() = default;
        ~StaticContextVec() = default;
        std::mutex mutexForContextVec;
        std::set<RequestContext *> contextSet;
    };
    static StaticContextVec staticContextSet_;

    static std::string GetCacheFileName();

private:
    static size_t OnWritingMemoryBody(const void* data, size_t size, void* userData);

    static size_t OnWritingMemoryHeader(const void* data, size_t size, void* userData);

    static bool OnSuccessResponse(HttpResponse& response, void* userData);

    static bool OnFailedResponse(int32_t errCode, std::string& errMessage, void* userData);

    static struct curl_slist* MakeHeaders(const std::vector<std::string>& vec);

    static napi_value MakeResponseHeader(napi_env env, void *ctx);

    static void OnHeaderReceive(RequestContext* context, napi_value header);

    static bool IsUnReserved(unsigned char in);

    static bool ProcByExpectDataType(napi_value object, RequestContext* context);

    static void RunThread();

    static void OnDataReceive(napi_env env, napi_status status, void* data);

    static void OnHeaderReceive(napi_env env, napi_status status, void* data);

    static void OnDataProgress(napi_env env, napi_status status, void* data);

    static void OnDataUploadProgress(napi_env env, napi_status status, void *data);

    static int ProgressCallback(void *userData, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal,
                                curl_off_t ulnow);

    static void SendRequest();

    static void ReleaseRequestInfo(RequestContext* context);

    enum class RequestStatus {
        REQUEST_NOTSTART,
        REQUEST_STARTED,
    };

    struct RequestInfo {
        RequestInfo() = delete;
        explicit RequestInfo(std::shared_ptr<HttpExecInterface> httpexe)
            : httpExec(httpexe), requestStatus(RequestStatus::REQUEST_NOTSTART) {}

        ~RequestInfo() = default;

        std::shared_ptr<HttpExecInterface> httpExec;
        RequestStatus requestStatus;
    };

    struct StaticVariable {
        StaticVariable() : initialized(false), runThread(true) {}

        ~StaticVariable()
        {
            if (HttpExec::IsInitialized()) {
                HttpExec::DeInitialize();
            }
        }

        std::mutex mutex;
        std::mutex mutexForInitialize;
        std::map<RequestContext*, RequestInfo> contextMap;
        std::thread workThread;
        std::condition_variable conditionVariable;

        std::atomic_bool initialized;
        std::atomic_bool runThread;
    };

    static StaticVariable staticVariable_;
};
} // namespace OHOS::NetStack::Http
#endif /* COMMUNICATIONNETSTACK_HTTP_REQUEST_EXEC_H */
