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

#include "plugins/device_info/android/java/jni/device_info_jni.h"

#include <jni.h>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

namespace OHOS::Plugin {
namespace {
const char DEVICEINFO_CLASS_NAME[] = "ohos/ace/plugin/device_infoplugin/DeviceInfoPlugin";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(DeviceInfoJni::NativeInit) },
};

struct {
    jmethodID methodIDs[MAX_METHOD_ID];
    jobject globalRef;
} g_pluginClass;
} // namespace

bool DeviceInfoJni::Register(void* env)
{
    LOGI("DeviceInfoJni JNI: Register");
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(DEVICEINFO_CLASS_NAME);
    if (jniEnv->ExceptionCheck()) {
        LOGE("DeviceInfoJni JNI: call Register has exception %{public}s", DEVICEINFO_CLASS_NAME);
        jniEnv->ExceptionDescribe();
        jniEnv->ExceptionClear();
        return false;
    }
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("DeviceInfoJni JNI: RegisterNatives fail.");
        return false;
    }
    LOGI("DeviceInfoJni JNI: Register success");
    return true;
}

void DeviceInfoJni::NativeInit(JNIEnv* env, jobject jobj)
{
    LOGI("DeviceInfoJni JNI: NativeInit");
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    if (env->ExceptionCheck()) {
        LOGW("DeviceInfoJni JNI: NewGlobalRef");
        env->ExceptionClear();
    }
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    for (size_t i = 0; i < METHOD_ID_getMajorVersion; i++) {
        g_pluginClass.methodIDs[i] = env->GetMethodID(cls,
            METHOD_NAMES[i].methodName, "(Ljava/lang/String;)Ljava/lang/String;");
        if (env->ExceptionCheck()) {
            LOGW("DeviceInfoJni JNI: no method for %{public}s id %{public}d", METHOD_NAMES[i].methodName, i);
            env->ExceptionClear();
        }
    }
    for (size_t i = METHOD_ID_getMajorVersion; i < MAX_METHOD_ID; i++) {
        g_pluginClass.methodIDs[i] = env->GetMethodID(cls, METHOD_NAMES[i].methodName, "(I)I");
        if (env->ExceptionCheck()) {
            LOGW("DeviceInfoJni JNI: no method for %{public}s id %{public}d", METHOD_NAMES[i].methodName, i);
            env->ExceptionClear();
        }
    }
    env->DeleteLocalRef(cls);
}

std::string DeviceInfoJni::CallMethod(int id, const std::string &defValue)
{
    if (id >= MAX_METHOD_ID) {
        LOGI("DeviceInfoJni JNI: CallMethod invalid method id %{public}d", id);
        return defValue;
    }
    LOGD("DeviceInfoJni JNI: CallMethod %{public}s", METHOD_NAMES[id].methodName);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, defValue);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, defValue);
    if (g_pluginClass.methodIDs[id] == nullptr) {
        LOGE("DeviceInfoJni JNI: no method for %s id %d", METHOD_NAMES[id].methodName, id);
        return defValue;
    }

    jstring jsDef = env->NewStringUTF(defValue.c_str());
    jstring jResult = static_cast<jstring>(env->CallObjectMethod(
        g_pluginClass.globalRef, g_pluginClass.methodIDs[id], jsDef));
    if (env->ExceptionCheck()) {
        LOGE("DeviceInfoJni JNI: call GetDeviceInfo has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return defValue;
    }

    std::string result;
    const char* content = env->GetStringUTFChars(jResult, nullptr);
    if (content != nullptr) {
        result.assign(content);
        env->ReleaseStringUTFChars(jResult, content);
    }
    if (jResult != nullptr) {
        env->DeleteLocalRef(jResult);
    }
    return result;
}

int DeviceInfoJni::CallIntMethod(int id, int defValue)
{
    if (id >= MAX_METHOD_ID || id < METHOD_ID_getMajorVersion) {
        LOGI("DeviceInfoJni JNI: CallMethod invalid method id %{public}d", id);
        return defValue;
    }
    LOGD("DeviceInfoJni JNI: CallMethod %{public}s", METHOD_NAMES[id].methodName);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, defValue);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, defValue);
    if (g_pluginClass.methodIDs[id] == nullptr) {
        LOGE("DeviceInfoJni JNI: no method for %s id %d", METHOD_NAMES[id].methodName, id);
        return defValue;
    }
    return env->CallIntMethod(g_pluginClass.globalRef, g_pluginClass.methodIDs[id], defValue);
}
} // namespace OHOS::Plugin
