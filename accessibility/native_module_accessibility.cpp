/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_accessibility_system_ability_client.h"
#include "plugin_utils.h"
#ifdef ANDROID_PLATFORM
#include "plugins/accessibility/android/java/jni/accessibility_system_ability_client_jni.h"
#endif

namespace OHOS::Plugin {

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("isOpenAccessibilitySync", NAccessibilityClient::IsOpenStateAccessibilitySync),
        DECLARE_NAPI_FUNCTION("isOpenTouchGuideSync", NAccessibilityClient::IsOpenAccessibilitySync),
        DECLARE_NAPI_FUNCTION("isScreenReaderOpenSync", NAccessibilityClient::IsOpenAccessibilitySync),
        DECLARE_NAPI_FUNCTION("getTouchModeSync", NAccessibilityClient::GetTouchModeSync),
        DECLARE_NAPI_FUNCTION("sendAccessibilityEvent", NAccessibilityClient::SendAccessibilityEvent),
        DECLARE_NAPI_FUNCTION("on", NAccessibilityClient::SubscribeState),
        DECLARE_NAPI_FUNCTION("off", NAccessibilityClient::UnsubscribeState),
        DECLARE_NAPI_FUNCTION("getAccessibilityExtensionListSync",
            NAccessibilityClient::GetAccessibilityExtensionListSync),
        DECLARE_NAPI_FUNCTION("getAccessibilityExtensionList", NAccessibilityClient::GetAccessibilityExtensionList),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "accessibility",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

#ifdef ANDROID_PLATFORM
static void AccessibilityPluginJniRegister()
{
    const char className[] = "ohos.ace.plugin.accessibility.Accessibility";
    ARKUI_X_Plugin_RegisterJavaPlugin(&OHOS::Plugin::AccessibilityClientJni::Register, className);
}
#endif

extern "C" __attribute__((constructor)) void RegisterNapiAccessibilityModule(void)
{
#ifdef ANDROID_PLATFORM
    AccessibilityPluginJniRegister();
#endif
    napi_module_register(&_module);
}
} // namespace OHOS::Plugin