/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "vibrator_napi_error.h"

#include <optional>

#undef LOG_TAG
#define LOG_TAG "VibratorNapiError"

namespace OHOS {
namespace Sensors {
napi_value CreateBusinessError(const napi_env &env, const int32_t errCode, const std::string &errMessage)
{
    napi_value businessError = nullptr;
    napi_value code = nullptr;
    napi_value msg = nullptr;
    NAPI_CALL(env, napi_create_int32(env, errCode, &code));
    NAPI_CALL(env, napi_create_string_utf8(env, errMessage.c_str(), NAPI_AUTO_LENGTH, &msg));
    NAPI_CALL(env, napi_create_error(env, nullptr, msg, &businessError));
    napi_set_named_property(env, businessError, "code", code);
    return businessError;
}

std::optional<std::string> GetNapiError(int32_t errorCode)
{
    auto iter = ERROR_MESSAGES.find(errorCode);
    if (iter != ERROR_MESSAGES.end()) {
        return iter->second;
    }
    return std::nullopt;
}

void ThrowErr(const napi_env &env, const int32_t errCode, const std::string &printMsg)
{
    MISC_HILOGE("Message:%{public}s, code:%{public}d", printMsg.c_str(), errCode);
    auto msg = GetNapiError(errCode);
    if (!msg) {
        MISC_HILOGE("ErrCode:%{public}d is invalid", errCode);
        return;
    }
    napi_handle_scope scope = nullptr;
    napi_open_handle_scope(env, &scope);
    napi_value error = CreateBusinessError(env, errCode, msg.value());
    napi_throw(env, error);
    napi_close_handle_scope(env, scope);
}
} // namespace Sensors
} // namespace OHOS