/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef PLUGIN_ACCESSIBILITY_ANDROID_ACCESSIBILITY_SYSTEM_ABILITY_CLIENT_JNI_H
#define PLUGIN_ACCESSIBILITY_ANDROID_ACCESSIBILITY_SYSTEM_ABILITY_CLIENT_JNI_H

#include <jni.h>
#include <mutex>
#include <string>
#include <unordered_map>

namespace OHOS::Plugin {
class AccessibilityClientJni {
public:
    AccessibilityClientJni() = default;
    ~AccessibilityClientJni() = default;
    static bool Register(void* env);
    static void NativeInit(JNIEnv* env, jobject jobj);
    static bool IsEnable();
    static bool RegisterStateListener();
    static void UnregisterStateListener();
    static void OnStateChangedCallback(JNIEnv* env, jobject jobj, bool state);
};
} // namespace OHOS::Plugin
#endif // PLUGIN_ACCESSIBILITY_ANDROID_ACCESSIBILITY_SYSTEM_ABILITY_CLIENT_JNI_H