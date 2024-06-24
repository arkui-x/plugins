/*
 * Copyright (C) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef WIFI_NAPI_UTILS_H_
#define WIFI_NAPI_UTILS_H_

#include <chrono>
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS::Plugin {
static const std::int32_t SYSCAP_WIFI_STA = 2500000;
static const int32_t WIFI_NAPI_MAX_PARA = 2;

#define EVENT_STA_POWER_STATE_CHANGE "wifiStateChange"
#define EVENT_STA_CONN_STATE_CHANGE "wifiConnectionChange"

class RegObj {
public:
    RegObj() : m_regEnv(0), m_regHanderRef(nullptr) {}
    explicit RegObj(const napi_env& env, const napi_ref& ref) : m_regEnv(env),m_regHanderRef(ref){}

    ~RegObj() {}

    bool operator==(const RegObj& other) const
    {
        return m_regEnv == other.m_regEnv && m_regHanderRef == other.m_regHanderRef;
    }

    bool operator!=(const RegObj& other) const
    {
        return !(*this == other);
    }

    bool operator<(const RegObj& other) const
    {
        return m_regEnv < other.m_regEnv || (m_regEnv == other.m_regEnv && m_regHanderRef < other.m_regHanderRef);
    }

    napi_env m_regEnv;
    napi_ref m_regHanderRef;
};

class TraceFuncCall final {
public:
    TraceFuncCall(std::string funcName);

    TraceFuncCall() = delete;

    ~TraceFuncCall();

private:
    std::string m_funcName;
    std::chrono::steady_clock::time_point m_startTime;
    bool m_isTrace = true;
};

#define TRACE_FUNC_CALL TraceFuncCall func(__func__)
#define TRACE_FUNC_CALL_NAME(name) TraceFuncCall funcName(name)

#ifndef NO_SANITIZE
#ifdef __has_attribute
#if __has_attribute(no_sanitize)
#define NO_SANITIZE(type) __attribute__((no_sanitize(type)))
#endif
#endif
#endif

#ifndef NO_SANITIZE
#define NO_SANITIZE(type)
#endif

constexpr int ERR_CODE_SUCCESS = 0;

struct WifiCallbackWorker {
    napi_env env = nullptr;
    napi_ref ref = nullptr;
    int32_t code = 0;
};

class AsyncContext {
public:
    napi_env env;
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callback[2] = { 0 };
    std::function<void(void*)> executeFunc;
    std::function<void(void*)> completeFunc;
    napi_value resourceName;
    napi_value result;
    int32_t sysCap;
    int errorCode;

    AsyncContext(napi_env e, napi_async_work w = nullptr, napi_deferred d = nullptr) : env(e), 
    work(w), 
    deferred(d), 
    executeFunc(nullptr), 
    completeFunc(nullptr), 
    result(nullptr), 
    sysCap(0), 
    errorCode(ERR_CODE_SUCCESS) {}

    AsyncContext() = delete;

    virtual ~AsyncContext() {}
};

napi_value UndefinedNapiValue(const napi_env& env);
napi_status SetValueUtf8String(
    const napi_env& env, const char* fieldStr, const char* str, napi_value& result, size_t strLen = NAPI_AUTO_LENGTH);
napi_status SetValueUtf8String(
    const napi_env& env, const std::string& fieldStr, const std::string& valueStr, napi_value& result);
napi_status SetValueInt32(const napi_env& env, const char* fieldStr, const int intValue, napi_value& result);
napi_status SetValueUnsignedInt32(const napi_env& env, const char* fieldStr, const int intValue, napi_value& result);
napi_status SetValueBool(const napi_env& env, const char* fieldStr, const bool boolValue, napi_value& result);
napi_value DoAsyncWork(const napi_env& env, AsyncContext* asyncContext, const size_t argc, const napi_value* argv,
    const size_t nonCallbackArgNum);
} // namespace OHOS::Plugin

#endif
