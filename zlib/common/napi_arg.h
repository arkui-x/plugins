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

#ifndef NAPI_ARG_H
#define NAPI_ARG_H

#include <memory>

#include "log.h"
#include "napi/native_api.h"
#include "napi/native_common.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace AppExecFwk {
class NapiArg {
public:
    NapiArg(napi_env env, napi_callback_info info) : env_(env), info_(info) {}
    ~NapiArg() {}

    bool Init(size_t minArgc, size_t maxArgc);

    size_t GetArgc() const;

    size_t GetMaxArgc() const;

    napi_value GetArgv(size_t pos) const;

    napi_value operator[](size_t pos) const;

private:
    napi_env env_ = nullptr;
    napi_callback_info info_ = nullptr;
    size_t argc_ = 0;
    size_t maxArgc_ = 0;
    std::unique_ptr<napi_value[]> argv_ = { nullptr };
    napi_value thisArg_ = nullptr;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif