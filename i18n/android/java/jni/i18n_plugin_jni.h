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

#ifndef PLUGINS_I18N_PLUGIN_ANDROID_JAVA_JNI_I18N_PLUGIN_JNI_H
#define PLUGINS_I18N_PLUGIN_ANDROID_JAVA_JNI_I18N_PLUGIN_JNI_H

#include <jni.h>
#include <memory>

namespace OHOS::Plugin {
class I18NPluginJni final {
public:
    I18NPluginJni() = delete;
    ~I18NPluginJni() = delete;
    static bool Register(void* env);
    // Called by Java
    static void NativeInit(JNIEnv* env, jobject jobj);
    // Called by C++
    static bool Is24HourClock();
    static std::string GetSystemLocale();
    static std::string GetSystemLanguage();
    static std::string GetSystemRegion();
    static std::string GetSystemTimezone();
    static std::string GetAppPreferredLanguage();
    static void SetAppPreferredLanguage(const std::string& languageTag);
};
} // namespace OHOS::Plugin
#endif // PLUGINS_I18N_PLUGIN_ANDROID_JAVA_JNI_I18N_PLUGIN_JNI_H
