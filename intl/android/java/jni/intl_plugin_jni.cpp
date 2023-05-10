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

#include "plugins/intl/android/java/jni/intl_plugin_jni.h"

#include <jni.h>
#include <string>

#include "log.h"
#include "plugin_c_utils.h"
#include "plugin_utils.h"

namespace OHOS::Plugin {
namespace {
const char INTL_PLUGIN_CLASS_NAME[] = "ohos/ace/plugin/intlplugin/INTLPlugin";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(INTLPluginJni::NativeInit) },
};

const char METHOD_IS24HOUR_CLOCK[] = "is24HourClock";
const char METHOD_GET_SYSTEM_LOCALE[] = "getSystemLocale";
const char METHOD_GET_SYSTEM_TIMEZONE[] = "getSystemTimezone";

const char SIGNATURE_IS24HOUR_CLOCK[] = "()Z";
const char SIGNATURE_GET_SYSTEM_LOCALE[] = "()Ljava/lang/String;";
const char SIGNATURE_GET_SYSTEM_TIMEZONE[] = "()Ljava/lang/String;";

struct {
    jmethodID is24HourClock;
    jmethodID getSystemLocale;
    jmethodID getSystemTimezone;
    jobject globalRef;
} g_pluginClass;
} // namespace

bool INTLPluginJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(INTL_PLUGIN_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("INTLPlugin JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void INTLPluginJni::NativeInit(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    g_pluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_pluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_pluginClass.is24HourClock = env->GetMethodID(cls, METHOD_IS24HOUR_CLOCK, SIGNATURE_IS24HOUR_CLOCK);
    CHECK_NULL_VOID(g_pluginClass.is24HourClock);

    g_pluginClass.getSystemLocale = env->GetMethodID(cls, METHOD_GET_SYSTEM_LOCALE, SIGNATURE_GET_SYSTEM_LOCALE);
    CHECK_NULL_VOID(g_pluginClass.getSystemLocale);

    g_pluginClass.getSystemTimezone = env->GetMethodID(cls, METHOD_GET_SYSTEM_TIMEZONE, SIGNATURE_GET_SYSTEM_TIMEZONE);
    CHECK_NULL_VOID(g_pluginClass.getSystemTimezone);

    env->DeleteLocalRef(cls);
}

bool INTLPluginJni::Is24HourClock()
{
    bool result = true;
    auto env = OH_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.is24HourClock)) {
        LOGW("INTLPluginJni get none ptr error");
        return result;
    }
    result = static_cast<bool>(env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.is24HourClock));
    if (env->ExceptionCheck()) {
        LOGE("INTL JNI: call is24HourClock failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return result;
}

std::string INTLPluginJni::GetSystemLocale()
{
    jstring result;
    auto env = OH_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getSystemLocale)) {
        LOGW("INTLPluginJni get none ptr error");
        return "";
    }
    result = static_cast<jstring>(env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getSystemLocale));
    if (env->ExceptionCheck()) {
        LOGE("INTL JNI: call getSystemLocale failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    std::string locale = env->GetStringUTFChars(result, NULL);
    return locale;
}

std::string INTLPluginJni::GetSystemTimezone()
{
    jstring result;
    auto env = OH_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getSystemTimezone)) {
        LOGW("INTLPluginJni get none ptr error");
        return "";
    }
    result = static_cast<jstring>(env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getSystemTimezone));
    if (env->ExceptionCheck()) {
        LOGE("INTL JNI: call getSystemTimezone failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    std::string timezone = env->GetStringUTFChars(result, NULL);
    return timezone;
}
} // namespace OHOS::Plugin
