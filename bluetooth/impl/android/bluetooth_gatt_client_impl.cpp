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

#include "bluetooth_gatt_client_impl.h"

#include <functional>
#include <thread>

#include "bluetooth_jni.h"
#include "bluetooth_log.h"

using namespace OHOS::bluetooth;
namespace OHOS {
namespace Bluetooth {
namespace {
constexpr uint16_t MIN_HANDLE = 1;
constexpr uint16_t MAX_HANDLE = 65535;
constexpr uint32_t DEFAULT = 0x02;
constexpr uint32_t NO_RESPONSE = 0x01;
constexpr uint32_t SIGNED = 0x04;
constexpr int32_t ONE_HUNDRED_MS = 100;
constexpr size_t BLE_ATT_HEADER_SIZE = 3;
constexpr size_t DELAY_COUNT = 5;
}
std::map<int32_t, std::shared_ptr<GattClientData>> BluetoothGattClientImpl::gattClientDataMap_;
std::map<int32_t, std::list<std::pair<uint16_t, uint16_t>>> BluetoothGattClientImpl::gattClientHandleMap_;
std::map<int32_t, std::map<std::pair<uint16_t, uint16_t>, std::string>> BluetoothGattClientImpl::serviceHandleMap_;
std::map<int32_t, std::map<std::pair<uint16_t, uint16_t>, std::string>> BluetoothGattClientImpl::characterHandleMap_;
std::map<int32_t, std::map<uint16_t, std::string>> BluetoothGattClientImpl::descriptorHandleMap_;
int32_t BluetoothGattClientImpl::appId_ = 0;
std::mutex BluetoothGattClientImpl::gattClientMutex_;

BluetoothGattClientImpl::BluetoothGattClientImpl() {}

BluetoothGattClientImpl::~BluetoothGattClientImpl() {}

int BluetoothGattClientImpl::RegisterApplication(
    const sptr<IBluetoothGattClientCallback>& callback, const BluetoothRawAddress& addr, int32_t transport)
{
    return BT_NO_ERROR;
}

int BluetoothGattClientImpl::RegisterApplication(
    const sptr<IBluetoothGattClientCallback>& callback, const BluetoothRawAddress& addr, int32_t transport, int& appId)
{
    std::lock_guard<std::mutex> lock(gattClientMutex_);
    int result = GattStatus::DEVICE_ALREADY_BIND;
    appId_ += 1;
    appId = appId_;
    std::shared_ptr<GattClientData> gattClientData = std::make_shared<GattClientData>(callback, addr, transport);
    if (gattClientDataMap_.emplace(appId, gattClientData).second) {
        gattClientHandleMap_[appId].emplace_front(std::make_pair(MIN_HANDLE, MAX_HANDLE));
        result = GattStatus::GATT_SUCCESS;
    } else {
        HILOGE("BluetoothGattClientImpl RegisterApplication failed.");
        result = GattStatus::GATT_FAILURE;
    }
    return result;
}

int BluetoothGattClientImpl::DeregisterApplication(int32_t appId)
{
    int result = BT_NO_ERROR;
    std::lock_guard<std::mutex> lock(gattClientMutex_);
    auto it = gattClientDataMap_.find(appId);
    if (it != gattClientDataMap_.end()) {
        result = BluetoothJni::GattClientClose(appId);
    } else {
        HILOGE("BluetoothGattClientImpl DeregisterApplication failed, the appId is %{public}d", appId);
        result = INVALID_PARAMETER;
    }
    if (result == BT_NO_ERROR) {
        gattClientDataMap_.erase(appId);
        BluetoothGattClientImpl::EraseHandleByAppId(appId);
    }
    return result;
}

int BluetoothGattClientImpl::Connect(int32_t appId, bool autoConnect)
{
    int result = BT_NO_ERROR;
    auto gattClientData = BluetoothGattClientImpl::GetGattClientData(appId);
    if (gattClientData) {
        auto address = gattClientData->rawAddress_.GetAddress();
        result = BluetoothJni::GattClientConnect(appId, address, autoConnect);
    } else {
        HILOGE("BluetoothGattClientImpl Connect failed, the appId is %{public}d", appId);
        result = BT_ERR_INTERNAL_ERROR;
    }
    return result;
}

int BluetoothGattClientImpl::Disconnect(int32_t appId)
{
    return BluetoothJni::GattClientDisconnect(appId);
}

int BluetoothGattClientImpl::DiscoveryServices(int32_t appId)
{
    auto gattClientData = BluetoothGattClientImpl::GetGattClientData(appId);
    if (gattClientData && gattClientData->callback_) {
        gattClientData->callback_->OnServicesDiscovered(GattStatus::GATT_SUCCESS);
    } else {
        HILOGE("BluetoothGattClientImpl DiscoveryServices failed, the appId is %{public}d", appId);
    }
    return GattStatus::GATT_SUCCESS;
}

int BluetoothGattClientImpl::ReadCharacteristic(int32_t appId, const BluetoothGattCharacteristic& characteristic)
{
    auto gattClientData = BluetoothGattClientImpl::GetGattClientData(appId);
    int32_t result = BT_NO_ERROR;
    if (gattClientData) {
        BluetoothGattCharacteristic gattCharacteristic = BluetoothGattCharacteristic(characteristic);
        result = BluetoothJni::ClientReadCharacter(appId, gattCharacteristic);
    } else {
        HILOGE("BluetoothGattClientImpl ReadCharacteristic failed, the appId is %{public}d", appId);
        result = BT_ERR_INTERNAL_ERROR;
    }
    return result;
}

int BluetoothGattClientImpl::WriteCharacteristic(
    int32_t appId, BluetoothGattCharacteristic* characteristic, bool withoutRespond)
{
    int32_t result = BT_NO_ERROR;
    auto gattClientData = BluetoothGattClientImpl::GetGattClientData(appId);
    if (gattClientData) {
        BluetoothGattCharacteristic gattCharacteristic = BluetoothGattCharacteristic(*characteristic);
        size_t maxMtu = gattClientData->mtu_ - BLE_ATT_HEADER_SIZE;
        if (maxMtu < gattCharacteristic.length_) {
            HILOGE("Write data is greater than the maximum transfer rate");
            return BT_ERR_INTERNAL_ERROR;
        }
        int32_t type = withoutRespond ? NO_RESPONSE : DEFAULT;
        result = BluetoothJni::ClientWriteCharacter(appId, gattCharacteristic, type);
    } else {
        HILOGE("BluetoothGattClientImpl WriteCharacteristic failed, the appId is %{public}d", appId);
        result = BT_ERR_INTERNAL_ERROR;
    }
    return result;
}

int BluetoothGattClientImpl::SignedWriteCharacteristic(int32_t appId, BluetoothGattCharacteristic* characteristic)
{
    int32_t result = BT_NO_ERROR;
    auto gattClientData = BluetoothGattClientImpl::GetGattClientData(appId);
    if (gattClientData) {
        BluetoothGattCharacteristic gattCharacteristic = BluetoothGattCharacteristic(*characteristic);
        result = BluetoothJni::ClientWriteCharacter(appId, gattCharacteristic, SIGNED);
    } else {
        HILOGE("BluetoothGattClientImpl WriteCharacteristic failed, the appId is %{public}d", appId);
        result = BT_ERR_INTERNAL_ERROR;
    }
    return result;
}

int BluetoothGattClientImpl::ReadDescriptor(int32_t appId, const BluetoothGattDescriptor& descriptor)
{
    int32_t result = BT_NO_ERROR;
    auto gattClientData = BluetoothGattClientImpl::GetGattClientData(appId);
    if (gattClientData) {
        BluetoothGattDescriptor gattDescriptor = BluetoothGattDescriptor(descriptor);
        result = BluetoothJni::ClientReadDescriptor(appId, gattDescriptor);
    } else {
        HILOGE("BluetoothGattClientImpl ReadDescriptor failed, the appId is %{public}d", appId);
        result = BT_ERR_INTERNAL_ERROR;
    }
    return result;
}

int BluetoothGattClientImpl::WriteDescriptor(int32_t appId, BluetoothGattDescriptor* descriptor)
{
    int32_t result = BT_NO_ERROR;
    auto gattClientData = BluetoothGattClientImpl::GetGattClientData(appId);
    if (gattClientData) {
        BluetoothGattDescriptor gattDescriptor = BluetoothGattDescriptor(*descriptor);
        size_t maxMtu = gattClientData->mtu_ - BLE_ATT_HEADER_SIZE;
        if (maxMtu < gattDescriptor.length_) {
            HILOGE("Write data is greater than the maximum transfer rate");
            return BT_ERR_INTERNAL_ERROR;
        }
        result = BluetoothJni::ClientWriteDescriptor(appId, gattDescriptor);
    } else {
        HILOGE("BluetoothGattClientImpl WriteDescriptor failed, the appId is %{public}d", appId);
        result = BT_ERR_INTERNAL_ERROR;
    }
    return result;
}

int BluetoothGattClientImpl::RequestExchangeMtu(int32_t appId, int32_t mtu)
{
    return BluetoothJni::RequestExchangeMtu(appId, mtu);
}

void BluetoothGattClientImpl::GetAllDevice(std::vector<BluetoothGattDevice>& device) {}

int BluetoothGattClientImpl::RequestConnectionPriority(int32_t appId, int32_t connPriority)
{
    return BT_NO_ERROR;
}

void BluetoothGattClientImpl::CopyBluetoothGattService(
    std::vector<BluetoothGattService>& services, std::vector<BluetoothGattService>& copeServices)
{
    for (auto& iter : services) {
        copeServices.push_back(std::move(iter));
    }
}

int BluetoothGattClientImpl::GetServices(int32_t appId, std::vector<BluetoothGattService>& service)
{
    auto gattClientData = BluetoothGattClientImpl::GetGattClientData(appId);
    if (gattClientData == nullptr) {
        HILOGE("BluetoothGattClientImpl GetServices failed, the appId is %{public}d", appId);
        return BT_ERR_INTERNAL_ERROR;
    }
    std::vector<BluetoothGattService> copeServices;
    CopyBluetoothGattService(gattClientData->services_, copeServices);
    gattClientData->services_.clear();
    int32_t result = BluetoothJni::ClientDiscoverServices(appId);
    if (result == BT_NO_ERROR) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ONE_HUNDRED_MS));
        int32_t checkCount = 0;
        int32_t servicesSize = -1;
        while (checkCount < DELAY_COUNT) {
            std::lock_guard<std::mutex> lock(BluetoothJni::gattClientServiceLock_);
            if (gattClientData->services_.size() == servicesSize) {
                break;
            }
            ++checkCount;
            servicesSize = gattClientData->services_.size();
        }
    } else {
        CopyBluetoothGattService(copeServices, gattClientData->services_);
    }
    CopyBluetoothGattService(gattClientData->services_, service);
    return BT_NO_ERROR;
}

