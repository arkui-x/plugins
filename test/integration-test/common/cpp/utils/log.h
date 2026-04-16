/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
 * Mock HILOG macros for Android JNI integration testing.
 */
#ifndef MOCK_UTILS_LOG_H
#define MOCK_UTILS_LOG_H
#include <android/log.h>
#define HILOG_INFO(...)  __android_log_print(ANDROID_LOG_INFO, "ArkUI-X-Test", __VA_ARGS__)
#define HILOG_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, "ArkUI-X-Test", __VA_ARGS__)
#define HILOG_DEBUG(...) __android_log_print(ANDROID_LOG_DEBUG, "ArkUI-X-Test", __VA_ARGS__)
#define HILOG_WARN(...)  __android_log_print(ANDROID_LOG_WARN, "ArkUI-X-Test", __VA_ARGS__)
#define HILOG_FATAL(...) __android_log_print(ANDROID_LOG_FATAL, "ArkUI-X-Test", __VA_ARGS__)
#endif
