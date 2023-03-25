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

#ifndef PLUGIN_INTERFACE_NATIVE_PLUGIN_C_UTILS_H
#define PLUGIN_INTERFACE_NATIVE_PLUGIN_C_UTILS_H

#ifdef ANDROID_PLATFORM
#include "jni.h"

// Android plugin utils
JNIEnv* OH_Plugin_GetJniEnv();

void OH_Plugin_RegisterPlugin(bool (*func)(void*), const char* name);
#endif

typedef void (*OH_Plugin_Task)();
void OH_Plugin_RunTaskOnPlatform(OH_Plugin_Task task);

#endif // PLUGIN_INTERFACE_NATIVE_PLUGIN_C_UTILS_H
