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

#include "plugins/test_plugin/android/java/jni/test_plugin_jni.h"

#include <jni.h>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

namespace OHOS::Plugin {
namespace {

const char TEST_PLUGIN_CLASS_NAME[] = "ohos/ace/plugin/testplugin/TestPlugin";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(TestPluginJni::NativeInit) },
};

const char METHOD_HELLO[] = "hello";

const char SIGNATURE_HELLO[] = "()V";

struct {
    jmethodID hello;
    jobject globalRef;
} g_pluginClass;

} // namespace

bool TestPluginJni::Register(void* env)
{
    LOGI("TestPlugin JNI: Register");
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(TEST_PLUGIN_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("TestPlugin JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void TestPluginJni::NativeInit(JNIEnv* env, jobject jobj)
{
    LOGI("TestPlugin JNI: NativeInit");
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);
    g_pluginClass.hello = env->GetMethodID(cls, METHOD_HELLO, SIGNATURE_HELLO);
    CHECK_NULL_VOID(g_pluginClass.hello);
    env->DeleteLocalRef(cls);
}

void TestPluginJni::Hello()
{
    LOGI("TestPlugin JNI: Hello");
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_VOID(env);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    CHECK_NULL_VOID(g_pluginClass.hello);
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.hello);
    if (env->ExceptionCheck()) {
        LOGE("Battery JNI: call Hello failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}

} // namespace OHOS::Plugin
