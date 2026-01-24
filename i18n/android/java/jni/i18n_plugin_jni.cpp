/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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

#include <codecvt>
#include <jni.h>
#include <locale>
#include <map>
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

struct {
    jmethodID is24HourClock;
    jmethodID getSystemLocale;
    jmethodID getSystemLanguage;
    jmethodID getSystemRegion;
    jmethodID getSystemTimezone;
    jmethodID getAppPreferredLanguage;
    jmethodID setAppPreferredLanguage;
    jmethodID getSystemCountries;
    jmethodID getSystemLanguages;
    jmethodID getPreferredLanguages;
    jmethodID getFirstPreferredLanguage;
    jmethodID getUsingLocalDigit;
    jmethodID isSuggested;
    jmethodID getAvailableIDs;
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

    static std::map<const char*, std::pair<jmethodID&, const char*>> jniFuncMap = {
        {"is24HourClock", {g_pluginClass.is24HourClock, "()Z"}},
        {"getSystemLocale", {g_pluginClass.getSystemLocale, "()Ljava/lang/String;"}},
        {"getSystemLanguage", {g_pluginClass.getSystemLanguage, "()Ljava/lang/String;"}},
        {"getSystemRegion", {g_pluginClass.getSystemRegion, "()Ljava/lang/String;"}},
        {"getSystemTimezone", {g_pluginClass.getSystemTimezone, "()Ljava/lang/String;"}},
        {"getAppPreferredLanguage", {g_pluginClass.getAppPreferredLanguage, "()Ljava/lang/String;"}},
        {"setAppPreferredLanguage", {g_pluginClass.setAppPreferredLanguage, "(Ljava/lang/String;)V"}},
        {"getSystemLanguages", {g_pluginClass.getSystemLanguages, "()[Ljava/lang/String;"}},
        {"getSystemCountries", {g_pluginClass.getSystemCountries, "(Ljava/lang/String;)[Ljava/lang/String;"}},
        {"getPreferredLanguageList", {g_pluginClass.getPreferredLanguages, "()[Ljava/lang/String;"}},
        {"getFirstPreferredLanguage", {g_pluginClass.getFirstPreferredLanguage, "()Ljava/lang/String;"}},
        {"getUsingLocalDigit", {g_pluginClass.getUsingLocalDigit, "()Z"}},
        {"isSuggested", {g_pluginClass.isSuggested, "(Ljava/lang/String;Ljava/lang/String;)Z"}},
        {"getAvailableIDs", {g_pluginClass.getAvailableIDs, "()[Ljava/lang/String;"}},
    };

    for (auto& jniInterface : jniFuncMap) {
        jniInterface.second.first = env->GetMethodID(cls, jniInterface.first, jniInterface.second.second);
        CHECK_NULL_VOID(jniInterface.second.first);
    }

    env->DeleteLocalRef(cls);
}

bool I18NPluginJni::Is24HourClock()
{
    bool result = true;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.is24HourClock)) {
        LOGE("I18NPluginJni get none ptr error");
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
        LOGE("I18NPluginJni get none ptr error");
        return "";
    }
    result = static_cast<jstring>(env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getSystemLocale));
    if (env->ExceptionCheck()) {
        LOGE("I18N JNI: call getSystemLocale failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    std::string locale = env->GetStringUTFChars(result, NULL);
    if (result != nullptr) {
        env->DeleteLocalRef(result);
    }
    return locale;
}

std::string I18NPluginJni::GetSystemLanguage()
{
    jstring result;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getSystemLanguage)) {
        LOGE("I18NPluginJni get none ptr error");
        return "";
    }
    result = static_cast<jstring>(env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getSystemLanguage));
    if (env->ExceptionCheck()) {
        LOGE("I18N JNI: call getSystemLanguage failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    std::string language = env->GetStringUTFChars(result, NULL);
    if (result != nullptr) {
        env->DeleteLocalRef(result);
    }
    return language;
}

std::string I18NPluginJni::GetSystemRegion()
{
    jstring result;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getSystemRegion)) {
        LOGE("I18NPluginJni get none ptr error");
        return "";
    }
    result = static_cast<jstring>(env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getSystemRegion));
    if (env->ExceptionCheck()) {
        LOGE("I18N JNI: call getSystemRegion failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    std::string region = env->GetStringUTFChars(result, NULL);
    if (result != nullptr) {
        env->DeleteLocalRef(result);
    }
    return region;
}

std::string I18NPluginJni::GetSystemTimezone()
{
    jstring result;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getSystemTimezone)) {
        LOGE("I18NPluginJni get none ptr error");
        return "";
    }
    result = static_cast<jstring>(env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getSystemTimezone));
    if (env->ExceptionCheck()) {
        LOGE("I18N JNI: call getSystemTimezone failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    std::string timezone = env->GetStringUTFChars(result, NULL);
    if (result != nullptr) {
        env->DeleteLocalRef(result);
    }
    return timezone;
}

std::string I18NPluginJni::GetAppPreferredLanguage()
{
    jstring result;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getAppPreferredLanguage)) {
        LOGE("getAppPreferredLanguage: I18NPluginJni get none ptr error");
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
    if (result != nullptr) {
        env->DeleteLocalRef(result);
    }
    return appPreferredLanguage;
}

