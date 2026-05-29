/*
 * Copyright (C) 2024-2026 Huawei Device Co., Ltd.
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

#include "bluetooth_gatt_server_impl.h"

#include "bluetooth_impl_utils.h"
#include "bluetooth_jni.h"
#include "bluetooth_log.h"
#include "bluetooth_profile_manager.h"
#include "gatt_data.h"

namespace OHOS {
namespace Bluetooth {
const uint16_t MIN_SERVICE_HANDLE = 1;
const uint16_t MAX_SERVICE_HANDLE = 65535;

namespace {
std::string BuildGattValue(const uint8_t* value, size_t length)
{
    if (value == nullptr || length == 0) {
        return "";
    }
    constexpr size_t MAX_GATT_VALUE_LENGTH = 512;
    if (length > MAX_GATT_VALUE_LENGTH) {
        HILOGE("BuildGattValue failed, length is too large");
        return "";
    }
    return std::string(reinterpret_cast<const char*>(value), length);
}
} // namespace

BluetoothGattServerImpl::BluetoothGattServerImpl() {}

BluetoothGattServerImpl::~BluetoothGattServerImpl() {}

int BluetoothGattServerImpl::RegisterApplication(const sptr<IBluetoothGattServerCallback>& callback)
{
    if (callback == nullptr) {
        return 0;
    }
    std::lock_guard<std::mutex> lock(gattServerMutex_);
    ++applicationIdInc_;
    auto ret = BluetoothJni::RegisterApplicationGattServer(applicationIdInc_);
    if (ret == BT_NO_ERROR) {
        gattServerCallbackMap_.emplace(applicationIdInc_, callback);
        gattServiceHandleMap_[applicationIdInc_].emplace_front(std::make_pair(MIN_SERVICE_HANDLE, MAX_SERVICE_HANDLE));
    }
    return (ret != BT_NO_ERROR) ? 0 : applicationIdInc_;
}

int BluetoothGattServerImpl::DeregisterApplication(int appId)
{
    int ret = BluetoothJni::GattServerClose(appId);
    if (ret == BT_NO_ERROR) {
        std::lock_guard<std::mutex> lock(gattServerMutex_);
        gattServerCallbackMap_.erase(appId);
        gattServiceHandleMap_.erase(appId);
        RemoveAppTracking(appId);
    }
    return ret;
}

int BluetoothGattServerImpl::AddService(int32_t appId, BluetoothGattService* services)
{
    int ret = BT_ERR_INTERNAL_ERROR;
    if (services == nullptr) {
        return ret;
    }
    if (!BluetoothImplUtils::CheckServicesIsLegality(*services)) {
        return ret;
    }
    int gattErr = GattStatus::GATT_SUCCESS;
    std::lock_guard<std::mutex> lock(gattServerMutex_);
    bool success = BluetoothImplUtils::CalculateAndAssignHandle(appId, gattServiceHandleMap_, *services);
    if (!success) {
        gattErr = GattStatus::NOT_ENOUGH_HANDLES;
    } else {
        ret = BluetoothJni::AddService(appId, services);
        gattErr = (ret == BT_NO_ERROR) ? GattStatus::GATT_SUCCESS : GattStatus::GATT_FAILURE;
        if (ret == BT_NO_ERROR) {
            TrackServiceHandles(appId, *services);
        }
    }
    if (gattServerCallbackMap_[appId] != nullptr) {
        gattServerCallbackMap_[appId]->OnAddService(gattErr, *services);
    }
    return ret;
}

int BluetoothGattServerImpl::RemoveService(int32_t appId, const BluetoothGattService& services)
{
    int ret = BluetoothJni::RemoveService(appId, services);
    if (ret == BT_NO_ERROR) {
        std::lock_guard<std::mutex> lock(gattServerMutex_);
        BluetoothImplUtils::ReleaseHandle(appId, gattServiceHandleMap_, services);
        UntrackServiceHandles(appId, services);
    }
    return ret;
}

int BluetoothGattServerImpl::NotifyClient(
    const BluetoothGattDevice& device, BluetoothGattCharacteristic* characteristic, bool needConfirm)
{
    CHECK_AND_RETURN_LOG_RET(characteristic != nullptr, BT_ERR_INTERNAL_ERROR, "failed: characteristic is null");

    int32_t appId = 0;
    {
        std::lock_guard<std::mutex> lock(gattServerMutex_);
        appId = ResolveAppId(device, characteristic->handle_, false);
    }
    CHECK_AND_RETURN_LOG_RET(appId > 0, BT_ERR_INTERNAL_ERROR,
        "failed: unable to resolve appId for characteristic handle %{public}d", characteristic->handle_);
    return BluetoothJni::NotifyCharacteristicChanged(appId, device, characteristic, needConfirm);
}

int BluetoothGattServerImpl::RespondCharacteristicRead(
    const BluetoothGattDevice& device, BluetoothGattCharacteristic* characteristic, int32_t ret)
{
    CHECK_AND_RETURN_LOG_RET(characteristic != nullptr, BT_ERR_INTERNAL_ERROR, "failed: characteristic is null");

    int32_t appId = 0;
    {
        std::lock_guard<std::mutex> lock(gattServerMutex_);
        appId = ResolveAppId(device, characteristic->handle_, false);
    }
    CHECK_AND_RETURN_LOG_RET(appId > 0, BT_ERR_INTERNAL_ERROR,
        "failed: unable to resolve appId for characteristic handle %{public}d", characteristic->handle_);

    auto value = BuildGattValue(characteristic->value_.get(), characteristic->length_);
    return BluetoothJni::RespondCharacteristicRead(
        appId, device.addr_.GetAddress(), ret, value, characteristic->length_);
}

int BluetoothGattServerImpl::RespondCharacteristicWrite(
    const BluetoothGattDevice& device, const BluetoothGattCharacteristic& characteristic, int32_t ret)
{
    int32_t appId = 0;
    {
        std::lock_guard<std::mutex> lock(gattServerMutex_);
        appId = ResolveAppId(device, characteristic.handle_, false);
    }
    CHECK_AND_RETURN_LOG_RET(appId > 0, BT_ERR_INTERNAL_ERROR,
        "failed: unable to resolve appId for characteristic handle %{public}d", characteristic.handle_);
    return BluetoothJni::RespondCharacteristicWrite(appId, device.addr_.GetAddress(), ret, characteristic.handle_);
}

int BluetoothGattServerImpl::RespondDescriptorRead(
    const BluetoothGattDevice& device, BluetoothGattDescriptor* descriptor, int32_t ret)
{
    CHECK_AND_RETURN_LOG_RET(descriptor != nullptr, BT_ERR_INTERNAL_ERROR, "failed: descriptor is null");

    int32_t appId = 0;
    {
        std::lock_guard<std::mutex> lock(gattServerMutex_);
        appId = ResolveAppId(device, descriptor->handle_, true);
    }
    CHECK_AND_RETURN_LOG_RET(appId > 0, BT_ERR_INTERNAL_ERROR,
        "failed: unable to resolve appId for descriptor handle %{public}d", descriptor->handle_);

    auto value = BuildGattValue(descriptor->value_.get(), descriptor->length_);
    return BluetoothJni::RespondDescriptorRead(appId, device.addr_.GetAddress(), ret, value, descriptor->length_);
}

int BluetoothGattServerImpl::RespondDescriptorWrite(
    const BluetoothGattDevice& device, const BluetoothGattDescriptor& descriptor, int32_t ret)
{
    int32_t appId = 0;
    {
        std::lock_guard<std::mutex> lock(gattServerMutex_);
        appId = ResolveAppId(device, descriptor.handle_, true);
    }
    CHECK_AND_RETURN_LOG_RET(appId > 0, BT_ERR_INTERNAL_ERROR,
        "failed: unable to resolve appId for descriptor handle %{public}d", descriptor.handle_);
    return BluetoothJni::RespondDescriptorWrite(appId, device.addr_.GetAddress(), ret, descriptor.handle_);
}

void BluetoothGattServerImpl::OnCharacteristicReadRequest(
    int appId, const BluetoothGattDevice& device, const BluetoothGattCharacteristic& characteristic)
{
    std::lock_guard<std::mutex> lock(gattServerMutex_);
    TrackRequestAppId(appId, device, characteristic.handle_);
    if (gattServerCallbackMap_[appId] != nullptr) {
        gattServerCallbackMap_[appId]->OnCharacteristicReadRequest(device, characteristic);
    }
}

void BluetoothGattServerImpl::OnCharacteristicWriteRequest(
    int appId, const BluetoothGattDevice& device, const BluetoothGattCharacteristic& characteristic, bool needRespones)
{
    std::lock_guard<std::mutex> lock(gattServerMutex_);
    TrackRequestAppId(appId, device, characteristic.handle_);
    if (gattServerCallbackMap_[appId] != nullptr) {
        gattServerCallbackMap_[appId]->OnCharacteristicWriteRequest(device, characteristic, needRespones);
    }
}

void BluetoothGattServerImpl::OnDescriptorReadRequest(
    int appId, const BluetoothGattDevice& device, const BluetoothGattDescriptor& descriptor)
{
    std::lock_guard<std::mutex> lock(gattServerMutex_);
    TrackRequestAppId(appId, device, descriptor.handle_);
    if (gattServerCallbackMap_[appId] != nullptr) {
        gattServerCallbackMap_[appId]->OnDescriptorReadRequest(device, descriptor);
    }
}

void BluetoothGattServerImpl::OnDescriptorWriteRequest(
    int appId, const BluetoothGattDevice& device, const BluetoothGattDescriptor& descriptor)
{
    std::lock_guard<std::mutex> lock(gattServerMutex_);
    TrackRequestAppId(appId, device, descriptor.handle_);
    if (gattServerCallbackMap_[appId] != nullptr) {
        gattServerCallbackMap_[appId]->OnDescriptorWriteRequest(device, descriptor);
    }
}

void BluetoothGattServerImpl::OnNotifyConfirm(int appId, const BluetoothGattDevice& device, int result)
{
    std::lock_guard<std::mutex> lock(gattServerMutex_);
    if (gattServerCallbackMap_[appId] != nullptr) {
        gattServerCallbackMap_[appId]->OnNotifyConfirm(device, BluetoothGattCharacteristic(), result);
    }
}

void BluetoothGattServerImpl::OnMtuChanged(int appId, const BluetoothGattDevice& device, int32_t mtu)
{
    std::lock_guard<std::mutex> lock(gattServerMutex_);
    if (gattServerCallbackMap_[appId] != nullptr) {
        gattServerCallbackMap_[appId]->OnMtuChanged(device, mtu);
    }
}

void BluetoothGattServerImpl::OnConnectionStateChanged(
    int appId, const BluetoothGattDevice& device, int32_t ret, int32_t state)
{
    std::lock_guard<std::mutex> lock(gattServerMutex_);
    UpdateConnectedAppIds(appId, device, state);
    if (gattServerCallbackMap_[appId] != nullptr) {
        gattServerCallbackMap_[appId]->OnConnectionStateChanged(
            device, ret, state, static_cast<int>(GattDisconnectReason::CONN_UNKNOWN), "");
    }
}

void BluetoothGattServerImpl::TrackServiceHandles(int32_t appId, const BluetoothGattService& service)
{
    auto& characteristicHandles = characteristicHandleMap_[appId];
    auto& descriptorHandles = descriptorHandleMap_[appId];
    for (const auto& characteristic : service.characteristics_) {
        characteristicHandles.insert(characteristic.handle_);
        for (const auto& descriptor : characteristic.descriptors_) {
            descriptorHandles.insert(descriptor.handle_);
        }
    }
}

void BluetoothGattServerImpl::UntrackServiceHandles(int32_t appId, const BluetoothGattService& service)
{
    auto characteristicIt = characteristicHandleMap_.find(appId);
    auto descriptorIt = descriptorHandleMap_.find(appId);

    for (const auto& characteristic : service.characteristics_) {
        if (characteristicIt != characteristicHandleMap_.end()) {
            characteristicIt->second.erase(characteristic.handle_);
        }
        if (descriptorIt != descriptorHandleMap_.end()) {
            for (const auto& descriptor : characteristic.descriptors_) {
                descriptorIt->second.erase(descriptor.handle_);
            }
        }
    }

    if (characteristicIt != characteristicHandleMap_.end() && characteristicIt->second.empty()) {
        characteristicHandleMap_.erase(characteristicIt);
    }
    if (descriptorIt != descriptorHandleMap_.end() && descriptorIt->second.empty()) {
        descriptorHandleMap_.erase(descriptorIt);
    }
}

void BluetoothGattServerImpl::TrackRequestAppId(int32_t appId, const BluetoothGattDevice& device, uint16_t handle)
{
    auto address = device.addr_.GetAddress();
    if (address.empty()) {
        return;
    }
    requestAppIdMap_[{address, handle}] = appId;
}

void BluetoothGattServerImpl::UpdateConnectedAppIds(int32_t appId, const BluetoothGattDevice& device, int32_t state)
{
    auto address = device.addr_.GetAddress();
    if (address.empty()) {
        return;
    }

    if (state == static_cast<int32_t>(BTConnectState::CONNECTED)) {
        deviceAppIdMap_[address].insert(appId);
        return;
    }

    if (state != static_cast<int32_t>(BTConnectState::DISCONNECTED)) {
        return;
    }

    auto deviceIt = deviceAppIdMap_.find(address);
    if (deviceIt != deviceAppIdMap_.end()) {
        deviceIt->second.erase(appId);
        if (deviceIt->second.empty()) {
            deviceAppIdMap_.erase(deviceIt);
        }
    }

    for (auto it = requestAppIdMap_.begin(); it != requestAppIdMap_.end();) {
        if (it->first.first == address && it->second == appId) {
            it = requestAppIdMap_.erase(it);
            continue;
        }
        ++it;
    }
}

void BluetoothGattServerImpl::RemoveAppTracking(int32_t appId)
{
    characteristicHandleMap_.erase(appId);
    descriptorHandleMap_.erase(appId);

    for (auto it = deviceAppIdMap_.begin(); it != deviceAppIdMap_.end();) {
        it->second.erase(appId);
        if (it->second.empty()) {
            it = deviceAppIdMap_.erase(it);
            continue;
        }
        ++it;
    }

    for (auto it = requestAppIdMap_.begin(); it != requestAppIdMap_.end();) {
        if (it->second == appId) {
            it = requestAppIdMap_.erase(it);
            continue;
        }
        ++it;
    }
}

int32_t BluetoothGattServerImpl::ResolveAppId(
    const BluetoothGattDevice& device, uint16_t handle, bool isDescriptor) const
{
    auto address = device.addr_.GetAddress();
    auto requestIt = requestAppIdMap_.find({address, handle});
    if (requestIt != requestAppIdMap_.end()) {
        return requestIt->second;
    }

    const auto& handleMap = isDescriptor ? descriptorHandleMap_ : characteristicHandleMap_;
    std::set<int32_t> candidates;
    auto deviceIt = deviceAppIdMap_.find(address);

    for (const auto& [appId, handles] : handleMap) {
        if (handles.find(handle) == handles.end()) {
            continue;
        }
        if (deviceIt != deviceAppIdMap_.end() && deviceIt->second.find(appId) == deviceIt->second.end()) {
            continue;
        }
        candidates.insert(appId);
    }
    if (candidates.size() == 1) {
        return *candidates.begin();
    }

    if (deviceIt != deviceAppIdMap_.end() && deviceIt->second.size() == 1) {
        auto appId = *deviceIt->second.begin();
        auto handleIt = handleMap.find(appId);
        if (handleIt != handleMap.end() && handleIt->second.find(handle) != handleIt->second.end()) {
            return appId;
        }
    }

    candidates.clear();
    for (const auto& [appId, handles] : handleMap) {
        if (handles.find(handle) != handles.end()) {
            candidates.insert(appId);
        }
    }
    if (candidates.size() == 1) {
        return *candidates.begin();
    }
    return 0;
}

int BluetoothGattServerImpl::ClearServices(int appId)
{
    HILOGE("Unsupport GattServer ClearServices");
    return BT_ERR_INTERNAL_ERROR;
}

int BluetoothGattServerImpl::Connect(int appId, const BluetoothGattDevice& device, bool isDirect)
{
    HILOGE("Unsupport GattServer Connect");
    return BT_ERR_INTERNAL_ERROR;
}

int BluetoothGattServerImpl::CancelConnection(int appId, const BluetoothGattDevice& device)
{
    HILOGE("Unsupport GattServer CancelConnection");
    return BT_ERR_INTERNAL_ERROR;
}

int BluetoothGattServerImpl::GetConnectedState(const std::string& deviceId, int& state)
{
    HILOGE("Unsupport GattServer GetConnectedState");
    return BT_ERR_INTERNAL_ERROR;
}

int BluetoothGattServerImpl::SetPhy(int32_t appId, const std::string &deviceId,
    int32_t txPhy, int32_t rxPhy, int32_t phyOptions)
{
    HILOGE("Unsupport GattServer SetPhy");
    return BT_ERR_INTERNAL_ERROR;
}

int BluetoothGattServerImpl::ReadPhy(int32_t appId, const std::string &deviceId)
{
    HILOGE("Unsupport GattServer ReadPhy");
    return BT_ERR_INTERNAL_ERROR;
}
} // namespace Bluetooth
} // namespace OHOS
