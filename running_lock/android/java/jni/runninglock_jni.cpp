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

#include "plugins/running_lock/android/java/jni/runninglock_jni.h"

#include <codecvt>
#include <jni.h>
#include <locale>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"
#include "plugins/running_lock/android/java/jni/runninglock_receiver.h"

namespace OHOS::Plugin {
namespace {
const char RUNNINGLOCK_CLASS_NAME[] = "ohos/ace/plugin/runninglockplugin/RunningLockPlugin";

static const JNINativeMethod METHODS[] = {
    {"nativeInit", "()V", reinterpret_cast<void *>(RunningLockJni::NativeInit)},
};

static const char METHOD_INIT[] = "init";
static const char METHOD_IS_USED[] = "isUsed";
static const char METHOD_LOCK[] = "lock";
static const char METHOD_UNLOCK[] = "unLock";

static const char SIGNATURE_INIT[] = "(Ljava/lang/String;I)Z";
static const char SIGNATURE_IS_USED[] = "()Z";
static const char SIGNATURE_LOCK[] = "(J)Z";
static const char SIGNATURE_UNLOCK[] = "()Z";

struct {
    jmethodID init;
    jmethodID isUsed;
    jmethodID lock;
    jmethodID unLock;
    jobject globalRef;
} g_pluginClass;
}  // namespace

static jstring StringToJavaString(JNIEnv* env, const std::string &string)
{
    std::u16string str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(string);
    return env->NewString(reinterpret_cast<const jchar *>(str.data()), str.length());
}

bool RunningLockJni::Register(void *env)
{
    LOGI("RunningLock JNI: Register");
    auto *jniEnv = static_cast<JNIEnv *>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(RUNNINGLOCK_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("RunningLock JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void RunningLockJni::NativeInit(JNIEnv *env, jobject jobj)
{
    LOGI("RunningLock JNI: NativeInit");
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_pluginClass.init = env->GetMethodID(cls, METHOD_INIT, SIGNATURE_INIT);
    CHECK_NULL_VOID(g_pluginClass.init);

    g_pluginClass.isUsed = env->GetMethodID(cls, METHOD_IS_USED, SIGNATURE_IS_USED);
    CHECK_NULL_VOID(g_pluginClass.isUsed);

    g_pluginClass.lock = env->GetMethodID(cls, METHOD_LOCK, SIGNATURE_LOCK);
    CHECK_NULL_VOID(g_pluginClass.lock);

    g_pluginClass.unLock = env->GetMethodID(cls, METHOD_UNLOCK, SIGNATURE_UNLOCK);
    CHECK_NULL_VOID(g_pluginClass.unLock);

    env->DeleteLocalRef(cls);
}

void RunningLockJni::Init(const std::string &name, RunningLockType type, RunningLockAsyncCallbackInfo *ptr)
{
    LOGI("RunningLockJni JNI: Init");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    CHECK_NULL_VOID(g_pluginClass.init);
    jstring jname = StringToJavaString(env, name);
    env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.init, jname, static_cast<int>(type));
    if (env->ExceptionCheck()) {
        LOGE("RunningLockJni JNI: call Init has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    RunningLockReceiver::ReceiveCallBack(ptr);
}

bool RunningLockJni::IsUsed()
{
    LOGI("RunningLockJni JNI: IsUsed");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, false);
    CHECK_NULL_RETURN(g_pluginClass.globalRef, false);
    CHECK_NULL_RETURN(g_pluginClass.isUsed, false);

    bool jResult = env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.isUsed);
    if (env->ExceptionCheck()) {
        LOGE("RunningLockJni JNI: call isUsed has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return false;
    }
    return jResult;
}

void RunningLockJni::Lock(uint32_t timeOutMs)
{
    LOGI("RunningLockJni JNI: Lock");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    CHECK_NULL_VOID(g_pluginClass.lock);

    env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.lock, timeOutMs);
    if (env->ExceptionCheck()) {
        LOGE("RunningLockJni JNI: call Lock has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

void RunningLockJni::UnLock()
{
    LOGI("RunningLockJni JNI: UnLock");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    CHECK_NULL_VOID(g_pluginClass.unLock);

    env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.unLock);
    if (env->ExceptionCheck()) {
        LOGE("RunningLockJni JNI: call UnLock has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}
}  // namespace OHOS::Plugin
