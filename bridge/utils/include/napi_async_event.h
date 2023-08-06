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

#ifndef PLUGINS_BRIDGE_NAPI_ASYNC_EVENT_H
#define PLUGINS_BRIDGE_NAPI_ASYNC_EVENT_H

#include <functional>
#include <memory>
#include <string>

#include "error_code.h"
#include "napi/native_api.h"
#include "plugins/interfaces/native/plugin_utils.h"
#include "plugins/interfaces/native/inner_api/plugin_utils_napi.h"

namespace OHOS::Plugin::Bridge {
using OnAsyncEventSuccess = std::function<void(
    napi_env env, const std::string& bridgeName, const std::string& methodName, napi_value resultValue)>;
using OnAsyncEventError =
    std::function<void(napi_env env, const std::string& bridgeName, const std::string& methodName, int errorCode)>;

class NAPIAsyncEvent {
public:
    explicit NAPIAsyncEvent(napi_env env);
    ~NAPIAsyncEvent();

    void SetAsyncEventSuccess(OnAsyncEventSuccess eventSuccess);
    void SetAsyncEventError(OnAsyncEventError eventError);
    void SetErrorCode(int errorCode);
    napi_env GetEnv(void);
    int GetErrorCode(void);
    const std::string& GetAsyncWorkName(void);
    void SetBridgeName(const std::string& bridgeName);
    void SetData(napi_value data);
    napi_value GetData(void);
    void SetRefData(napi_value data);
    napi_value GetRefData(void);
    void SetRefErrorData(napi_value data);
    napi_value GetRefErrorData(void);
    void SetMethodParameter(const std::string& jsonStr);

    bool CreateCallback(napi_value callback);
    void DeleteCallback(void);
    bool IsCallback(void);
    bool CreateAsyncWork(const std::string& asyncWorkName,
        AsyncWorkExecutor executor, AsyncWorkComplete callback);
    void DeleteAsyncWork(void);
    napi_value CreatePromise(void);
    void AsyncWorkCallback(void);
    void AsyncWorkCallMethod(void);
    void AsyncWorkMessage(void);

private:
    napi_env env_ = nullptr;
    napi_ref callback_ = nullptr;
    napi_async_work asyncWork_ = nullptr;
    napi_deferred deferred_ = nullptr;
    std::string asyncWorkName_;
    std::string bridgeName_;
    napi_value data_ = nullptr;
    napi_ref refData_ = nullptr;
    napi_ref refErrorData_ = nullptr;
    int errorCode_ = 0;
    std::string methodParameter_;
    OnAsyncEventSuccess eventSuccess_ = nullptr;
    OnAsyncEventError eventError_ = nullptr;

    void TriggerEventSuccess(napi_value result);
    void TriggerEventError(ErrorCode code);
};
} // namespace OHOS::Plugin::Bridge
#endif // PLUGINS_BRIDGE_NAPI_ASYNC_EVENT_H