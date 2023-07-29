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

#include "plugins/running_lock/js_runninglock.h"

#include <cstddef>

#include "log.h"
#include "inner_api/plugin_utils_napi.h"
#include "plugin_utils.h"
#ifdef ANDROID_PLATFORM
#include "plugins/running_lock/android/java/jni/runninglock_jni.h"
#endif
#include "plugins/running_lock/power_mgr.h"

namespace OHOS::Plugin {
struct RunningLockEntity {
    std::shared_ptr<RunningLock> runningLock;
};

static thread_local napi_ref g_runningLockConstructor;
static std::shared_ptr<PowerMgr> g_powerMgr = PowerMgr::GetInstance();

static napi_value Unlock(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    NAPI_ASSERT(env, (status == napi_ok), "Unlock: failed to get cb info");

    RunningLockEntity *entity = nullptr;
    status = napi_unwrap(env, thisArg, (void **)&entity);
    if (status != napi_ok) {
        LOGE("Cannot unwrap for pointer");
        return nullptr;
    }
    if (entity == nullptr || entity->runningLock == nullptr) {
        LOGE("Entity runningLock is nullptr");
        return nullptr;
    }
    entity->runningLock->UnLock();
    return nullptr;
}

static napi_value IsUsed(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    napi_value result = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    NAPI_ASSERT(env, (status == napi_ok), "Failed to get cb info");
    napi_get_boolean(env, false, &result);
    RunningLockEntity *entity = nullptr;
    status = napi_unwrap(env, thisArg, (void **)&entity);
    if (status != napi_ok) {
        LOGE("Cannot unwrap for pointer");
        return result;
    }
    if (entity == nullptr || entity->runningLock == nullptr) {
        LOGE("Entity runningLock is nullptr");
        return result;
    }
    bool isUsed = entity->runningLock->IsUsed();
    napi_get_boolean(env, isUsed, &result);
    return result;
}

static napi_value Lock(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {0};
    napi_value thisArg = nullptr;
    void *data = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &thisArg, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 1), "Failed to get cb info");
    napi_valuetype type = napi_undefined;
    NAPI_CALL(env, napi_typeof(env, args[0], &type));
    NAPI_ASSERT(env, type == napi_number, "Wrong argument type. number expected.");

    uint32_t timeOut;
    status = napi_get_value_uint32(env, args[0], &timeOut);
    if (status != napi_ok) {
        LOGE("napi_get_value_uint32 failed");
        return nullptr;
    }
    RunningLockEntity *entity = nullptr;
    status = napi_unwrap(env, thisArg, (void **)&entity);
    if (status != napi_ok) {
        LOGE("Cannot unwrap for pointer");
        return nullptr;
    }
    if (entity == nullptr || entity->runningLock == nullptr) {
        LOGE("Entity runningLock is nullptr");
        return nullptr;
    }
    entity->runningLock->Lock(timeOut);
    return nullptr;
}

napi_value CreateInstanceForRunningLock(napi_env env, RunningLockAsyncCallbackInfo *asyncCallbackInfo)
{
    napi_value cons = nullptr;
    napi_value instance = nullptr;
    napi_status callBackStatus;
    RunningLockEntity *entity = nullptr;

    if (asyncCallbackInfo->runningLock == nullptr) {
        LOGE("RunningLock is nullptr");
        return nullptr;
    }
    callBackStatus = napi_get_reference_value(env, g_runningLockConstructor, &cons);
    if (callBackStatus != napi_ok) {
        LOGE("NAPI failed to create a reference value");
        return nullptr;
    }
    callBackStatus = napi_new_instance(env, cons, 0, nullptr, &instance);
    if (callBackStatus != napi_ok || instance == nullptr) {
        LOGE("NAPI failed to create a reference");
        return nullptr;
    }
    callBackStatus = napi_unwrap(env, instance, (void **)&entity);
    if (callBackStatus != napi_ok || entity == nullptr) {
        LOGE("Cannot unwrap entity from instance");
        return nullptr;
    }
    entity->runningLock = asyncCallbackInfo->runningLock;
    return instance;
}