int BluetoothGattClientImpl::RequestFastestConn(const BluetoothRawAddress& addr)
{
    return BT_NO_ERROR;
}

int BluetoothGattClientImpl::ReadRemoteRssiValue(int32_t appId)
{
    HILOGI("NOT SUPPORT NOW");
    return BT_NO_ERROR;
}

int BluetoothGattClientImpl::RequestNotification(int32_t appId, uint16_t characterHandle, bool enable)
{
    HILOGI("NOT SUPPORT NOW");
    return BT_NO_ERROR;
}

std::shared_ptr<GattClientData> BluetoothGattClientImpl::GetGattClientData(int32_t appId)
{
    std::lock_guard<std::mutex> lock(gattClientMutex_);
    auto it = gattClientDataMap_.find(appId);
    if (it != gattClientDataMap_.end()) {
        return it->second;
    } else {
        HILOGE("BluetoothGattClientImpl GetGattClientData failed, the appId is %{public}d", appId);
        return nullptr;
    }
}

bool BluetoothGattClientImpl::SetHandle(const int appId, BluetoothGattService& service)
{
    uint16_t count = 1;
    std::pair<uint16_t, uint16_t> handlePair(0, 0);
    count += service.includeServices_.size();
    count += service.characteristics_.size() * 0x2;
    for (const auto& iter : service.characteristics_) {
        count += iter.descriptors_.size();
    }
    bool result = false;
    std::lock_guard<std::mutex> lock(gattClientMutex_);
    for (auto item = gattClientHandleMap_[appId].begin(); item != gattClientHandleMap_[appId].end(); item++) {
        auto availableLength = item->second - item->first + 1;
        if (availableLength >= count) {
            handlePair = std::pair<uint16_t, uint16_t>(item->first, item->first + count - 1);
            if (handlePair.second < item->second) {
                gattClientHandleMap_[appId].insert(
                    item, std::pair<uint16_t, uint16_t>(handlePair.second + 1, item->second));
                result = true;
            }
            gattClientHandleMap_[appId].erase(item);
            break;
        }
    }
    service.handle_ = handlePair.first;
    service.endHandle_ = handlePair.second;
    if (result) {
        serviceHandleMap_[appId][handlePair] = service.uuid_.ToString();
    }
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
            descriptorHandleMap_[appId][descriptor.handle_] = descriptor.uuid_.ToString();
        }
        character.endHandle_ = currentHandle - 1;
        std::pair<uint16_t, uint16_t> value(character.handle_, character.endHandle_);
        characterHandleMap_[appId][value] = character.uuid_.ToString();
    }
    return true;
}

