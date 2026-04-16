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
#ifndef MOCK_PLUGIN_UTILS_H
#define MOCK_PLUGIN_UTILS_H
#include <jni.h>

void Mock_SetJavaVM(JavaVM* vm);
JNIEnv* ARKUI_X_Plugin_GetJniEnv();
void ARKUI_X_Plugin_RegisterJavaPlugin(bool (*func)(void*), const char* name);

typedef void (*ARKUI_X_Plugin_Task)(void);

typedef enum {
    ARKUI_X_PLUGIN_PLATFORM_THREAD = 1,
    ARKUI_X_PLUGIN_JS_THREAD = 2,
} ARKUI_X_Plugin_Thread_Mode;

void ARKUI_X_Plugin_RunAsyncTask(ARKUI_X_Plugin_Task task, ARKUI_X_Plugin_Thread_Mode mode);

#endif
