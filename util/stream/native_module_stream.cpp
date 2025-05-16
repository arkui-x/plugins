 /*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "napi/native_node_api.h"

extern const char _binary_stream_js_js_start[];
extern const char _binary_stream_js_js_end[];
extern const char _binary_stream_abc_start[];
extern const char _binary_stream_abc_end[];

static napi_value StreamInit(napi_env env, napi_value exports)
{
    return exports;
}

extern "C"
__attribute__((visibility("default"))) void NAPI_util_stream_GetJSCode(const char **buf, int *bufLen)
{
    if (buf != nullptr) {
        *buf = _binary_stream_js_js_start;
    }

    if (bufLen != nullptr) {
        *bufLen = _binary_stream_js_js_end - _binary_stream_js_js_start;
    }
}
extern "C"
__attribute__((visibility("default"))) void NAPI_util_stream_GetABCCode(const char** buf, int* buflen)
{
    if (buf != nullptr) {
        *buf = _binary_stream_abc_start;
    }
    if (buflen != nullptr) {
        *buflen = _binary_stream_abc_end - _binary_stream_abc_start;
    }
}

static napi_module_with_js streamModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = StreamInit,
    .nm_modname = "util.stream",
    .nm_priv = ((void*)0),
    .nm_get_abc_code = NAPI_util_stream_GetABCCode,
    .nm_get_js_code = NAPI_util_stream_GetJSCode,
};

extern "C" __attribute__ ((constructor)) void StreamRegisterModule()
{
    napi_module_with_js_register(&streamModule);
}
