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

#ifndef PLUGINS_NET_HTTP_HTTP_EXEC_INTERFACE_H
#define PLUGINS_NET_HTTP_HTTP_EXEC_INTERFACE_H

#include "http_request_options.h"
#include "http_response.h"

namespace OHOS::NetStack::Http {
using WritingBodyCallback = std::function<size_t(const void* data, size_t size, void* userData)>;

using WritingHeaderCallback = std::function<size_t(const void* data, size_t size, void* userData)>;

using ProgressCallback = std::function<int(void* userData, long dltotal, long dlnow, long ultotal, long ulnow)>;

using SuccessCallback = std::function<bool(HttpResponse& httpResponse, void* userData)>;

using FailedCallback = std::function<bool(int32_t errCode, std::string& errMessage, void* userData)>;

class HttpExecInterface {
public:
    HttpExecInterface() = default;

    virtual ~HttpExecInterface() = default;

    virtual bool SendRequest(HttpRequestOptions& requestOptions, void* userData) = 0;

    virtual void SetSuccessCallback(SuccessCallback successResponseCallback) = 0;

    virtual void SetFailedCallback(FailedCallback failedResponseCallback) = 0;

    virtual void SetProgressCallback(ProgressCallback progressCallback) = 0;

    virtual void SetWritingBodyCallback(WritingBodyCallback writingBodyCallback) = 0;

    virtual void SetWritingHeaderCallback(WritingHeaderCallback writingHeaderCallback) = 0;

    virtual std::string GetCacheFileName() = 0;
};
} // namespace OHOS::NetStack::Http
#endif /* PLUGINS_NET_HTTP_HTTP_EXEC_INTERFACE_H */
