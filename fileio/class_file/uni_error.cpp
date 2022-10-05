/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "uni_error.h"

#include <cerrno>
#include <string>

#include "js_native_api.h"
#include "native/log.h"
#include "napi/n_val.h"

namespace OHOS {
namespace DistributedFS {
using namespace std;

UniError::UniError() {}

UniError::UniError(ELegacy eLegacy) : errno_(static_cast<int>(eLegacy)),
    codingSystem_(ErrCodeSystem::ERR_CODE_SYSTEM_LEGACY) {}

UniError::UniError(int ePosix) : errno_(ePosix), codingSystem_(ErrCodeSystem::ERR_CODE_SYSTEM_POSIX) {}

UniError::operator bool() const
{
    return errno_ != 0;
}

int UniError::GetErrno(ErrCodeSystem cs)
{
    if (errno_ == 0) {
        return 0;
    }
    if (cs == codingSystem_) {
        return errno_;
    }

    if (cs == ErrCodeSystem::ERR_CODE_SYSTEM_POSIX) {
        // Note that we should support more codes here
        return EINVAL;
    }

    // Note that this shall be done properly
    return static_cast<int>(ELegacy::ELEGACY_INVAL);
}

void UniError::SetErrno(ELegacy eLegacy)
{
    errno_ = static_cast<int>(eLegacy);
    codingSystem_ = ErrCodeSystem::ERR_CODE_SYSTEM_LEGACY;
}

void UniError::SetErrno(int ePosix)
{
    errno_ = ePosix;
    codingSystem_ = ErrCodeSystem::ERR_CODE_SYSTEM_POSIX;
}

std::string UniError::GetDefaultErrstr()
{
    if (codingSystem_ != ErrCodeSystem::ERR_CODE_SYSTEM_POSIX &&
        codingSystem_ != ErrCodeSystem::ERR_CODE_SYSTEM_LEGACY) {
        return "BUG: Curious coding system";
    }
    return strerror(GetErrno(ErrCodeSystem::ERR_CODE_SYSTEM_POSIX));
}

napi_value UniError::GetNapiErr(napi_env env)
{
    return GetNapiErr(env, GetDefaultErrstr());
}

napi_value UniError::GetNapiErr(napi_env env, string errMsg)
{
    int errCode = GetErrno(codingSystem_);
    napi_value code = NVal::CreateUTF8String(env, to_string(errCode)).val_;
    napi_value msg = NVal::CreateUTF8String(env, errMsg).val_;

    napi_value res = nullptr;
    napi_status createRes = napi_create_error(env, code, msg, &res);
    if (createRes) {
        LOGE("Failed to create an exception, msg = %{public}s", errMsg.c_str());
    }
    return res;
}

void UniError::ThrowErr(napi_env env)
{
    string msg = GetDefaultErrstr();
    napi_value tmp = nullptr;
    napi_get_and_clear_last_exception(env, &tmp);
    // Note that ace engine cannot throw errors created by napi_create_error so far
    napi_status throwStatus = napi_throw_error(env, nullptr, msg.c_str());
    if (throwStatus != napi_ok) {
        LOGE("Failed to throw an exception, %{public}d, code = %{public}s", throwStatus, msg.c_str());
    }
}

void UniError::ThrowErr(napi_env env, string errMsg)
{
    napi_value tmp = nullptr;
    napi_get_and_clear_last_exception(env, &tmp);
    // Note that ace engine cannot throw errors created by napi_create_error so far
    napi_status throwStatus = napi_throw_error(env, nullptr, errMsg.c_str());
    if (throwStatus != napi_ok) {
        LOGE("Failed to throw an exception, %{public}d, code = %{public}s", throwStatus, errMsg.c_str());
    }
}
} // namespace DistributedFS
} // namespace OHOS