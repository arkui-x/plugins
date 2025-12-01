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

#ifndef ASYNC_CONTEXT_H
#define ASYNC_CONTEXT_H

#include "constant_definition.h"
#include "napi/native_api.h"

namespace OHOS {
namespace Location {
class AsyncContext {
public:
    napi_env env;
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callback[MAX_CALLBACK_NUM] = { 0 };
    std::function<void(void*)> executeFunc;
    std::function<void(void*)> completeFunc;
    napi_value resourceName;
    napi_value result[RESULT_SIZE];
    int errCode;
    int64_t beginTime;

    explicit AsyncContext(napi_env e, napi_async_work w = nullptr, napi_deferred d = nullptr)
    {
        env = e;
        work = w;
        deferred = d;
        executeFunc = nullptr;
        completeFunc = nullptr;
        resourceName = nullptr;
        result[PARAM0] = nullptr;
        result[PARAM1] = nullptr;
        errCode = 0;
        beginTime = 0;
    }

    AsyncContext() = delete;

    virtual ~AsyncContext()
    {
    }
};
}  // namespace Location
}  // namespace OHOS
#endif // ASYNC_CONTEXT_H