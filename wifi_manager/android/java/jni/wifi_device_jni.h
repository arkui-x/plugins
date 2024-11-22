/*
 * Copyright (C) 2024 Huawei Device Co., Ltd.
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

#ifndef PLUGINS_WIFI_MANAGER_ANDROID_JAVA_JNI_WIFI_DEVICE_JNI_H
#define PLUGINS_WIFI_MANAGER_ANDROID_JAVA_JNI_WIFI_DEVICE_JNI_H

#include <jni.h>
#include <string>

#include "wifi_device_impl.h"

#include "plugins/wifi_manager/wifi_errcode.h"
#include "plugins/wifi_manager/wifi_msg.h"

namespace OHOS::Plugin {
class WiFiDeviceJni final {
public:
    WiFiDeviceJni() = default;
    ~WiFiDeviceJni() = default;

    static WiFiDeviceJni& GetInstance();
    static bool Register(void* env);
    void RegisterWiFiManager();
    // Called by Java
    static void NativeInit(JNIEnv* env, jobject jobj);
    static void NativeReceiveCallback(JNIEnv* env, jobject jobj, jstring key, jint code);
    // Called by C++
    ErrCode GetLinkedInfo(WifiLinkedInfo& info);
    ErrCode IsWifiActive(bool& bActive);
    ErrCode IsConnected(bool& isConnected);
    ErrCode On(const std::string& key);
    ErrCode Off(const std::string& key);

private:
    void JavaWifiLinkedInfoToOh(const std::string& event, WifiLinkedInfo& info);
};
} // namespace OHOS::Plugin

#endif
