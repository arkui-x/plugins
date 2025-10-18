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
#include <vector>
#include <string>
#include <unordered_map>

#include "foundation/arkui/ace_engine/adapter/android/osal/accessibility_event_manager.h"

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
    static bool IsTouchExplorationEnable();
    static bool RegisterTouchExplorationListener();
    static void UnregisterTouchExplorationListener();
    static void OnStateChangedCallback(JNIEnv* env, jobject jobj, bool state, jstring jEventName);
    static std::vector<OHOS::Ace::Framework::AccessibilityAbilityInfo> GetAccessibilityExtensionListSync(
        const std::string& abilityType, const std::string& stateType);

private:
    static jobject CallJavaMethod(JNIEnv* env, jstring jAbilityType, jstring jStateType);
    static void ProcessJavaListResult(
        JNIEnv* env, jobject jList, std::vector<OHOS::Ace::Framework::AccessibilityAbilityInfo>& result);
    static void ProcessJavaInfoObject(JNIEnv* env, jobject jInfo, OHOS::Ace::Framework::AccessibilityAbilityInfo& info);
    static void ExtractBasicFields(
        JNIEnv* env, jobject jInfo, jclass infoClass, OHOS::Ace::Framework ::AccessibilityAbilityInfo& info);
    static void ExtractListFields(
        JNIEnv* env, jobject jInfo, jclass infoClass, OHOS::Ace::Framework ::AccessibilityAbilityInfo& info);
    static void ExtractStringListField(
        JNIEnv* env, jobject jInfo, jfieldID fieldId, std::vector<std::string>& targetVector);
};
} // namespace OHOS::Plugin
#endif // PLUGIN_ACCESSIBILITY_ANDROID_ACCESSIBILITY_SYSTEM_ABILITY_CLIENT_JNI_H