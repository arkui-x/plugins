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

#ifndef PLUGINS_BLUETOOTH_IMPL_ANDROID_JAVA_JNI_BLUETOOTH_JNI_H
#define PLUGINS_BLUETOOTH_IMPL_ANDROID_JAVA_JNI_BLUETOOTH_JNI_H

#include <jni.h>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>

#include "bluetooth_ble_advertiser_data.h"
#include "bluetooth_ble_advertiser_settings.h"
#include "bluetooth_ble_scan_filter.h"
#include "bluetooth_ble_scan_settings.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_gatt_server_stub.h"
#include "bluetooth_raw_address.h"
#include "bt_uuid.h"
#include "gatt_data.h"
#include "i_bluetooth_ble_central_manager_callback.h"
#include "i_bluetooth_gatt_server.h"
#include "i_bluetooth_gatt_server_callback.h"
#include "iremote_stub.h"

namespace OHOS::Bluetooth {
using Json = nlohmann::json;
class BluetoothJni final {
public:
    BluetoothJni() = delete;
    ~BluetoothJni() = delete;
    static void BluetoothJniRegister();
    static bool Register(void* env);

    // JAVA call C++
    static void NativeInit(JNIEnv* env, jobject jobj);
    static void NativeBleInit(JNIEnv* env, jobject jobj);
    static void NativeAccessInit(JNIEnv* env, jobject jobj);
    static void NativeConnectionInit(JNIEnv* env, jobject jobj);
    static void NativeBaseProfileInit(JNIEnv* env, jobject jobj);
    static void NativeGattServerInit(JNIEnv* env, jobject jobj);
    static void NativeGattClientInit(JNIEnv* env, jobject jobj);

    static void NativeOnScanResult(JNIEnv* env, jobject jobj, jstring jsonString, int scannerId);
    static void NativeOnStartAdvertisingResult(JNIEnv* env, jobject jobj, int errCode, int advHandle, int opCode);
    static void OnChangeStateCallBack(JNIEnv* env, jobject jobj, jint state);
    static void OnDiscoveryResultCallBack(
        JNIEnv* env, jobject jobj, jstring address, jint rssi, jstring deviceName, jint deviceClass);
    static void OnPairStatusChangedCallBack(
        JNIEnv* env, jobject jobj, jint transport, jstring address, jint status, jint cause);
    static void OnConnectionStateChanged(JNIEnv* env, jobject jobj, jstring address, int state, int cause);
    static void NativeServerOnMtuChangedCallback(JNIEnv* env, jobject jobj, jstring deviceData, int mtu, int appId);
    static void NativeServerOnConnectionStateChangeCallback(
        JNIEnv* env, jobject jobj, jstring deviceData, int newState, int appId);
    static void NativeServerOnCharacteristicReadRequestCallback(
        JNIEnv* env, jobject jobj, jstring deviceData, jstring characteristicData, int appId);
    static void NativeServerOnCharacteristicWriteRequestCallback(
        JNIEnv* env, jobject jobj, jstring deviceData, jstring characteristicData, jboolean responseNeeded, int appId);
    static void NativeServerOnDescriptorReadRequestCallback(
        JNIEnv* env, jobject jobj, jstring deviceData, jstring descriptorData, int appId);
    static void NativeServerOnDescriptorWriteRequestCallback(
        JNIEnv* env, jobject jobj, jstring deviceData, jstring descriptorData, int appId);
    static void NativeOnCharacteristicRead(JNIEnv* env, jobject jobj, jint appId, jstring jsonString, jint status);
    static void NativeOnCharacteristicWrite(JNIEnv* env, jobject jobj, jint appId, jstring jsonString, jint status);
    static void NativeOnConnectionStateChanged(JNIEnv* env, jobject jobj, jint appId, jint state, jint newState);
    static void NativeGattClientSetServices(JNIEnv* env, jobject jobj, jint appId, jstring sJObject,
        jobjectArray cJObjects, jobject clientCharacteristicMap);
    static void NativeOnDescriptorRead(JNIEnv* env, jobject jobj, jint appId, jstring jsonString, jint status);
    static void NativeOnMtuChanged(JNIEnv* env, jobject jobj, jint appId, jint mtu, jint state);
    static void NativeOnDescriptorWrite(JNIEnv* env, jobject jobj, jint appId, jstring jsonString, jint status);

