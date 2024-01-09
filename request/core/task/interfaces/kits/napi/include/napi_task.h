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

#ifndef PLUGINS_REQUEST_NAPI_TASK_H
#define PLUGINS_REQUEST_NAPI_TASK_H

#include <string>
#include "async_call.h"
#include "constant.h"
#include "i_task.h"

namespace OHOS::Plugin::Request {
struct TaskContext : public AsyncCall::Context {
    Version version = Version::API9;
};

class NapiTask {
public:
    static napi_value JsUpload(napi_env env, napi_callback_info info);
    static napi_value JsDownload(napi_env env, napi_callback_info info);

    static napi_value Create(napi_env env, napi_callback_info info);
    static napi_value Remove(napi_env env, napi_callback_info info);
    static napi_value Show(napi_env env, napi_callback_info info);
    static napi_value Touch(napi_env env, napi_callback_info info);
    static napi_value Search(napi_env env, napi_callback_info info);
private:
    struct TouchContext : public TaskContext {
        int64_t tid = 0;
        std::string token = "";
        TaskInfo taskInfo {};
    };

    static napi_value DefineClass(napi_env env, const napi_property_descriptor* desc, size_t count,
        napi_callback cb, napi_ref *ctor);
    static napi_value JsMain(napi_env env, napi_callback_info info, Version version);
    static napi_value GetCtor(napi_env env, Version version);
    static napi_value GetCtorV9(napi_env env);
    static napi_value GetCtorV10(napi_env env);
    static napi_value InitV9(napi_env env, napi_callback_info info);
    static napi_value InitV10(napi_env env, napi_callback_info info);

    static int32_t AutoStart(ITask *task, Version version);
    static bool ParseTid(napi_env env, size_t argc, napi_value *argv, int64_t &taskId);
    static napi_value TouchInner(napi_env env, napi_callback_info info, AsyncCall::Context::InputAction action,
        std::shared_ptr<TouchContext> context);
    static bool ParseSearch(napi_env env, size_t argc, napi_value *argv, Filter &filter);
    static State ParseState(napi_env env, napi_value value);
    static Action ParseAction(napi_env env, napi_value value);
    static Mode ParseMode(napi_env env, napi_value value);
    static bool ParseTouch(napi_env env, size_t argc, napi_value *argv, std::shared_ptr<TouchContext> context);
    static int64_t ParseBefore(napi_env env, napi_value value);
    static int64_t ParseAfter(napi_env env, napi_value value, int64_t before);
};
} // namespace OHOS::Plugin::Request

#endif // PLUGINS_REQUEST_NAPI_TASK_H
