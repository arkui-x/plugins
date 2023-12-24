/*
 * Copyright (C) 2023 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_REQUEST_JS_REF_HOLDER_H
#define PLUGINS_REQUEST_JS_REF_HOLDER_H

#include "async_call.h"
#include "napi/native_api.h"

namespace OHOS::Plugin::Request {
class JsRefHolder {
public:
    explicit JsRefHolder(napi_env env, napi_ref ref);
    ~JsRefHolder();
    bool IsValid() const;
    napi_ref Get();
    napi_ref Get() const;

private:
    napi_env env_ {nullptr};
    napi_ref ref_ {nullptr};
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_JS_REF_HOLDER_H