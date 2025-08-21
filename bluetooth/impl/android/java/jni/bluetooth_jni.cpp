/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "bluetooth_jni.h"

#include <algorithm>
#include <cctype>
#include <jni.h>
#include <sstream>
#include <string>

#include "bluetooth_a2dp_impl.h"
#include "bluetooth_ble_advertiser_impl.h"
#include "bluetooth_ble_centralmanager_impl.h"
#include "bluetooth_gatt_client_impl.h"
#include "bluetooth_gatt_server_impl.h"
#include "bluetooth_host_impl.h"
#include "bluetooth_impl_utils.h"
#include "bluetooth_profile_manager.h"
#include "plugin_utils.h"
#include "plugin_utils_inner.h"
#include "raw_address.h"
#include "securec.h"
#include "utils.h"

namespace OHOS::Bluetooth {
namespace {
const char BLUETOOTH_CLASS_NAME[] = "ohos/ace/plugin/bluetoothplugin/BluetoothPlugin";
static OHOS::sptr<IBluetoothBleCentralManagerCallback> callback_;
static const JNINativeMethod METHODS[] = {
    {
        "nativeInit",
        "()V",
        reinterpret_cast<void*>(&BluetoothJni::NativeInit)
    },
    {
        "nativeOnScanResult",
        "(Ljava/lang/String;I)V",
        reinterpret_cast<void*>(&BluetoothJni::NativeOnScanResult)
    },
    {
        "nativeOnStartAdvertisingResult",
        "(III)V",
        reinterpret_cast<void*>(&BluetoothJni::NativeOnStartAdvertisingResult)
    },
    {
        "onDiscoveryResultCallBack",
        "(Ljava/lang/String;ILjava/lang/String;I)V",
        reinterpret_cast<void*>(&BluetoothJni::OnDiscoveryResultCallBack)
    },
    {
        "onPairStatusChangedCallBack",
        "(ILjava/lang/String;II)V",
        reinterpret_cast<void*>(&BluetoothJni::OnPairStatusChangedCallBack)
    },
    {
        "OnConnectionStateChanged",
        "(Ljava/lang/String;II)V",
        reinterpret_cast<void*>(&BluetoothJni::OnConnectionStateChanged)
    },
    {
        "nativeServerOnMtuChangedCallback",
        "(Ljava/lang/String;II)V",
        reinterpret_cast<void*>(&BluetoothJni::NativeServerOnMtuChangedCallback)
    },
    {
        "nativeServerOnConnectionStateChangeCallback",
        "(Ljava/lang/String;II)V",
        reinterpret_cast<void*>(&BluetoothJni::NativeServerOnConnectionStateChangeCallback)
    },
    {
        "nativeServerOnCharacteristicReadRequestCallback",
        "(Ljava/lang/String;Ljava/lang/String;I)V",
        reinterpret_cast<void*>(&BluetoothJni::NativeServerOnCharacteristicReadRequestCallback)
    },
    {
        "nativeServerOnCharacteristicWriteRequestCallback",
        "(Ljava/lang/String;Ljava/lang/String;ZI)V",
        reinterpret_cast<void*>(&BluetoothJni::NativeServerOnCharacteristicWriteRequestCallback)
    },
    {
        "nativeServerOnDescriptorReadRequestCallback",
        "(Ljava/lang/String;Ljava/lang/String;I)V",
        reinterpret_cast<void*>(&BluetoothJni::NativeServerOnDescriptorReadRequestCallback)
    },
    {
        "nativeServerOnDescriptorWriteRequestCallback",
        "(Ljava/lang/String;Ljava/lang/String;I)V",
        reinterpret_cast<void*>(&BluetoothJni::NativeServerOnDescriptorWriteRequestCallback)
    },
    {
        "onChangeStateCallBack",
        "(I)V",
        reinterpret_cast<void*>(&BluetoothJni::OnChangeStateCallBack)
    },
    {
        "nativeOnCharacteristicRead",
        "(ILjava/lang/String;I)V",
        reinterpret_cast<void*>(&BluetoothJni::NativeOnCharacteristicRead)
    },
    {
        "nativeOnCharacteristicWrite",
        "(ILjava/lang/String;I)V",
        reinterpret_cast<void*>(&BluetoothJni::NativeOnCharacteristicWrite)
    },
    {
        "nativeOnConnectionStateChanged",
        "(III)V",
        reinterpret_cast<void*>(&BluetoothJni::NativeOnConnectionStateChanged)
    },
    {
        "nativeGattClientSetServices",
        "(ILjava/lang/String;[Ljava/lang/String;Ljava/util/HashMap;)V",
        reinterpret_cast<void*>(&BluetoothJni::NativeGattClientSetServices)
    },
    {
        "nativeOnDescriptorRead",
        "(ILjava/lang/String;I)V",
        reinterpret_cast<void*>(&BluetoothJni::NativeOnDescriptorRead)
    },
    {
        "nativeOnMtuChanged",
        "(III)V",
        reinterpret_cast<void*>(&BluetoothJni::NativeOnMtuChanged)
    },
    {
        "nativeOnDescriptorWrite",
        "(ILjava/lang/String;I)V",
        reinterpret_cast<void*>(&BluetoothJni::NativeOnDescriptorWrite)
    }
};

// ble
static const char START_SCAN_METHOD[] = "startScan";
static const char START_SCAN_METHOD_PARAM[] = "(Ljava/lang/String;[Ljava/lang/String;I)I";
static const char STOP_SCAN_METHOD[] = "stopScan";
static const char STOP_SCAN_METHOD_PARAM[] = "(I)I";
static const char START_ADVERTISING_METHOD[] = "startAdvertising";
static const char START_ADVERTISING_METHOD_PARAM[] = "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;IZ)I";
static const char STOP_ADVERTISING_METHOD[] = "stopAdvertising";
static const char STOP_ADVERTISING_METHOD_PARAM[] = "(I)I";

// access and connection
static const char START_PAIR_METHOD[] = "startPair";
static const char START_PAIR_METHOD_PARAM[] = "(Ljava/lang/String;)I";
static const char GET_BT_PROFILE_CONN_STATE_METHOD[] = "getBtProfileConnState";
static const char GET_BT_PROFILE_CONN_STATE_METHOD_PARAM[] = "(I[I)I";
static const char GET_BT_PROFILES_CONN_STATE_METHOD[] = "getBtProfilesConnState";
static const char GET_BT_PROFILES_CONN_STATE_METHOD_PARAM[] = "([I[I)I";
static const char GET_DEVICENAME_METHOD[] = "getDeviceName";
static const char GET_DEVICENAME_METHOD_PARAM[] = "(Ljava/lang/String;[Ljava/lang/String;)I";
static const char GET_LOCALNAME_METHOD[] = "getLocalName";
static const char GET_LOCALNAME_METHOD_PARAM[] = "([Ljava/lang/String;)I";
static const char GET_PAIREDDEVICES_METHOD[] = "getPairedDevices";
static const char GET_PAIREDDEVICES_METHOD_PARAM[] = "([Ljava/lang/String;)I";
static const char GET_PAIRSTATE_METHOD[] = "getPairState";
static const char GET_PAIRSTATE_METHOD_PARAM[] = "(Ljava/lang/String;[I)I";
static const char GET_DEVICE_CLASS_METHOD[] = "getDeviceClass";
static const char GET_DEVICE_CLASS_METHOD_PARAM[] = "(Ljava/lang/String;[I)I";
static const char GET_DEVICE_UUIDS_METHOD[] = "getDeviceUuids";
static const char GET_DEVICE_UUIDS_METHOD_PARAM[] = "(Ljava/lang/String;[Ljava/lang/String;)I";
static const char SET_LOCAL_NAME_METHOD[] = "setLocalName";
static const char SET_LOCAL_NAME_METHOD_PARAM[] = "(Ljava/lang/String;)I";
static const char GET_BT_SCAN_MODE_METHOD[] = "getBtScanMode";
static const char GET_BT_SCAN_MODE_METHOD_PARAM[] = "([I)I";
static const char START_BT_DISCOVERY_METHOD[] = "startBtDiscovery";
static const char START_BT_DISCOVERY_METHOD_PARAM[] = "()I";
static const char CANCEL_BT_DISCOVERY_METHOD[] = "cancelBtDiscovery";
static const char CANCEL_BT_DISCOVERY_METHOD_PARAM[] = "()I";
static const char IS_BT_DISCOVERING_METHOD[] = "isBtDiscovering";
static const char IS_BT_DISCOVERING_METHOD_PARAM[] = "([Z)I";
static const char ENABLE_BT_METHOD[] = "enableBt";
static const char ENABLE_BT_METHOD_PARAM[] = "()I";
static const char DISABLE_BT_METHOD[] = "disableBt";
static const char DISABLE_BT_METHOD_PARAM[] = "()I";
static const char GET_BT_STATE_METHOD[] = "getBtState";
static const char GET_BT_STATE_METHOD_PARAM[] = "([I)I";
static const char IS_BT_ENABLE_METHOD[] = "isBtEnable";
static const char IS_BT_ENABLE_METHOD_PARAM[] = "()Z";

// basePorfile
static const char GET_CONNECTIONSTATE_METHOD[] = "getConnectionState";
static const char GET_CONNECTIONSTATE_METHOD_PARAM[] = "(Ljava/lang/String;[I)I";
static const char GET_DEVICESBYSTATES_METHOD[] = "getDevicesByStates";
static const char GET_DEVICESBYSTATES_METHOD_PARAM[] = "([Ljava/lang/String;)I";

// Gatt Server
static const char REGISTER_APPLICATION_GATTSERVER_METHOD[] = "registerApplicationGattServer";
static const char REGISTER_APPLICATION_GATTSERVER_METHOD_PARAM[] = "(I)I";
static const char ADD_SERVICE_METHOD[] = "addService";
static const char ADD_SERVICE_METHOD_PARAM[] = "(IILjava/lang/String;)I";
static const char REMOVE_SERVICE_METHOD[] = "removeService";
static const char REMOVE_SERVICE_METHOD_PARAM[] = "(II)I";
static const char GATT_SERVER_CLOSE_METHOD[] = "gattServerClose";
static const char GATT_SERVER_CLOSE_METHOD_PARAM[] = "(I)I";
static const char NOTIFY_CHARACTERISTI_CHANGED_METHOD[] = "notifyCharacteristicChanged";
static const char NOTIFY_CHARACTERISTI_CHANGED_METHOD_PARAM[] = "(IILjava/lang/String;Ljava/lang/String;Z)I";
static const char GATT_SERVER_RESPOND_CHARACTERISTIC_READ_METHOD[] = "respondCharacteristicRead";
static const char GATT_SERVER_RESPOND_CHARACTERISTIC_READ_METHOD_PARAM[] = "(Ljava/lang/String;Ljava/lang/String;III)I";
static const char GATT_SERVER_RESPOND_CHARACTERISTIC_WRITE_METHOD[] = "respondCharacteristicWrite";
static const char GATT_SERVER_RESPOND_CHARACTERISTIC_WRITE_METHOD_PARAM[] = "(Ljava/lang/String;III)I";
static const char GATT_SERVER_RESPOND_DESCRIPTOR_READ_METHOD[] = "respondDescriptorRead";
static const char GATT_SERVER_RESPOND_DESCRIPTOR_READ_METHOD_PARAM[] = "(Ljava/lang/String;Ljava/lang/String;III)I";
static const char GATT_SERVER_RESPOND_DESCRIPTOR_WRITE_METHOD[] = "respondDescriptorWrite";
static const char GATT_SERVER_RESPOND_DESCRIPTOR_WRITE_METHOD_PARAM[] = "(Ljava/lang/String;III)I";

// Gatt Client
static const char GATT_CLIENT_CONNECT_METHOD[] = "gattClientConnect";
static const char GATT_CLIENT_CONNECT_METHOD_PARAM[] = "(ILjava/lang/String;Z)I";
static const char GATT_CLIENT_DISCONNECT_METHOD[] = "gattClientDisconnect";
static const char GATT_CLIENT_DISCONNECT_METHOD_PARAM[] = "(I)I";
static const char GATT_CLIENT_CLOSE_METHOD[] = "gattClientClose";
static const char GATT_CLIENT_CLOSE_METHOD_PARAM[] = "(I)I";
static const char REQUEST_EXCHANGE_MTU_METHOD[] = "gattClientRequestExchangeMtu";
static const char REQUEST_EXCHANGE_MTU_METHOD_PARAM[] = "(II)I";
static const char CLIENT_READ_CHARACTER_METHOD[] = "clientReadCharacter";
static const char CLIENT_READ_CHARACTER_METHOD_PARAM[] = "(ILjava/lang/String;Ljava/lang/String;)I";
static const char CLIENT_WRITE_CHARACTER_METHOD[] = "clientWriteCharacter";
static const char CLIENT_WRITE_CHARACTER_METHOD_PARAM[] = "(ILjava/lang/String;Ljava/lang/String;[BI)I";
static const char CLIENT_WRITE_DESCRIPTOR_METHOD[] = "clientWriteDescriptor";
static const char CLIENT_WRITE_DESCRIPTOR_METHOD_PARAM[] =
    "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;[B)I";
static const char CLIENT_READ_DESCRIPTOR_METHOD[] = "clientReadDescriptor";
static const char CLIENT_READ_DESCRIPTOR_METHOD_PARAM[] = "(ILjava/lang/String;Ljava/lang/String;Ljava/lang/String;)I";
static const char CLIENT_DISCOVER_SERVICES_METHOD[] = "clientDiscoverServices";
static const char CLIENT_DISCOVER_SERVICES_METHOD_PARAM[] = "(I)I";
struct {
    jobject globalRef;

