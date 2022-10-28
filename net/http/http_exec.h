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

#ifndef PLUGINS_NET_HTTP_REQUEST_EXEC_H
#define PLUGINS_NET_HTTP_REQUEST_EXEC_H

#include <mutex>
#include <vector>

#include "curl/curl.h"
#include "napi/native_api.h"
#include "request_context.h"
#ifndef MAC_PLATFORM
#include "thread_pool.h"
#endif

namespace OHOS::NetStack {
#ifndef MAC_PLATFORM
static constexpr const size_t DEFAULT_THREAD_NUM = 5;
static constexpr const size_t MAX_THREAD_NUM = 100;
static constexpr const uint32_t DEFAULT_TIMEOUT = 5;
#endif

class HttpResponseCacheExec final {
public:
    HttpResponseCacheExec() = default;

    ~HttpResponseCacheExec() = default;

    static bool ExecFlush(BaseContext *context);

    static napi_value FlushCallback(BaseContext *context);

    static bool ExecDelete(BaseContext *context);

    static napi_value DeleteCallback(BaseContext *context);
};

class HttpExec final {
public:
    HttpExec() = default;

    ~HttpExec() = default;

    static bool RequestWithoutCache(RequestContext *context, const std::string &cacert = "");

    static bool ExecRequest(RequestContext *context);

    static napi_value RequestCallback(RequestContext *context);

    static std::string MakeUrl(const std::string &url, std::string param, const std::string &extraParam);

    static bool MethodForGet(const std::string &method);

    static bool MethodForPost(const std::string &method);

    static bool EncodeUrlParam(std::string &str);

    static bool Initialize();

#ifndef MAC_PLATFORM
    static void AsyncRunRequest(RequestContext *context);
#endif

private:
    static bool SetCertification(CURL *curl, RequestContext *context, const std::string &cacert);
    static bool SetOptionParam(CURL *curl, RequestContext *context, struct curl_slist *requestHeader,
        const std::string &method);
    static bool SetOption(CURL *curl, RequestContext *context, struct curl_slist *requestHeader,
        const std::string &cacert);

    static size_t OnWritingMemoryBody(const void *data, size_t size, size_t memBytes, void *userData);

    static size_t OnWritingMemoryHeader(const void *data, size_t size, size_t memBytes, void *userData);

    static struct curl_slist *MakeHeaders(const std::vector<std::string> &vec);

    static napi_value MakeResponseHeader(RequestContext *context);

    static void OnHeaderReceive(RequestContext *context, napi_value header);

    static bool IsUnReserved(unsigned char in);

    static bool ProcByExpectDataType(napi_value object, RequestContext *context);

    static void GetCacertListFromSystem(std::vector<std::string> &cacertList);

#ifndef MAC_PLATFORM
private:
    class Task {
    public:
        Task() = default;

        explicit Task(RequestContext *context);

        bool operator<(const Task &e) const;

        void Execute();

    private:
        RequestContext *context_ = nullptr;
    };
#endif

    static std::mutex mutex_;

#ifndef MAC_PLATFORM
    static ThreadPool<Task, DEFAULT_THREAD_NUM, MAX_THREAD_NUM> threadPool_;

    static std::atomic_bool initialized_;
#else
    static bool initialized_;
#endif
};
} // namespace OHOS::NetStack

#endif /* PLUGINS_NET_HTTP_REQUEST_EXEC_H */
