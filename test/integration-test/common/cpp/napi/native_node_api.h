/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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
#ifndef MOCK_NAPI_NATIVE_NODE_API_H
#define MOCK_NAPI_NATIVE_NODE_API_H

#include "napi/native_api.h"

typedef napi_value (*napi_addon_register_func)(napi_env env, napi_value exports);

typedef struct {
    int nm_version;
    unsigned int nm_flags;
    const char* nm_filename;
    napi_addon_register_func nm_register_func;
    const char* nm_modname;
    void* nm_priv;
    void* reserved[4];
} napi_module_with_js;

void napi_module_with_js_register(napi_module_with_js* mod);

#define NAPI_CALL_BASE(env, theCall, retVal) \
    do { napi_status status = (theCall); if (status != napi_ok) return retVal; } while (0)

#define NAPI_CALL_RETURN_VOID(env, theCall) \
    do { napi_status status = (theCall); if (status != napi_ok) return; } while (0)

#endif
