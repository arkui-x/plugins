/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_REQUEST_JS_NOTIFY_H
#define PLUGINS_REQUEST_JS_NOTIFY_H

#include <string>
#include <vector>

#include "async_call.h"
#include "constant.h"
#include "i_task_notify.h"
#include "js_ref_holder.h"
#include "napi/native_api.h"

namespace OHOS::Plugin::Request {
class JsNotify : public TaskNotifyInterface, public std::enable_shared_from_this<JsNotify> {
public:
    explicit JsNotify(napi_env env, napi_value cb, Version version, Action action);
    virtual ~JsNotify();
    void OnCallback(const std::string &params) override;
    napi_ref GetRef() const;

protected:
    virtual void HandleCallback(napi_env env, napi_value cb, const std::string &param) = 0;

protected:
    napi_env env_ = nullptr;
    Version version_ = Version::UNKNOWN;
    Action action_ = Action::ANY;
    std::shared_ptr<JsRefHolder> callback_ = nullptr;
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_JS_NOTIFY_H