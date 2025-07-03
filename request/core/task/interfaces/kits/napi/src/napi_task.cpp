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

#include "napi_task.h"

#include <chrono>
#include <cstring>
#include <mutex>
#include <securec.h>
#include <sys/stat.h>

#include "async_call.h"
#include "constant.h"
#include "js_initialize.h"
#include "log.h"
#include "napi_utils.h"
#include "js_task.h"
#include "task_manager.h"

namespace OHOS::Plugin::Request {
constexpr int64_t MILLISECONDS_IN_ONE_DAY = 24 * 60 * 60 * 1000;
static std::mutex g_CtorMutex;
static thread_local napi_ref g_CtorV9 = nullptr;
static thread_local napi_ref g_CtorV10 = nullptr;

napi_property_descriptor clzDes[] = {
    DECLARE_NAPI_FUNCTION(FUNCTION_ON, JsTask::On),
    DECLARE_NAPI_FUNCTION(FUNCTION_OFF, JsTask::Off),
    DECLARE_NAPI_FUNCTION(FUNCTION_START, JsTask::Start),
    DECLARE_NAPI_FUNCTION(FUNCTION_STOP, JsTask::Stop),
    DECLARE_NAPI_FUNCTION(FUNCTION_PAUSE, JsTask::Pause),
    DECLARE_NAPI_FUNCTION(FUNCTION_DELETE, JsTask::Remove),
    DECLARE_NAPI_FUNCTION(FUNCTION_RESUME, JsTask::Resume),
};

napi_property_descriptor clzDesV9[] = {
    DECLARE_NAPI_FUNCTION(FUNCTION_ON, JsTask::On),
    DECLARE_NAPI_FUNCTION(FUNCTION_OFF, JsTask::Off),
    DECLARE_NAPI_FUNCTION(FUNCTION_SUSPEND, JsTask::Pause),
    DECLARE_NAPI_FUNCTION(FUNCTION_GET_TASK_INFO, JsTask::Query),
    DECLARE_NAPI_FUNCTION(FUNCTION_GET_TASK_MIME_TYPE, JsTask::QueryMimeType),
    DECLARE_NAPI_FUNCTION(FUNCTION_DELETE, JsTask::Remove),
    DECLARE_NAPI_FUNCTION(FUNCTION_RESTORE, JsTask::Resume),
};

struct ContextInfo : public TaskContext {
    ITask *task = nullptr;
    napi_ref taskRef = nullptr;
    napi_ref jsConfig = nullptr;
    Config config {};
};

napi_value NapiTask::JsUpload(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("JsUpload in");
    return JsMain(env, info, Version::API9);
}

napi_value NapiTask::JsDownload(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("JsDownload in");
    return JsMain(env, info, Version::API9);
}

napi_value NapiTask::JsMain(napi_env env, napi_callback_info info, Version version)
{
    auto context = std::make_shared<ContextInfo>();
    if (context == nullptr) {
        REQUEST_HILOGE("fail to create context info");
        return nullptr;
    }
    context->version = version;
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (context->version == Version::API10) {
            napi_create_reference(env, argv[1], 1, &(context->jsConfig));
        }
        napi_value ctor = GetCtor(env, context->version);
        napi_value jsTask = nullptr;
        napi_status status = napi_new_instance(env, ctor, argc, argv, &jsTask);
        if (jsTask == nullptr || status != napi_ok) {
            REQUEST_HILOGE("Get jsTask failed");
            return napi_generic_failure;
        }
        napi_unwrap(env, jsTask, reinterpret_cast<void **>(&context->task));
        if (context->task == nullptr) {
            REQUEST_HILOGE("Get c++ task failed");
            return napi_generic_failure;
        }
        napi_create_reference(env, jsTask, 1, &(context->taskRef));
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        context->retCode = AutoStart(context->task, context->version);
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        if (result == nullptr || context->retCode != E_OK) {
            return napi_generic_failure;
        }
        napi_status status = napi_get_reference_value(env, context->taskRef, result);
        napi_value config = nullptr;
        napi_get_reference_value(env, context->jsConfig, &config);
        JsInitialize::CreatProperties(env, *result, config, context->task);
        REQUEST_HILOGI("JsMain output");
        return status;
    };
    context->SetAction(input, output);
    AsyncCall asyncCall(env, info, context);
    return asyncCall.Call(env, std::move(exec));
}