    // ble
    jmethodID startScan;
    jmethodID stopScan;
    jmethodID startAdvertising;
    jmethodID stopAdvertising;

    // access and connection
    jmethodID startPair;
    jmethodID getBtProfileConnState;
    jmethodID getBtProfilesConnState;
    jmethodID getDeviceName;
    jmethodID getLocalName;
    jmethodID getPairedDevices;
    jmethodID getPairState;
    jmethodID getDeviceClass;
    jmethodID getDeviceUuids;
    jmethodID setLocalName;
    jmethodID getBtScanMode;
    jmethodID startBtDiscovery;
    jmethodID cancelBtDiscovery;
    jmethodID isBtDiscovering;
    jmethodID enableBt;
    jmethodID disableBt;
    jmethodID getBtState;
    jmethodID isBtEnable;

    //baseProfile
    jmethodID getConnectionState;
    jmethodID getDevicesByStates;

    // Gatt Server
    jmethodID registerApplicationGattServer;
    jmethodID addService;
    jmethodID removeService;
    jmethodID gattServerClose;
    jmethodID notifyCharacteristicChanged;
    jmethodID respondCharacteristicRead;
    jmethodID respondCharacteristicWrite;
    jmethodID respondDescriptorRead;
    jmethodID respondDescriptorWrite;

    // Gatt Client
    jmethodID gattClientConnect;
    jmethodID gattClientDisconnect;
    jmethodID gattClientRequestExchangeMtu;
    jmethodID gattClientClose;
    jmethodID clientReadCharacter;
    jmethodID clientWriteCharacter;
    jmethodID clientReadDescriptor;
    jmethodID clientWriteDescriptor;
    jmethodID clientDiscoverServices;
} g_bluetoothClass;
} // namespace
std::mutex BluetoothJni::gattClientLock_;
std::mutex BluetoothJni::gattClientServiceLock_;

bool BluetoothJni::Register(void* env)
{
    HILOGI("BluetoothJni Register start");
    auto* jniEnv = static_cast<JNIEnv*>(env);
    CHECK_NULL_RETURN(jniEnv, false);
    jclass cls = jniEnv->FindClass(BLUETOOTH_CLASS_NAME);
    CHECK_NULL_RETURN(cls, false);
    bool ret = jniEnv->RegisterNatives(cls, METHODS, sizeof(METHODS) / sizeof(METHODS[0])) == 0;
    jniEnv->DeleteLocalRef(cls);
    if (!ret) {
        HILOGE("BluetoothJni JNI: RegisterNatives fail.");
        return false;
    }
    HILOGI("BluetoothJni Register end");
    return true;
}

void BluetoothJni::NativeBleInit(JNIEnv* env, jobject jobj)
{
    HILOGI("BluetoothJni NativeBleInit start");
    CHECK_NULL_VOID(env);
    g_bluetoothClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_bluetoothClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_bluetoothClass.startScan = env->GetMethodID(cls, START_SCAN_METHOD, START_SCAN_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.startScan);
    g_bluetoothClass.stopScan = env->GetMethodID(cls, STOP_SCAN_METHOD, STOP_SCAN_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.stopScan);
    g_bluetoothClass.startAdvertising = env->GetMethodID(cls, START_ADVERTISING_METHOD, START_ADVERTISING_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.startAdvertising);
    g_bluetoothClass.stopAdvertising = env->GetMethodID(cls, STOP_ADVERTISING_METHOD, STOP_ADVERTISING_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.stopAdvertising);

    env->DeleteLocalRef(cls);
    HILOGI("BluetoothJni NativeBleInit end");
}

void BluetoothJni::NativeAccessInit(JNIEnv* env, jobject jobj)
{
    HILOGI("BluetoothJni NativeAccessInit start");
    CHECK_NULL_VOID(env);
    g_bluetoothClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_bluetoothClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_bluetoothClass.enableBt = env->GetMethodID(cls, ENABLE_BT_METHOD, ENABLE_BT_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.enableBt);
    g_bluetoothClass.disableBt = env->GetMethodID(cls, DISABLE_BT_METHOD, DISABLE_BT_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.disableBt);
    g_bluetoothClass.getBtState = env->GetMethodID(cls, GET_BT_STATE_METHOD, GET_BT_STATE_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.getBtState);
    g_bluetoothClass.isBtEnable = env->GetMethodID(cls, IS_BT_ENABLE_METHOD, IS_BT_ENABLE_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.isBtEnable);

    env->DeleteLocalRef(cls);
    HILOGI("BluetoothJni NativeAccessInit end");
}

void BluetoothJni::NativeConnectionInit(JNIEnv* env, jobject jobj)
{
    CHECK_NULL_VOID(env);
    g_bluetoothClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_bluetoothClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);
    g_bluetoothClass.startPair = env->GetMethodID(cls, START_PAIR_METHOD, START_PAIR_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.startPair);
    g_bluetoothClass.getBtProfileConnState =
        env->GetMethodID(cls, GET_BT_PROFILE_CONN_STATE_METHOD, GET_BT_PROFILE_CONN_STATE_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.getBtProfileConnState);
    g_bluetoothClass.getBtProfilesConnState =
        env->GetMethodID(cls, GET_BT_PROFILES_CONN_STATE_METHOD, GET_BT_PROFILES_CONN_STATE_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.getBtProfilesConnState);
    g_bluetoothClass.getDeviceName = env->GetMethodID(cls, GET_DEVICENAME_METHOD, GET_DEVICENAME_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.getDeviceName);
    g_bluetoothClass.getLocalName = env->GetMethodID(cls, GET_LOCALNAME_METHOD, GET_LOCALNAME_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.getLocalName);
    g_bluetoothClass.getPairedDevices = env->GetMethodID(cls, GET_PAIREDDEVICES_METHOD, GET_PAIREDDEVICES_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.getPairedDevices);
    g_bluetoothClass.getPairState = env->GetMethodID(cls, GET_PAIRSTATE_METHOD, GET_PAIRSTATE_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.getPairState);
    g_bluetoothClass.getDeviceClass = env->GetMethodID(cls, GET_DEVICE_CLASS_METHOD, GET_DEVICE_CLASS_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.getDeviceClass);
    g_bluetoothClass.getDeviceUuids = env->GetMethodID(cls, GET_DEVICE_UUIDS_METHOD, GET_DEVICE_UUIDS_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.getDeviceUuids);
    g_bluetoothClass.setLocalName = env->GetMethodID(cls, SET_LOCAL_NAME_METHOD, SET_LOCAL_NAME_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.setLocalName);
    g_bluetoothClass.getBtScanMode = env->GetMethodID(cls, GET_BT_SCAN_MODE_METHOD, GET_BT_SCAN_MODE_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.getBtScanMode);
    g_bluetoothClass.startBtDiscovery =
        env->GetMethodID(cls, START_BT_DISCOVERY_METHOD, START_BT_DISCOVERY_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.startBtDiscovery);
    g_bluetoothClass.cancelBtDiscovery =
        env->GetMethodID(cls, CANCEL_BT_DISCOVERY_METHOD, CANCEL_BT_DISCOVERY_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.cancelBtDiscovery);
    g_bluetoothClass.isBtDiscovering = env->GetMethodID(cls, IS_BT_DISCOVERING_METHOD, IS_BT_DISCOVERING_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.isBtDiscovering);
    g_bluetoothClass.enableBt = env->GetMethodID(cls, ENABLE_BT_METHOD, ENABLE_BT_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.enableBt);
    g_bluetoothClass.disableBt = env->GetMethodID(cls, DISABLE_BT_METHOD, DISABLE_BT_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.disableBt);
    g_bluetoothClass.getBtState = env->GetMethodID(cls, GET_BT_STATE_METHOD, GET_BT_STATE_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.getBtState);
    g_bluetoothClass.isBtEnable = env->GetMethodID(cls, IS_BT_ENABLE_METHOD, IS_BT_ENABLE_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.isBtEnable);
    env->DeleteLocalRef(cls);
}

