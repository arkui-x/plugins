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

#include <cstddef>

#include "log.h"
#include "plugin_c_utils.h"

#include "plugins/test_plugin/test_plugin.h"

#ifdef ANDROID_PLATFORM
#include "plugins/test_plugin/android/java/jni/test_plugin_jni.h"
#endif

namespace OHOS::Plugin {

static napi_value JSTestPluginHello(napi_env env, napi_callback_info info)
{
    auto plugin = TestPlugin::Create();
    if (!plugin) {
        LOGW("JSTestPluginHello: testPlugin null return");
        return nullptr;
    }
    plugin->Hello();
    return nullptr;
}

static napi_value TestPluginExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("hello", JSTestPluginHello),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

static napi_module testPluginModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = TestPluginExport,
    .nm_modname = "testPlugin",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

#ifdef ANDROID_PLATFORM
static void TestPluginJniRegister()
{
    const char className[] = "ohos.ace.plugin.testplugin.TestPlugin";
    OH_Plugin_RegisterPlugin(&TestPluginJni::Register, className);
}
#endif

extern "C" __attribute__((constructor)) void TestPluginRegister()
{
    napi_module_register(&testPluginModule);
#ifdef ANDROID_PLATFORM
    OH_Plugin_RunTaskOnPlatform(&TestPluginJniRegister);
#endif
}

} // namespace OHOS::Plugin
