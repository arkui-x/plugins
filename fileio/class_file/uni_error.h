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

#ifndef INTERFACES_KITS_NAPI_COMMON_UNI_ERROR_H
#define INTERFACES_KITS_NAPI_COMMON_UNI_ERROR_H

#include <iosfwd>

#include "node_api.h"
#include "js_native_api_types.h"

namespace OHOS {
namespace DistributedFS {
enum class ELegacy {
    ELEGACY_INVAL = 202,
    ELEGACY_IO = 300,
    ELEGACY_NOENT = 301,
};

enum class ErrCodeSystem {
    ERR_CODE_SYSTEM_LEGACY,
    ERR_CODE_SYSTEM_POSIX,
};

class UniError {
public:
    UniError();
    explicit UniError(ELegacy eLegacy);
    explicit UniError(int ePosix);
    UniError(const UniError &) = default;
    ~UniError() = default;

    UniError &operator = (const UniError &) = default;

    explicit operator bool() const;

    void SetErrno(ELegacy eLegacy);
    void SetErrno(int ePosix);
    int GetErrno(ErrCodeSystem cs);

    std::string GetDefaultErrstr();
    napi_value GetNapiErr(napi_env env);
    napi_value GetNapiErr(napi_env env, std::string errMsg);
    void ThrowErr(napi_env env);
    void ThrowErr(napi_env env, std::string errMsg);

private:
    int errno_ = 0;
    ErrCodeSystem codingSystem_ = ErrCodeSystem::ERR_CODE_SYSTEM_POSIX;
};
} // namespace DistributedFS
} // namespace OHOS
#endif