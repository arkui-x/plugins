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

#include "napi_arg.h"

namespace OHOS {
namespace AppExecFwk {
bool NapiArg::Init(size_t minArgc, size_t maxArgc)
{
    argc_ = 0;
    argv_.reset();
    size_t argc;
    napi_value thisArg;
    // get argc first, in case of argv overflow
    napi_status status = napi_get_cb_info(env_, info_, &argc, nullptr, &thisArg, nullptr);
    if (status != napi_ok) {
        LOGE("Cannot get num of func args for %{public}d", status);
        return false;
    }
    // argc larger than maxArgc is permitted, but we only use the first $maxArgc$ args
    if (argc < minArgc) {
        LOGE("Incorrect number of arguments, argc:%{public}zu, minArgc:%{public}zu", argc, minArgc);
        return false;
    }
    if (argc != 0) {
        argv_ = std::make_unique<napi_value[]>(argc);
        status = napi_get_cb_info(env_, info_, &argc, argv_.get(), &thisArg, nullptr);
        if (status != napi_ok) {
            LOGE("Cannot get func args for %{public}d", status);
            return false;
        }
    }
    argc_ = argc;
    thisArg_ = thisArg;
    maxArgc_ = (argc < maxArgc) ? argc : maxArgc;
    return true;
}

size_t NapiArg::GetArgc() const
{
    return argc_;
}

size_t NapiArg::GetMaxArgc() const
{
    return maxArgc_;
}

napi_value NapiArg::GetArgv(size_t pos) const
{
    return (pos < argc_) ? argv_[pos] : nullptr;
}

napi_value NapiArg::operator[](size_t pos) const
{
    return GetArgv(pos);
}
} // namespace AppExecFwk
} // namespace OHOS