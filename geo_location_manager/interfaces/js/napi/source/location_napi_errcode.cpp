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

#include "location_napi_errcode.h"
#include <map>
#include "common_utils.h"
#include "napi_util.h"

namespace OHOS {
namespace Location {
void HandleSyncErrCode(const napi_env &env, int32_t errCode)
{
    LBSLOGI(LOCATOR_STANDARD, "HandleSyncErrCode, errCode = %{public}d", errCode);
    std::string errMsg = GetErrorMsgByCode(errCode);
    errCode = ConvertErrorCode(errCode);
    if (errMsg != "") {
        napi_throw_error(env, std::to_string(errCode).c_str(), errMsg.c_str());
    }
}

void ThrowBusinessError(const napi_env &env, int32_t errCode)
{
    LBSLOGI(LOCATOR_STANDARD, "ThrowBusinessError, errCode = %{public}d", errCode);
    std::string errMsg = GetErrorMsgByCode(errCode);
    if (errMsg == "") {
        return;
    }
    errCode = ConvertErrorCode(errCode);
    auto businessError = CreateError(env, errCode, errMsg);
    napi_throw(env, businessError);
}
}  // namespace Location
}  // namespace OHOS
