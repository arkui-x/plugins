/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "common_event_manager.h"
#include "napi_common_event.h"
#include "support.h"

#ifdef ANDROID_PLATFORM
#include "plugins/common_event_manager/android/java/jni/broadcast_jni.h"
#endif

namespace OHOS {
namespace Plugin {
#ifdef ANDROID_PLATFORM
static void CommonEventPluginJniRegister()
{
    const char className[] = "ohos.ace.plugin.broadcastplugin.BroadcastPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(&BroadcastJni::Register, className);
}
#endif

static napi_value Init(napi_env env, napi_value exports)
{
    SupportInit(env, exports);
    CommonEventSubscriberInit(env, exports);
    CommonEventInit(env, exports);
    return exports;
}

static napi_module commonEventModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "commonEventManager",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
#ifdef ANDROID_PLATFORM
    CommonEventPluginJniRegister();
#endif
    napi_module_register(&commonEventModule);
}
} // namespace Plugin
} // namespace OHOS