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

namespace OHOS::Plugin {
namespace {
const char ACCESSIBLIITY_CLASS_NAME[] = "ohos/ace/plugin/accessibility/Accessibility";
static const JNINativeMethod METHODS[] = {
    {
        "nativeInit",
        "()V",
        reinterpret_cast<void*>(&AccessibilityClientJni::NativeInit),
    },
    { "onAccessibilityStateChangedCallback", "(Z)V",
        reinterpret_cast<void*>(&AccessibilityClientJni::OnStateChangedCallback) },
};

static const char IS_ACCESSIBILITY_ENABLED_METHOD[] = "isAccessibilityEnabled";
static const char IS_ACCESSIBILITY_ENABLED_METHOD_PARAM[] = "()Z";
static const char REGISTER_ACCESSIBILITY_STATE_LISTENER_METHOD[] = "registerAccessibilityStateListener";
static const char REGISTER_ACCESSIBILITY_STATE_LISTENER_METHOD_PARAM[] = "()V";
static const char UNREGISTER_ACCESSIBILITY_STATE_LISTENER_METHOD[] = "unRegisterAccessibilityStateListener";
static const char UNREGISTER_ACCESSIBILITY_STATE_LISTENER_METHOD_PARAM[] = "()V";

struct {
    jobject globalRef;
    jmethodID isAccessibilityEnabled;
    jmethodID registerAccessibilityStateListener;
    jmethodID unRegisterAccessibilityStateListener;
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

void AccessibilityClientJni::OnStateChangedCallback(JNIEnv* env, jobject jobj, bool state)
{
    AccessibilitySystemAbilityEventCallback::ExcuteEventCallback("accessibilityStateChange", state);
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
} // namespace OHOS::Plugin