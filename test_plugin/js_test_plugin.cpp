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

#ifdef IOS_PLATFORM
static napi_value JSTestPluginGetFilesDir(napi_env env, napi_callback_info info)
{
    auto plugin = TestPlugin::Create();
    if (!plugin) {
        LOGW("JSTestPluginGetFilesDir: testPlugin null return");
        return nullptr;
    }
    std::string filesPath = plugin->GetFilesDir();
    const char *filesPathStr = filesPath.c_str();
    napi_value result = nullptr;
    napi_status status = napi_create_string_utf8(env, filesPathStr, strlen(filesPathStr), &result);
    if (status != napi_ok) {
        LOGE("JSTestPluginGetFilesDir: failed to create string item");
        return nullptr;
    }
    return result;
}
#endif

static napi_value TestPluginExport(napi_env env, napi_value exports)
{
    static napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("hello", JSTestPluginHello),
#ifdef IOS_PLATFORM
        DECLARE_NAPI_FUNCTION("getFilesDir", JSTestPluginGetFilesDir),
#endif
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
    ARKUI_X_Plugin_RegisterJavaPlugin(&TestPluginJni::Register, className);
}
#endif

extern "C" __attribute__((constructor)) void TestPluginRegister()
{
#ifdef ANDROID_PLATFORM
    TestPluginJniRegister();
#endif
    napi_module_register(&testPluginModule);
}

} // namespace OHOS::Plugin
