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

#include "plugins/i18n/android/java/jni/i18n_plugin_jni.h"

#include <jni.h>
#include <string>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"

namespace OHOS::Plugin {
namespace {
const char I18N_PLUGIN_CLASS_NAME[] = "ohos/ace/plugin/i18nplugin/I18NPlugin";

static const JNINativeMethod METHODS[] = {
    { "nativeInit", "()V", reinterpret_cast<void*>(I18NPluginJni::NativeInit) },
};

const char METHOD_IS24HOUR_CLOCK[] = "is24HourClock";
const char METHOD_GET_SYSTEM_LOCALE[] = "getSystemLocale";
const char METHOD_GET_SYSTEM_LANGUAGE[] = "getSystemLanguage";
const char METHOD_GET_SYSTEM_REGION[] = "getSystemRegion";
const char METHOD_GET_SYSTEM_TIMEZONE[] = "getSystemTimezone";
const char METHOD_GET_APP_PREFERRED_LANGUAGE[] = "getAppPreferredLanguage";
const char METHOD_SET_APP_PREFERRED_LANGUAGE[] = "setAppPreferredLanguage";

const char SIGNATURE_IS24HOUR_CLOCK[] = "()Z";
const char SIGNATURE_GET_SYSTEM_LOCALE[] = "()Ljava/lang/String;";
const char SIGNATURE_GET_SYSTEM_LANGUAGE[] = "()Ljava/lang/String;";
const char SIGNATURE_GET_SYSTEM_REGION[] = "()Ljava/lang/String;";
const char SIGNATURE_GET_SYSTEM_TIMEZONE[] = "()Ljava/lang/String;";
const char SIGNATURE_GET_APP_PREFERRED_LANGUAGE[] = "()Ljava/lang/String;";
const char SIGNATURE_SET_APP_PREFERRED_LANGUAGE[] = "(Ljava/lang/String;)V";

struct {
    jmethodID is24HourClock;
    jmethodID getSystemLocale;
    jmethodID getSystemLanguage;
    jmethodID getSystemRegion;
    jmethodID getSystemTimezone;
    jmethodID getAppPreferredLanguage;
    jmethodID setAppPreferredLanguage;
    jobject globalRef;
} g_pluginClass;
} // namespace

bool I18NPluginJni::Register(void* env)
{
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(I18N_PLUGIN_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("I18NPlugin JNI: RegisterNatives fail.");
        return false;
    }
    return true;
}

void I18NPluginJni::NativeInit(JNIEnv* env, jobject jobj)
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

    g_pluginClass.getSystemLanguage = env->GetMethodID(cls, METHOD_GET_SYSTEM_LANGUAGE, SIGNATURE_GET_SYSTEM_LANGUAGE);
    CHECK_NULL_VOID(g_pluginClass.getSystemLanguage);

    g_pluginClass.getSystemRegion = env->GetMethodID(cls, METHOD_GET_SYSTEM_REGION, SIGNATURE_GET_SYSTEM_REGION);
    CHECK_NULL_VOID(g_pluginClass.getSystemRegion);

    g_pluginClass.getSystemTimezone = env->GetMethodID(cls, METHOD_GET_SYSTEM_TIMEZONE, SIGNATURE_GET_SYSTEM_TIMEZONE);
    CHECK_NULL_VOID(g_pluginClass.getSystemTimezone);

    g_pluginClass.getAppPreferredLanguage =
        env->GetMethodID(cls, METHOD_GET_APP_PREFERRED_LANGUAGE, SIGNATURE_GET_APP_PREFERRED_LANGUAGE);
    CHECK_NULL_VOID(g_pluginClass.getAppPreferredLanguage);

    g_pluginClass.setAppPreferredLanguage =
        env->GetMethodID(cls, METHOD_SET_APP_PREFERRED_LANGUAGE, SIGNATURE_SET_APP_PREFERRED_LANGUAGE);
    CHECK_NULL_VOID(g_pluginClass.getAppPreferredLanguage);

    env->DeleteLocalRef(cls);
}

bool I18NPluginJni::Is24HourClock()
{
    bool result = true;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.is24HourClock)) {
        LOGW("I18NPluginJni get none ptr error");
        return result;
    }
    result = static_cast<bool>(env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.is24HourClock));
    if (env->ExceptionCheck()) {
        LOGE("I18N JNI: call is24HourClock failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return result;
}

std::string I18NPluginJni::GetSystemLocale()
{
    jstring result;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getSystemLocale)) {
        LOGW("I18NPluginJni get none ptr error");
        return "";
    }
    result = static_cast<jstring>(env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getSystemLocale));
    if (env->ExceptionCheck()) {
        LOGE("I18N JNI: call getSystemLocale failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    std::string locale = env->GetStringUTFChars(result, NULL);
    return locale;
}

std::string I18NPluginJni::GetSystemLanguage()
{
    jstring result;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getSystemLanguage)) {
        LOGW("I18NPluginJni get none ptr error");
        return "";
    }
    result = static_cast<jstring>(env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getSystemLanguage));
    if (env->ExceptionCheck()) {
        LOGE("I18N JNI: call getSystemLanguage failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    std::string language = env->GetStringUTFChars(result, NULL);
    return language;
}

std::string I18NPluginJni::GetSystemRegion()
{
    jstring result;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getSystemRegion)) {
        LOGW("I18NPluginJni get none ptr error");
        return "";
    }
    result = static_cast<jstring>(env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getSystemRegion));
    if (env->ExceptionCheck()) {
        LOGE("I18N JNI: call getSystemRegion failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    std::string region = env->GetStringUTFChars(result, NULL);
    return region;
}

std::string I18NPluginJni::GetSystemTimezone()
{
    jstring result;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getSystemTimezone)) {
        LOGW("I18NPluginJni get none ptr error");
        return "";
    }
    result = static_cast<jstring>(env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getSystemTimezone));
    if (env->ExceptionCheck()) {
        LOGE("I18N JNI: call getSystemTimezone failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    std::string timezone = env->GetStringUTFChars(result, NULL);
    return timezone;
}

std::string I18NPluginJni::GetAppPreferredLanguage()
{
    jstring result;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getAppPreferredLanguage)) {
        LOGW("getAppPreferredLanguage: I18NPluginJni get none ptr error");
        return "";
    }
    result = static_cast<jstring>(
        env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getAppPreferredLanguage));
    if (env->ExceptionCheck()) {
        LOGE("getAppPreferredLanguage: I18N JNI: call getAppPreferredLanguage failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    std::string appPreferredLanguage = env->GetStringUTFChars(result, NULL);
    return appPreferredLanguage;
}

void I18NPluginJni::SetAppPreferredLanguage(const std::string& languageTag)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.setAppPreferredLanguage)) {
        LOGW("SetAppPreferredLanguage: I18NPluginJni get none ptr error");
        return;
    }

    jstring jLanguageTag = env->NewStringUTF(languageTag.c_str());
    env->CallVoidMethod(g_pluginClass.globalRef, g_pluginClass.setAppPreferredLanguage, jLanguageTag);
    env->DeleteLocalRef(jLanguageTag);
    if (env->ExceptionCheck()) {
        LOGE("I18N JNI: call setAppPreferredLanguage failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
}
} // namespace OHOS::Plugin
