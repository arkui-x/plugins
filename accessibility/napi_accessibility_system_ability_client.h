/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef PLUGIN_ACCESSIBILITY_NAPI_ACCESSIBILITY_SYSTEM_ABILITY_CLIENT_H
#define PLUGIN_ACCESSIBILITY_NAPI_ACCESSIBILITY_SYSTEM_ABILITY_CLIENT_H

#include <map>

#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

constexpr size_t ARGS_SIZE_ZERO = 0;
constexpr size_t ARGS_SIZE_ONE = 1;
constexpr size_t ARGS_SIZE_TWO = 2;
const std::string ERROR_MESSAGE_PARAMETER_ERROR =
    "Parameter error. Possible causes:"
    "1. Mandatory parameters are left unspecified; 2. Incorrect parameter types; 3. Parameter verification failed.";
const std::string ERROR_MESSAGE_SYSTEM_ABNORMALITY = "System abnormality";

enum RetError : int32_t {
    RET_OK = 0,
    RET_ERR_FAILED = -1,
    RET_ERR_INVALID_PARAM = 1001,
};

enum class NAccessibilityErrorCode : int32_t {
    ACCESSIBILITY_OK = 0,
    ACCESSIBILITY_ERROR_INVALID_PARAM = 401,
    ACCESSIBILITY_ERROR_SYSTEM_ABNORMALITY = 9300000,
};

struct NAccessibilityErrMsg {
    NAccessibilityErrorCode errCode;
    std::string message;
};

namespace OHOS::Plugin {
class NAccessibilityClient {
public:
    NAccessibilityClient() = default;
    ~NAccessibilityClient() = default;
    static napi_value IsOpenAccessibilitySync(napi_env env, napi_callback_info info);
    static napi_value SubscribeState(napi_env env, napi_callback_info info);
    static napi_value UnsubscribeState(napi_env env, napi_callback_info info);
};
} // namespace OHOS::Plugin
#endif // PLUGIN_ACCESSIBILITY_NAPI_ACCESSIBILITY_SYSTEM_ABILITY_CLIENT_H
