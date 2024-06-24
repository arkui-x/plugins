/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef NAPI_COMMON_H
#define NAPI_COMMON_H

#include "js_native_api.h"
#include "js_native_api_types.h"
#include "log.h"
#include "napi/native_api.h"
#include "native_document_view_picker.h"

namespace OHOS::Plugin {
#define NAPI_CALL_CHECK(theCall)                                                                  \
    do {                                                                                          \
        napi_status status = (theCall);                                                           \
        if (status != napi_ok) {                                                                  \
            LOGE("NAPI_CALL error, status:%d, filename:%s, line:%d", status, __FILE__, __LINE__); \
            return nullptr;                                                                       \
        }                                                                                         \
    } while (0)

bool IsArray(const napi_env& env, const napi_value& value);
bool CheckType(const napi_env& env, const napi_value& value, const napi_valuetype& type);
bool ParseString(const napi_env& env, const napi_value& value, std::string& result);
bool ParseStringArray(const napi_env& env, const napi_value& value, std::vector<std::string>& result);
} // namespace OHOS::Plugin

#endif // NAPI_COMMON_H