void BluetoothGattClientImpl::EraseHandleByAppId(const int appId)
{
    auto serviceIter = serviceHandleMap_.find(appId);
    if (serviceIter != serviceHandleMap_.end()) {
        serviceHandleMap_.erase(serviceIter);
    }
    auto characterIter = characterHandleMap_.find(appId);
    if (characterIter != characterHandleMap_.end()) {
        characterHandleMap_.erase(characterIter);
    }
    auto descriptorIter = descriptorHandleMap_.find(appId);
    if (descriptorIter != descriptorHandleMap_.end()) {
        descriptorHandleMap_.erase(descriptorIter);
    }
}

std::string BluetoothGattClientImpl::GetServiceUuidByHandle(const int appId, const int handle)
{
    auto iter = serviceHandleMap_.find(appId);
    if (iter != serviceHandleMap_.end()) {
        auto serviceUuidMap = iter->second;
        for (const auto& entry : serviceUuidMap) {
            const std::pair<uint16_t, uint16_t>& value = entry.first;
            if (handle >= value.first && handle <= value.second) {
                return entry.second;
            }
        }
    } else {
        HILOGE("Get service uuid by handle failed, the appId is %{public}d", appId);
        return std::string("");
    }
}

std::string BluetoothGattClientImpl::GetCharacterUuidByHandle(const int appId, const int handle)
{
    auto iter = characterHandleMap_.find(appId);
    if (iter != characterHandleMap_.end()) {
        auto characterUuidMap = iter->second;
        for (const auto& entry : characterUuidMap) {
            const std::pair<uint16_t, uint16_t>& value = entry.first;
            if (handle >= value.first && handle <= value.second) {
                return entry.second;
            }
        }
    } else {
        HILOGE("Get character uuid by handle failed, the appId is %{public}d", appId);
        return std::string("");
    }
}

std::string BluetoothGattClientImpl::GetDescriptorUuidByHandle(const int appId, const int handle)
{
    auto iter = descriptorHandleMap_.find(appId);
    if (iter != descriptorHandleMap_.end()) {
        auto descriptorHandleMap = iter->second;
        for (const auto& entry : descriptorHandleMap) {
            const uint16_t& value = entry.first;
            if (handle == value) {
                return entry.second;
            }
        }
    } else {
        HILOGE("Get descriptor uuid by handle failed, the appId is %{public}d", appId);
        return std::string("");
    }
}
} // namespace Bluetooth
} // namespace OHOS