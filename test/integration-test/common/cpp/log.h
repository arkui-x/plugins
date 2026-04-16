/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Mock LOG macros - uses Android log for testing.
 */
#ifndef MOCK_LOG_H
#define MOCK_LOG_H
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "ArkUI-X-Test", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "ArkUI-X-Test", __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "ArkUI-X-Test", __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, "ArkUI-X-Test", __VA_ARGS__)
#define LOGF(...) __android_log_print(ANDROID_LOG_FATAL, "ArkUI-X-Test", __VA_ARGS__)
#endif
