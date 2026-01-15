/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2026. All rights reserved.
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

#ifdef ANDROID_PLATFORM
#include "clipboard_jni.h"
#include "plugin_utils.h"
#endif
#include "pasteboard_napi.h"
#include "pasteboard_progress_signal_napi.h"
#include "pastedata_record_napi.h"

namespace OHOS {
namespace MiscServicesNapi {
EXTERN_C_START
/*
 * Module export function
 */
static napi_value NapiInit(napi_env env, napi_value exports)
{
    /*
     * Properties define
     */
    PasteDataRecordNapi::PasteDataRecordInit(env, exports);
    PasteDataNapi::PasteDataInit(env, exports);
    SystemPasteboardNapi::SystemPasteboardInit(env, exports);
    PasteboardNapi::PasteBoardInit(env, exports);
    ProgressSignalNapi::ProgressSignalNapiInit(env, exports);
    return exports;
}
EXTERN_C_END

/*
 * Module define
 */
static napi_module _module = { .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = NapiInit,
    .nm_modname = "pasteboard",
    .nm_priv = nullptr,
    .reserved = { 0 } };

/*
 * Module register function
 */
extern "C" __attribute__((constructor)) void RegisterModule(void)
{
#ifdef ANDROID_PLATFORM
    const char clipboardPluginName[] = "ohos.ace.plugin.clipboard.ClipboardAosp";
    ARKUI_X_Plugin_RegisterJavaPlugin(&OHOS::Plugin::ClipboardJni::Register, clipboardPluginName);
#endif
    napi_module_register(&_module);
}
} // namespace MiscServicesNapi
} // namespace OHOS
