/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "resource_manager_addon.h"

#include <fstream>
#include <memory>
#include <vector>

#if defined(__ARKUI_CROSS__)
#include "plugins/interfaces/native/log.h"
#else
#include "hilog/log.h"
#include "ability.h"
#include "foundation/ability/ability_runtime/interfaces/kits/native/appkit/ability_runtime/context/context.h"
#include "hitrace_meter.h"
#include "js_runtime_utils.h"
#include "hisysevent_adapter.h"
#endif
#ifdef ANDROID_PLATFORM
#include "foundation/arkui/ace_engine/adapter/android/entrance/java/jni/ace_application_info_impl.h"
#endif
#ifdef IOS_PLATFORM
#include "foundation/arkui/ace_engine/adapter/ios/entrance/ace_application_info_impl.h"
#endif
#include "node_api.h"

namespace OHOS {
namespace Plugin {
namespace Resource {
#define GET_PARAMS(env, info, num)    \
    size_t argc = num;                \
    napi_value argv[num] = {nullptr}; \
    napi_value thisVar = nullptr;     \
    void *data = nullptr;             \
    napi_get_cb_info(env, info, &argc, argv, &thisVar, &data)

#if !defined(__ARKUI_CROSS__)
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, 0xD001E00, "ResourceManagerJs" };
using namespace OHOS::HiviewDFX;
using namespace OHOS::AppExecFwk;
#endif

static void ExecuteGetResMgr(napi_env env, void* data)
{
    if (data == nullptr) {
        return;
    }
    ResMgrAsyncContext *asyncContext = static_cast<ResMgrAsyncContext*>(data);
    asyncContext->createValueFunc_ = [](napi_env env, ResMgrAsyncContext &context) -> napi_value {
#if defined(__ARKUI_CROSS__)
        napi_value result = ResourceManagerAddon::Create(env, context.bundleName_, context.resMgr_);
#else
        std::string traceVal = "Create ResourceManager";
        StartTrace(HITRACE_TAG_GLOBAL_RESMGR, traceVal);
        napi_value result = ResourceManagerAddon::Create(env, context.bundleName_, context.resMgr_, nullptr);
        FinishTrace(HITRACE_TAG_GLOBAL_RESMGR);
#endif
        if (result == nullptr) {
            context.SetErrorMsg("Failed to get ResourceManagerAddon");
#if !defined(__ARKUI_CROSS__)
            ReportInitResourceManagerFail(context.bundleName_, "failed to get ResourceManagerAddon");
#endif
            return nullptr;
        }
        return result;
    };
}

#if !defined(__ARKUI_CROSS__)
Ability* GetGlobalAbility(napi_env env)
{
    napi_value global;
    napi_status status = napi_get_global(env, &global);
    if (status != napi_ok) {
        LOGE("Failed to get global");
        return nullptr;
    }

    napi_value abilityObj;
    status = napi_get_named_property(env, global, "ability", &abilityObj);
    if (status != napi_ok || abilityObj == nullptr) {
        HILOG_WARM("Failed to get ability property");
        return nullptr;
    }

    Ability* ability = nullptr;
    status = napi_get_value_external(env, abilityObj, (void **)&ability);
    if (status == napi_ok && ability != nullptr) {
        return ability;
    }

    return nullptr;
}
#endif

#if defined(__ARKUI_CROSS__)
static bool InitAsyncContext(napi_env env, const std::string &bundleName, ResMgrAsyncContext &asyncContext)
{
    std::shared_ptr<ResourceManager> resMgr;
    if (bundleName.empty()) {
        resMgr = OHOS::Ace::Platform::AceApplicationInfoImpl::GetInstance().GetResourceManager();
        if (resMgr == nullptr) {
            LOGE("getResourceManager failed, resmgr is nullptr.");
        }
    } else {
        LOGE("getResourceManager by target bundle name, will support in the future.");
    }

    asyncContext.resMgr_ = resMgr;
    asyncContext.bundleName_ = bundleName;
    return resMgr != nullptr;
}
#else
static bool InitAsyncContext(napi_env env, const std::string &bundleName, Ability* ability,
    const std::shared_ptr<AbilityRuntime::Context>& context, ResMgrAsyncContext &asyncContext)
{
    std::shared_ptr<ResourceManager> resMgr;
    if (ability != nullptr) {
        if (bundleName.empty()) {
            resMgr = ability->GetResourceManager();
        } else {
            std::shared_ptr<Context> bundleContext = ability->CreateBundleContext(bundleName, 0);
            if (bundleContext != nullptr) {
                resMgr = bundleContext->GetResourceManager();
            }
        }
    } else if (context != nullptr) {
        if (bundleName.empty()) {
            resMgr = context->GetResourceManager();
        } else {
            std::shared_ptr<OHOS::AbilityRuntime::Context> bundleContext = context->CreateBundleContext(bundleName);
            if (bundleContext != nullptr) {
                resMgr = bundleContext->GetResourceManager();
            }
        }
    }
    asyncContext.resMgr_ = resMgr;
    asyncContext.bundleName_ = bundleName;
    return resMgr != nullptr;
}
#endif

#if defined(__ARKUI_CROSS__)
static napi_value getResult(napi_env env, std::unique_ptr<ResMgrAsyncContext> &asyncContext,
    std::string &bundleName)
#else
static napi_value getResult(napi_env env, std::unique_ptr<ResMgrAsyncContext> &asyncContext,
    std::string &bundleName, const std::shared_ptr<AbilityRuntime::Context> &abilityRuntimeContext)
#endif
{
    napi_value result = nullptr;
    if (asyncContext->callbackRef_ == nullptr) {
        napi_create_promise(env, &asyncContext->deferred_, &result);
    } else {
        napi_get_undefined(env, &result);
    }
#if defined(__ARKUI_CROSS__)
    if (!InitAsyncContext(env, bundleName, *asyncContext)) {
        LOGE("init async context failed");
        return nullptr;
    }
#else
    if (!InitAsyncContext(env, bundleName, GetGlobalAbility(env), abilityRuntimeContext, *asyncContext)) {
        LOGE("init async context failed");
        ReportInitResourceManagerFail(bundleName, "failed to init async context");
        return nullptr;
    }
#endif
    napi_value resource = nullptr;
    napi_create_string_utf8(env, "getResourceManager", NAPI_AUTO_LENGTH, &resource);
    napi_status status = napi_create_async_work(env, nullptr, resource, ExecuteGetResMgr,
        ResMgrAsyncContext::Complete, static_cast<void*>(asyncContext.get()), &asyncContext->work_);
    if (status != napi_ok) {
        LOGE("Failed to create async work for getResourceManager %{public}d", status);
        return result;
    }
    status = napi_queue_async_work(env, asyncContext->work_);
    if (status != napi_ok) {
        LOGE("Failed to queue async work for getResourceManager %{public}d", status);
        return result;
    }
    asyncContext.release();
    return result;
}

static napi_value GetResourceManager(napi_env env, napi_callback_info info)
{
    GET_PARAMS(env, info, 3);

    std::unique_ptr<ResMgrAsyncContext> asyncContext = std::make_unique<ResMgrAsyncContext>();
    
#if !defined(__ARKUI_CROSS__)
    std::shared_ptr<AbilityRuntime::Context> abilityRuntimeContext;
#endif
    std::string bundleName;
    for (size_t i = 0; i < argc; i++) {
        napi_valuetype valueType;
        napi_typeof(env, argv[i], &valueType);
#if defined(__ARKUI_CROSS__)
        if (i == 0 && valueType == napi_object) {
            LOGE("getResourceManager by context currently not supported.");
            return nullptr;
        } else if ((i == 0 || i == 1) && valueType == napi_string) {
            LOGE("getResourceManager by target bundle name, will support in the future.");
            return nullptr;
#else
        if (i == 0 && valueType == napi_object) {
            using WeakContextPtr = std::weak_ptr<AbilityRuntime::Context> *;
            WeakContextPtr objContext;
            napi_status status = napi_unwrap(env, argv[0], reinterpret_cast<void **>(&objContext));
            if (status != napi_ok || objContext == nullptr) {
                LOGE("Failed to get objContext");
                return nullptr;
            }
            auto context = objContext->lock();
            if (context == nullptr) {
                LOGE("Failed to get context");
                return nullptr;
            }
            abilityRuntimeContext = context;
        } else if ((i == 0 || i == 1) && valueType == napi_string) {
            size_t len = 0;
            napi_status status = napi_get_value_string_utf8(env, argv[i], nullptr, 0, &len);
            if (status != napi_ok) {
                LOGE("Failed to get bundle name length");
                return nullptr;
            }
            std::vector<char> buf(len + 1);
            status = napi_get_value_string_utf8(env, argv[i], buf.data(), len + 1, &len);
            if (status != napi_ok) {
                LOGE("Failed to get bundle name");
                return nullptr;
            }
            bundleName = buf.data();
#endif
        } else if ((i == 0 || i == 1 || i == 2) && valueType == napi_function) { // 2 means the third parameter
            napi_create_reference(env, argv[i], 1, &asyncContext->callbackRef_);
            break;
        } else {
            // self resourcemanager with promise
        }
    }
#if defined(__ARKUI_CROSS__)
    napi_value result = getResult(env, asyncContext, bundleName);
#else
    napi_value result = getResult(env, asyncContext, bundleName, abilityRuntimeContext);
#endif
    return result;
}

static napi_value ResMgrInit(napi_env env, napi_value exports)
{
#if defined(__ARKUI_CROSS__)
    napi_property_descriptor creatorProp[] = {
        DECLARE_NAPI_FUNCTION("getResourceManager", GetResourceManager),
    };
    napi_status status = napi_define_properties(env, exports, 1, creatorProp);
#else
    std::string traceVal = "GetResourceManager";
    StartTrace(HITRACE_TAG_GLOBAL_RESMGR, traceVal);
    napi_property_descriptor creatorProp[] = {
        DECLARE_NAPI_FUNCTION("getResourceManager", GetResourceManager),
    };
    napi_status status = napi_define_properties(env, exports, 1, creatorProp);
    FinishTrace(HITRACE_TAG_GLOBAL_RESMGR);
#endif
    if (status != napi_ok) {
        LOGE("Failed to set getResourceManager at init");
        return nullptr;
    }
    return exports;
}

static napi_module g_resourceManagerModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ResMgrInit,
    .nm_modname = "resourcemanager",
    .nm_priv = ((void*)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void ResMgrRegister()
{
    napi_module_register(&g_resourceManagerModule);
}
} // namespace Resource
} // namespace Plugin
} // namespace OHOS