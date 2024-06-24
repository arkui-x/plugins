/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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
#include "android/java/jni/filepicker_jni.h"
#endif
#include "js_native_api.h"
#include "js_native_api_types.h"
#include "log.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_document_view_picker.h"
#include "plugin_utils.h"

extern const char _binary_picker_js_start[];
extern const char _binary_picker_js_end[];
extern const char _binary_picker_abc_start[];
extern const char _binary_picker_abc_end[];

namespace OHOS::Plugin {
static napi_value Export(napi_env env, napi_value exports)
{
    DocumentViewPickerInit(env, exports);
    return exports;
}

extern "C" __attribute__((visibility("default"))) void NAPI_file_picker_GetJSCode(const char** buf, int* bufLen)
{
    if (buf != nullptr) {
        *buf = _binary_picker_js_start;
    }

    if (bufLen != nullptr) {
        *bufLen = _binary_picker_js_end - _binary_picker_js_start;
    }
}

extern "C" __attribute__((visibility("default"))) void NAPI_file_picker_GetABCCode(const char** buf, int* bufLen)
{
    if (buf != nullptr) {
        *buf = _binary_picker_abc_start;
    }

    if (bufLen != nullptr) {
        *bufLen = _binary_picker_abc_end - _binary_picker_abc_start;
    }
}

/*
 * module define
 */
static napi_module_with_js g_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Export,
    .nm_modname = "file.picker",
    .nm_priv = reinterpret_cast<void*>(0),
    .nm_get_abc_code = NAPI_file_picker_GetABCCode,
    .nm_get_js_code = NAPI_file_picker_GetJSCode,
};

/*
 * module register
 */
extern "C" __attribute__((constructor)) void RegisterModule()
{
    LOGI("ohos.picker RegisterModule enter");
#ifdef ANDROID_PLATFORM
    const char className[] = "ohos.ace.plugin.file.fs.picker.filepicker.FilePicker";
    ARKUI_X_Plugin_RegisterJavaPlugin(&FilePickerJni::Register, className);
#endif
    napi_module_with_js_register(&g_module);
}
} // namespace OHOS::Plugin