int32_t NapiTask::AutoStart(ITask *task, Version version)
{
    if (task == nullptr) {
        REQUEST_HILOGE("invalid task object");
        return E_SERVICE_ERROR;
    }
    if (version == Version::API9) {
        return task->Start();
    }
    return E_OK;
}

napi_value NapiTask::GetCtor(napi_env env, Version version)
{
    switch (version) {
        case Version::API9:
            return GetCtorV9(env);
        case Version::API10:
            return GetCtorV10(env);
        default:
            break;
    }
    return nullptr;
}

napi_value NapiTask::GetCtorV9(napi_env env)
{
    REQUEST_HILOGI("GetCtorV9 in");
    std::lock_guard<std::mutex> lock(g_CtorMutex);
    if (g_CtorV9 != nullptr) {
        napi_value cons;
        NAPI_CALL(env, napi_get_reference_value(env, g_CtorV9, &cons));
        return cons;
    }
    size_t count = sizeof(clzDesV9) / sizeof(napi_property_descriptor);
    return DefineClass(env, clzDesV9, count, InitV9, &g_CtorV9);
}

napi_value NapiTask::GetCtorV10(napi_env env)
{
    REQUEST_HILOGI("GetCtorV10 in");
    std::lock_guard<std::mutex> lock(g_CtorMutex);
    if (g_CtorV10 != nullptr) {
        napi_value cons;
        NAPI_CALL(env, napi_get_reference_value(env, g_CtorV10, &cons));
        return cons;
    }
    size_t count = sizeof(clzDes) / sizeof(napi_property_descriptor);
    return DefineClass(env, clzDes, count, InitV10, &g_CtorV10);
}

napi_value NapiTask::DefineClass(
    napi_env env, const napi_property_descriptor *desc, size_t count, napi_callback cb, napi_ref *ctor)
{
    napi_value cons = nullptr;
    napi_status status = napi_define_class(env, "Request", NAPI_AUTO_LENGTH, cb, nullptr, count, desc, &cons);
    if (status != napi_ok) {
        REQUEST_HILOGE("napi_define_class failed");
        return nullptr;
    }
    status = napi_create_reference(env, cons, 1, ctor);
    if (status != napi_ok) {
        REQUEST_HILOGE("napi_create_reference failed");
        return nullptr;
    }
    return cons;
}

napi_value NapiTask::InitV9(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("RequestFile API9");
    return JsInitialize::Initialize(env, info, Version::API9);
}

napi_value NapiTask::InitV10(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("Create API10");
    return JsInitialize::Initialize(env, info, Version::API10);
}

napi_value NapiTask::Create(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("JsCreate in");
    return JsMain(env, info, Version::API10);
}

napi_value NapiTask::Remove(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("JsRemove in");
    struct RemoveContext : public TaskContext {
        int64_t tid = 0;
        bool res = false;
    };

    auto context = std::make_shared<RemoveContext>();
    context->version = Version::API10;
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (!ParseTid(env, argc, argv, context->tid)) {
            NapiUtils::ThrowError(env, E_PARAMETER_CHECK, "Parse tid fail!");
            return napi_invalid_arg;
        }
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        if (context->retCode != E_OK) {
            return napi_generic_failure;
        }
        context->res = true;
        return NapiUtils::Convert2JSValue(env, context->res, *result);
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        context->retCode = TaskManager::Get().Remove(context->tid);
    };
    context->SetAction(input, output);
    AsyncCall asyncCall(env, info, context);
    return asyncCall.Call(env, std::move(exec));
}

