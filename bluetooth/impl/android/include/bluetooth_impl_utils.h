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

#ifndef PLUGINS_BLUETOOTH_IMPL_ANDROID_INCLUDE_BLUETOOTH_IMPL_UTILS_H
#define PLUGINS_BLUETOOTH_IMPL_ANDROID_INCLUDE_BLUETOOTH_IMPL_UTILS_H

#include <list>
#include <map>
#include <nlohmann/json.hpp>

#include "bluetooth_ble_advertiser_data.h"
#include "bluetooth_ble_advertiser_settings.h"
#include "bluetooth_ble_scan_filter.h"
#include "bluetooth_def.h"
#include "bt_def.h"
#include "i_bluetooth_gatt_server.h"

namespace OHOS {
namespace Bluetooth {
using Json = nlohmann::json;
class BluetoothImplUtils {
public:
    static int GetBluetoothProfileFromOhProfile(uint32_t profileId, int& profile);
    static int GetBluetoothTransportTypeFromBluetoothAdapter(int value, uint8_t& transportType);
    static int GetOhHostBtStateFromBluetoothAdapter(int value, int& btState);
    static int GetOhHostBtStateIDFromBluetoothAdapter(int value, int& btState);
    static int GetOhPairStateFromBluetoothAdapter(int value, int& btState);
    static int GetOhProfileStateFromBTProfileState(int value, int& btState);
    static int GetOhScanModeFromBluetoothAdapter(int value, int& btScanMode);
    static std::string GetOhUuidFromBluetoothuuid(std::string uuid);

    static Json ConvertAdvertiserDataToJson(const BluetoothBleAdvertiserData& advData);
    static Json ConvertAdvertiserSettingToJson(const BluetoothBleAdvertiserSettings& settings);
    static Json ConvertCharacterToJson(const BluetoothGattCharacteristic& characteristic);
    static Json ConvertGattServiceToJson(const BluetoothGattService& services);
    static Json ConvertScanFilterToJson(const BluetoothBleScanFilter& filter);

    static bluetooth::Characteristic ConvertJsonToCharacteristic(Json characteristicData);
    static bluetooth::Descriptor ConvertJsonToDescriptor(Json descriptorData);
    static bluetooth::GattDevice ConvertJsonToGattDevice(Json deviceData);

    static bool CheckServicesIsLegality(const BluetoothGattService& service);
    static bool CalculateAndAssignHandle(const int applicationId,
        std::map<int32_t, std::list<std::pair<uint16_t, uint16_t>>>& handleMap, BluetoothGattService& service);
    static void ReleaseHandle(const int applicationId,
        std::map<int32_t, std::list<std::pair<uint16_t, uint16_t>>>& handleMap, const BluetoothGattService& service);

    static void ParseGattServiceFromJson(const std::string& jsonString, bluetooth::Service& service);
    static void ParseCharacteristicFromJson(const std::string& jsonString, bluetooth::Characteristic& characteristic);
    static void ParseDescriptorFromJson(const std::string& jsonString, bluetooth::Descriptor& descriptor);
};

class BluetoothCtrlJniRegister {
public:
    static void AbilityBluetoothCtrlJniInit();

private:
    static bool isInited_;
};

class BluetoothAdapterConstants {
public:
    static const int STATE_CONNECTED;
    static const int STATE_CONNECTING;
    static const int STATE_DISCONNECTED;
    static const int STATE_DISCONNECTING;
    static const int STATE_OFF;
    static const int STATE_ON;
    static const int STATE_TURNING_OFF;
    static const int STATE_TURNING_ON;
    static const int PAIR_STATE_BOND_NONE;
    static const int PAIR_STATE_BOND_BONDING;
    static const int PAIR_STATE_BOND_BONDED;
    static const int SCAN_MODE_NONE;
    static const int SCAN_MODE_CONNECTABLE;
    static const int SCAN_MODE_CONNECTABLE_DISCOVERABLE;
    static const int TRANSPORT_AUTO;
    static const int TRANSPORT_BREDR;
    static const int TRANSPORT_LE;
};

class BluetoothProfileConstants {
public:
    static const int A2DP;
    static const int HEADSET;
};

class ProfileUUIDConstants {
public:
    static const std::string PROFILE_UUID_BASE_UUID;
    static const std::string PROFILE_UUID_HFP_AG;
    static const std::string PROFILE_UUID_HFP_HF;
    static const std::string PROFILE_UUID_HSP_AG;
    static const std::string PROFILE_UUID_HSP_HS;
    static const std::string PROFILE_UUID_A2DP_SRC;
    static const std::string PROFILE_UUID_A2DP_SINK;
    static const std::string PROFILE_UUID_AVRCP_CT;
    static const std::string PROFILE_UUID_AVRCP_TG;
    static const std::string PROFILE_UUID_HID;
    static const std::string PROFILE_UUID_HOGP;
    static const std::string PROFILE_UUID_PBAP_PSE;
};

class BluetoothProfileStateConstants {
public:
    static const int PROFILE_STATE_DISCONNECTED;
    static const int PROFILE_STATE_CONNECTING;
    static const int PROFILE_STATE_CONNECTED;
    static const int PROFILE_STATE_DISCONNECTING;
};

class BluetoothBleAdvertiserState {
public:
    static const int ALREADY_START;
    static const int INTERNAL;
};
} // namespace Bluetooth
} // namespace OHOS
#endif // PLUGINS_BLUETOOTH_IMPL_ANDROID_INCLUDE_BLUETOOTH_IMPL_UTILS_H