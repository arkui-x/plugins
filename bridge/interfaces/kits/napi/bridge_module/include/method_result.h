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

#ifndef PLUGINS_Bridge_METHOD_RESULT_H
#define PLUGINS_Bridge_METHOD_RESULT_H

#include <string>

#include "buffer_mapping.h"
#include "base/utils/macros.h"
#include "napi/native_api.h"
#include "nlohmann/json.hpp"

namespace OHOS::Plugin::Bridge {
using Json = nlohmann::json;
class MethodResult final {
public:
    MethodResult() = default;
    ~MethodResult() = default;

    void SetMethodName(const std::string& methodName);
    const std::string& GetMethodName(void) const;
    void SetErrorCode(int errorCode);
    void SetErrorCodeInfo(int errorCode);
    int GetErrorCode(void) const;
    void SetResult(const std::string& value);
    const std::string& GetResult(void) const;
    void SetErrorResult(napi_value value);
    napi_value GetErrorResult(void) const;
    void SetOkResult(napi_value value);
    napi_value GetOkResult(void) const;

    void ParsePlatformMethodResult(napi_env env, const std::string& result);
    void ParseJSMethodResult(napi_env env, napi_value result);
    void CreateMethodResultForError(void);
    void CreateErrorObject(napi_env env);
    void CreateDefaultJsonString(void);

    void ParsePlatformMethodResultBinary(napi_env env, int errorCode,
        const std::string& errorMessage, std::unique_ptr<Ace::Platform::BufferMapping> result);
    void ParseJSMethodResultBinary(napi_env env, napi_value result);
    std::vector<uint8_t>* GetResultBinary(void);
    std::string GetErrorMessage() const;

private:
    int errorCode_ = 0;
    std::string methodName_;
    std::string result_;
    std::string errcodeMessage_;
    napi_value errorResult_ = nullptr;
    napi_value okResult_ = nullptr;
    // for binary codec
    std::vector<uint8_t>* binaryResult_;

    void GetErrorInfoByErrorCode(void);
};
} // namespace OHOS::Plugin::Bridge
#endif