void BluetoothJni::NativeBaseProfileInit(JNIEnv* env, jobject jobj)
{
    HILOGI("BluetoothJni NativeBaseProfileInit start");
    CHECK_NULL_VOID(env);
    g_bluetoothClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_bluetoothClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_bluetoothClass.getConnectionState =
        env->GetMethodID(cls, GET_CONNECTIONSTATE_METHOD, GET_CONNECTIONSTATE_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.getConnectionState);
    g_bluetoothClass.getDevicesByStates =
        env->GetMethodID(cls, GET_DEVICESBYSTATES_METHOD, GET_DEVICESBYSTATES_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.getDevicesByStates);

    env->DeleteLocalRef(cls);
    HILOGI("BluetoothJni NativeBaseProfileInit end");
}

void BluetoothJni::NativeGattServerInit(JNIEnv* env, jobject jobj)
{
    HILOGI("BluetoothJni NativeGattServerInit start");
    CHECK_NULL_VOID(env);
    g_bluetoothClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_bluetoothClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_bluetoothClass.registerApplicationGattServer =
        env->GetMethodID(cls, REGISTER_APPLICATION_GATTSERVER_METHOD, REGISTER_APPLICATION_GATTSERVER_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.registerApplicationGattServer);
    g_bluetoothClass.addService = env->GetMethodID(cls, ADD_SERVICE_METHOD, ADD_SERVICE_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.addService);
    g_bluetoothClass.removeService = env->GetMethodID(cls, REMOVE_SERVICE_METHOD, REMOVE_SERVICE_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.removeService);
    g_bluetoothClass.gattServerClose = env->GetMethodID(cls, GATT_SERVER_CLOSE_METHOD, GATT_SERVER_CLOSE_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.gattServerClose);
    g_bluetoothClass.notifyCharacteristicChanged =
        env->GetMethodID(cls, NOTIFY_CHARACTERISTI_CHANGED_METHOD, NOTIFY_CHARACTERISTI_CHANGED_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.notifyCharacteristicChanged);
    g_bluetoothClass.respondCharacteristicRead = env->GetMethodID(
        cls, GATT_SERVER_RESPOND_CHARACTERISTIC_READ_METHOD, GATT_SERVER_RESPOND_CHARACTERISTIC_READ_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.respondCharacteristicRead);
    g_bluetoothClass.respondCharacteristicWrite = env->GetMethodID(
        cls, GATT_SERVER_RESPOND_CHARACTERISTIC_WRITE_METHOD, GATT_SERVER_RESPOND_CHARACTERISTIC_WRITE_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.respondCharacteristicWrite);
    g_bluetoothClass.respondDescriptorRead = env->GetMethodID(
        cls, GATT_SERVER_RESPOND_DESCRIPTOR_READ_METHOD, GATT_SERVER_RESPOND_DESCRIPTOR_READ_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.respondDescriptorRead);
    g_bluetoothClass.respondDescriptorWrite = env->GetMethodID(
        cls, GATT_SERVER_RESPOND_DESCRIPTOR_WRITE_METHOD, GATT_SERVER_RESPOND_DESCRIPTOR_WRITE_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.respondDescriptorWrite);

    env->DeleteLocalRef(cls);
    HILOGI("BluetoothJni NativeGattServerInit end");
}

void BluetoothJni::NativeGattClientInit(JNIEnv* env, jobject jobj)
{
    HILOGI("BluetoothJni NativeGattClientInit start");
    CHECK_NULL_VOID(env);
    g_bluetoothClass.globalRef = env->NewGlobalRef(jobj);
    CHECK_NULL_VOID(g_bluetoothClass.globalRef);
    jclass cls = env->GetObjectClass(jobj);
    CHECK_NULL_VOID(cls);

    g_bluetoothClass.gattClientConnect =
        env->GetMethodID(cls, GATT_CLIENT_CONNECT_METHOD, GATT_CLIENT_CONNECT_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.gattClientConnect);
    g_bluetoothClass.gattClientDisconnect =
        env->GetMethodID(cls, GATT_CLIENT_DISCONNECT_METHOD, GATT_CLIENT_DISCONNECT_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.gattClientDisconnect);
    g_bluetoothClass.gattClientRequestExchangeMtu =
        env->GetMethodID(cls, REQUEST_EXCHANGE_MTU_METHOD, REQUEST_EXCHANGE_MTU_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.gattClientRequestExchangeMtu);
    g_bluetoothClass.gattClientClose = env->GetMethodID(cls, GATT_CLIENT_CLOSE_METHOD, GATT_CLIENT_CLOSE_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.gattClientClose);
    g_bluetoothClass.clientReadCharacter =
        env->GetMethodID(cls, CLIENT_READ_CHARACTER_METHOD, CLIENT_READ_CHARACTER_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.clientReadCharacter);
    g_bluetoothClass.clientWriteCharacter =
        env->GetMethodID(cls, CLIENT_WRITE_CHARACTER_METHOD, CLIENT_WRITE_CHARACTER_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.clientWriteCharacter);
    g_bluetoothClass.clientWriteDescriptor =
        env->GetMethodID(cls, CLIENT_WRITE_DESCRIPTOR_METHOD, CLIENT_WRITE_DESCRIPTOR_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.clientWriteDescriptor);
    g_bluetoothClass.clientReadDescriptor =
        env->GetMethodID(cls, CLIENT_READ_DESCRIPTOR_METHOD, CLIENT_READ_DESCRIPTOR_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.clientReadDescriptor);
    g_bluetoothClass.clientDiscoverServices =
        env->GetMethodID(cls, CLIENT_DISCOVER_SERVICES_METHOD, CLIENT_DISCOVER_SERVICES_METHOD_PARAM);
    CHECK_NULL_VOID(g_bluetoothClass.clientDiscoverServices);

    env->DeleteLocalRef(cls);
    HILOGI("BluetoothJni NativeGattClientInit end");
}

void BluetoothJni::NativeInit(JNIEnv* env, jobject jobj)
{
    NativeBleInit(env, jobj);
    NativeAccessInit(env, jobj);
    NativeConnectionInit(env, jobj);
    NativeBaseProfileInit(env, jobj);
    NativeGattServerInit(env, jobj);
    NativeGattClientInit(env, jobj);
}

void BluetoothJni::NativeOnScanResult(JNIEnv* env, jobject jobj, jstring scanResultData, int scannerId)
{
    CHECK_NULL_VOID(env);
    std::string JsonString(env->GetStringUTFChars(scanResultData, nullptr));
    Json jsonData = Json::parse(JsonString);
    std::string addrData = jsonData["addr"];
    int8_t rssiData = jsonData["rssi"];
    bool connectableData = jsonData["connectable"];
    std::string nameData = jsonData["name"];

    auto jsonArray = jsonData["payload"];
    std::vector<uint8_t> payloadData(jsonArray.size());
    for (size_t i = 0; i < jsonArray.size(); ++i) {
        payloadData[i] = jsonArray[i].get<uint8_t>();
    }
    std::string payloadStr(payloadData.begin(), payloadData.end());

    BluetoothBleScanResult scanResult;
    scanResult.SetPeripheralDevice(bluetooth::RawAddress(addrData));
    scanResult.SetRssi(rssiData);
    scanResult.SetConnectable(connectableData);
    scanResult.SetName(nameData);
    scanResult.SetPayload(payloadStr);
    sptr<IRemoteObject> proxy = BluetoothProfileManager::GetInstance().GetProfileRemote(BLE_CENTRAL_MANAGER_SERVER);
    sptr<BluetoothBleCentralManagerImpl> impl = iface_cast<BluetoothBleCentralManagerImpl>(proxy);
    if (!impl) {
        HILOGE("BluetoothJni BluetoothBleCentralManagerImpl is null.");
        return;
    }
    impl->OnScanResult(scannerId, scanResult);
}

void BluetoothJni::NativeOnStartAdvertisingResult(JNIEnv* env, jobject jobj, int errCode, int advHandle, int opCode)
{
    sptr<IRemoteObject> proxy = BluetoothProfileManager::GetInstance().GetProfileRemote(BLE_ADVERTISER_SERVER);
    sptr<BluetoothBleAdvertiserImpl> impl = iface_cast<BluetoothBleAdvertiserImpl>(proxy);
    if (!impl) {
        HILOGE("BluetoothJni BluetoothBleAdvertiserImpl is null.");
        return;
    }
    impl->OnStartResultEvent(errCode, advHandle, opCode);
}

void BluetoothJni::NativeServerOnMtuChangedCallback(JNIEnv* env, jobject jobj, jstring deviceData, int mtu, int appId)
{
    HILOGI("new mtu is %{public}d", mtu);
    CHECK_NULL_VOID(env);
    std::string JsonString(env->GetStringUTFChars(deviceData, nullptr));
    Json jsonData = Json::parse(JsonString);
    auto device = BluetoothImplUtils::ConvertJsonToGattDevice(jsonData);
    sptr<IRemoteObject> proxy = BluetoothProfileManager::GetInstance().GetProfileRemote(PROFILE_GATT_SERVER);
    sptr<BluetoothGattServerImpl> impl = iface_cast<BluetoothGattServerImpl>(proxy);
    if (!impl) {
        HILOGE("BluetoothJni BluetoothGattServerImpl is null.");
        return;
    }

    impl->OnMtuChanged(appId, BluetoothGattDevice(device), mtu);
}

