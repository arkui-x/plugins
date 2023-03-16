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

#ifndef PLUGINS_ABILITY_ACCESS_CONTROL_ANDROID_JAVA_JNI_H
#define PLUGINS_ABILITY_ACCESS_CONTROL_ANDROID_JAVA_JNI_H

#include <jni.h>
#include <map>
#include <memory>
#include <vector>
#include "plugin_c_utils.h"

namespace OHOS::Plugin {
class AbilityAccessCtrlJni final {
public:
    AbilityAccessCtrlJni() = delete;
    ~AbilityAccessCtrlJni() = delete;
    static bool Register(void* env);
    // Called by Java
    static void NativeInit(JNIEnv* env, jobject jobj);
    // Called by C++
    static bool CheckPermission(const std::string& permission);
    static void RequestPermissions(const std::vector<std::string> permissions);
};
} // namespace OHOS::Plugin

#endif // PLUGINS_ABILITY_ACCESS_CONTROL_ANDROID_JAVA_JNI_H
