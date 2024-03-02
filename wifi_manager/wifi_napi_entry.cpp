/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "log.h"
#include "plugin_utils.h"
#include "wifi_napi_device.h"

#ifdef ANDROID_PLATFORM
#include "plugins/wifi_manager/android/java/jni/wifi_device_jni.h"
#endif

namespace OHOS::Plugin {
#ifdef ANDROID_PLATFORM
static void WifiManagerPluginJniRegister()
{
    const char className[] = "ohos.ace.plugin.wifimanager.WifiDevicePlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&WiFiDeviceJni::Register, className);
}
#endif
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("isWifiActive", IsWifiActive),
        DECLARE_NAPI_FUNCTION("getLinkedInfo", GetLinkedInfo),
        DECLARE_NAPI_FUNCTION("on", On),
        DECLARE_NAPI_FUNCTION("off", Off),
        DECLARE_NAPI_FUNCTION("isConnected", IsConnected),
    };

    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(napi_property_descriptor), desc));
    return exports;
}

static napi_module wifiJsModule = { .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = NULL,
    .nm_register_func = Init,
    .nm_modname = "wifiManager",
    .nm_priv = ((void*)0),
    .reserved = { 0 } };

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
#ifdef ANDROID_PLATFORM
    WifiManagerPluginJniRegister();
#endif
    napi_module_register(&wifiJsModule);
}
} // namespace OHOS::Plugin
