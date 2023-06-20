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

#include <cstddef>

#include "log.h"
#include "inner_api/plugin_utils_napi.h"
#include "plugin_utils.h"

#include "plugins/display/display.h"

#ifdef ANDROID_PLATFORM
#include "plugins/display/android/java/jni/display_jni.h"
#endif

namespace OHOS::Plugin {
static std::unique_ptr<Display> display_plugin;
const int ARGS_ASYNC_COUNT = 1;
const int ARGS_MAX_COUNT = 10;

static napi_value JSDisplayGetDefaultDisplay(napi_env env, napi_callback_info info)
{
    size_t argcAsync = 1;
    const size_t argcPromise = 0;
    const size_t argCountWithAsync = argcPromise + ARGS_ASYNC_COUNT;
    napi_value args[ARGS_MAX_COUNT] = { nullptr };
    NAPI_CALL(env, napi_get_cb_info(env, info, &argcAsync, args, nullptr, nullptr));
    if (argcAsync > argCountWithAsync) {
        LOGE("JSDisplayGetDefaultDisplay: Wrong argument count.");
        return nullptr;
    }
    if (!display_plugin) {
        display_plugin = Display::Create();
        if (!display_plugin) {
            LOGW("JSDisplayGetDefaultDisplay: Display null return");
            return nullptr;
        }
    }
    AsyncCallbackInfo* ptr =
        new (std::nothrow) AsyncCallbackInfo { .env = env, .asyncWork = nullptr };
    if (ptr == nullptr) {
        LOGE("AsyncCallbackInfo create failed!");
        return nullptr;
    } else {
        LOGI("JSDisplayGetDefaultDisplay new AsyncCallbackInfo = %{private}p.", ptr);
    }
    napi_value result = nullptr;
    if (argcAsync > argcPromise) {
        NAPI_CALL(env, napi_create_reference(env, args[0], 1, &ptr->callback[0]));
        napi_get_undefined(env, &result);
    } else {
        napi_create_promise(env, &ptr->deferred, &result);
    }
    display_plugin->GetDefaultDisplay(ptr);
    LOGI("JSDisplayGetDefaultDisplay: asyncCallback end.");
    return result;
}

static napi_value DisplayExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getDefaultDisplay", JSDisplayGetDefaultDisplay),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module displayModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = DisplayExport,
    .nm_modname = "display",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

#ifdef ANDROID_PLATFORM
static void DisplayJniRegister()
{
    const char className[] = "ohos.ace.plugin.displayplugin.DisplayPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&DisplayJni::Register, className);
}
#endif

extern "C" __attribute__((constructor)) void DisplayRegister()
{
    napi_module_register(&displayModule);
#ifdef ANDROID_PLATFORM
    ARKUI_X_Plugin_RunAsyncTask(&DisplayJniRegister, ARKUI_X_PLUGIN_PLATFORM_THREAD);
#endif
}
} // namespace OHOS::Plugin