void I18NPluginJni::SetAppPreferredLanguage(const std::string& languageTag)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.setAppPreferredLanguage)) {
        LOGE("SetAppPreferredLanguage: I18NPluginJni get none ptr error");
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

std::unordered_set<std::string> I18NPluginJni::GetSystemLanguages()
{
    std::unordered_set<std::string> result;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getSystemLanguages)) {
        LOGE("I18NPluginJni get none ptr error");
        return result;
    }
    jobjectArray sysLanguages = static_cast<jobjectArray>(
        env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getSystemLanguages));

    jsize arrayLength = env->GetArrayLength(sysLanguages);
    jstring jstr = nullptr;
    const char* str = nullptr;
    for (int32_t index = 0; index < arrayLength; index++) {
        jstr = static_cast<jstring>(env->GetObjectArrayElement(sysLanguages, index));
        str = env->GetStringUTFChars(jstr, NULL);
        if (str != nullptr) {
            result.insert(std::string(str));
            env->ReleaseStringUTFChars(jstr, str);
        }

        if (jstr != nullptr) {
            env->DeleteLocalRef(jstr);
        }
        str = nullptr;
    }
    env->DeleteLocalRef(sysLanguages);

    if (env->ExceptionCheck()) {
        LOGE("I18N JNI: call getSystemLanguages failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return result;
}

jstring I18NPluginJni::StringToJavaString(JNIEnv* env, const std::string& str)
{
    if (env == nullptr) {
        return nullptr;
    }
    if (str.empty()) {
        return env->NewStringUTF("");
    }
    std::u16string u16str = std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.from_bytes(str);
    return env->NewString(reinterpret_cast<const jchar*>(u16str.data()), u16str.length());
}

std::unordered_set<std::string> I18NPluginJni::GetSystemCountries(const std::string& language)
{
    std::unordered_set<std::string> result;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getSystemCountries)) {
        LOGE("I18NPluginJni get none ptr error");
        return result;
    }
    jstring jLanguage = StringToJavaString(env, language);
    jobjectArray countryCodes = static_cast<jobjectArray>(
        env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getSystemCountries, jLanguage));

    jsize arrayLength = env->GetArrayLength(countryCodes);
    jstring jstr = nullptr;
    const char* str = nullptr;
    for (int32_t index = 0; index < arrayLength; index++) {
        jstr = static_cast<jstring>(env->GetObjectArrayElement(countryCodes, index));
        str = env->GetStringUTFChars(jstr, NULL);
        if (str != nullptr) {
            result.insert(std::string(str));
            env->ReleaseStringUTFChars(jstr, str);
        }

        if (jstr != nullptr) {
            env->DeleteLocalRef(jstr);
        }
        str = nullptr;
    }
    env->DeleteLocalRef(jLanguage);
    env->DeleteLocalRef(countryCodes);

    if (env->ExceptionCheck()) {
        LOGE("I18N JNI: call getSystemCountries failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return result;
}

std::unordered_set<std::string> I18NPluginJni::GetAvailableIDs()
{
    std::unordered_set<std::string> result;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getAvailableIDs)) {
        LOGE("I18NPluginJni get none ptr error");
        return result;
    }
    jobjectArray availableIds = static_cast<jobjectArray>(
        env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getAvailableIDs));

    jsize arrayLength = env->GetArrayLength(availableIds);
    jstring jstr = nullptr;
    const char* str = nullptr;
    for (int32_t index = 0; index < arrayLength; index++) {
        jstr = static_cast<jstring>(env->GetObjectArrayElement(availableIds, index));
        str = env->GetStringUTFChars(jstr, NULL);
        if (str != nullptr) {
            result.insert(std::string(str));
            env->ReleaseStringUTFChars(jstr, str);
        }

        if (jstr != nullptr) {
            env->DeleteLocalRef(jstr);
        }
        str = nullptr;
    }
    env->DeleteLocalRef(availableIds);

    if (env->ExceptionCheck()) {
        LOGE("I18N JNI: call getAvailableIDs failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return result;
}

std::vector<std::string> I18NPluginJni::GetPreferredLanguages()
{
    std::vector<std::string> result;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getPreferredLanguages)) {
        LOGE("I18NPluginJni get none ptr error");
        return result;
    }
    jobjectArray sysLanguages = static_cast<jobjectArray>(
        env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getPreferredLanguages));

    jsize arrayLength = env->GetArrayLength(sysLanguages);
    jstring jstr = nullptr;
    const char* str = nullptr;
    for (int32_t index = 0; index < arrayLength; index++) {
        jstr = static_cast<jstring>(env->GetObjectArrayElement(sysLanguages, index));
        str = env->GetStringUTFChars(jstr, NULL);
        if (str != nullptr) {
            result.push_back(std::string(str));
            env->ReleaseStringUTFChars(jstr, str);
        }

        if (jstr != nullptr) {
            env->DeleteLocalRef(jstr);
        }
        str = nullptr;
    }
    env->DeleteLocalRef(sysLanguages);

    if (env->ExceptionCheck()) {
        LOGE("I18N JNI: call GetPreferredLanguages failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return result;
}

std::string I18NPluginJni::GetFirstPreferredLanguage()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getFirstPreferredLanguage)) {
        LOGE("I18NPluginJni get none ptr error");
        return std::string {""};
    }

    jstring language = static_cast<jstring>(
        env->CallObjectMethod(g_pluginClass.globalRef, g_pluginClass.getFirstPreferredLanguage));
        
    std::string locale = env->GetStringUTFChars(language, NULL);
    if (env->ExceptionCheck()) {
        LOGE("I18N JNI: call getFirstPreferredLanguage failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    if (language != nullptr) {
        env->DeleteLocalRef(language);
    }
    return locale;
}

