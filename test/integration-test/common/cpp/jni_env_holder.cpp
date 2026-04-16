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
#include "plugin_utils.h"
#include <android/log.h>

#define LOG_TAG "JniEnvHolder"

// Note: g_javaVM is set once during test initialization (JNI_OnLoad) on the main thread.
// This mock assumes single-threaded instrumentation test execution.
static JavaVM* g_javaVM = nullptr;

void Mock_SetJavaVM(JavaVM* vm)
{
    g_javaVM = vm;
}

JNIEnv* ARKUI_X_Plugin_GetJniEnv()
{
    if (!g_javaVM) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "JavaVM is null");
        return nullptr;
    }
    JNIEnv* env = nullptr;
    if (g_javaVM->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_OK) {
        return env;
    }
    __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, "GetEnv failed");
    return nullptr;
}

/**
 * Mock of ARKUI_X_Plugin_RegisterJavaPlugin.
 * In production, this is called during plugin initialization to register
 * JNI native methods for Java→C++ callbacks. In the test environment,
 * we immediately invoke the register callback since JNI_OnLoad has already
 * set up the JNI environment.
 */
void ARKUI_X_Plugin_RegisterJavaPlugin(bool (*func)(void*), const char* name)
{
    if (!func) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
            "ARKUI_X_Plugin_RegisterJavaPlugin: func is null");
        return;
    }
    JNIEnv* env = ARKUI_X_Plugin_GetJniEnv();
    if (!env) {
        __android_log_print(ANDROID_LOG_ERROR, LOG_TAG,
            "ARKUI_X_Plugin_RegisterJavaPlugin: env is null");
        return;
    }
    __android_log_print(ANDROID_LOG_INFO, LOG_TAG,
        "ARKUI_X_Plugin_RegisterJavaPlugin: invoking register for %s", name);
    func(env);
}

void ARKUI_X_Plugin_RunAsyncTask(ARKUI_X_Plugin_Task task, ARKUI_X_Plugin_Thread_Mode mode)
{
    if (task) {
        task();
    }
}
