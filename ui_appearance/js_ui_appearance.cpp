/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
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

#include "plugins/ui_appearance/ui_appearance.h"

#ifdef ANDROID_PLATFORM
#include "plugins/ui_appearance/android/java/jni/ui_appearance_jni.h"
#endif

namespace OHOS::Plugin {

static napi_value JSGetDarkMode(napi_env env, napi_callback_info info)
{
    auto plugin = UiAppearance::Create();
    if (!plugin) {
        LOGE("JSGetDarkMode: uiAppearance null return");
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    }
    napi_value result = nullptr;
    napi_create_int32(env, plugin->GetDarkMode(), &result);
    return result;
}

static napi_value JSGetFontScale(napi_env env, napi_callback_info info)
{
    auto plugin = UiAppearance::Create();
    if (!plugin) {
        LOGE("JSGetFontScale: uiAppearance null return");
        napi_value result = nullptr;
        napi_create_double(env, 1.0, &result);
        return result;
    }
    napi_value result = nullptr;
    napi_create_double(env, plugin->GetFontScale(), &result);
    return result;
}

static napi_value UiAppearanceExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("getDarkMode", JSGetDarkMode),
        DECLARE_NAPI_FUNCTION("getFontScale", JSGetFontScale),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module uiAppearanceModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = UiAppearanceExport,
    .nm_modname = "uiAppearance",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

#ifdef ANDROID_PLATFORM
static void UiAppearanceJniRegister()
{
    const char className[] = "ohos.ace.plugin.uiappearance.UiAppearancePlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&UiAppearanceJni::Register, className);
}
#endif

extern "C" __attribute__((constructor)) void UiAppearanceRegister()
{
#ifdef ANDROID_PLATFORM
    UiAppearanceJniRegister();
#endif
    napi_module_register(&uiAppearanceModule);
}

} // namespace OHOS::Plugin