bool NapiTask::ParseTid(napi_env env, size_t argc, napi_value *argv, int64_t &taskId)
{
    if (argc < 1) {
        REQUEST_HILOGE("Wrong number of arguments");
        return false;
    }
    if (NapiUtils::GetValueType(env, argv[0]) != napi_string) {
        REQUEST_HILOGE("The first parameter is not of string type");
        return false;
    }
    auto tidText = NapiUtils::Convert2String(env, argv[0]);
    if (tidText.empty()) {
        return false;
    }
    taskId = atoll(tidText.c_str());
    REQUEST_HILOGI("tidText = %{public}s:%{public}ld", tidText.c_str(), taskId);
    return true;
}

napi_value NapiTask::Show(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("JsShow in");
    auto context = std::make_shared<TouchContext>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        if (!ParseTid(env, argc, argv, context->tid)) {
            NapiUtils::ThrowError(env, E_PARAMETER_CHECK, "Parse tid fail!");
            return napi_invalid_arg;
        }
        return napi_ok;
    };
    return TouchInner(env, info, std::move(input), std::move(context));
}

napi_value NapiTask::Touch(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("JsTouch in");
    auto context = std::make_shared<TouchContext>();
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        bool ret = ParseTouch(env, argc, argv, context);
        if (!ret) {
            NapiUtils::ThrowError(env, E_PARAMETER_CHECK, "Parse tid or token fail!");
            return napi_invalid_arg;
        }
        return napi_ok;
    };
    return TouchInner(env, info, std::move(input), std::move(context));
}

napi_value NapiTask::TouchInner(napi_env env, napi_callback_info info, AsyncCall::Context::InputAction input,
    std::shared_ptr<TouchContext> context)
{
    context->version = Version::API10;
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        if (context->retCode != E_OK) {
            return napi_generic_failure;
        }
        *result = NapiUtils::Convert2JSValue(env, context->taskInfo);
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        context->retCode = TaskManager::Get().GetTaskInfo(context->tid, context->token, context->taskInfo);
    };
    context->SetAction(input, output);
    AsyncCall asyncCall(env, info, context);
    return asyncCall.Call(env, std::move(exec));
}

bool NapiTask::ParseTouch(napi_env env, size_t argc, napi_value *argv, std::shared_ptr<TouchContext> context)
{
    if (argc < 2) {
        REQUEST_HILOGE("Wrong number of arguments");
        return false;
    }
    if (NapiUtils::GetValueType(env, argv[0]) != napi_string || NapiUtils::GetValueType(env, argv[1]) != napi_string) {
        REQUEST_HILOGE("The parameter is not of string type");
        return false;
    }

    auto taskId = NapiUtils::Convert2String(env, argv[0]);
    if (taskId.empty()) {
        REQUEST_HILOGE("tid is empty");
        return false;
    }
    context->tid = atoll(taskId.c_str());
    std::vector<char> token;
    token.resize(TOKEN_MAX_BYTES + 2);
    size_t len = 0;
    napi_status status = napi_get_value_string_utf8(env, argv[1], token.data(), token.size(), &len);
    if (status != napi_ok || len < TOKEN_MIN_BYTES || len > TOKEN_MAX_BYTES) {
        return false;
    }
    context->token = token.data();
    return true;
}

bool NapiTask::ParseSearch(napi_env env, size_t argc, napi_value *argv, Filter &filter)
{
    if (argc < 1) {
        return true;
    }
    napi_valuetype valueType = NapiUtils::GetValueType(env, argv[0]);
    if (valueType == napi_null || valueType == napi_undefined) {
        return true;
    }
    if (valueType != napi_object) {
        REQUEST_HILOGE("The parameter is not of object type");
        return false;
    }
    filter.before = ParseBefore(env, argv[0]);
    filter.after = ParseAfter(env, argv[0], filter.before);
    if (filter.before < filter.after) {
        REQUEST_HILOGE("before is small than after");
        return false;
    }
    filter.state = ParseState(env, argv[0]);
    filter.action = ParseAction(env, argv[0]);
    filter.mode = ParseMode(env, argv[0]);
    return true;
}

