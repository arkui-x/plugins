/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "date_time_init.h"

#include "plugin_utils.h"
#ifdef ANDROID_PLATFORM
#include "plugins/system_date_time/android/java/jni/time_jni.h"
#endif
#include "time_hilog.h"

namespace OHOS {
namespace MiscServices {
namespace Time {
EXTERN_C_START
/*
* Module export function
*/
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {};
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);

    NapiSystemDateTime::SystemDateTimeInit(env, exports);
    return exports;
}
EXTERN_C_END

/*
 * Module define
 */
static napi_module _module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "systemDateTime",
    .nm_priv = ((void *)0),
    .reserved = { 0 } };

#ifdef ANDROID_PLATFORM
static void TimePluginJniRegister()
{
    const char className[] = "ohos.ace.plugin.timeplugin.TimePlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&OHOS::Time::TimeJni::Register, className);
}
#endif

/*
 * Module register function
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
#ifdef ANDROID_PLATFORM
    TimePluginJniRegister();
#endif
    napi_module_register(&_module);
}
} // namespace Time
} // namespace MiscServices
} // namespace OHOS