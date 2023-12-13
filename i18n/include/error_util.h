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
#ifndef ERROR_UTIL_H
#define ERROR_UTIL_H

#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Global {
namespace I18n {
// Common error code
const int32_t I18N_NO_PERMISSION  = 201;          // The app don't have requested permission
const int32_t I18N_NOT_FOUND     = 401;          // Input parameter is missing

// I18N error code
const int32_t I18N_NOT_VALID  = 890001;    // Unspported para value
const int32_t I18N_OPTION_NOT_VALID   = 890002;    // Unspported option value

class ErrorUtil {
public:
    static void NapiThrow(napi_env env, int32_t errCode, bool throwError);
};
} // namespace I18n
} // namespace Global
} // namespace OHOS
#endif