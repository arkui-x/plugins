/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Mock plugin_utils - provides ARKUI_X_Plugin_GetJniEnv() via JavaVM.
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
