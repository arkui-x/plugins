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

#include "bluetooth_impl_utils.h"

#include <algorithm>
#include <map>

#include "bluetooth_ble_scan_filter.h"
#include "bluetooth_log.h"
#include "bluetooth_jni.h"
#include "plugin_utils.h"

using namespace OHOS::bluetooth;
namespace OHOS {
namespace Bluetooth {
const int CONVERT_SIZE = 0x2;
const int INVALID_DATA = -1;
const int MAX_VALUE_LENGTH = 256;

const int BluetoothAdapterConstants::STATE_CONNECTED = 2;
const int BluetoothAdapterConstants::STATE_CONNECTING = 1;
const int BluetoothAdapterConstants::STATE_DISCONNECTED = 0;
const int BluetoothAdapterConstants::STATE_DISCONNECTING = 3;
const int BluetoothAdapterConstants::STATE_OFF = 10;
const int BluetoothAdapterConstants::STATE_ON = 12;
const int BluetoothAdapterConstants::STATE_TURNING_OFF = 13;
const int BluetoothAdapterConstants::STATE_TURNING_ON = 11;

const int BluetoothProfileConstants::A2DP = 2;
const int BluetoothProfileConstants::HEADSET = 1;

const int BluetoothAdapterConstants::PAIR_STATE_BOND_NONE = 10;
const int BluetoothAdapterConstants::PAIR_STATE_BOND_BONDING = 11;
const int BluetoothAdapterConstants::PAIR_STATE_BOND_BONDED = 12;

const int BluetoothAdapterConstants::TRANSPORT_AUTO = 0;
const int BluetoothAdapterConstants::TRANSPORT_BREDR = 1;
const int BluetoothAdapterConstants::TRANSPORT_LE = 2;

const int BluetoothBleAdvertiserState::ALREADY_START = 3;
const int BluetoothBleAdvertiserState::INTERNAL = 4;

const int BluetoothAdapterConstants::SCAN_MODE_NONE = 20;
const int BluetoothAdapterConstants::SCAN_MODE_CONNECTABLE = 21;
const int BluetoothAdapterConstants::SCAN_MODE_CONNECTABLE_DISCOVERABLE = 23;

const int BluetoothProfileStateConstants::PROFILE_STATE_DISCONNECTED = 0;
const int BluetoothProfileStateConstants::PROFILE_STATE_CONNECTING = 1;
const int BluetoothProfileStateConstants::PROFILE_STATE_CONNECTED = 2;
const int BluetoothProfileStateConstants::PROFILE_STATE_DISCONNECTING = 3;

const std::string ProfileUUIDConstants::PROFILE_UUID_BASE_UUID = "00000000-0000-1000-8000-00805F9B34FB";
const std::string ProfileUUIDConstants::PROFILE_UUID_HFP_AG = "0000111F-0000-1000-8000-00805F9B34FB";
const std::string ProfileUUIDConstants::PROFILE_UUID_HFP_HF = "0000111E-0000-1000-8000-00805F9B34FB";
const std::string ProfileUUIDConstants::PROFILE_UUID_HSP_AG = "00001112-0000-1000-8000-00805F9B34FB";
const std::string ProfileUUIDConstants::PROFILE_UUID_HSP_HS = "00001108-0000-1000-8000-00805F9B34FB";
const std::string ProfileUUIDConstants::PROFILE_UUID_A2DP_SRC = "0000110A-0000-1000-8000-00805F9B34FB";
const std::string ProfileUUIDConstants::PROFILE_UUID_A2DP_SINK = "0000110B-0000-1000-8000-00805F9B34FB";
const std::string ProfileUUIDConstants::PROFILE_UUID_AVRCP_CT = "0000110E-0000-1000-8000-00805F9B34FB";
const std::string ProfileUUIDConstants::PROFILE_UUID_AVRCP_TG = "0000110C-0000-1000-8000-00805F9B34FB";
const std::string ProfileUUIDConstants::PROFILE_UUID_HID = "00001124-0000-1000-8000-00805F9B34FB";
const std::string ProfileUUIDConstants::PROFILE_UUID_HOGP = "00001812-0000-1000-8000-00805F9B34FB";
const std::string ProfileUUIDConstants::PROFILE_UUID_PBAP_PSE = "0000112f-0000-1000-8000-00805F9B34FB";

bool BluetoothCtrlJniRegister::isInited_ = false;

static const std::map<int, int> BT_STATE_MAP = {
    { BluetoothAdapterConstants::STATE_TURNING_ON, static_cast<int>(BluetoothState::STATE_TURNING_ON) },
    { BluetoothAdapterConstants::STATE_ON, static_cast<int>(BluetoothState::STATE_ON) },
    { BluetoothAdapterConstants::STATE_TURNING_OFF, static_cast<int>(BluetoothState::STATE_TURNING_OFF) },
    { BluetoothAdapterConstants::STATE_OFF, static_cast<int>(BluetoothState::STATE_OFF) },
    { BluetoothAdapterConstants::STATE_CONNECTED, static_cast<int>(BTConnectState::CONNECTED) },
    { BluetoothAdapterConstants::STATE_CONNECTING, static_cast<int>(BTConnectState::CONNECTING) },
    { BluetoothAdapterConstants::STATE_DISCONNECTED, static_cast<int>(BTConnectState::DISCONNECTED) },
    { BluetoothAdapterConstants::STATE_DISCONNECTING, static_cast<int>(BTConnectState::DISCONNECTING) },
};

static const std::map<int, int> BT_STATE_ID_MAP = {
    { BluetoothAdapterConstants::STATE_TURNING_ON, static_cast<int>(BTStateID::STATE_TURNING_ON) },
    { BluetoothAdapterConstants::STATE_ON, static_cast<int>(BTStateID::STATE_TURN_ON) },
    { BluetoothAdapterConstants::STATE_TURNING_OFF, static_cast<int>(BTStateID::STATE_TURNING_OFF) },
    { BluetoothAdapterConstants::STATE_OFF, static_cast<int>(BTStateID::STATE_TURN_OFF) },
    { BluetoothAdapterConstants::STATE_CONNECTED, static_cast<int>(BTConnectState::CONNECTED) },
    { BluetoothAdapterConstants::STATE_CONNECTING, static_cast<int>(BTConnectState::CONNECTING) },
    { BluetoothAdapterConstants::STATE_DISCONNECTED, static_cast<int>(BTConnectState::DISCONNECTED) },
    { BluetoothAdapterConstants::STATE_DISCONNECTING, static_cast<int>(BTConnectState::DISCONNECTING) },
};

static const std::map<uint32_t, int> PROFILE_ID_MAP = {
    { PROFILE_ID_A2DP_SRC, BluetoothProfileConstants::A2DP },
    { PROFILE_ID_HFP_AG, BluetoothProfileConstants::HEADSET }
};

static const std::map<int, int> BT_PAIRSTATE_MAP {
    { BluetoothAdapterConstants::PAIR_STATE_BOND_NONE, PAIR_NONE },
    { BluetoothAdapterConstants::PAIR_STATE_BOND_BONDING, PAIR_PAIRING },
    { BluetoothAdapterConstants::PAIR_STATE_BOND_BONDED, PAIR_PAIRED },
};

static const std::map<std::string, std::string> BT_UUID_MAP = {
    { "00000000-0000-1000-8000-00805F9B34FB", ProfileUUIDConstants::PROFILE_UUID_BASE_UUID },
    { "0000111F-0000-1000-8000-00805F9B34FB", ProfileUUIDConstants::PROFILE_UUID_HFP_AG },
    { "0000111E-0000-1000-8000-00805F9B34FB", ProfileUUIDConstants::PROFILE_UUID_HFP_HF },
    { "00001112-0000-1000-8000-00805F9B34FB", ProfileUUIDConstants::PROFILE_UUID_HSP_AG },
    { "00001108-0000-1000-8000-00805F9B34FB", ProfileUUIDConstants::PROFILE_UUID_HSP_HS },
    { "0000110A-0000-1000-8000-00805F9B34FB", ProfileUUIDConstants::PROFILE_UUID_A2DP_SRC },
    { "0000110B-0000-1000-8000-00805F9B34FB", ProfileUUIDConstants::PROFILE_UUID_A2DP_SINK },
    { "0000110E-0000-1000-8000-00805F9B34FB", ProfileUUIDConstants::PROFILE_UUID_AVRCP_CT },
    { "0000110C-0000-1000-8000-00805F9B34FB", ProfileUUIDConstants::PROFILE_UUID_AVRCP_TG },
    { "00001124-0000-1000-8000-00805F9B34FB", ProfileUUIDConstants::PROFILE_UUID_HID },
    { "00001812-0000-1000-8000-00805F9B34FB", ProfileUUIDConstants::PROFILE_UUID_HOGP },
    { "0000112f-0000-1000-8000-00805F9B34FB", ProfileUUIDConstants::PROFILE_UUID_PBAP_PSE },
};

static const std::map<int, int> BT_SCANMODE_MAP {
    { BluetoothAdapterConstants::SCAN_MODE_NONE, static_cast<int>(SCAN_MODE_NONE) },
    { BluetoothAdapterConstants::SCAN_MODE_CONNECTABLE, static_cast<int>(SCAN_MODE_CONNECTABLE) },
    { BluetoothAdapterConstants::SCAN_MODE_CONNECTABLE_DISCOVERABLE,
        static_cast<int>(SCAN_MODE_CONNECTABLE_GENERAL_DISCOVERABLE) },
};

static const std::map<int, int> PROFILE_STATE_MAP = {
    { BluetoothProfileStateConstants::PROFILE_STATE_DISCONNECTED, static_cast<int>(BTConnectState::DISCONNECTED) },
    { BluetoothProfileStateConstants::PROFILE_STATE_CONNECTING, static_cast<int>(BTConnectState::CONNECTING) },
    { BluetoothProfileStateConstants::PROFILE_STATE_CONNECTED, static_cast<int>(BTConnectState::CONNECTED) },
    { BluetoothProfileStateConstants::PROFILE_STATE_DISCONNECTING, static_cast<int>(BTConnectState::DISCONNECTING) },
};

static const std::map<int, uint8_t> BT_TRANSPORT_MAP {
    { BluetoothAdapterConstants::TRANSPORT_AUTO, GATT_TRANSPORT_TYPE_AUTO },
    { BluetoothAdapterConstants::TRANSPORT_BREDR, GATT_TRANSPORT_TYPE_CLASSIC },
    { BluetoothAdapterConstants::TRANSPORT_LE, GATT_TRANSPORT_TYPE_LE },
};

void BluetoothCtrlJniRegister::AbilityBluetoothCtrlJniInit()
{
    if (isInited_) {
        return;
    }

    const char className[] = "ohos.ace.plugin.bluetoothplugin.BluetoothPlugin";
    ARKUI_X_Plugin_RegisterJavaPlugin(BluetoothJni::Register, className);
    isInited_ = true;
}

int BluetoothImplUtils::GetBluetoothProfileFromOhProfile(uint32_t profileId, int& profile)
{
    auto it = PROFILE_ID_MAP.find(profileId);
    if (it != PROFILE_ID_MAP.end()) {
        profile = it->second;
        return RET_NO_ERROR;
    }

    return RET_NO_SUPPORT;
}

int BluetoothImplUtils::GetBluetoothTransportTypeFromBluetoothAdapter(int value, uint8_t& transportType)
{
    auto it = BT_TRANSPORT_MAP.find(value);
    if (it != BT_TRANSPORT_MAP.end()) {
        transportType = it->second;
        return RET_NO_ERROR;
    }

    return RET_NO_SUPPORT;
}

int BluetoothImplUtils::GetOhHostBtStateFromBluetoothAdapter(int value, int& btState)
{
    auto it = BT_STATE_MAP.find(value);
    if (it != BT_STATE_MAP.end()) {
        btState = it->second;
        return RET_NO_ERROR;
    }

    return RET_NO_SUPPORT;
}

int BluetoothImplUtils::GetOhHostBtStateIDFromBluetoothAdapter(int value, int& btState)
{
    auto it = BT_STATE_ID_MAP.find(value);
    if (it != BT_STATE_ID_MAP.end()) {
        btState = it->second;
        return RET_NO_ERROR;
    }

    return RET_NO_SUPPORT;
}

int BluetoothImplUtils::GetOhPairStateFromBluetoothAdapter(int value, int& btState)
{
    auto it = BT_PAIRSTATE_MAP.find(value);
    if (it != BT_PAIRSTATE_MAP.end()) {
        btState = it->second;
        return RET_NO_ERROR;
    }

    return RET_NO_SUPPORT;
}

std::string BluetoothImplUtils::GetOhUuidFromBluetoothuuid(std::string uuid)
{
    std::string ret = "";
    auto it = BT_UUID_MAP.find(uuid);
    if (it != BT_UUID_MAP.end()) {
        ret = it->second;
    }
    return ret;
}

int BluetoothImplUtils::GetOhScanModeFromBluetoothAdapter(int value, int& btScanMode)
{
    auto it = BT_SCANMODE_MAP.find(value);
    if (it != BT_SCANMODE_MAP.end()) {
        btScanMode = it->second;
        return RET_NO_ERROR;
    }

    return RET_NO_SUPPORT;
}

int BluetoothImplUtils::GetOhProfileStateFromBTProfileState(int value, int& btState)
{
    auto it = PROFILE_STATE_MAP.find(value);
    if (it != PROFILE_STATE_MAP.end()) {
        btState = it->second;
        return RET_NO_ERROR;
    }
    return RET_NO_SUPPORT;
}

bool ConvertVectorData(const std::vector<u_int8_t> data, std::vector<u_int32_t>& dataJson)
{
    for (auto it = data.begin(); it != data.end(); ++it) {
        dataJson.push_back(static_cast<u_int32_t>(*it));
    }
    return dataJson.empty() ? false : true;
}

Json BluetoothImplUtils::ConvertScanFilterToJson(const BluetoothBleScanFilter& filter)
{
    std::vector<u_int32_t> serviceDataJson;
    std::vector<u_int32_t> serviceDataMaskJson;
    std::vector<u_int32_t> manufactureDataJson;
    std::vector<u_int32_t> manufactureDataMaskJson;
    Json result;
    result["deviceId"] = filter.GetDeviceId();
    result["name"] = filter.GetName();
    if (filter.HasServiceUuid()) {
        result["serviceUuid"] = filter.GetServiceUuid().ToString();
    }
    if (filter.HasServiceUuidMask()) {
        result["serviceUuidMask"] = filter.GetServiceUuidMask().ToString();
    }
    if (filter.HasSolicitationUuid()) {
        result["serviceSolicitationUuid"] = filter.GetServiceSolicitationUuid().ToString();
    }
    if (filter.HasSolicitationUuidMask()) {
        result["serviceSolicitationUuidMask"] = filter.GetServiceSolicitationUuidMask().ToString();
    }
    if (ConvertVectorData(filter.GetServiceData(), serviceDataJson)) {
        result["serviceData"] = serviceDataJson;
    }
    if (ConvertVectorData(filter.GetServiceDataMask(), serviceDataMaskJson)) {
        result["serviceDataMask"] = serviceDataMaskJson;
    }
    result["manufactureId"] = filter.GetManufacturerId();
    if (ConvertVectorData(filter.GetManufactureData(), manufactureDataJson)) {
        result["manufactureData"] = manufactureDataJson;
    }
    if (ConvertVectorData(filter.GetManufactureDataMask(), manufactureDataMaskJson)) {
        result["manufactureDataMask"] = manufactureDataMaskJson;
    }
    return result;
}

bluetooth::GattDevice BluetoothImplUtils::ConvertJsonToGattDevice(Json deviceData)
{
    std::string addrData = deviceData["addr"];
    uint8_t transportData;
    GetBluetoothTransportTypeFromBluetoothAdapter((int)deviceData["transport"], transportData);
    auto device = bluetooth::GattDevice(bluetooth::RawAddress(addrData), transportData);
    return device;
}

bluetooth::Characteristic BluetoothImplUtils::ConvertJsonToCharacteristic(Json characteristicData)
{
    int handle = characteristicData["handle"];
    std::string uuidString = characteristicData["uuid"];
    auto uuid = bluetooth::Uuid::ConvertFromString(uuidString);
    std::string valueString = characteristicData["value"];
    size_t length = static_cast<size_t>(characteristicData["length"]);
    uint8_t* value = (length == 0) ? nullptr : reinterpret_cast<uint8_t*>(const_cast<char*>(valueString.c_str()));
    if (handle == INVALID_DATA) {
        handle = 0;
    }
    auto characteristic = bluetooth::Characteristic(handle, value, length);
    characteristic.uuid_ = uuid;
    return characteristic;
}

bluetooth::Descriptor BluetoothImplUtils::ConvertJsonToDescriptor(Json descriptorData)
{
    int handle = descriptorData["handle"];
    std::string uuidString = descriptorData["uuid"];
    auto uuid = bluetooth::Uuid::ConvertFromString(uuidString);
    int permissions = descriptorData["permissions"];
    std::string valueString = descriptorData["value"];
    size_t length = static_cast<size_t>(descriptorData["length"]);
    uint8_t* value = (length == 0) ? nullptr : reinterpret_cast<uint8_t*>(const_cast<char*>(valueString.c_str()));
    if (handle == INVALID_HANDLE) {
        handle = 0;
    }
    auto descriptor = bluetooth::Descriptor(uuid, handle, permissions, value, length);
    return descriptor;
}

bool ConvertByteArray(const uint8_t input[], int length, std::vector<u_int32_t>& dataJson)
{
    for (int i = 0; i < length; ++i) {
        dataJson.push_back(static_cast<u_int32_t>(input[i]));
    }
    return dataJson.empty() ? false : true;
}

Json BluetoothImplUtils::ConvertGattServiceToJson(const BluetoothGattService& services)
{
    Json bleCharacterArray;
    for (auto& character : services.characteristics_) {
        Json bleDesArray;
        for (auto& des : character.descriptors_) {
            std::vector<u_int32_t> descriptorValueJson;
            Json bleDes = {
                { "serviceUuid", services.uuid_.ToString() },
                { "characteristicUuid", character.uuid_.ToString() },
                { "desHandle", des.handle_ },
                { "permissions", des.permissions_ },
                { "descriptorUuid", des.uuid_.ToString() }
            };
            if (ConvertByteArray(des.value_.get(), des.length_, descriptorValueJson)) {
                bleDes["descriptorValue"] = descriptorValueJson;
            }
            bleDesArray.push_back(bleDes);
        }
        Json bleCharacteristic = {
            { "serviceUuid", services.uuid_.ToString() },
            { "characteristicUuid", character.uuid_.ToString() },
            { "characterHandle", character.handle_ },
            { "permissions", character.permissions_ },
            { "descriptors", bleDesArray },
            { "properties", character.properties_ }
        };
        std::vector<u_int32_t> characterValueJson;
        if (ConvertByteArray(character.value_.get(), character.length_, characterValueJson)) {
            bleCharacteristic["characteristicValue"] = characterValueJson;
        }
        bleCharacterArray.push_back(bleCharacteristic);
    }
    Json gattServiceData = {
        { "handle", services.handle_ },
        { "endHandle", services.endHandle_ },
        { "serviceUuid", services.uuid_.ToString() },
        { "isPrimary", services.isPrimary_ },
        { "characteristics", bleCharacterArray }
    };
    return gattServiceData;
}

Json BluetoothImplUtils::ConvertCharacterToJson(const BluetoothGattCharacteristic& characteristic)
{
    Json characterJson;
    std::vector<u_int32_t> characterValueJson;
    if (ConvertByteArray(characteristic.value_.get(), characteristic.length_, characterValueJson)) {
        characterJson["characteristicValue"] = characterValueJson;
    }
    characterJson["characteristicUuid"] = characteristic.uuid_.ToString();
    return characterJson;
}

Json BluetoothImplUtils::ConvertAdvertiserSettingToJson(const BluetoothBleAdvertiserSettings& settings)
{
    Json settingsData;
    settingsData["interval"] = settings.GetInterval();
    settingsData["txPower"] = settings.GetTxPower();
    settingsData["connectable"] = settings.IsConnectable();
    return settingsData;
}

Json BluetoothImplUtils::ConvertAdvertiserDataToJson(const BluetoothBleAdvertiserData& advData)
{
    Json advDataJson;
    Json serviceUuidsArray;
    Json manufactureDataArray;
    Json serviceDataArray;

    for (auto& uuid : advData.serviceUuids_) {
        serviceUuidsArray.push_back(uuid.ToString());
    }
    for (auto& manufactureData : advData.manufacturerSpecificData_) {
        Json manufactureDataJson;
        manufactureDataJson["manufactureId"] = manufactureData.first;
        std::vector<u_int32_t> valueArray;
        for (auto& word : manufactureData.second) {
            valueArray.push_back(static_cast<u_int32_t>(word));
        }
        manufactureDataJson["manufactureValue"] = valueArray;
        manufactureDataArray.push_back(manufactureDataJson);
    }
    for (auto& serviceData : advData.serviceData_) {
        Json serviceDataJson;
        serviceDataJson["serviceUuid"] = serviceData.first.ToString();
        std::vector<u_int32_t> valueArray;
        for (auto& word : serviceData.second) {
            valueArray.push_back(static_cast<u_int32_t>(word));
        }
        serviceDataJson["manufactureValue"] = valueArray;
        serviceDataArray.push_back(serviceDataJson);
    }
    advDataJson["serviceUuids"] = serviceUuidsArray;
    advDataJson["manufactureData"] = manufactureDataArray;
    advDataJson["serviceData"] = serviceDataArray;
    advDataJson["includeDeviceName"] = advData.includeDeviceName_;
    advDataJson["includeTxPower"] = advData.includeTxPower_;
    return advDataJson;
}

bool BluetoothImplUtils::CheckServicesIsLegality(const BluetoothGattService& service)
{
    for (auto& character : service.characteristics_) {
        if (character.length_ > MAX_VALUE_LENGTH) {
            return false;
        }
        for (auto& descriptor : character.descriptors_) {
            if (descriptor.length_ > MAX_VALUE_LENGTH) {
                return false;
            }
        }
    }
    return true;
}

bool BluetoothImplUtils::CalculateAndAssignHandle(const int applicationId,
    std::map<int32_t, std::list<std::pair<uint16_t, uint16_t>>>& handleMap, BluetoothGattService& service)
{
    uint16_t count = 1;
    std::pair<uint16_t, uint16_t> handlePair(0, 0);
    count += service.includeServices_.size();
    count += service.characteristics_.size() * CONVERT_SIZE;
    for (auto& ccc : service.characteristics_) {
        count += ccc.descriptors_.size();
    }
    for (auto item = handleMap[applicationId].begin(); item != handleMap[applicationId].end(); item++) {
        auto availableLength = item->second - item->first + 1;
        if (availableLength >= count) {
            handlePair = std::pair<uint16_t, uint16_t>(item->first, item->first + count - 1);
            if (handlePair.second < item->second) {
                handleMap[applicationId].insert(
                    item, std::pair<uint16_t, uint16_t>(handlePair.second + 1, item->second));
            }
            handleMap[applicationId].erase(item);
            break;
        }
    }
    service.handle_ = handlePair.first;
    service.endHandle_ = handlePair.second;
    if (handlePair == std::pair<uint16_t, uint16_t>(0, 0)) {
        return false;
    }
    uint32_t currentHandle = handlePair.first + 1;
    for (auto& includeService : service.includeServices_) {
        includeService.handle_ = currentHandle++;
    }
    for (auto& character : service.characteristics_) {
        character.handle_ = currentHandle++;
        character.valueHandle_ = currentHandle++;
        for (auto& descriptor : character.descriptors_) {
            descriptor.handle_ = currentHandle++;
        }
        character.endHandle_ = currentHandle - 1;
    }
    return true;
}

void BluetoothImplUtils::ReleaseHandle(const int applicationId,
    std::map<int32_t, std::list<std::pair<uint16_t, uint16_t>>>& handleMap, const BluetoothGattService& service)
{
    if (handleMap[applicationId].empty()) {
        handleMap[applicationId].emplace_front(service.handle_, service.endHandle_);
        return;
    }
    auto item = handleMap[applicationId].begin();
    while (item != handleMap[applicationId].end()) {
        auto currentNode = item;
        auto nextNode = ++item;
        if (currentNode->first > service.endHandle_) {
            if (currentNode->first == service.endHandle_ + 1) {
                handleMap[applicationId].emplace(currentNode, service.handle_, currentNode->second);
                handleMap[applicationId].erase(currentNode);
            } else {
                handleMap[applicationId].emplace(currentNode, service.handle_, service.endHandle_);
            }
            break;
        }
        if (nextNode == handleMap[applicationId].end()) {
            if (service.handle_ == currentNode->second + 1) {
                handleMap[applicationId].emplace(currentNode, currentNode->first, service.endHandle_);
                handleMap[applicationId].erase(currentNode);
            } else {
                handleMap[applicationId].emplace(nextNode, service.handle_, service.endHandle_);
            }
            break;
        } else {
            if (service.handle_ > nextNode->second) {
                continue;
            }
            if (service.handle_ == currentNode->second + 1 && service.endHandle_ + 1 != nextNode->first) {
                handleMap[applicationId].emplace(currentNode, currentNode->first, service.endHandle_);
                handleMap[applicationId].erase(currentNode);
            } else if (service.handle_ != currentNode->second + 1 && service.endHandle_ + 1 == nextNode->first) {
                handleMap[applicationId].emplace(nextNode, service.handle_, nextNode->second);
                handleMap[applicationId].erase(nextNode);
            } else if (service.handle_ == currentNode->second + 1 && service.endHandle_ + 1 == nextNode->first) {
                handleMap[applicationId].emplace(nextNode, currentNode->first, nextNode->second);
                handleMap[applicationId].erase(currentNode);
                handleMap[applicationId].erase(nextNode);
            } else {
                handleMap[applicationId].emplace(nextNode, service.handle_, service.endHandle_);
            }
            break;
        }
    }
}

void BluetoothImplUtils::ParseGattServiceFromJson(const std::string& jsonString, bluetooth::Service& service)
{
    Json jsonData = Json::parse(jsonString);
    std::string uuidString = jsonData["uuid"];
    std::transform(uuidString.begin(), uuidString.end(), uuidString.begin(), ::toupper);
    auto uuid = bluetooth::Uuid::ConvertFromString(uuidString);
    int32_t type = static_cast<int32_t>(jsonData["type"]);
    bool isPrimary = (type == 0) ? true : false;
    service.uuid_ = uuid;
    service.isPrimary_ = isPrimary;
}

void BluetoothImplUtils::ParseCharacteristicFromJson(
    const std::string& jsonString, bluetooth::Characteristic& characteristic)
{
    Json jsonData = Json::parse(jsonString);
    std::string uuidString = jsonData["uuid"];
    std::transform(uuidString.begin(), uuidString.end(), uuidString.begin(), ::toupper);
    auto uuid = bluetooth::Uuid::ConvertFromString(uuidString);
    int32_t properties = static_cast<int32_t>(jsonData["properties"]);
    int32_t permissions = static_cast<int32_t>(jsonData["permissions"]);
    auto jsonArray = jsonData["value"];
    std::vector<uint8_t> uint8Array(jsonArray.size());
    for (size_t i = 0; i < jsonArray.size(); ++i) {
        uint8Array[i] = jsonArray[i].get<unsigned int>();
    }
    int32_t length = static_cast<int32_t>(jsonData["length"]);

    characteristic.uuid_ = uuid;
    characteristic.properties_ = properties;
    characteristic.permissions_ = permissions;
    characteristic.SetValue(uint8Array.data(), length);
}

void BluetoothImplUtils::ParseDescriptorFromJson(const std::string& jsonString, bluetooth::Descriptor& descriptor)
{
    Json jsonData = Json::parse(jsonString);
    std::string uuidString = jsonData["uuid"];
    std::transform(uuidString.begin(), uuidString.end(), uuidString.begin(), ::toupper);
    auto uuid = bluetooth::Uuid::ConvertFromString(uuidString);
    int32_t permissions = static_cast<int32_t>(jsonData["permissions"]);
    auto jsonArray = jsonData["value"];
    std::vector<uint8_t> uint8Array(jsonArray.size());
    for (size_t i = 0; i < jsonArray.size(); ++i) {
        uint8Array[i] = jsonArray[i].get<unsigned int>();
    }
    int32_t length = static_cast<int32_t>(jsonData["length"]);
    descriptor.uuid_ = uuid;
    descriptor.permissions_ = permissions;
    if (descriptor.value_ != nullptr) {
        descriptor.value_.reset(nullptr);
    }
    descriptor.length_ = length;
    descriptor.value_ = std::make_unique<uint8_t[]>(descriptor.length_);
    if (memcpy_s(descriptor.value_.get(), descriptor.length_, uint8Array.data(), descriptor.length_) != EOK) {
        HILOGE("ParseDescriptorFromJson Failure");
    }
}
} // namespace Bluetooth
} // namespace OHOS
