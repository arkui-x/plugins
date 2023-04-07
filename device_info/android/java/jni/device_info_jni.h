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

#ifndef PLUGINS_ANDROID_JAVA_JNI_DEVICE_INFO_JNI_H
#define PLUGINS_ANDROID_JAVA_JNI_DEVICE_INFO_JNI_H

#include <iostream>
#include <jni.h>

#include "plugins/device_info/device_info.h"

namespace OHOS::Plugin {
class DeviceInfoJni final {
public:
    DeviceInfoJni() = default;
    ~DeviceInfoJni() = default;
    static bool Register(void* env);
    // Called by Java
    static void NativeInit(JNIEnv* env, jobject jobj);
    // Called by C++
    static int CallIntMethod(int id, int defValue);
    static std::string CallMethod(int id, const std::string &defValue);
};
} // namespace OHOS::Plugin
#endif // init_common_service.c
