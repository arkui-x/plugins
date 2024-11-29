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

#include "plugins/net/connection/android/java/jni/net_conn_client_jni.h"

#include <codecvt>
#include <jni.h>
#include <locale>
#include <securec.h>

#include "inner_api/plugin_utils_inner.h"
#include "log.h"
#include "plugin_utils.h"
#include "net_handle.h"
#include "route.h"
#include "net_manager_constants.h"

namespace OHOS::Plugin {
namespace {
const char NetConnClient_CLASS_NAME[] = "ohos/ace/plugin/netconnclientplugin/NetConnClientPlugin";
static const JNINativeMethod METHODS[] = {
    {"nativeInit", "()V", reinterpret_cast<void *>(NetConnClientJni::NativeInit)},
    {"nativeOnAvailable", "(JI)V", reinterpret_cast<void *>(NetConnClientJni::NativeOnAvailable)},
    {"nativeOnCapabilitiesChanged", "(Landroid/net/NetworkCapabilities;JI)V",
        reinterpret_cast<void *>(NetConnClientJni::NativeOnCapabilitiesChanged)},
    {"nativeOnLost", "(JI)V", reinterpret_cast<void *>(NetConnClientJni::NativeOnLost)},
    {"nativeOnUnavailable", "(J)V", reinterpret_cast<void *>(NetConnClientJni::NativeOnUnavailable)}
};

static const char METHOD_REGISTE_DEFALT_NET_CONN_CALLBACK[] = "registerDefaultNetConnCallback";
static const char METHOD_REGISTER_NET_CONN_CALLBACK[] = "registerNetConnCallback";
static const char METHOD_UNREGISTER_NET_CONN_CALLBACK[] = "unregisterNetConnCallback";
static const char METHOD_IS_DEFAULT_NETWORK_ACTIVE[] = "isDefaultNetworkActive";

static const char SIGNATURE_REGISTE_DEFALT_NET_CONN_CALLBACK[] = "(J)V";
static const char SIGNATURE_REGISTER_NET_CONN_CALLBACK[] = "(J[I)V";
static const char SIGNATURE_UNREGISTER_NET_CONN_CALLBACK[] = "(J)V";
static const char SIGNATURE_IS_DEFAULT_NETWORK_ACTIVE[] = "()Z";

struct {
    jmethodID registerDefaultNetConnCallback;
    jmethodID registerNetConnCallback;
    jmethodID unregisterNetConnCallback;
    jmethodID isDefaultNetworkActive;
    jobject globalRef;
} g_netconnclientpluginClass;
}  // namespace
std::map<long, sptr<NetManagerStandard::INetConnCallback>> NetConnClientJni::observer_;
bool NetConnClientJni::hasInit_ = false;

bool NetConnClientJni::Register(void *env)
{
    LOGI("NetConnClientJni::Register");
    if (hasInit_) {
        return hasInit_;
    }
    auto *jniEnv = static_cast<JNIEnv *>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(NetConnClient_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        LOGE("NetConnClientJni JNI: RegisterNatives fail.");
        return false;
    }

    return true;
}

void NetConnClientJni::NativeInit(JNIEnv *env, jobject jobj)
{
    LOGI("NetConnClientJni::NativeInit");
    CHECK_NULL_VOID(env);
    g_netconnclientpluginClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_netconnclientpluginClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_netconnclientpluginClass.registerDefaultNetConnCallback = env->GetMethodID(
        cls, METHOD_REGISTE_DEFALT_NET_CONN_CALLBACK, SIGNATURE_REGISTE_DEFALT_NET_CONN_CALLBACK);
    CHECK_NULL_VOID(g_netconnclientpluginClass.registerDefaultNetConnCallback);

    g_netconnclientpluginClass.registerNetConnCallback = env->GetMethodID(
        cls, METHOD_REGISTER_NET_CONN_CALLBACK, SIGNATURE_REGISTER_NET_CONN_CALLBACK);
    CHECK_NULL_VOID(g_netconnclientpluginClass.registerNetConnCallback);

    g_netconnclientpluginClass.unregisterNetConnCallback = env->GetMethodID(
        cls, METHOD_UNREGISTER_NET_CONN_CALLBACK, SIGNATURE_UNREGISTER_NET_CONN_CALLBACK);
    CHECK_NULL_VOID(g_netconnclientpluginClass.unregisterNetConnCallback);

    g_netconnclientpluginClass.isDefaultNetworkActive = env->GetMethodID(
        cls, METHOD_IS_DEFAULT_NETWORK_ACTIVE, SIGNATURE_IS_DEFAULT_NETWORK_ACTIVE);
    CHECK_NULL_VOID(g_netconnclientpluginClass.isDefaultNetworkActive);

    env->DeleteLocalRef(cls);
    hasInit_ = true;
}

void NetConnClientJni::NativeOnAvailable(JNIEnv *env, jobject jthiz, jlong jcallbackKey, jint jNetId)
{
    if (env == nullptr || jthiz == nullptr) {
        return;
    }
    int32_t netId = (int32_t)jNetId;

    sptr<NetManagerStandard::NetHandle> netHandle =
        std::make_unique<NetManagerStandard::NetHandle>(netId).release();
    long key = (long)jcallbackKey;
    auto iter = observer_.find(key);
    if (iter != observer_.end()) {
        (iter->second)->NetAvailable(netHandle);
    }
}

void NetConnClientJni::NativeOnCapabilitiesChanged(JNIEnv *env, jobject jthiz, jobject jNetworkCapabilities,
    jlong jcallbackKey, jint jNetId)
{
    if (env == nullptr || jthiz == nullptr || jNetworkCapabilities == nullptr) {
        return;
    }
    int32_t netId = (int32_t)jNetId;

    jclass cls_networkcapabilities = env->GetObjectClass(jNetworkCapabilities);
    sptr<NetManagerStandard::NetAllCapabilities> netAllCap =
        std::make_unique<NetManagerStandard::NetAllCapabilities>().release();

    jmethodID jGetTransportTypes = env->GetMethodID(cls_networkcapabilities, "getTransportTypes", "()[I");
    jobject jtype_obj = env->CallObjectMethod(jNetworkCapabilities, jGetTransportTypes);
    jintArray jtype_int = (jintArray)jtype_obj;
    int *type = env->GetIntArrayElements(jtype_int, NULL);
    int nSize = env->GetArrayLength(jtype_int);
    int int_type[nSize];
    if (memcpy_s(int_type, nSize * sizeof(int), type, nSize * sizeof(int)) != EOK) {
        LOGE("[NativeOnCapabilitiesChanged] memory copy failed");
        return;
    }
    for (int i = 0; i < nSize; i++) {
        netAllCap->bearerTypes_.insert(static_cast<NetManagerStandard::NetBearType>(int_type[i]));
    }
    env->DeleteLocalRef(jtype_int);
    sptr<NetManagerStandard::NetHandle> netHandle =
        std::make_unique<NetManagerStandard::NetHandle>(netId).release();
    long key = (long)jcallbackKey;
    auto iter = observer_.find(key);
    if (iter != observer_.end()) {
        (iter->second)->NetCapabilitiesChange(netHandle, netAllCap);
    }
}

void NetConnClientJni::NativeOnLost(JNIEnv *env, jobject jthiz, jlong jcallbackKey, jint jNetId)
{
    if (env == nullptr || jthiz == nullptr) {
        return;
    }
    int32_t netId = (int32_t)jNetId;
    sptr<NetManagerStandard::NetHandle> netHandle =
        std::make_unique<NetManagerStandard::NetHandle>(netId).release();
    long key = (long)jcallbackKey;
    auto iter = observer_.find(key);
    if (iter != observer_.end()) {
        (iter->second)->NetLost(netHandle);
    }
}

void NetConnClientJni::NativeOnUnavailable(JNIEnv *env, jobject jthiz, jlong jcallbackKey)
{
    long key = (long)jcallbackKey;
    auto iter = observer_.find(key);
    if (iter != observer_.end()) {
        (iter->second)->NetUnavailable();
    }
}

void NetConnClientJni::RegisterCallback(const sptr<NetManagerStandard::INetConnCallback> &callback)
{
    long key = (long)callback.GetRefPtr();
    auto iter = observer_.find(key);
    if (iter == observer_.end()) {
        observer_[key] = callback;
    }
}

void NetConnClientJni::UnregisterCallback(const sptr<NetManagerStandard::INetConnCallback> &callback)
{
    long key = (long)callback.GetRefPtr();
    auto iter = observer_.find(key);
    if (iter != observer_.end()) {
        observer_.erase(iter);
    }
}

int32_t NetConnClientJni::RegisterDefaultNetConnCallback(const sptr<NetManagerStandard::INetConnCallback> &callback)
{
    if (!hasInit_) {
        NetConnClientJniRegister();
    }
    RegisterCallback(callback);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, NetManagerStandard::NETMANAGER_ERR_INTERNAL);
    CHECK_NULL_RETURN(g_netconnclientpluginClass.globalRef, NetManagerStandard::NETMANAGER_ERR_INTERNAL);
    CHECK_NULL_RETURN(g_netconnclientpluginClass.registerDefaultNetConnCallback,
        NetManagerStandard::NETMANAGER_ERR_INTERNAL);
    jlong callbackKey = (jlong)callback.GetRefPtr();
    env->CallVoidMethod(
        g_netconnclientpluginClass.globalRef, g_netconnclientpluginClass.registerDefaultNetConnCallback, callbackKey);
    if (env->ExceptionCheck()) {
        LOGE("NetConnClientJni JNI: call RegisterDefaultNetConnCallback has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return NetManagerStandard::NETMANAGER_ERR_PERMISSION_DENIED;
    }
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t NetConnClientJni::RegisterNetConnCallback(const sptr<NetManagerStandard::NetSpecifier> &netSpecifier,
    const sptr<NetManagerStandard::INetConnCallback> &callback, const uint32_t &timeoutMS)
{
    if (!hasInit_) {
        NetConnClientJniRegister();
    }
    RegisterCallback(callback);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, NetManagerStandard::NETMANAGER_ERR_INTERNAL);
    CHECK_NULL_RETURN(g_netconnclientpluginClass.globalRef, NetManagerStandard::NETMANAGER_ERR_INTERNAL);
    CHECK_NULL_RETURN(g_netconnclientpluginClass.registerNetConnCallback, NetManagerStandard::NETMANAGER_ERR_INTERNAL);
    int32_t size = netSpecifier->netCapabilities_.bearerTypes_.size();
    int types[size];
    int index = 0;
    for (auto it = netSpecifier->netCapabilities_.bearerTypes_.begin();
        it != netSpecifier->netCapabilities_.bearerTypes_.end(); ++it) {
        types[index] = *it;
    }
    jintArray jtypeArray = env->NewIntArray(size);
    env->SetIntArrayRegion(jtypeArray, 0, size, types);
    jlong callbackKey = (jlong)callback.GetRefPtr();
    env->CallVoidMethod(
        g_netconnclientpluginClass.globalRef, g_netconnclientpluginClass.registerNetConnCallback,
        callbackKey, jtypeArray);
    if (env->ExceptionCheck()) {
        LOGE("NetConnClientJni JNI: call RegisterNetConnCallback has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return NetManagerStandard::NETMANAGER_ERR_PERMISSION_DENIED;
    }
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t NetConnClientJni::UnregisterNetConnCallback(const sptr<NetManagerStandard::INetConnCallback> &callback)
{
    if (!hasInit_) {
        NetConnClientJniRegister();
    }
    UnregisterCallback(callback);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, NetManagerStandard::NETMANAGER_ERR_INTERNAL);
    CHECK_NULL_RETURN(g_netconnclientpluginClass.globalRef, NetManagerStandard::NETMANAGER_ERR_INTERNAL);
    CHECK_NULL_RETURN(g_netconnclientpluginClass.unregisterNetConnCallback,
        NetManagerStandard::NETMANAGER_ERR_INTERNAL);
    jlong callbackKey = (jlong)callback.GetRefPtr();
    env->CallVoidMethod(
        g_netconnclientpluginClass.globalRef, g_netconnclientpluginClass.unregisterNetConnCallback, callbackKey);
    if (env->ExceptionCheck()) {
        LOGE("NetConnClientJni JNI: call UnregisterNetConnCallback has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return NetManagerStandard::NETMANAGER_ERR_PERMISSION_DENIED;
    }
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

int32_t NetConnClientJni::HasDefaultNet(bool &flag)
{
    LOGI("NetConnClientJni HasDefaultNet");
    if (!hasInit_) {
        NetConnClientJniRegister();
    }
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, NetManagerStandard::NETMANAGER_ERR_INTERNAL);
    CHECK_NULL_RETURN(g_netconnclientpluginClass.globalRef, NetManagerStandard::NETMANAGER_ERR_INTERNAL);
    CHECK_NULL_RETURN(g_netconnclientpluginClass.isDefaultNetworkActive,
        NetManagerStandard::NETMANAGER_ERR_INTERNAL);
    flag = static_cast<bool>(env->CallBooleanMethod(
        g_netconnclientpluginClass.globalRef, g_netconnclientpluginClass.isDefaultNetworkActive));
    if (env->ExceptionCheck()) {
        LOGE("NetConnClientJni JNI: call HasDefaultNet has exception");
        env->ExceptionDescribe();
        env->ExceptionClear();
        return NetManagerStandard::NETMANAGER_ERR_PERMISSION_DENIED;
    }
    return NetManagerStandard::NETMANAGER_SUCCESS;
}

void NetConnClientJni::NetConnClientJniRegister()
{
    const char className[] = "ohos.ace.plugin.netconnclientplugin.NetConnClientPlugin";
    LOGI("NetConnClientJni::NetConnClientJniRegister!");
    ARKUI_X_Plugin_RegisterJavaPlugin(&Plugin::NetConnClientJni::Register, className);
}
}