bool I18NPluginJni::GetUsingLocalDigit()
{
    bool result = false;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.getUsingLocalDigit)) {
        LOGE("I18NPluginJni get none ptr error");
        return result;
    }
    result = static_cast<bool>(env->CallBooleanMethod(g_pluginClass.globalRef, g_pluginClass.getUsingLocalDigit));
    if (env->ExceptionCheck()) {
        LOGE("I18N JNI: call getUsingLocalDigit failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return result;
}

bool I18NPluginJni::IsSuggested(const std::string& language, const std::string& region)
{
    bool result = false;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    if (!(env) || !(g_pluginClass.globalRef) || !(g_pluginClass.isSuggested)) {
        LOGE("I18NPluginJni get none ptr error");
        return result;
    }
    jstring jLanguage = StringToJavaString(env, language);
    jstring jRegion = StringToJavaString(env, region);
    result = static_cast<bool>(env->CallBooleanMethod(
        g_pluginClass.globalRef, g_pluginClass.isSuggested, jLanguage, jRegion));
    env->DeleteLocalRef(jLanguage);
    env->DeleteLocalRef(jRegion);
    if (env->ExceptionCheck()) {
        LOGE("I18N JNI: call isSuggested failed");
        env->ExceptionDescribe();
        env->ExceptionClear();
    }
    return result;
}
} // namespace OHOS::Plugin
