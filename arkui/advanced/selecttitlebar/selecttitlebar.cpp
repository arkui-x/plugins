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

#include "napi/native_node_api.h"

extern const char _binary_selecttitlebar_abc_start[];
extern const char _binary_selecttitlebar_abc_end[];

// Napi get abc code function
extern "C" __attribute__((visibility("default"))) void NAPI_arkui_advanced_SelectTitleBar_GetABCCode(
    const char **buf, int *buflen)
{
    if (buf != nullptr) {
        *buf = _binary_selecttitlebar_abc_start;
    }
    if (buflen != nullptr) {
        *buflen = _binary_selecttitlebar_abc_end - _binary_selecttitlebar_abc_start;
    }
}

/*
 * Module define
 */
static napi_module_with_js SelectTitleBarModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_modname = "arkui.advanced.SelectTitleBar",
    .nm_priv = ((void*)0),
    .nm_get_abc_code = NAPI_arkui_advanced_SelectTitleBar_GetABCCode,
};

/*
 * Module registerfunction
 */
extern "C" __attribute__((constructor)) void SelectTitleBarRegisterModule(void)
{
    napi_module_with_js_register(&SelectTitleBarModule);
}