void BluetoothJni::NativeServerOnConnectionStateChangeCallback(
    JNIEnv* env, jobject jobj, jstring deviceData, int newState, int appId)
{
    HILOGI("newState is %{public}d", newState);
    CHECK_NULL_VOID(env);
    int state;
    BluetoothImplUtils::GetOhHostBtStateFromBluetoothAdapter(newState, state);
    std::string JsonString(env->GetStringUTFChars(deviceData, nullptr));
    Json jsonData = Json::parse(JsonString);
    auto device = BluetoothImplUtils::ConvertJsonToGattDevice(jsonData);
    sptr<IRemoteObject> proxy = BluetoothProfileManager::GetInstance().GetProfileRemote(PROFILE_GATT_SERVER);
    sptr<BluetoothGattServerImpl> impl = iface_cast<BluetoothGattServerImpl>(proxy);
    if (!impl) {
        HILOGE("BluetoothJni BluetoothGattServerImpl is null.");
        return;
    }

    impl->OnConnectionStateChanged(appId, BluetoothGattDevice(device), 0, state);
}

void BluetoothJni::NativeServerOnCharacteristicReadRequestCallback(
    JNIEnv* env, jobject jobj, jstring deviceData, jstring characteristicData, int appId)
{
    CHECK_NULL_VOID(env);
    std::string deviceDataJsonString(env->GetStringUTFChars(deviceData, nullptr));
    Json deviceJsonData = Json::parse(deviceDataJsonString);
    auto device = BluetoothImplUtils::ConvertJsonToGattDevice(deviceJsonData);

    std::string characteristicDataJsonString(env->GetStringUTFChars(characteristicData, nullptr));
    Json characteristicjsonData = Json::parse(characteristicDataJsonString);
    auto characteristic = BluetoothImplUtils::ConvertJsonToCharacteristic(characteristicjsonData);

    sptr<IRemoteObject> proxy = BluetoothProfileManager::GetInstance().GetProfileRemote(PROFILE_GATT_SERVER);
    sptr<BluetoothGattServerImpl> impl = iface_cast<BluetoothGattServerImpl>(proxy);
    if (!impl) {
        HILOGE("BluetoothJni BluetoothGattServerImpl is null.");
        return;
    }

    impl->OnCharacteristicReadRequest(appId, BluetoothGattDevice(device), BluetoothGattCharacteristic(characteristic));
}

void BluetoothJni::NativeServerOnCharacteristicWriteRequestCallback(
    JNIEnv* env, jobject jobj, jstring deviceData, jstring characteristicData, jboolean responseNeeded, int appId)
{
    CHECK_NULL_VOID(env);
    std::string deviceDataJsonString(env->GetStringUTFChars(deviceData, nullptr));
    Json deviceJsonData = Json::parse(deviceDataJsonString);
    auto device = BluetoothImplUtils::ConvertJsonToGattDevice(deviceJsonData);

    std::string characteristicDataJsonString(env->GetStringUTFChars(characteristicData, nullptr));
    Json characteristicjsonData = Json::parse(characteristicDataJsonString);
    auto characteristic = BluetoothImplUtils::ConvertJsonToCharacteristic(characteristicjsonData);

    sptr<IRemoteObject> proxy = BluetoothProfileManager::GetInstance().GetProfileRemote(PROFILE_GATT_SERVER);
    sptr<BluetoothGattServerImpl> impl = iface_cast<BluetoothGattServerImpl>(proxy);
    if (!impl) {
        HILOGE("BluetoothJni BluetoothGattServerImpl is null.");
        return;
    }

    impl->OnCharacteristicWriteRequest(appId, BluetoothGattDevice(device), BluetoothGattCharacteristic(characteristic),
        static_cast<bool>(responseNeeded));
}

void BluetoothJni::NativeServerOnDescriptorReadRequestCallback(
    JNIEnv* env, jobject jobj, jstring deviceData, jstring descriptorData, int appId)
{
    CHECK_NULL_VOID(env);
    std::string deviceDataJsonString(env->GetStringUTFChars(deviceData, nullptr));
    Json deviceJsonData = Json::parse(deviceDataJsonString);
    auto device = BluetoothImplUtils::ConvertJsonToGattDevice(deviceJsonData);

    std::string descriptorDataJsonString(env->GetStringUTFChars(descriptorData, nullptr));
    Json descriptorjsonData = Json::parse(descriptorDataJsonString);
    auto descriptor = BluetoothImplUtils::ConvertJsonToDescriptor(descriptorjsonData);

    sptr<IRemoteObject> proxy = BluetoothProfileManager::GetInstance().GetProfileRemote(PROFILE_GATT_SERVER);
    sptr<BluetoothGattServerImpl> impl = iface_cast<BluetoothGattServerImpl>(proxy);
    if (!impl) {
        HILOGE("BluetoothJni BluetoothGattServerImpl is null.");
        return;
    }

    impl->OnDescriptorReadRequest(appId, BluetoothGattDevice(device), BluetoothGattDescriptor(descriptor));
}

void BluetoothJni::NativeServerOnDescriptorWriteRequestCallback(
    JNIEnv* env, jobject jobj, jstring deviceData, jstring descriptorData, int appId)
{
    CHECK_NULL_VOID(env);
    std::string deviceDataJsonString(env->GetStringUTFChars(deviceData, nullptr));
    Json deviceJsonData = Json::parse(deviceDataJsonString);
    auto device = BluetoothImplUtils::ConvertJsonToGattDevice(deviceJsonData);

    std::string descriptorDataJsonString(env->GetStringUTFChars(descriptorData, nullptr));
    Json descriptorjsonData = Json::parse(descriptorDataJsonString);
    auto descriptor = BluetoothImplUtils::ConvertJsonToDescriptor(descriptorjsonData);

    sptr<IRemoteObject> proxy = BluetoothProfileManager::GetInstance().GetProfileRemote(PROFILE_GATT_SERVER);
    sptr<BluetoothGattServerImpl> impl = iface_cast<BluetoothGattServerImpl>(proxy);
    if (!impl) {
        HILOGE("BluetoothJni BluetoothGattServerImpl is null.");
        return;
    }

    impl->OnDescriptorWriteRequest(appId, BluetoothGattDevice(device), BluetoothGattDescriptor(descriptor));
}

void BluetoothJni::NativeOnCharacteristicRead(JNIEnv* env, jobject jobj, jint appId, jstring jsonString, jint status)
{
    CHECK_NULL_VOID(env);
    auto gattClientData = BluetoothGattClientImpl::GetGattClientData(static_cast<int32_t>(appId));
    if (gattClientData == nullptr || gattClientData->callback_ == nullptr) {
        HILOGE("gattClientData or gattClientDataCallback is nullptr");
        return;
    }
    std::string jsonCString(env->GetStringUTFChars(jsonString, nullptr));
    bluetooth::Characteristic characteristic;
    BluetoothImplUtils::ParseCharacteristicFromJson(jsonCString, characteristic);
    std::lock_guard<std::mutex> lock(gattClientLock_);
    BluetoothGattCharacteristic gattCharacteristic = BluetoothGattCharacteristic(characteristic);
    gattClientData->callback_->OnCharacteristicRead(status, gattCharacteristic);
}

void BluetoothJni::NativeOnCharacteristicWrite(JNIEnv* env, jobject jobj, jint appId, jstring jsonString, jint status)
{
    CHECK_NULL_VOID(env);
    auto gattClientData = BluetoothGattClientImpl::GetGattClientData(static_cast<int32_t>(appId));
    if (gattClientData == nullptr || gattClientData->callback_ == nullptr) {
        HILOGE("gattClientData or gattClientDataCallback is nullptr");
        return;
    }
    std::string jsonCString(env->GetStringUTFChars(jsonString, nullptr));
    bluetooth::Characteristic characteristic;
    BluetoothImplUtils::ParseCharacteristicFromJson(jsonCString, characteristic);
    std::lock_guard<std::mutex> lock(gattClientLock_);
    BluetoothGattCharacteristic gattCharacteristic = BluetoothGattCharacteristic(characteristic);
    gattClientData->callback_->OnCharacteristicWrite(status, gattCharacteristic);
}

void BluetoothJni::NativeOnDescriptorWrite(JNIEnv* env, jobject jobj, jint appId, jstring jsonString, jint status)
{
    CHECK_NULL_VOID(env);
    auto gattClientData = BluetoothGattClientImpl::GetGattClientData(static_cast<int32_t>(appId));
    if (gattClientData == nullptr || gattClientData->callback_ == nullptr) {
        HILOGE("gattClientData or gattClientDataCallback is nullptr");
        return;
    }
    std::string jsonCString(env->GetStringUTFChars(jsonString, nullptr));
    bluetooth::Descriptor descriptor;
    BluetoothImplUtils::ParseDescriptorFromJson(jsonCString, descriptor);
    std::lock_guard<std::mutex> lock(gattClientLock_);
    BluetoothGattDescriptor gattDescriptor = BluetoothGattDescriptor(descriptor);
    gattClientData->callback_->OnDescriptorWrite(status, gattDescriptor);
}

void BluetoothJni::NativeOnDescriptorRead(JNIEnv* env, jobject jobj, jint appId, jstring jsonString, jint status)
{
    CHECK_NULL_VOID(env);
    auto gattClientData = BluetoothGattClientImpl::GetGattClientData(static_cast<int32_t>(appId));
    if (gattClientData == nullptr || gattClientData->callback_ == nullptr) {
        HILOGE("gattClientData or gattClientDataCallback is nullptr");
        return;
    }
    std::string jsonCString(env->GetStringUTFChars(jsonString, nullptr));
    bluetooth::Descriptor descriptor;
    BluetoothImplUtils::ParseDescriptorFromJson(jsonCString, descriptor);
    std::lock_guard<std::mutex> lock(gattClientLock_);
    BluetoothGattDescriptor gattDescriptor = BluetoothGattDescriptor(descriptor);
    gattClientData->callback_->OnDescriptorRead(status, gattDescriptor);
}

