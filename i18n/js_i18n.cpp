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

#include "plugins/i18n/i18n.h"

#ifdef ANDROID_PLATFORM
#include "plugins/i18n/android/java/jni/i18n_plugin_jni.h"
#endif

namespace OHOS::Plugin {
static napi_value JSI18NPluginIs24HourClock(napi_env env, napi_callback_info info)
{
    auto plugin = I18N::Create();
    if (!plugin) {
        LOGW("JSI18NPluginIs24HourClock: i18nPlugin null return");
        return nullptr;
    }
    bool is24Hour = plugin->Is24HourClock();
    napi_value result = nullptr;
    napi_status status = napi_get_boolean(env, is24Hour, &result);
    if (status != napi_ok) {
        LOGW("JSI18NPluginIs24HourClock failed to create boolean item");
        return nullptr;
    }
    return result;
}

static napi_value I18NPluginExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("is24HourClock", JSI18NPluginIs24HourClock),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module i18nPluginModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = I18NPluginExport,
    .nm_modname = "i18n",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

#ifdef ANDROID_PLATFORM
static void I18NPluginJniRegister()
{
    const char className[] = "ohos.ace.plugin.i18nplugin.I18NPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&I18NPluginJni::Register, className);
}
#endif

extern "C" __attribute__((constructor)) void I18NPluginRegister()
{
#ifdef ANDROID_PLATFORM
    I18NPluginJniRegister();
#endif
    napi_module_register(&i18nPluginModule);
}
} // namespace OHOS::Plugin
