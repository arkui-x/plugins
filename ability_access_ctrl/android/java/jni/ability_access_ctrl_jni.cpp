/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "plugins/ability_access_ctrl/android/java/jni/ability_access_ctrl_jni.h"

#include <codecvt>
#include <locale>
#include <jni.h>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

namespace OHOS::Plugin {
namespace {
const char CLASS_NAME[] = "ohos/ace/plugin/abilityaccessctrl/AbilityAccessCtrl";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(AbilityAccessCtrlJni::NativeInit) },
};

const char METHOD_CHECK_PERMISSION[] = "checkPermission";
const char METHOD_REQUEST_PERMISSIONS[] = "requestPermissions";

const char SIGNATURE_CHECK_PERMISSION[] = "(Ljava/lang/String;)Z";
const char SIGNATURE_REQUEST_PERMISSIONS[] = "([Ljava/lang/String;)V";

struct {
    jmethodID checkPermission;
    jmethodID requestPermissions;
    jobject globalRef;
} g_pluginClass;
} // namespace

static jstring StringToJavaString(JNIEnv* env, const std::string &string)
{
    std::u16string str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(string);
    return env->NewString(reinterpret_cast<const jchar *>(str.data()), str.length());
}

bool AbilityAccessCtrlJni::Register(void* env)
{
    LOGI("AbilityAccessCtrl JNI: Register");
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("AbilityAccessCtrl JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void AbilityAccessCtrlJni::NativeInit(JNIEnv* env, jobject jobj)
{
    LOGI("AbilityAccessCtrl JNI: NativeInit");
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);
    g_pluginClass.checkPermission = env->GetMethodID(cls, METHOD_CHECK_PERMISSION, SIGNATURE_CHECK_PERMISSION);
    g_pluginClass.requestPermissions = env->GetMethodID(cls, METHOD_REQUEST_PERMISSIONS, SIGNATURE_REQUEST_PERMISSIONS);
    CHECK_NULL_VOID(g_pluginClass.checkPermission);
    CHECK_NULL_VOID(g_pluginClass.requestPermissions);
    env->DeleteLocalRef(cls);
}

bool AbilityAccessCtrlJni::CheckPermission(const std::string& permission)
{
    LOGI("AbilityAccessCtrl JNI: Check");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if ((env == nullptr) || (g_pluginClass.globalRef == nullptr) || (g_pluginClass.checkPermission == nullptr)) {
        LOGW("AbilityAccessCtrl JNI get none ptr error");
        return false;
    }

    jstring jPermission = StringToJavaString(env, permission);
    bool isGranted = static_cast<bool>(env->CallBooleanMethod(
        g_pluginClass.globalRef, g_pluginClass.checkPermission, jPermission));
    if (env->ExceptionCheck()) {
        LOGE("AbilityAccessCtrl JNI: call Check failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return isGranted;
}

void AbilityAccessCtrlJni::RequestPermissions(const std::vector<std::string>& permissions)
{
    LOGI("AbilityAccessCtrl JNI: Request");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if ((env == nullptr) || (g_pluginClass.globalRef == nullptr) || (g_pluginClass.requestPermissions == nullptr)) {
        LOGW("AbilityAccessCtrl JNI get none ptr error");
        return;
    }
    jobjectArray array;
    jclass jcl = env->FindClass("java/lang/String");
    array = env->NewObjectArray(permissions.size(), jcl, NULL);
    for (size_t i = 0; i < permissions.size(); i++) {
        jstring jpermission = StringToJavaString(env, permissions[i]);
        env->SetObjectArrayElement(array, i, jpermission);
    }
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.requestPermissions, array);
    if (env->ExceptionCheck()) {
        LOGE("AbilityAccessCtrl JNI: call Request failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}
} // namespace OHOS::Plugin
