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

#ifndef PLUGINS_ABILITY_ACCESS_CONTROL_NAPI_COMMON_H
#define PLUGINS_ABILITY_ACCESS_CONTROL_NAPI_COMMON_H
#include <string>
#include <vector>
#include "napi/native_api.h"
#include "napi/native_node_api.h"
namespace OHOS::Plugin {
struct AtManagerAsyncWorkData {
    explicit AtManagerAsyncWorkData(napi_env envValue);
    virtual ~AtManagerAsyncWorkData();

    napi_env        env = nullptr;
    napi_async_work work = nullptr;
    napi_deferred   deferred = nullptr;
    napi_ref        callbackRef = nullptr;
};

struct AtManagerAsyncContext : public AtManagerAsyncWorkData {
    explicit AtManagerAsyncContext(napi_env env) : AtManagerAsyncWorkData(env) {}
    uint32_t tokenId = 0;
    std::string permission;
    int32_t status;
    int32_t jsCode = 0;
};

struct ResultCallback {
    std::vector<std::string> permissions;
    std::vector<int32_t> grantResults;
    int32_t requestCode;
    void* data = nullptr;
};

struct RequestAsyncContext : public AtManagerAsyncWorkData {
    explicit RequestAsyncContext(napi_env env) : AtManagerAsyncWorkData(env) {}
    bool isResultCalled = true;
    int32_t result = 0;
    std::vector<std::string> permissionList;
    std::vector<int32_t> permissionsState;
    napi_value requestResult = nullptr;
};

bool ParseUint32(const napi_env& env, const napi_value& value, uint32_t& result);
bool ParseString(const napi_env& env, const napi_value& value, std::string& result);
bool ParseStringArray(const napi_env& env, const napi_value& value, std::vector<std::string>& result);
bool ParseCallback(const napi_env& env, const napi_value& value, napi_ref& result);
} // namespace OHOS::Plugin
#endif // PLUGINS_ABILITY_ACCESS_CONTROL_NAPI_COMMON_H