void BluetoothJni::NativeOnConnectionStateChanged(JNIEnv* env, jobject jobj, jint appId, jint state, jint newState)
{
    auto gattClientData = BluetoothGattClientImpl::GetGattClientData(static_cast<int32_t>(appId));
    if (gattClientData == nullptr || gattClientData->callback_ == nullptr) {
        HILOGE("gattClientData or gattClientDataCallback is nullptr");
        return;
    }
    int32_t btState = 0;
    int32_t btNewState = 0;
    BluetoothImplUtils::GetOhHostBtStateFromBluetoothAdapter(static_cast<int32_t>(state), btState);
    BluetoothImplUtils::GetOhHostBtStateFromBluetoothAdapter(static_cast<int32_t>(newState), btNewState);
    std::lock_guard<std::mutex> lock(gattClientLock_);
    gattClientData->callback_->OnConnectionStateChanged(
        btState, btNewState, static_cast<int>(GattDisconnectReason::CONN_UNKNOWN));
}

std::string BluetoothJni::ConvertJStringToCString(JNIEnv* env, jstring jStr)
{
    CHECK_NULL_RETURN(env, "");
    if (jStr == nullptr) {
        return std::string("");
    }
    const char* btJsString = env->GetStringUTFChars(jStr, nullptr);
    std::string cppString(btJsString);
    env->ReleaseStringUTFChars(jStr, btJsString);
    return cppString;
}

std::map<std::string, std::vector<std::string>> BluetoothJni::ConvertJMapToCMap(JNIEnv *env, jobject dUuidMap)
{
    std::map<std::string, std::vector<std::string>> gattClientCharacteristicMap;
    CHECK_NULL_RETURN(env, gattClientCharacteristicMap);
    jclass btHashMapClass = env->GetObjectClass(dUuidMap);
    jmethodID btHashMapGetMethod = env->GetMethodID(btHashMapClass, "get", "(Ljava/lang/Object;)Ljava/lang/Object;");
    jmethodID btHashMapContainsKeyMethod = env->GetMethodID(btHashMapClass, "containsKey", "(Ljava/lang/Object;)Z");

    jobject btKeySet = env->CallObjectMethod(dUuidMap, env->GetMethodID(btHashMapClass, "keySet", "()Ljava/util/Set;"));
    jobject btKeyIterator = env->CallObjectMethod(
        btKeySet, env->GetMethodID(env->GetObjectClass(btKeySet), "iterator", "()Ljava/util/Iterator;"));

    while (
        env->CallBooleanMethod(btKeyIterator, env->GetMethodID(env->GetObjectClass(btKeyIterator), "hasNext", "()Z"))) {
        jobject key = env->CallObjectMethod(
            btKeyIterator, env->GetMethodID(env->GetObjectClass(btKeyIterator), "next", "()Ljava/lang/Object;"));
        jstring btKey = static_cast<jstring>(key);

        if (env->CallBooleanMethod(dUuidMap, btHashMapContainsKeyMethod, key)) {
            jobject value = env->CallObjectMethod(dUuidMap, btHashMapGetMethod, key);

            jobjectArray valueArray = static_cast<jobjectArray>(value);
            jsize size = env->GetArrayLength(valueArray);
            std::vector<std::string> btValues;

            for (jint i = 0; i < size; ++i) {
                jstring value = static_cast<jstring>(env->GetObjectArrayElement(valueArray, i));
                btValues.push_back(ConvertJStringToCString(env, value));
            }

            std::string descriptorString = ConvertJStringToCString(env, btKey);
            std::transform(descriptorString.begin(), descriptorString.end(), descriptorString.begin(), ::toupper);
            gattClientCharacteristicMap[descriptorString] = btValues;
        }
    }
    env->DeleteLocalRef(btHashMapClass);
    env->DeleteLocalRef(btKeySet);
    env->DeleteLocalRef(btKeyIterator);
    return gattClientCharacteristicMap;
}

void BluetoothJni::NativeGattClientSetServices(JNIEnv* env, jobject jobj, jint appId, jstring sJObject,
    jobjectArray cJObjects, jobject clientCharacteristicMap)
{
    CHECK_NULL_VOID(env);
    std::string serviceString;
    auto transformData = env->GetStringUTFChars(sJObject, nullptr);
    if (transformData != nullptr) {
        serviceString = transformData;
        env->ReleaseStringUTFChars(sJObject, transformData);
    }
    std::vector<std::string> characterString;
    jsize length = env->GetArrayLength(cJObjects);
    for (jsize i = 0; i < length; ++i) {
        jstring jstr = static_cast<jstring>(env->GetObjectArrayElement(cJObjects, i));
        if (jstr != nullptr) {
            const char* transformChars = env->GetStringUTFChars(jstr, nullptr);
            std::string str(transformChars);
            env->ReleaseStringUTFChars(jstr, transformChars);
            characterString.push_back(str);
        }
        env->DeleteLocalRef(jstr);
    }
    std::map<std::string, std::vector<std::string>> gattClientCharacteristicMap =
        ConvertJMapToCMap(env, clientCharacteristicMap);
    bluetooth::Service service;
    BluetoothImplUtils::ParseGattServiceFromJson(serviceString, service);
    BluetoothGattService btService(std::move(service));
    for (auto& characteristicIter : characterString) {
        bluetooth::Characteristic characteristic;
        BluetoothImplUtils::ParseCharacteristicFromJson(characteristicIter, characteristic);
        auto it = gattClientCharacteristicMap.find(characteristic.uuid_.ToString());
        if (it != gattClientCharacteristicMap.end()) {
            for (auto &descriptorIter : it->second) {
                bluetooth::Descriptor descriptor;
                BluetoothImplUtils::ParseDescriptorFromJson(descriptorIter, descriptor);
                characteristic.descriptors_.push_back(descriptor);
            }
        }
        btService.characteristics_.push_back(characteristic);
    }
    BluetoothGattClientImpl::SetHandle(appId, btService);
    std::lock_guard<std::mutex> lock(gattClientServiceLock_);
    auto gattClientData = BluetoothGattClientImpl::GetGattClientData(static_cast<int32_t>(appId));
    gattClientData->services_.push_back(std::move(btService));
}

void BluetoothJni::NativeOnMtuChanged(JNIEnv* env, jobject jobj, jint appId, jint mtu, jint state)
{
    HILOGI("BluetoothJni NativeOnMtuChanged state: %{public}d, mtu: %{public}d", state, mtu);
    CHECK_NULL_VOID(env);
    auto gattClientData = BluetoothGattClientImpl::GetGattClientData(static_cast<int32_t>(appId));
    if (gattClientData == nullptr || gattClientData->callback_ == nullptr) {
        HILOGE("gattClientData or gattClientDataCallback is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(gattClientLock_);
    gattClientData->mtu_ = static_cast<size_t>(mtu);
    gattClientData->callback_->OnMtuChanged(static_cast<int32_t>(state), static_cast<int32_t>(mtu));
}

int BluetoothJni::StartScan(const int32_t scannerId, const BluetoothBleScanSettings& settings,
    const std::vector<BluetoothBleScanFilter>& filters)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.startScan, BtErrCode::BT_ERR_INTERNAL_ERROR);

    jobjectArray filtersDataJArray = env->NewObjectArray(filters.size(), env->FindClass("java/lang/String"), nullptr);
    for (size_t i = 0; i < filters.size(); ++i) {
        jstring filterDataJString =
            env->NewStringUTF(BluetoothImplUtils::ConvertScanFilterToJson(filters[i]).dump().c_str());
        env->SetObjectArrayElement(filtersDataJArray, i, filterDataJString);
        env->DeleteLocalRef(filterDataJString);
    }

    Json settingsDataJson = {
        { "reportDelayMillis", settings.GetReportDelayMillisValue() },
        { "scanMode", settings.GetScanMode() },
        { "legacy", settings.GetLegacy() },
        { "phy", settings.GetPhy() },
        { "callbackType", settings.GetCallbackType() },
    };
    jstring settingsDataJString = env->NewStringUTF(settingsDataJson.dump().c_str());

    jint ret = env->CallIntMethod(
        g_bluetoothClass.globalRef, g_bluetoothClass.startScan, settingsDataJString, filtersDataJArray, scannerId);

    env->DeleteLocalRef(filtersDataJArray);
    env->DeleteLocalRef(settingsDataJString);
    return ret;
}

int BluetoothJni::StopScan(const int32_t scannerId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.stopScan, BtErrCode::BT_ERR_INTERNAL_ERROR);
    jint ret = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.stopScan, scannerId);
    return ret;
}

int BluetoothJni::StartAdvertising(const BluetoothBleAdvertiserSettings& settings,
    const BluetoothBleAdvertiserData& advData, const BluetoothBleAdvertiserData& scanResponse, int32_t advHandle,
    uint16_t duration, bool isRawData)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.startAdvertising, BtErrCode::BT_ERR_INTERNAL_ERROR);

    jstring settingsDataJsonString = env->NewStringUTF(
        BluetoothImplUtils::ConvertAdvertiserSettingToJson(settings).dump().c_str());
    jstring advDataJsonString =
        env->NewStringUTF(BluetoothImplUtils::ConvertAdvertiserDataToJson(advData).dump().c_str());
    jstring scanResponseDataJsonString =
        env->NewStringUTF(BluetoothImplUtils::ConvertAdvertiserDataToJson(scanResponse).dump().c_str());

    int ret = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.startAdvertising, settingsDataJsonString,
        advDataJsonString, scanResponseDataJsonString, static_cast<int>(advHandle), isRawData);

    env->DeleteLocalRef(settingsDataJsonString);
    env->DeleteLocalRef(advDataJsonString);
    env->DeleteLocalRef(scanResponseDataJsonString);
    return ret;
}

int BluetoothJni::StopAdvertising(int32_t advHandle)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.stopAdvertising, BtErrCode::BT_ERR_INTERNAL_ERROR);
    int ret =
        env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.stopAdvertising, static_cast<int>(advHandle));
    return ret;
}

int32_t BluetoothJni::StartPair(const std::string& address)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.startPair, BT_ERR_INTERNAL_ERROR);

    jstring javaString = env->NewStringUTF(address.c_str());
    jint retJint = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.startPair, javaString);
    env->DeleteLocalRef(javaString);
    return static_cast<int>(retJint);
}

