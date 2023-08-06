/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "plugins/display/android/java/jni/display_jni.h"

#include <jni.h>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"
#include "plugins/display/android/java/jni/display_receiver.h"

namespace OHOS::Plugin {
namespace {
const char DISPLAY_CLASS_NAME[] = "ohos/ace/plugin/displayplugin/DisplayPlugin";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(DisplayJni::NativeInit) },
};

static const char METHOD_GET_DISPLAY_INFO[] = "getDisplayInfo";
static const char SIGNATURE_GET_DISPLAY_INFO[] = "()Ljava/lang/Object;";

struct {
    jmethodID getDisplayInfo;
    jobject globalRef;
} g_pluginClass;
} // namespace

bool DisplayJni::Register(void* env)
{
    LOGI("Display JNI: Register");
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(DISPLAY_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("Display JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void DisplayJni::NativeInit(JNIEnv* env, jobject jobj)
{
    LOGI("Display JNI: NativeInit");
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_pluginClass.getDisplayInfo = env->GetMethodID(cls, METHOD_GET_DISPLAY_INFO, SIGNATURE_GET_DISPLAY_INFO);
    CHECK_NULL_VOID(g_pluginClass.getDisplayInfo);

    env->DeleteLocalRef(cls);
}

void DisplayJni::GetDefaultDisplay(AsyncCallbackInfo* ptr)
{
    LOGI("Display JNI: GetDefaultDisplay");
    DisplayInfo info;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    CHECK_NULL_VOID(g_pluginClass.getDisplayInfo);

    jobject jResult = env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getDisplayInfo);
    if (env->ExceptionCheck()) {
        LOGE("Display JNI: call GetDefaultDisplay has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return;
    }

    jclass cls = env->GetObjectClass(jResult);
    CHECK_NULL_VOID(cls);

    DisplayJni displayJni;
    info.id = displayJni.GetId(env, cls, jResult);
    info.name = displayJni.GetName(env, cls, jResult);
    info.alive = displayJni.IsAlive(env, cls, jResult);
    info.state = (DisplayState)displayJni.GetState(env, cls, jResult);
    info.refreshRate = displayJni.GetRefreshRate(env, cls, jResult);
    info.rotation = displayJni.GetRotation(env, cls, jResult);
    info.width = displayJni.GetWidth(env, cls, jResult);
    info.height = displayJni.GetHeight(env, cls, jResult);
    info.densityDPI = displayJni.GetDensityDPI(env, cls, jResult);
    info.densityPixels = displayJni.GetDensityPixels(env, cls, jResult);
    info.scaledDensity = displayJni.GetScaledDensity(env, cls, jResult);
    info.xDPI = displayJni.GetXDPI(env, cls, jResult);
    info.yDPI = displayJni.GetYDPI(env, cls, jResult);
    DisplayReceiver::ReceiveFun(info, ptr);
}

jint DisplayJni::GetId(JNIEnv* env, jclass cls, jobject jobj)
{
    LOGI("Display JNI: GetId");
    jfieldID id = env->GetFieldID(cls, "id", "I");
    CHECK_NULL_RETURN(id, 0);
    return env->GetIntField(jobj, id);
}

std::string DisplayJni::GetName(JNIEnv* env, jclass cls, jobject jobj)
{
    LOGI("Display JNI: GetName");
    std::string result;
    jfieldID name = env->GetFieldID(cls, "name", "Ljava/lang/String;");
    CHECK_NULL_RETURN(name, result);
    jstring jResult = (jstring)env->GetObjectField(jobj, name);
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

jboolean DisplayJni::IsAlive(JNIEnv* env, jclass cls, jobject jobj)
{
    LOGI("Display JNI: IsAlive");
    jfieldID alive = env->GetFieldID(cls, "alive", "Z");
    CHECK_NULL_RETURN(alive, false);
    return env->GetBooleanField(jobj, alive);
}

jint DisplayJni::GetState(JNIEnv* env, jclass cls, jobject jobj)
{
    LOGI("Display JNI: GetState");
    jfieldID state = env->GetFieldID(cls, "state", "I");
    CHECK_NULL_RETURN(state, 0);
    return env->GetIntField(jobj, state);
}

jfloat DisplayJni::GetRefreshRate(JNIEnv* env, jclass cls, jobject jobj)
{
    LOGI("Display JNI: GetRefreshRate");
    jfieldID refreshRate = env->GetFieldID(cls, "refreshRate", "F");
    CHECK_NULL_RETURN(refreshRate, 0);
    return env->GetFloatField(jobj, refreshRate);
}

jint DisplayJni::GetRotation(JNIEnv* env, jclass cls, jobject jobj)
{
    LOGI("Display JNI: GetRotation");
    jfieldID rotation = env->GetFieldID(cls, "rotation", "I");
    CHECK_NULL_RETURN(rotation, 0);
    return env->GetIntField(jobj, rotation);
}

jint DisplayJni::GetWidth(JNIEnv* env, jclass cls, jobject jobj)
{
    LOGI("Display JNI: GetWidth");
    jfieldID width = env->GetFieldID(cls, "width", "I");
    CHECK_NULL_RETURN(width, 0);
    return env->GetIntField(jobj, width);
}

jint DisplayJni::GetHeight(JNIEnv* env, jclass cls, jobject jobj)
{
    LOGI("Display JNI: GetHeight");
    jfieldID height = env->GetFieldID(cls, "height", "I");
    CHECK_NULL_RETURN(height, 0);
    return env->GetIntField(jobj, height);
}

jint DisplayJni::GetDensityDPI(JNIEnv* env, jclass cls, jobject jobj)
{
    LOGI("Display JNI: GetDensityDPI");
    jfieldID densityDPI = env->GetFieldID(cls, "densityDPI", "I");
    CHECK_NULL_RETURN(densityDPI, 0);
    return env->GetIntField(jobj, densityDPI);
}

jfloat DisplayJni::GetDensityPixels(JNIEnv* env, jclass cls, jobject jobj)
{
    LOGI("Display JNI: GetDensityPixels");
    jfieldID densityPixels = env->GetFieldID(cls, "densityPixels", "F");
    CHECK_NULL_RETURN(densityPixels, 0);
    return env->GetFloatField(jobj, densityPixels);
}

jfloat DisplayJni::GetScaledDensity(JNIEnv* env, jclass cls, jobject jobj)
{
    LOGI("Display JNI: GetScaledDensity");
    jfieldID scaledDensity = env->GetFieldID(cls, "scaledDensity", "F");
    CHECK_NULL_RETURN(scaledDensity, 0);
    return env->GetFloatField(jobj, scaledDensity);
}

jfloat DisplayJni::GetXDPI(JNIEnv* env, jclass cls, jobject jobj)
{
    LOGI("Display JNI: GetXDPI");
    jfieldID xDPI = env->GetFieldID(cls, "xDPI", "F");
    CHECK_NULL_RETURN(xDPI, 0);
    return env->GetFloatField(jobj, xDPI);
}

jfloat DisplayJni::GetYDPI(JNIEnv* env, jclass cls, jobject jobj)
{
    LOGI("Display JNI: GetYDPI");
    jfieldID yDPI = env->GetFieldID(cls, "yDPI", "F");
    CHECK_NULL_RETURN(yDPI, 0);
    return env->GetFloatField(jobj, yDPI);
}
} // namespace OHOS::Plugin
