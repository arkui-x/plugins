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

#ifndef PLUGIN_INTERFACE_NATIVE_PLUGIN_C_UTILS_H
#define PLUGIN_INTERFACE_NATIVE_PLUGIN_C_UTILS_H

#include <cstddef>
#include <cstdint>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#ifdef ANDROID_PLATFORM
#include "jni.h"

// Android plugin utils
JNIEnv* OH_Plugin_GetJniEnv(bool isDetach = true);

void OH_Plugin_RegisterPlugin(bool (*func)(void*), const char* name);
#endif

typedef void (*OH_Plugin_Task)();
void OH_Plugin_RunTaskOnPlatform(OH_Plugin_Task task);

typedef enum ReportEventType {
    TYPE_CHECK_SUCCESS = true,
    TYPE_CHECK_FAIL = false,
} OH_Plugin_ReportEventType;

struct OH_Plugin_AsyncCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    napi_deferred deferred;
    napi_ref callback[1] = { 0 };
    napi_value jsdata;
    int32_t status;
};

OH_Plugin_ReportEventType OH_Plugin_IsMatchType(napi_value value, napi_valuetype type, napi_env env);
napi_value OH_Plugin_GetNapiInt32(int32_t number, napi_env env);
napi_value OH_Plugin_GetNamedProperty(napi_value jsonObject, std::string name, napi_env env);
int32_t OH_Plugin_GetCInt32(napi_value value, napi_env env);
int64_t OH_Plugin_GetCInt64(napi_value value, napi_env env);
size_t OH_Plugin_GetCString(napi_value value, napi_env env, char* buf, size_t size);
void OH_Plugin_EmitPromiseWork(OH_Plugin_AsyncCallbackInfo* asyncCallbackInfo);
void OH_Plugin_EmitAsyncCallbackWork(OH_Plugin_AsyncCallbackInfo* asyncCallbackInfo);

#endif // PLUGIN_INTERFACE_NATIVE_PLUGIN_C_UTILS_H