State NapiTask::ParseState(napi_env env, napi_value value)
{
    if (!NapiUtils::HasNamedProperty(env, value, "state")) {
        return State::ANY;
    }
    napi_value value1 = NapiUtils::GetNamedProperty(env, value, "state");
    if (NapiUtils::GetValueType(env, value1) != napi_number) {
        return State::ANY;
    }
    return static_cast<State>(NapiUtils::Convert2Uint32(env, value1));
}

Action NapiTask::ParseAction(napi_env env, napi_value value)
{
    if (!NapiUtils::HasNamedProperty(env, value, "action")) {
        return Action::ANY;
    }
    napi_value value1 = NapiUtils::GetNamedProperty(env, value, "action");
    if (NapiUtils::GetValueType(env, value1) != napi_number) {
        return Action::ANY;
    }
    return static_cast<Action>(NapiUtils::Convert2Uint32(env, value1));
}

Mode NapiTask::ParseMode(napi_env env, napi_value value)
{
    if (!NapiUtils::HasNamedProperty(env, value, "mode")) {
        return Mode::ANY;
    }
    napi_value value1 = NapiUtils::GetNamedProperty(env, value, "mode");
    if (NapiUtils::GetValueType(env, value1) != napi_number) {
        return Mode::ANY;
    }
    return static_cast<Mode>(NapiUtils::Convert2Uint32(env, value1));
}

int64_t NapiTask::ParseBefore(napi_env env, napi_value value)
{
    using namespace std::chrono;
    int64_t now = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    if (!NapiUtils::HasNamedProperty(env, value, "before")) {
        return now;
    }
    napi_value value1 = NapiUtils::GetNamedProperty(env, value, "before");
    if (NapiUtils::GetValueType(env, value1) != napi_number) {
        return now;
    }
    int64_t ret = 0;
    NAPI_CALL_BASE(env, napi_get_value_int64(env, value1, &ret), now);
    return ret;
}

int64_t NapiTask::ParseAfter(napi_env env, napi_value value, int64_t before)
{
    int64_t defaultValue = before - MILLISECONDS_IN_ONE_DAY;
    if (!NapiUtils::HasNamedProperty(env, value, "after")) {
        return defaultValue;
    }
    napi_value value1 = NapiUtils::GetNamedProperty(env, value, "after");
    if (NapiUtils::GetValueType(env, value1) != napi_number) {
        return defaultValue;
    }
    int64_t ret = 0;
    NAPI_CALL_BASE(env, napi_get_value_int64(env, value1, &ret), defaultValue);
    return ret;
}

napi_value NapiTask::Search(napi_env env, napi_callback_info info)
{
    REQUEST_HILOGI("JsSearch in");
    struct SearchContext : public TaskContext {
        Filter filter;
        std::vector<std::string> tids;
    };

    auto context = std::make_shared<SearchContext>();
    context->version = Version::API10;
    auto input = [context](napi_env env, size_t argc, napi_value *argv, napi_value self) -> napi_status {
        bool ret = ParseSearch(env, argc, argv, context->filter);
        if (!ret) {
            NapiUtils::ThrowError(env, E_PARAMETER_CHECK, "Parse filter fail!");
            return napi_invalid_arg;
        }
        return napi_ok;
    };
    auto output = [context](napi_env env, napi_value *result) -> napi_status {
        if (context->retCode != E_OK) {
            return napi_generic_failure;
        }
        *result = NapiUtils::Convert2JSValue(env, context->tids);
        return napi_ok;
    };
    auto exec = [context](AsyncCall::Context *ctx) {
        context->retCode = TaskManager::Get().Search(context->filter, context->tids);
    };
    context->SetAction(input, output);
    AsyncCall asyncCall(env, info, context);
    return asyncCall.Call(env, std::move(exec));
}
} // namespace OHOS::Plugin::Request