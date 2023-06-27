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
#ifndef PLUGINS_INTL_PLUGIN_ANDROID_JAVA_JNI_UTIL_PLUGIN_JNI_H
#define PLUGINS_INTL_PLUGIN_ANDROID_JAVA_JNI_UTIL_PLUGIN_JNI_H

#include <jni.h>
#include <memory>
#include <string>

namespace OHOS::Plugin {
class UtilPluginJni final {
public:
    UtilPluginJni() = delete;
    ~UtilPluginJni() = delete;
    static bool Register(void* env);
    // Called by Java
    static void NativeInit(JNIEnv* env, jobject jobj);
    // Called by C++
    static std::string EncodeIntoChinese(std::string input, std::string encoding);
    static std::string Decode(std::string input, std::string encoding);
};
} // namespace OHOS::Plugin
#endif // PLUGINS_INTL_PLUGIN_ANDROID_JAVA_JNI_UTIL_PLUGIN_JNI_H