static napi_value CreateRunningLock(napi_env env, napi_callback_info info)
{
    size_t argc = CREATRUNNINGLOCK_ARGC;
    napi_value argv[CREATRUNNINGLOCK_ARGC] = {0};
    napi_value thisArg = nullptr;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, argv, &thisArg, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= CREATRUNNINGLOCK_ARGC - 1), "Failed to get cb info");
    RunningLockAsyncCallbackInfo *asyncCallbackInfo =
        new (std::nothrow) RunningLockAsyncCallbackInfo {.env = env, .asyncWork = nullptr};
    if (asyncCallbackInfo == nullptr) {
        LOGE("asyncCallbackInfo is nullptr");
        return nullptr;
    }

    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType = napi_undefined;
        napi_typeof(env, argv[i], &valueType);
        if ((i == 0) && (valueType == napi_string)) {
            napi_get_value_string_utf8(
                env, argv[i], asyncCallbackInfo->name, RUNNINGLOCK_NAME_MAX + 1, &asyncCallbackInfo->nameLen);
        }
        if (i == 1) {
            int32_t type = static_cast<int32_t>(RunningLockType::RUNNINGLOCK_BUTT);
            napi_get_value_int32(env, argv[i], &type);
            asyncCallbackInfo->type = static_cast<RunningLockType>(type);
        }
        if (valueType == napi_function) {
            napi_create_reference(env, argv[i], 1, &asyncCallbackInfo->callbackRef);
        }
    }
    asyncCallbackInfo->runningLock =
        g_powerMgr->CreateRunningLock(std::string(asyncCallbackInfo->name), asyncCallbackInfo->type);
    napi_value result = nullptr;
    if (asyncCallbackInfo->runningLock) {
        if (asyncCallbackInfo->callbackRef != nullptr) {
            napi_get_undefined(env, &result);
        } else {
            napi_create_promise(env, &asyncCallbackInfo->deferred, &result);
        }
        asyncCallbackInfo->runningLock->Init(asyncCallbackInfo);
    } else {
        LOGE("asyncCallbackInfo->runningLock is null");
    }
    return result;
}

static napi_value EnumRunningLockTypeConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisArg = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisArg, &data);
    napi_value global = nullptr;
    napi_get_global(env, &global);
    return thisArg;
}

static napi_value CreateEnumRunningLockType(napi_env env, napi_value exports)
{
    napi_value background = nullptr;
    napi_value proximityscreencontrol = nullptr;

    napi_create_int32(env, (int32_t)RunningLockType::RUNNINGLOCK_BACKGROUND, &background);
    napi_create_int32(env, (int32_t)RunningLockType::RUNNINGLOCK_PROXIMITY_SCREEN_CONTROL, &proximityscreencontrol);

    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("BACKGROUND", background),
        DECLARE_NAPI_STATIC_PROPERTY("PROXIMITY_SCREEN_CONTROL", proximityscreencontrol),
    };
    napi_value result = nullptr;
    napi_define_class(env,
        "RunningLockType",
        NAPI_AUTO_LENGTH,
        EnumRunningLockTypeConstructor,
        nullptr,
        sizeof(desc) / sizeof(*desc),
        desc,
        &result);

    napi_set_named_property(env, exports, "RunningLockType", result);
    return exports;
}

static napi_value RunningLockConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, &data);

    auto entity = new RunningLockEntity();
    napi_wrap(
        env,
        thisVar,
        entity,
        [](napi_env env, void *data, void *hint) {
            LOGI("Destructor");
            auto entity = (RunningLockEntity *)data;
            delete entity;
        },
        nullptr,
        nullptr);

    return thisVar;
}

static napi_value CreateRunningLockClass(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("unlock", Unlock),
        DECLARE_NAPI_FUNCTION("isUsed", IsUsed),
        DECLARE_NAPI_FUNCTION("lock", Lock),
    };

    napi_value result = nullptr;
    napi_define_class(env,
        "RunningLock",
        NAPI_AUTO_LENGTH,
        RunningLockConstructor,
        nullptr,
        sizeof(desc) / sizeof(*desc),
        desc,
        &result);

    napi_create_reference(env, result, 1, &g_runningLockConstructor);
    napi_set_named_property(env, exports, "RunningLock", result);
    return exports;
}

EXTERN_C_START
/*
 * function for module exports
 */
static napi_value RunningLockInit(napi_env env, napi_value exports)
{
    LOGI("Initialize the RunningLock module");
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("createRunningLock", CreateRunningLock),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    CreateRunningLockClass(env, exports);
    CreateEnumRunningLockType(env, exports);
    LOGI("The initialization of the RunningLock module is complete");

    return exports;
}
EXTERN_C_END

/*
 * Module definition
 */
static napi_module g_module = {.nm_version = 1,
    .nm_flags = 0,
    .nm_filename = "runningLock",
    .nm_register_func = RunningLockInit,
    .nm_modname = "runningLock",
    .nm_priv = ((void *)0),
    .reserved = {0}};

#ifdef ANDROID_PLATFORM
static void RunningLockJniRegister()
{
    const char className[] = "ohos.ace.plugin.runninglockplugin.RunningLockPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&RunningLockJni::Register, className);
}
#endif

/*
 * Module registration
 */
extern "C" __attribute__((constructor)) void RegisterRunninglockModule(void)
{
#ifdef ANDROID_PLATFORM
    RunningLockJniRegister();
#endif
    napi_module_register(&g_module);
}
}  // namespace OHOS::Plugin
