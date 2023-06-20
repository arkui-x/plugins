/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include <string>
#include "log.h"
#include "inner_api/plugin_utils_napi.h"
#include "plugin_utils.h"
#include "trace.h"

namespace OHOS::Plugin {
struct CallbackContext {
    napi_env env = nullptr;
    napi_async_work work = nullptr;

    napi_ref sucCallbackRef = nullptr;
    napi_ref failCallbackRef = nullptr;
    napi_ref cmpCallbackRef = nullptr;

    napi_value infoList = nullptr;
    bool status = 0;
};
namespace {
static std::unique_ptr<Trace> tracePlugin;
constexpr int ARGC_NUMBER_TWO = 2;
constexpr int NAME_MAX_SIZE = 1024;
}
static napi_value JSHiTraceMeterStart(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_NUMBER_TWO;
    napi_value argv[ARGC_NUMBER_TWO];
    napi_value thisVar;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == ARGC_NUMBER_TWO || argc == ARGC_NUMBER_TWO, "Wrong number of arguments");

    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "First arg type error, should is string");
    char buf[NAME_MAX_SIZE] = {0};
    size_t len = 0;
    napi_get_value_string_utf8(env, argv[0], buf, NAME_MAX_SIZE, &len);
    std::string name(buf);
    NAPI_CALL(env, napi_typeof(env, argv[1], &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "Second arg type error, should is number");
    int taskId = 0;
    napi_get_value_int32(env, argv[1], &taskId);
    if (!tracePlugin) {
        tracePlugin = Trace::Create();
        if (!tracePlugin) {
            return nullptr;
        }
    }
    tracePlugin->AsyncTraceBegin(taskId, buf);
    return nullptr;
}

static napi_value JSHiTraceMeterFinish(napi_env env, napi_callback_info info)
{
    size_t argc = ARGC_NUMBER_TWO;
    napi_value argv[ARGC_NUMBER_TWO];
    napi_value thisVar;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, NULL));
    NAPI_ASSERT(env, argc == ARGC_NUMBER_TWO, "Wrong number of arguments");

    napi_valuetype valueType;
    NAPI_CALL(env, napi_typeof(env, argv[0], &valueType));
    NAPI_ASSERT(env, valueType == napi_string, "First arg type error, should is string");
    char buf[NAME_MAX_SIZE] = {0};
    size_t len = 0;
    napi_get_value_string_utf8(env, argv[0], buf, NAME_MAX_SIZE, &len);
    std::string name(buf);
    NAPI_CALL(env, napi_typeof(env, argv[1], &valueType));
    NAPI_ASSERT(env, valueType == napi_number, "Second arg type error, should is number");
    int taskId = 0;
    napi_get_value_int32(env, argv[1], &taskId);
    if (!tracePlugin) {
        tracePlugin = Trace::Create();
        if (!tracePlugin) {
            return nullptr;
        }
    }
    tracePlugin->AsyncTraceEnd(taskId, buf);
    return nullptr;
}

static napi_value HiTraceMeterExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("startTrace", JSHiTraceMeterStart),
        DECLARE_NAPI_FUNCTION("finishTrace", JSHiTraceMeterFinish)
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module hiTraceMeterModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = HiTraceMeterExport,
    .nm_modname = "hiTraceMeter",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void HiTraceMeterRegister()
{
    napi_module_register(&hiTraceMeterModule);
}
} // namespace OHOS::Ace::Napi