int32_t BluetoothJni::GetBtProfileConnState(uint32_t profileId, int& state)
{
    int ret = BT_NO_ERROR;
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.getBtProfileConnState, BT_ERR_INTERNAL_ERROR);

    int profileIdBT = -1;
    ret = BluetoothImplUtils::GetBluetoothProfileFromOhProfile(static_cast<int>(profileId), profileIdBT);
    if (ret != BT_NO_ERROR) {
        return BT_ERR_INVALID_PARAM;
    }

    state = static_cast<int>(BTConnectState::DISCONNECTED);
    jintArray jIntArray = env->NewIntArray(1);
    jint retJint =
        env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.getBtProfileConnState, profileIdBT, jIntArray);
    ret = static_cast<int>(retJint);
    if (ret == BT_NO_ERROR) {
        jint* cArray = env->GetIntArrayElements(jIntArray, NULL);
        if (cArray != NULL) {
            int err = BluetoothImplUtils::GetOhHostBtStateFromBluetoothAdapter(static_cast<int>(cArray[0]), state);
            ret = (err == BT_NO_ERROR) ? BT_NO_ERROR : BT_ERR_INTERNAL_ERROR;
            env->ReleaseIntArrayElements(jIntArray, cArray, 0);
        } else {
            ret = BT_ERR_INTERNAL_ERROR;
        }
    }
    env->DeleteLocalRef(jIntArray);
    return ret;
}

int32_t BluetoothJni::GetBtProfileConnState(int& state)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.getBtProfilesConnState, BT_ERR_INTERNAL_ERROR);

    int stateTemp = static_cast<int>(BTConnectState::DISCONNECTED);
    int ret = BT_NO_ERROR;

    jintArray jIntArrayProfile = env->NewIntArray(2);
    jint* cArrayProfile = env->GetIntArrayElements(jIntArrayProfile, NULL);
    if (cArrayProfile != NULL) {
        cArrayProfile[0] = BluetoothProfileConstants::A2DP;
        cArrayProfile[1] = BluetoothProfileConstants::HEADSET;
        env->ReleaseIntArrayElements(jIntArrayProfile, cArrayProfile, 0);
    } else {
        ret = BT_ERR_INTERNAL_ERROR;
    }

    if (ret == BT_NO_ERROR) {
        jintArray jIntArray = env->NewIntArray(1);
        jint retJint = env->CallIntMethod(
            g_bluetoothClass.globalRef, g_bluetoothClass.getBtProfilesConnState, jIntArrayProfile, jIntArray);
        ret = static_cast<int>(retJint);
        if (ret == BT_NO_ERROR) {
            jint* cArray = env->GetIntArrayElements(jIntArray, NULL);
            if (cArray != NULL) {
                BluetoothImplUtils::GetOhHostBtStateFromBluetoothAdapter(static_cast<int>(cArray[0]), stateTemp);
                env->ReleaseIntArrayElements(jIntArray, cArray, 0);
            }
        }
        env->DeleteLocalRef(jIntArray);
    }

    state = (static_cast<int>(BTConnectState::CONNECTED) == stateTemp) ? static_cast<int>(BTConnectState::CONNECTED)
                                                                    : static_cast<int>(BTConnectState::DISCONNECTED);
    env->DeleteLocalRef(jIntArrayProfile);
    return ret;
}

int32_t BluetoothJni::GetDeviceName(const std::string& address, std::string& name)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.getDeviceName, BT_ERR_INTERNAL_ERROR);

    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray javaArray = env->NewObjectArray(1, stringClass, nullptr);
    jstring javaString = env->NewStringUTF(address.c_str());
    jint retJint =
        env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.getDeviceName, javaString, javaArray);
    int ret = static_cast<int>(retJint);
    if (ret == BT_NO_ERROR) {
        jstring stringElement = (jstring)env->GetObjectArrayElement(javaArray, 0);
        const char* pStringElement = env->GetStringUTFChars(stringElement, nullptr);
        if (pStringElement != nullptr) {
            name = pStringElement;
            env->ReleaseStringUTFChars(stringElement, pStringElement);
        } else {
            ret = BT_ERR_INTERNAL_ERROR;
        }
        env->DeleteLocalRef(stringElement);
    }
    env->DeleteLocalRef(javaString);
    env->DeleteLocalRef(javaArray);
    return ret;
}

int32_t BluetoothJni::GetLocalName(std::string& name)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.getLocalName, BT_ERR_INTERNAL_ERROR);

    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray javaArray = env->NewObjectArray(1, stringClass, nullptr);
    jint retJint = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.getLocalName, javaArray);
    int ret = static_cast<int>(retJint);
    if (ret == BT_NO_ERROR) {
        jstring stringElement = (jstring)env->GetObjectArrayElement(javaArray, 0);
        const char* pStringElement = env->GetStringUTFChars(stringElement, nullptr);
        if (pStringElement != nullptr) {
            name = pStringElement;
            env->ReleaseStringUTFChars(stringElement, pStringElement);
        } else {
            ret = BT_ERR_INTERNAL_ERROR;
        }
        env->DeleteLocalRef(stringElement);
    }
    env->DeleteLocalRef(javaArray);
    return ret;
}

int32_t BluetoothJni::GetPairedDevices(std::vector<BluetoothRawAddress>& pairedAddr)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.getPairedDevices, BT_ERR_INTERNAL_ERROR);

    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray javaArray = env->NewObjectArray(1, stringClass, nullptr);
    jint retJint = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.getPairedDevices, javaArray);
    int ret = static_cast<int>(retJint);
    if (ret == BT_NO_ERROR) {
        jstring stringElement = (jstring)env->GetObjectArrayElement(javaArray, 0);
        const char* pStringElement = env->GetStringUTFChars(stringElement, nullptr);
        if (pStringElement != nullptr) {
            std::string jsonString = pStringElement;
            env->ReleaseStringUTFChars(stringElement, pStringElement);
            Json jsonData = Json::parse(jsonString);
            std::vector<std::string> jsonVector = jsonData.get<std::vector<std::string>>();
            for (auto jsonElement : jsonVector) {
                BluetoothRawAddress device(jsonElement);
                pairedAddr.push_back(device);
            }
        } else {
            ret = BT_ERR_INTERNAL_ERROR;
        }
        env->DeleteLocalRef(stringElement);
    }
    env->DeleteLocalRef(javaArray);
    return ret;
}

int32_t BluetoothJni::GetPairState(const std::string& address, int32_t& pairState)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.getPairState, BT_ERR_INTERNAL_ERROR);

    pairState = PAIR_NONE;
    jstring javaAddrString = env->NewStringUTF(address.c_str());
    jintArray jIntArray = env->NewIntArray(1);
    jint retJint =
        env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.getPairState, javaAddrString, jIntArray);
    int ret = static_cast<int>(retJint);
    if (ret == BT_NO_ERROR) {
        jint* cArray = env->GetIntArrayElements(jIntArray, NULL);
        if (cArray != NULL) {
            int err = BluetoothImplUtils::GetOhPairStateFromBluetoothAdapter(static_cast<int>(cArray[0]), pairState);
            ret = (RET_NO_ERROR == err) ? BT_NO_ERROR : BT_ERR_INVALID_PARAM;
            env->ReleaseIntArrayElements(jIntArray, cArray, 0);
        } else {
            ret = BT_ERR_INTERNAL_ERROR;
        }
    }
    env->DeleteLocalRef(javaAddrString);
    env->DeleteLocalRef(jIntArray);
    return ret;
}

int32_t BluetoothJni::GetDeviceClass(const std::string& address, int& cod)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.getDeviceClass, BT_ERR_INTERNAL_ERROR);

    jstring javaAddrString = env->NewStringUTF(address.c_str());
    jintArray jIntArray = env->NewIntArray(1);
    jint retJint =
        env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.getDeviceClass, javaAddrString, jIntArray);
    int ret = static_cast<int>(retJint);
    if (ret == BT_NO_ERROR) {
        jint* cArray = env->GetIntArrayElements(jIntArray, NULL);
        if (cArray != NULL) {
            cod = static_cast<int>(cArray[0]);
            env->ReleaseIntArrayElements(jIntArray, cArray, 0);
        } else {
            ret = BT_ERR_INTERNAL_ERROR;
        }
    }
    env->DeleteLocalRef(javaAddrString);
    env->DeleteLocalRef(jIntArray);
    return ret;
}

int32_t BluetoothJni::GetDeviceUuids(const std::string& address, std::vector<std::string>& uuids)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.getDeviceUuids, BT_ERR_INTERNAL_ERROR);

    jstring javaAddrString = env->NewStringUTF(address.c_str());
    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray javaArray = env->NewObjectArray(1, stringClass, nullptr);
    jint retJint =
        env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.getDeviceUuids, javaAddrString, javaArray);
    int ret = static_cast<int>(retJint);
    if (ret == BT_NO_ERROR) {
        jstring stringElement = (jstring)env->GetObjectArrayElement(javaArray, 0);
        const char* pStringElement = env->GetStringUTFChars(stringElement, nullptr);
        if (pStringElement == nullptr) {
            env->DeleteLocalRef(stringElement);
            env->DeleteLocalRef(javaAddrString);
            env->DeleteLocalRef(javaArray);
            return BT_ERR_INTERNAL_ERROR;
        }
        std::string jsonString = pStringElement;
        env->ReleaseStringUTFChars(stringElement, pStringElement);
        Json jsonData = Json::parse(jsonString);
        std::vector<std::string> uuidsTemp = jsonData.get<std::vector<std::string>>();
        for (auto uuid : uuidsTemp) {
            if (!BluetoothImplUtils::GetOhUuidFromBluetoothuuid(uuid).empty()) {
                uuids.push_back(uuid);
            }
        }
        env->DeleteLocalRef(stringElement);
    }
    env->DeleteLocalRef(javaAddrString);
    env->DeleteLocalRef(javaArray);
    return ret;
}

int32_t BluetoothJni::SetLocalName(const std::string& name)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.setLocalName, BT_ERR_INTERNAL_ERROR);

    jstring javaString = env->NewStringUTF(name.c_str());
    int ret = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.setLocalName, javaString);
    env->DeleteLocalRef(javaString);
    return ret;
}

