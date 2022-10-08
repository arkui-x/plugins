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

#ifndef PLUGINS_DISPLAY_ANDROID_JAVA_JNI_DISPLAY_JNI_H
#define PLUGINS_DISPLAY_ANDROID_JAVA_JNI_DISPLAY_JNI_H

#include <jni.h>

#include "plugins/display/display.h"

namespace OHOS::Plugin {
class DisplayJni final {
public:
    DisplayJni() = default;
    ~DisplayJni() = default;
    static bool Register(void* env);
    // Called by Java
    static void NativeInit(JNIEnv* env, jobject jobj);
    // Called by C++
    static void GetDefaultDisplay(OH_Plugin_AsyncCallbackInfo *ptr);
private:
    jint GetId(JNIEnv* env, jclass cls, jobject jobj);
    std::string GetName(JNIEnv* env, jclass cls, jobject jobj);
    jboolean IsAlive(JNIEnv* env, jclass cls, jobject jobj);
    jint GetState(JNIEnv* env, jclass cls, jobject jobj);
    jfloat GetRefreshRate(JNIEnv* env, jclass cls, jobject jobj);
    jint GetRotation(JNIEnv* env, jclass cls, jobject jobj);
    jint GetWidth(JNIEnv* env, jclass cls, jobject jobj);
    jint GetHeight(JNIEnv* env, jclass cls, jobject jobj);
    jint GetDensityDPI(JNIEnv* env, jclass cls, jobject jobj);
    jfloat GetDensityPixels(JNIEnv* env, jclass cls, jobject jobj);
    jfloat GetScaledDensity(JNIEnv* env, jclass cls, jobject jobj);
    jfloat GetXDPI(JNIEnv* env, jclass cls, jobject jobj);
    jfloat GetYDPI(JNIEnv* env, jclass cls, jobject jobj);
};
} // namespace OHOS::Plugin
#endif // PLUGINS_DISPLAY_ANDROID_JAVA_JNI_DISPLAY_JNI_H
