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

#include "accessibility_system_ability_client_jni.h"

#include "accessibility_system_ability_event_callback.h"
#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

using namespace OHOS::Ace::Framework;

namespace OHOS::Plugin {
namespace {
const char ACCESSIBILITY_STATE_CHANGE_STRING[] = "accessibilityStateChange";
const char ACCESSIBLIITY_CLASS_NAME[] = "ohos/ace/plugin/accessibility/Accessibility";
static const JNINativeMethod METHODS[] = {
    {
        "nativeInit",
        "()V",
        reinterpret_cast<void*>(&AccessibilityClientJni::NativeInit),
    },
    { "onAccessibilityStateChangedCallback", "(ZLjava/lang/String;)V",
        reinterpret_cast<void*>(&AccessibilityClientJni::OnStateChangedCallback) },
};

static const char IS_ACCESSIBILITY_ENABLED_METHOD[] = "isAccessibilityEnabled";
static const char IS_ACCESSIBILITY_ENABLED_METHOD_PARAM[] = "()Z";
static const char REGISTER_ACCESSIBILITY_STATE_LISTENER_METHOD[] = "registerAccessibilityStateListener";
static const char REGISTER_ACCESSIBILITY_STATE_LISTENER_METHOD_PARAM[] = "()V";
static const char UNREGISTER_ACCESSIBILITY_STATE_LISTENER_METHOD[] = "unRegisterAccessibilityStateListener";
static const char UNREGISTER_ACCESSIBILITY_STATE_LISTENER_METHOD_PARAM[] = "()V";
static const char IS_TOUCH_EXPLORATION_ENABLED_METHOD[] = "isTouchExplorationEnabled";
static const char IS_TOUCH_EXPLORATION_ENABLED_METHOD_PARAM[] = "()Z";
static const char REGISTER_TOUCH_EXPLORATION_LISTENER_METHOD[] = "registerTouchExplorationListener";
static const char REGISTER_TOUCH_EXPLORATION_LISTENER_METHOD_PARAM[] = "()V";
static const char UNREGISTER_TOUCH_EXPLORATION_LISTENER_METHOD[] = "unRegisterTouchExplorationListener";
static const char UNREGISTER_TOUCH_EXPLORATION_LISTENER_METHOD_PARAM[] = "()V";
static const char GET_ACCESSIBILITY_EXTENSION_LIST_SYNC_METHOD[] = "getAccessibilityExtensionList";
static const char GET_ACCESSIBILITY_EXTENSION_LIST_SYNC_METHOD_PARAM[] =
    "(Ljava/lang/String;Ljava/lang/String;)Ljava/util/List;";

struct {
    jobject globalRef;
    jmethodID isAccessibilityEnabled;
    jmethodID registerAccessibilityStateListener;
    jmethodID unRegisterAccessibilityStateListener;
    jmethodID isTouchExplorationEnabled;
    jmethodID registerTouchExplorationListener;
    jmethodID unRegisterTouchExplorationListener;
    jmethodID getAccessibilityExtensionListSync;
} g_accessibilityClass;
} // namespace

bool AccessibilityClientJni::Register(void* env)
{
    JNIEnv* jniEnv = reinterpret_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(ACCESSIBLIITY_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("AccessibilityClientJni JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void AccessibilityClientJni::NativeInit(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    g_accessibilityClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_accessibilityClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_accessibilityClass.isAccessibilityEnabled =
        env->GetMethodID(cls, IS_ACCESSIBILITY_ENABLED_METHOD, IS_ACCESSIBILITY_ENABLED_METHOD_PARAM);
    g_accessibilityClass.registerAccessibilityStateListener = env->GetMethodID(
        cls, REGISTER_ACCESSIBILITY_STATE_LISTENER_METHOD, REGISTER_ACCESSIBILITY_STATE_LISTENER_METHOD_PARAM);
    g_accessibilityClass.unRegisterAccessibilityStateListener = env->GetMethodID(
        cls, UNREGISTER_ACCESSIBILITY_STATE_LISTENER_METHOD, UNREGISTER_ACCESSIBILITY_STATE_LISTENER_METHOD_PARAM);
    g_accessibilityClass.isTouchExplorationEnabled =
        env->GetMethodID(cls, IS_TOUCH_EXPLORATION_ENABLED_METHOD, IS_TOUCH_EXPLORATION_ENABLED_METHOD_PARAM);
    g_accessibilityClass.registerTouchExplorationListener = env->GetMethodID(
        cls, REGISTER_TOUCH_EXPLORATION_LISTENER_METHOD, REGISTER_TOUCH_EXPLORATION_LISTENER_METHOD_PARAM);
    g_accessibilityClass.unRegisterTouchExplorationListener = env->GetMethodID(
        cls, UNREGISTER_TOUCH_EXPLORATION_LISTENER_METHOD, UNREGISTER_TOUCH_EXPLORATION_LISTENER_METHOD_PARAM);
    g_accessibilityClass.getAccessibilityExtensionListSync = env->GetMethodID(
        cls, GET_ACCESSIBILITY_EXTENSION_LIST_SYNC_METHOD, GET_ACCESSIBILITY_EXTENSION_LIST_SYNC_METHOD_PARAM);
    env->DeleteLocalRef(cls);
}

bool AccessibilityClientJni::IsEnable()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_accessibilityClass.globalRef, false);
    CHECK_NULL_RETURN(g_accessibilityClass.isAccessibilityEnabled, false);
    bool status = env->CallBooleanMethod(g_accessibilityClass.globalRef, g_accessibilityClass.isAccessibilityEnabled);
    if (env->ExceptionCheck()) {
        LOGE("AccessibilityClientJni: call IsEnable failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return status;
}

void AccessibilityClientJni::OnStateChangedCallback(JNIEnv* env, jobject jobj, bool state, jstring jEventName)
{
    std::string eventName = "";
    if (jEventName != nullptr) {
        const char* eventNameStr = env->GetStringUTFChars(jEventName, nullptr);
        eventName = eventNameStr;
        env->ReleaseStringUTFChars(jEventName, eventNameStr);
    }
    if (eventName == ACCESSIBILITY_STATE_CHANGE_STRING) {
        AccessibilitySystemAbilityEventCallback::ExcuteStateEventCallback(state);
    } else {
        AccessibilitySystemAbilityEventCallback::ExcuteEventCallback(state);
    }
}

bool AccessibilityClientJni::RegisterStateListener()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_accessibilityClass.globalRef, false);
    CHECK_NULL_RETURN(g_accessibilityClass.registerAccessibilityStateListener, false);
    env->CallVoidMethod(g_accessibilityClass.globalRef, g_accessibilityClass.registerAccessibilityStateListener);
    if (env->ExceptionCheck()) {
        LOGE("AccessibilityClientJni: call RegisterStateListener failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return true;
}

void AccessibilityClientJni::UnregisterStateListener()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_accessibilityClass.globalRef);
    CHECK_NULL_VOID(g_accessibilityClass.unRegisterAccessibilityStateListener);
    env->CallVoidMethod(g_accessibilityClass.globalRef, g_accessibilityClass.unRegisterAccessibilityStateListener);
    if (env->ExceptionCheck()) {
        LOGE("AccessibilityClientJni: call UnregisterStateListener failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

bool AccessibilityClientJni::IsTouchExplorationEnable()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_accessibilityClass.globalRef, false);
    CHECK_NULL_RETURN(g_accessibilityClass.isTouchExplorationEnabled, false);
    bool status =
        env->CallBooleanMethod(g_accessibilityClass.globalRef, g_accessibilityClass.isTouchExplorationEnabled);
    if (env->ExceptionCheck()) {
        LOGE("AccessibilityClientJni: call isTouchExplorationEnabled failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return status;
}

bool AccessibilityClientJni::RegisterTouchExplorationListener()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_accessibilityClass.globalRef, false);
    CHECK_NULL_RETURN(g_accessibilityClass.registerTouchExplorationListener, false);
    env->CallVoidMethod(g_accessibilityClass.globalRef, g_accessibilityClass.registerTouchExplorationListener);
    if (env->ExceptionCheck()) {
        LOGE("AccessibilityClientJni: call registerTouchExplorationListener failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return true;
}

void AccessibilityClientJni::UnregisterTouchExplorationListener()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_accessibilityClass.globalRef);
    CHECK_NULL_VOID(g_accessibilityClass.unRegisterTouchExplorationListener);
    env->CallVoidMethod(g_accessibilityClass.globalRef, g_accessibilityClass.unRegisterTouchExplorationListener);
    if (env->ExceptionCheck()) {
        LOGE("AccessibilityClientJni: call unRegisterTouchExplorationListener failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

std::vector<AccessibilityAbilityInfo> AccessibilityClientJni::GetAccessibilityExtensionListSync(
    const std::string& abilityType, const std::string& stateType)
{
    std::vector<AccessibilityAbilityInfo> result;

    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, result);
    CHECK_NULL_RETURN(g_accessibilityClass.globalRef, result);
    CHECK_NULL_RETURN(g_accessibilityClass.getAccessibilityExtensionListSync, result);

    jstring jAbilityType = env->NewStringUTF(abilityType.c_str());
    jstring jStateType = env->NewStringUTF(stateType.c_str());

    jobject jList = CallJavaMethod(env, jAbilityType, jStateType);
    if (!jList) {
        env->DeleteLocalRef(jAbilityType);
        env->DeleteLocalRef(jStateType);
        return result;
    }

    ProcessJavaListResult(env, jList, result);

    env->DeleteLocalRef(jAbilityType);
    env->DeleteLocalRef(jStateType);
    env->DeleteLocalRef(jList);
    return result;
}

jobject AccessibilityClientJni::CallJavaMethod(JNIEnv* env, jstring jAbilityType, jstring jStateType)
{
    jobject jList = env->CallObjectMethod(g_accessibilityClass.globalRef,
        g_accessibilityClass.getAccessibilityExtensionListSync, jAbilityType, jStateType);

    if (env->ExceptionCheck()) {
        LOGE("AccessibilityClientJni: call GetAccessibilityExtensionListSync failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return nullptr;
    }

    return jList;
}

void AccessibilityClientJni::ProcessJavaListResult(
    JNIEnv* env, jobject jList, std::vector<AccessibilityAbilityInfo>& result)
{
    jclass listClass = env->GetObjectClass(jList);
    jmethodID sizeMethod = env->GetMethodID(listClass, "size", "()I");
    jint size = env->CallIntMethod(jList, sizeMethod);

    jmethodID getMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
    jobject jInfo = nullptr;
    for (int i = 0; i < size; i++) {
        jInfo = env->CallObjectMethod(jList, getMethod, i);
        if (jInfo != nullptr) {
            AccessibilityAbilityInfo info;
            ProcessJavaInfoObject(env, jInfo, info);
            result.push_back(info);
            env->DeleteLocalRef(jInfo);
        }
    }
    env->DeleteLocalRef(listClass);
}

void AccessibilityClientJni::ProcessJavaInfoObject(JNIEnv* env, jobject jInfo, AccessibilityAbilityInfo& info)
{
    jclass infoClass = env->GetObjectClass(jInfo);

    ExtractBasicFields(env, jInfo, infoClass, info);
    ExtractListFields(env, jInfo, infoClass, info);

    env->DeleteLocalRef(infoClass);
}

void AccessibilityClientJni::ExtractBasicFields(
    JNIEnv* env, jobject jInfo, jclass infoClass, AccessibilityAbilityInfo& info)
{
    jfieldID idField = env->GetFieldID(infoClass, "id", "Ljava/lang/String;");
    jfieldID nameField = env->GetFieldID(infoClass, "name", "Ljava/lang/String;");
    jfieldID bundleNameField = env->GetFieldID(infoClass, "bundleName", "Ljava/lang/String;");
    jfieldID descriptionField = env->GetFieldID(infoClass, "description", "Ljava/lang/String;");
    if (idField) {
        jstring jId = (jstring)env->GetObjectField(jInfo, idField);
        if (jId) {
            const char* idStr = env->GetStringUTFChars(jId, nullptr);
            info.id = idStr;
            env->ReleaseStringUTFChars(jId, idStr);
            env->DeleteLocalRef(jId);
        }
    }
    if (nameField) {
        jstring jName = (jstring)env->GetObjectField(jInfo, nameField);
        if (jName) {
            const char* nameStr = env->GetStringUTFChars(jName, nullptr);
            info.name = nameStr;
            env->ReleaseStringUTFChars(jName, nameStr);
            env->DeleteLocalRef(jName);
        }
    }
    if (bundleNameField) {
        jstring jBundleName = (jstring)env->GetObjectField(jInfo, bundleNameField);
        if (jBundleName) {
            const char* bundleNameStr = env->GetStringUTFChars(jBundleName, nullptr);
            info.bundleName = bundleNameStr;
            env->ReleaseStringUTFChars(jBundleName, bundleNameStr);
            env->DeleteLocalRef(jBundleName);
        }
    }
    if (descriptionField) {
        jstring jDescription = (jstring)env->GetObjectField(jInfo, descriptionField);
        if (jDescription) {
            const char* descriptionStr = env->GetStringUTFChars(jDescription, nullptr);
            info.description = descriptionStr;
            env->ReleaseStringUTFChars(jDescription, descriptionStr);
            env->DeleteLocalRef(jDescription);
        }
    }
}

void AccessibilityClientJni::ExtractListFields(
    JNIEnv* env, jobject jInfo, jclass infoClass, AccessibilityAbilityInfo& info)
{
    jfieldID targetBundleNamesField = env->GetFieldID(infoClass, "targetBundleNames", "Ljava/util/List;");
    jfieldID abilityTypesField = env->GetFieldID(infoClass, "abilityTypes", "Ljava/util/List;");
    jfieldID capabilitiesField = env->GetFieldID(infoClass, "capabilities", "Ljava/util/List;");
    jfieldID eventTypesField = env->GetFieldID(infoClass, "eventTypes", "Ljava/util/List;");

    if (targetBundleNamesField) {
        ExtractStringListField(env, jInfo, targetBundleNamesField, info.targetBundleNames);
    }

    if (abilityTypesField) {
        ExtractStringListField(env, jInfo, abilityTypesField, info.abilityTypes);
    }

    if (capabilitiesField) {
        ExtractStringListField(env, jInfo, capabilitiesField, info.capabilities);
    }

    if (eventTypesField) {
        ExtractStringListField(env, jInfo, eventTypesField, info.eventTypes);
    }
}

void AccessibilityClientJni::ExtractStringListField(
    JNIEnv* env, jobject jInfo, jfieldID fieldId, std::vector<std::string>& targetVector)
{
    jobject jList = env->GetObjectField(jInfo, fieldId);
    if (!jList) {
        return;
    }

    jclass listClass = env->GetObjectClass(jList);
    jmethodID sizeMethod = env->GetMethodID(listClass, "size", "()I");
    jmethodID getMethod = env->GetMethodID(listClass, "get", "(I)Ljava/lang/Object;");
    jint size = env->CallIntMethod(jList, sizeMethod);

    for (int i = 0; i < size; i++) {
        jstring jItem = (jstring)env->CallObjectMethod(jList, getMethod, i);
        if (jItem) {
            const char* itemStr = env->GetStringUTFChars(jItem, nullptr);
            targetVector.push_back(itemStr);
            env->ReleaseStringUTFChars(jItem, itemStr);
            env->DeleteLocalRef(jItem);
        }
    }

    env->DeleteLocalRef(jList);
    env->DeleteLocalRef(listClass);
}
} // namespace OHOS::Plugin