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

#ifndef PLUGINS_NET_CONNECTION_JNI_NET_CONN_CLIENT_JNI_H
#define PLUGINS_NET_CONNECTION_JNI_NET_CONN_CLIENT_JNI_H

#include <jni.h>
#include <memory>
#include <map>

#include "plugins/net/connection/net_conn_client.h"
#include "net_conn_callback_observer.h"

namespace OHOS::Plugin {
class NetConnClientJni final {
public:
    NetConnClientJni() = delete;
    ~NetConnClientJni() = delete;
    static bool Register(void *env);
    // Called by Java
    static void NativeInit(JNIEnv *env, jobject jobj);
    static void NativeOnAvailable(JNIEnv *env, jobject jthiz, jlong jcallbackKey, jint networkId);
    static void NativeOnCapabilitiesChanged(JNIEnv *env, jobject jthiz, jobject jNetworkCapabilities,
        jlong jcallbackKey, jint networkId);
    static void NativeOnLost(JNIEnv *env, jobject jthiz, jlong jcallbackKey, jint networkId);
    static void NativeOnUnavailable(JNIEnv *env, jobject jthiz, jlong jcallbackKey);

    // Called by C++
    static int32_t RegisterDefaultNetConnCallback(const sptr<NetManagerStandard::INetConnCallback> &callback);
    static int32_t RegisterNetConnCallback(const sptr<NetManagerStandard::NetSpecifier> &netSpecifier,
        const sptr<NetManagerStandard::INetConnCallback> &callback, const uint32_t &timeoutMS);
    static int32_t UnregisterNetConnCallback(const sptr<NetManagerStandard::INetConnCallback> &callback);
    static int32_t HasDefaultNet(bool &flag);

private:
    static void RegisterCallback(const sptr<NetManagerStandard::INetConnCallback> &callback);
    static void UnregisterCallback(const sptr<NetManagerStandard::INetConnCallback> &callback);
    static void NetConnClientJniRegister();

    static std::map<long, sptr<NetManagerStandard::INetConnCallback>> observer_;
    static bool hasInit_;
};
} // namespace OHOS::Plugin
#endif // PLUGINS_NET_CONNECTION_JNI_NET_CONN_CLIENT_JNI_H