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
#ifndef PLUGINS_NOTIFICATION_MANAGER_ANDROID_JAVA_JNI__H
#define PLUGINS_NOTIFICATION_MANAGER_ANDROID_JAVA_JNI__H
#include <jni.h>
#include <string>

#include "plugins/notification_manager/notification_request.h"

namespace OHOS {
namespace Notification {
class NotificationJni final {
public:
    NotificationJni() = default;
    ~NotificationJni() = default;

    static NotificationJni& GetInstance();
    static bool Register(void* env);
    void RegisterWiFiManager();
    // Called by Java
    static void NativeInit(JNIEnv* env, jobject jobj);
    static void NativeReceiveCallback(JNIEnv* env, jobject jobj, jstring key, jint code);

    // Called by C++
    ErrorCode publish(const NotificationRequest& info);
    ErrorCode cancel(int32_t id, std::string label);
    ErrorCode cancelAll();
    ErrorCode setBadgeNumber(int32_t number);
    ErrorCode RequestEnableNotification();
    bool isHighVersion();
    bool isGranted();

private:
    // void JavaWifiLinkedInfoToOh(const std::string& event, WifiLinkedInfo& info);
};
} // namespace Notification
} // namespace OHOS
#endif
