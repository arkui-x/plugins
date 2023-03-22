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

#ifndef PLUGINS_BROADCAST_ANDROID_JAVA_JNI_BROADCAST_JNI_H
#define PLUGINS_BROADCAST_ANDROID_JAVA_JNI_BROADCAST_JNI_H

#include <jni.h>
#include <memory>
#include <map>
#include <string>
#include <vector>

#include "inner_utils/plugin_inner_napi_utils.h"
#include "plugin_c_utils.h"

namespace OHOS::Plugin {
static std::map<std::string, std::string> g_eventMap;
class BroadcastJni final {
public:
    BroadcastJni() = delete;
    ~BroadcastJni() = delete;
    static bool Register(void* env);
    // Called by Java
    static void NativeInit(JNIEnv* env, jobject jobj);
    static void NativeReceiveBroadcast(JNIEnv* env, jobject jobj, jstring action, jstring json);
    // Called by C++
    static void SendBroadcast(std::string action, std::string json, AsyncCallbackInfo* ptr);
    static void RegisterBroadcast(std::string key, std::vector<std::string> actions);
    static void UnRegisterBroadcast(std::string key, AsyncCallbackInfo* ptr);
};
}  // namespace OHOS::Plugin

#endif  // PLUGINS_BROADCAST_ANDROID_JAVA_JNI_BROADCAST_JNI_H