int32_t BluetoothJni::GetBtScanMode(int32_t& scanMode)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.getBtScanMode, BT_ERR_INTERNAL_ERROR);

    jintArray jIntArray = env->NewIntArray(1);
    int ret = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.getBtScanMode, jIntArray);
    if (ret == BT_NO_ERROR) {
        jint* pArray = env->GetIntArrayElements(jIntArray, NULL);
        CHECK_NULL_RETURN(pArray, BT_ERR_INTERNAL_ERROR);
        scanMode = pArray[0];
    }
    env->DeleteLocalRef(jIntArray);
    return ret;
}

int32_t BluetoothJni::StartBtDiscovery()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.startBtDiscovery, BT_ERR_INTERNAL_ERROR);
    return env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.startBtDiscovery);
}

int32_t BluetoothJni::CancelBtDiscovery()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.cancelBtDiscovery, BT_ERR_INTERNAL_ERROR);
    return env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.cancelBtDiscovery);
}

int32_t BluetoothJni::IsBtDiscovering(bool& isDisCovering)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.isBtDiscovering, BT_ERR_INTERNAL_ERROR);

    jbooleanArray jBoolArray = env->NewBooleanArray(1);
    int ret = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.isBtDiscovering, jBoolArray);
    if (ret == BT_NO_ERROR) {
        jboolean* pArray = env->GetBooleanArrayElements(jBoolArray, NULL);
        CHECK_NULL_RETURN(pArray, BT_ERR_INTERNAL_ERROR);
        isDisCovering = pArray[0];
    }
    env->DeleteLocalRef(jBoolArray);
    return ret;
}

int32_t BluetoothJni::EnableBt()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.enableBt, BT_ERR_INTERNAL_ERROR);
    return env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.enableBt);
}

int32_t BluetoothJni::DisableBt()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.disableBt, BT_ERR_INTERNAL_ERROR);
    return env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.disableBt);
}

int32_t BluetoothJni::GetBtState(int& state)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.getBtState, BT_ERR_INTERNAL_ERROR);

    jintArray jIntArray = env->NewIntArray(1);
    int ret = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.getBtState, jIntArray);
    if (ret == BT_NO_ERROR) {
        jint* pArray = env->GetIntArrayElements(jIntArray, NULL);
        CHECK_NULL_RETURN(pArray, BT_ERR_INTERNAL_ERROR);
        state = pArray[0];
        HILOGI("GetBtState state: %{public}d", state);
    }
    env->DeleteLocalRef(jIntArray);
    return ret;
}

bool BluetoothJni::IsBtEnable()
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.isBtEnable, BT_ERR_INTERNAL_ERROR);
    return env->CallBooleanMethod(g_bluetoothClass.globalRef, g_bluetoothClass.isBtEnable);
}

void BluetoothJni::OnChangeStateCallBack(JNIEnv* env, jobject jobj, jint state)
{
    sptr<IRemoteObject> proxy = BluetoothProfileManager::GetInstance().GetProfileRemote(BLUETOOTH_HOST);
    sptr<BluetoothHostImpl> hostImpl = iface_cast<BluetoothHostImpl>(proxy);
    if (hostImpl) {
        hostImpl->OnChangeStateCallBack(state);
    }
}

void BluetoothJni::OnDiscoveryResultCallBack(
    JNIEnv* env, jobject jobj, jstring address, jint rssi, jstring deviceName, jint deviceClass)
{
    if (address == nullptr || deviceName == nullptr) {
        return;
    }

    CHECK_NULL_VOID(env);
    char* pAddress = (char*)env->GetStringUTFChars(address, 0);
    CHECK_NULL_VOID(pAddress);
    std::string strAddress(pAddress);

    char* pDeviveName = (char*)env->GetStringUTFChars(deviceName, 0);
    CHECK_NULL_VOID(pDeviveName);
    std::string strDeviveName(pDeviveName);

    sptr<IRemoteObject> proxy = BluetoothProfileManager::GetInstance().GetProfileRemote(BLUETOOTH_HOST);
    sptr<BluetoothHostImpl> hostImpl = iface_cast<BluetoothHostImpl>(proxy);
    if (hostImpl) {
        hostImpl->OnDiscoveryResultCallBack(strAddress, rssi, strDeviveName, deviceClass);
    }

    env->ReleaseStringUTFChars(address, pAddress);
    env->ReleaseStringUTFChars(deviceName, pDeviveName);
}

void BluetoothJni::OnPairStatusChangedCallBack(
    JNIEnv* env, jobject jobj, jint transport, jstring address, jint status, jint cause)
{
    if (address == nullptr) {
        return;
    }

    CHECK_NULL_VOID(env);
    char* pAddress = (char*)env->GetStringUTFChars(address, 0);
    CHECK_NULL_VOID(pAddress);
    std::string strAddress(pAddress);

    sptr<IRemoteObject> proxy = BluetoothProfileManager::GetInstance().GetProfileRemote(BLUETOOTH_HOST);
    sptr<BluetoothHostImpl> hostImpl = iface_cast<BluetoothHostImpl>(proxy);
    if (hostImpl) {
        hostImpl->OnPairStatusChangedCallBack(transport, strAddress, status, cause);
    }

    env->ReleaseStringUTFChars(address, pAddress);
}

int32_t BluetoothJni::GetConnectionState(const bluetooth::RawAddress& device, int& state)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.getConnectionState, BT_ERR_INTERNAL_ERROR);

    jstring javaAddrString = env->NewStringUTF(device.GetAddress().c_str());
    jintArray jIntArray = env->NewIntArray(1);
    jint retJint =
        env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.getConnectionState, javaAddrString, jIntArray);
    int ret = static_cast<int>(retJint);
    if (ret == BT_NO_ERROR) {
        jint* cArray = env->GetIntArrayElements(jIntArray, NULL);
        if (cArray != NULL) {
            int err = BluetoothImplUtils::GetOhProfileStateFromBTProfileState(static_cast<int>(cArray[0]), state);
            ret = (err == RET_NO_ERROR) ? BT_NO_ERROR : BT_ERR_INVALID_PARAM;
            env->ReleaseIntArrayElements(jIntArray, cArray, 0);
        } else {
            ret = BT_ERR_INTERNAL_ERROR;
        }
    }
    env->DeleteLocalRef(javaAddrString);
    env->DeleteLocalRef(jIntArray);
    return ret;
}

int32_t BluetoothJni::GetDevicesByStates(
    const std::vector<int32_t>& states, std::vector<bluetooth::RawAddress>& rawAddrs)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.getDevicesByStates, BT_ERR_INTERNAL_ERROR);

    jclass stringClass = env->FindClass("java/lang/String");
    jobjectArray javaArray = env->NewObjectArray(1, stringClass, nullptr);
    jint retJint = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.getDevicesByStates, javaArray);
    int ret = static_cast<int>(retJint);
    if (ret == BT_NO_ERROR) {
        jstring stringElement = (jstring)env->GetObjectArrayElement(javaArray, 0);
        const char* pStringElement = env->GetStringUTFChars(stringElement, nullptr);
        if (pStringElement != nullptr) {
            std::string jsonString = pStringElement;
            env->ReleaseStringUTFChars(stringElement, pStringElement);
            Json jsonData = Json::parse(jsonString);
            std::vector<std::string> jsonVector = jsonData.get<std::vector<std::string>>();
            for (auto jsonElement : jsonVector) {
                bluetooth::RawAddress device(jsonElement);
                rawAddrs.push_back(device);
            }
        } else {
            ret = BT_ERR_INTERNAL_ERROR;
        }
        env->DeleteLocalRef(stringElement);
    }
    env->DeleteLocalRef(javaArray);
    return ret;
}

void BluetoothJni::OnConnectionStateChanged(JNIEnv* env, jobject jobj, jstring address, int state, int cause)
{
    sptr<IRemoteObject> proxy = BluetoothProfileManager::GetInstance().GetProfileRemote(PROFILE_A2DP_SRC);
    sptr<BluetoothA2DPImpl> hostImpl = iface_cast<BluetoothA2DPImpl>(proxy);
    if (!hostImpl) {
        HILOGE("OnConnectionStateChanged]BluetoothA2DPImpl is null.");
        return;
    }

    int profileState = static_cast<int>(BTConnectState::DISCONNECTED);
    int err = BluetoothImplUtils::GetOhProfileStateFromBTProfileState(state, profileState);
    if (RET_NO_ERROR == err) {
        const char* pStringElement = env->GetStringUTFChars(address, nullptr);
        if (pStringElement != nullptr) {
            std::string macAddress = pStringElement;
            env->ReleaseStringUTFChars(address, pStringElement);
            RawAddress device(macAddress);
            hostImpl->OnConnectionStateChanged(device, profileState, cause);
        } else {
            HILOGE("OnConnectionStateChanged GetStringUTFChars error.");
        }
    } else {
        HILOGE("OnConnectionStateChanged GetOhProfileStateFromBTProfileState fail.");
    }
}

int BluetoothJni::RegisterApplicationGattServer(int32_t appId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.registerApplicationGattServer, BT_ERR_INTERNAL_ERROR);
    jint ret = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.registerApplicationGattServer, appId);
    return static_cast<int>(ret);
}

int BluetoothJni::AddService(int32_t appId, BluetoothGattService* services)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.addService, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(services, BT_ERR_INTERNAL_ERROR);
    jstring servicesString = env->NewStringUTF(BluetoothImplUtils::ConvertGattServiceToJson(*services).dump().c_str());
    jint ret = env->CallIntMethod(
        g_bluetoothClass.globalRef, g_bluetoothClass.addService, appId, services->handle_, servicesString);
    env->DeleteLocalRef(servicesString);
    return static_cast<int>(ret);
}

int BluetoothJni::RemoveService(int32_t appId, const BluetoothGattService& services)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.removeService, BT_ERR_INTERNAL_ERROR);
    jint ret = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.removeService, appId, services.handle_);
    return static_cast<int>(ret);
}

int BluetoothJni::GattServerClose(int32_t appId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.gattServerClose, BT_ERR_INTERNAL_ERROR);

    jint ret = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.gattServerClose, appId);
    return static_cast<int>(ret);
}

int BluetoothJni::NotifyCharacteristicChanged(
    int32_t appId, const BluetoothGattDevice& device, BluetoothGattCharacteristic* characteristic, bool needConfirm)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.notifyCharacteristicChanged, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(characteristic, BT_ERR_INTERNAL_ERROR);
    Json characteristicJson = BluetoothImplUtils::ConvertCharacterToJson(*characteristic);
    jstring address = env->NewStringUTF(device.addr_.GetAddress().c_str());
    jstring characteristicString = env->NewStringUTF(characteristicJson.dump().c_str());

    jint ret = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.notifyCharacteristicChanged, appId,
        characteristic->handle_, address, characteristicString, needConfirm);
    env->DeleteLocalRef(address);
    env->DeleteLocalRef(characteristicString);
    return static_cast<int>(ret);
}