    // C++ call JAVA
    static int StartScan(const int32_t scannerId, const BluetoothBleScanSettings& settings,
        const std::vector<BluetoothBleScanFilter>& filters);
    static int StopScan(const int32_t scannerId);
    static int StartAdvertising(const BluetoothBleAdvertiserSettings& settings,
        const BluetoothBleAdvertiserData& advData, const BluetoothBleAdvertiserData& scanResponse, int32_t advHandle,
        uint16_t duration, bool isRawData);
    static int StopAdvertising(int32_t advHandle);

    static int32_t StartPair(const std::string& address);
    static int32_t GetBtProfileConnState(uint32_t profileId, int& state);
    static int32_t GetBtProfileConnState(int& state);
    static int32_t GetDeviceName(const std::string& address, std::string& name);
    static int32_t GetLocalName(std::string& name);
    static int32_t GetPairedDevices(std::vector<BluetoothRawAddress>& pairedAddr);
    static int32_t GetPairState(const std::string& address, int32_t& pairState);
    static int32_t GetDeviceClass(const std::string& address, int& cod);
    static int32_t GetDeviceUuids(const std::string& address, std::vector<std::string>& uuids);
    static int32_t SetLocalName(const std::string& name);
    static int32_t GetBtScanMode(int32_t& scanMode);
    static int32_t StartBtDiscovery();
    static int32_t CancelBtDiscovery();
    static int32_t IsBtDiscovering(bool& isDisCovering);
    static int32_t EnableBt();
    static int32_t DisableBt();
    static int32_t GetBtState(int& state);
    static bool IsBtEnable();
    static int32_t GetConnectionState(const bluetooth::RawAddress& device, int& state);
    static int32_t GetDevicesByStates(const std::vector<int32_t>& states, std::vector<bluetooth::RawAddress>& rawAddrs);

    static int RegisterApplicationGattServer(int32_t appId);
    static int AddService(int32_t appId, BluetoothGattService* services);
    static int RemoveService(int32_t appId, const BluetoothGattService& services);
    static int GattServerClose(int32_t appId);
    static int NotifyCharacteristicChanged(int32_t appId, const BluetoothGattDevice& device,
        BluetoothGattCharacteristic* characteristic, bool needConfirm);
    static int RespondCharacteristicRead(
        const int appId, const std::string address, const int32_t status, const std::string value, const size_t length);
    static int RespondCharacteristicWrite(
        const int appId, const std::string address, const int32_t status, const uint16_t handle);
    static int RespondDescriptorRead(
        const int appId, const std::string address, const int32_t status, const std::string value, const size_t length);
    static int RespondDescriptorWrite(
        const int appId, const std::string address, const int32_t status, const uint16_t handle);

    static int32_t GattClientConnect(const int32_t appId, std::string& address, bool autoConnect);
    static int32_t GattClientDisconnect(const int32_t appId);
    static int32_t GattClientClose(const int32_t appId);
    static int32_t RequestExchangeMtu(const int32_t appId, const int32_t mtu);
    static int32_t ClientReadCharacter(const int32_t appId, BluetoothGattCharacteristic& characteristic);
    static int32_t ClientWriteCharacter(
        const int32_t appId, BluetoothGattCharacteristic& characteristic, const int32_t writeType);
    static int32_t ClientWriteDescriptor(const int32_t appId, BluetoothGattDescriptor& descriptor);
    static int32_t ClientReadDescriptor(const int32_t appId, BluetoothGattDescriptor& descriptor);
    static int32_t ClientDiscoverServices(const int32_t appId);

    static std::string ConvertJStringToCString(JNIEnv* env, jstring jStr);
    static std::map<std::string, std::vector<std::string>> ConvertJMapToCMap(JNIEnv *env, jobject dUuidMap);
public:
    static std::mutex gattClientLock_;
    static std::mutex gattClientServiceLock_;
};
} // namespace OHOS::Bluetooth
#endif // PLUGINS_BLUETOOTH_IMPL_ANDROID_JAVA_JNI_BLUETOOTH_JNI_H
