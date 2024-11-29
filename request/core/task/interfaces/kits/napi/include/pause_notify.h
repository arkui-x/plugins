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

#ifndef PLUGINS_REQUEST_PAUSE_NOTIFY_H
#define PLUGINS_REQUEST_PAUSE_NOTIFY_H

#include "js_notify.h"
#include <string>
#include <map>

namespace OHOS::Plugin::Request {
class PauseNotify : public JsNotify
{
public:
    explicit PauseNotify(napi_env env, napi_value cb, Version version, Action action);
    virtual ~PauseNotify() = default;

    void HandleCallback(napi_env env, napi_value cb, const std::string &param) override;
};
} // namespace OHOS::Plugin::Request
#endif // PLUGINS_REQUEST_PAUSE_NOTIFY_H