int BluetoothJni::RespondCharacteristicRead(
    const int appId, const std::string address, const int32_t status, const std::string value, const size_t length)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.respondCharacteristicRead, BtErrCode::BT_ERR_INTERNAL_ERROR);

    jstring jAddress = env->NewStringUTF(address.c_str());
    jstring jValue = env->NewStringUTF(value.c_str());
    jint res = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.respondCharacteristicRead, jAddress,
        jValue, appId, status, length);

    env->DeleteLocalRef(jAddress);
    env->DeleteLocalRef(jValue);
    return static_cast<int>(res);
}

int BluetoothJni::RespondCharacteristicWrite(
    const int appId, const std::string address, const int32_t status, const uint16_t handle)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.respondCharacteristicWrite, BtErrCode::BT_ERR_INTERNAL_ERROR);

    jstring jAddress = env->NewStringUTF(address.c_str());
    jint res = env->CallIntMethod(
        g_bluetoothClass.globalRef, g_bluetoothClass.respondCharacteristicWrite, jAddress, appId, status, handle);

    env->DeleteLocalRef(jAddress);
    return static_cast<int>(res);
}

int BluetoothJni::RespondDescriptorRead(
    const int appId, const std::string address, const int32_t status, const std::string value, const size_t length)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.respondDescriptorRead, BtErrCode::BT_ERR_INTERNAL_ERROR);

    jstring jAddress = env->NewStringUTF(address.c_str());
    jstring jValue = env->NewStringUTF(value.c_str());

    jint res = env->CallIntMethod(
        g_bluetoothClass.globalRef, g_bluetoothClass.respondDescriptorRead, jAddress, jValue, appId, status, length);

    env->DeleteLocalRef(jAddress);
    env->DeleteLocalRef(jValue);
    return static_cast<int>(res);
}

int BluetoothJni::RespondDescriptorWrite(
    const int appId, const std::string address, const int32_t status, const uint16_t handle)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.respondDescriptorWrite, BtErrCode::BT_ERR_INTERNAL_ERROR);

    jstring jAddress = env->NewStringUTF(address.c_str());
    jint res = env->CallIntMethod(
        g_bluetoothClass.globalRef, g_bluetoothClass.respondDescriptorWrite, jAddress, appId, status, handle);

    env->DeleteLocalRef(jAddress);
    return static_cast<int>(res);
}

int32_t BluetoothJni::GattClientConnect(const int32_t appId, std::string& address, bool autoConnect)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.gattClientConnect, BtErrCode::BT_ERR_INTERNAL_ERROR);

    std::transform(address.begin(), address.end(), address.begin(), ::toupper);
    jstring jAddress = env->NewStringUTF(address.c_str());
    jboolean jAutoConnect = autoConnect ? JNI_TRUE : JNI_FALSE;
    jint ret = env->CallIntMethod(
        g_bluetoothClass.globalRef, g_bluetoothClass.gattClientConnect, appId, jAddress, jAutoConnect);
    env->DeleteLocalRef(jAddress);
    return static_cast<int32_t>(ret);
}

int32_t BluetoothJni::GattClientDisconnect(const int32_t appId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.gattClientDisconnect, BtErrCode::BT_ERR_INTERNAL_ERROR);

    jint ret = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.gattClientDisconnect, appId);
    return static_cast<int32_t>(ret);
}

int32_t BluetoothJni::RequestExchangeMtu(int32_t appId, int32_t mtu)
{
    HILOGI("The mtu is %{public}d", mtu);
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.gattClientRequestExchangeMtu, BT_ERR_INTERNAL_ERROR);
    jint retJint =
        env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.gattClientRequestExchangeMtu, appId, mtu);
    return static_cast<int>(retJint);
}

int32_t BluetoothJni::GattClientClose(int32_t appId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.gattClientClose, BtErrCode::BT_ERR_INTERNAL_ERROR);

    jint ret = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.gattClientClose, appId);
    return static_cast<int32_t>(ret);
}

int32_t BluetoothJni::ClientReadCharacter(const int32_t appId, BluetoothGattCharacteristic& characteristic)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.clientReadCharacter, BtErrCode::BT_ERR_INTERNAL_ERROR);
    auto sUuidString = BluetoothGattClientImpl::GetServiceUuidByHandle(appId, characteristic.handle_);
    auto cUuidString = BluetoothGattClientImpl::GetCharacterUuidByHandle(appId, characteristic.handle_);
    if (sUuidString.empty() || cUuidString.empty()) {
        return BtErrCode::BT_ERR_INTERNAL_ERROR;
    }

    jstring jServiceUuid = env->NewStringUTF(sUuidString.c_str());
    jstring jCharacterUuid = env->NewStringUTF(cUuidString.c_str());
    jint result = env->CallIntMethod(
        g_bluetoothClass.globalRef, g_bluetoothClass.clientReadCharacter, appId, jServiceUuid, jCharacterUuid);

    env->DeleteLocalRef(jServiceUuid);
    env->DeleteLocalRef(jCharacterUuid);
    return static_cast<int32_t>(result);
}

int32_t BluetoothJni::ClientWriteCharacter(
    const int32_t appId, BluetoothGattCharacteristic& characteristic, const int32_t writeType)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.clientWriteCharacter, BtErrCode::BT_ERR_INTERNAL_ERROR);

    auto sUuidString = BluetoothGattClientImpl::GetServiceUuidByHandle(appId, characteristic.handle_);
    auto cUuidString = BluetoothGattClientImpl::GetCharacterUuidByHandle(appId, characteristic.handle_);
    if (sUuidString.empty() || cUuidString.empty()) {
        return BtErrCode::BT_ERR_INTERNAL_ERROR;
    }
    jstring jServiceUuid = env->NewStringUTF(sUuidString.c_str());
    jstring jCharacterUuid = env->NewStringUTF(cUuidString.c_str());
    auto length = characteristic.length_;
    auto value = characteristic.value_.get();
    jbyteArray newByte = env->NewByteArray(characteristic.length_);
    env->SetByteArrayRegion(newByte, 0, length, (jbyte *)value);
    jint ret = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.clientWriteCharacter, appId,
        jServiceUuid, jCharacterUuid, newByte, writeType);

    env->DeleteLocalRef(jServiceUuid);
    env->DeleteLocalRef(jCharacterUuid);
    env->DeleteLocalRef(newByte);
    return static_cast<int32_t>(ret);
}

int32_t BluetoothJni::ClientWriteDescriptor(const int32_t appId, BluetoothGattDescriptor& descriptor)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.clientWriteDescriptor, BtErrCode::BT_ERR_INTERNAL_ERROR);

    auto sUuidString = BluetoothGattClientImpl::GetServiceUuidByHandle(appId, descriptor.handle_);
    auto cUuidString = BluetoothGattClientImpl::GetCharacterUuidByHandle(appId, descriptor.handle_);
    auto dUuidString = BluetoothGattClientImpl::GetDescriptorUuidByHandle(appId, descriptor.handle_);
    if (sUuidString.empty() || cUuidString.empty() || dUuidString.empty()) {
        return BtErrCode::BT_ERR_INTERNAL_ERROR;
    }
    jstring jServiceUuid = env->NewStringUTF(sUuidString.c_str());
    jstring jCharacterUuid = env->NewStringUTF(cUuidString.c_str());
    jstring jDescriptorUuid = env->NewStringUTF(dUuidString.c_str());
    auto length = descriptor.length_;
    auto value = descriptor.value_.get();
    jbyteArray newByte = env->NewByteArray(descriptor.length_);
    env->SetByteArrayRegion(newByte, 0, length, (jbyte *)value);
    jint ret = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.clientWriteDescriptor,
        appId, jServiceUuid, jCharacterUuid, jDescriptorUuid, newByte);

    env->DeleteLocalRef(jServiceUuid);
    env->DeleteLocalRef(jCharacterUuid);
    env->DeleteLocalRef(jDescriptorUuid);
    env->DeleteLocalRef(newByte);
    return static_cast<int32_t>(ret);
}

int32_t BluetoothJni::ClientReadDescriptor(const int32_t appId, BluetoothGattDescriptor& descriptor)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.clientReadDescriptor, BtErrCode::BT_ERR_INTERNAL_ERROR);

    auto sUuidString = BluetoothGattClientImpl::GetServiceUuidByHandle(appId, descriptor.handle_);
    auto cUuidString = BluetoothGattClientImpl::GetCharacterUuidByHandle(appId, descriptor.handle_);
    auto dUuidString = BluetoothGattClientImpl::GetDescriptorUuidByHandle(appId, descriptor.handle_);
    if (sUuidString.empty() || cUuidString.empty() || dUuidString.empty()) {
        return BtErrCode::BT_ERR_INTERNAL_ERROR;
    }
    jstring jServiceUuid = env->NewStringUTF(sUuidString.c_str());
    jstring jCharacterUuid = env->NewStringUTF(cUuidString.c_str());
    jstring jDescriptorUuid = env->NewStringUTF(dUuidString.c_str());
    jint result = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.clientReadDescriptor, appId,
        jServiceUuid, jCharacterUuid, jDescriptorUuid);

    env->DeleteLocalRef(jServiceUuid);
    env->DeleteLocalRef(jCharacterUuid);
    env->DeleteLocalRef(jDescriptorUuid);
    return static_cast<int32_t>(result);
}

int32_t BluetoothJni::ClientDiscoverServices(const int32_t appId)
{
    auto env = ARKUI_X_Plugin_GetJniEnv();
    CHECK_NULL_RETURN(env, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.globalRef, BtErrCode::BT_ERR_INTERNAL_ERROR);
    CHECK_NULL_RETURN(g_bluetoothClass.clientDiscoverServices, BtErrCode::BT_ERR_INTERNAL_ERROR);
    jint result = env->CallIntMethod(g_bluetoothClass.globalRef, g_bluetoothClass.clientDiscoverServices, appId);
    return static_cast<int32_t>(result);
}
} // namespace OHOS::Bluetooth
