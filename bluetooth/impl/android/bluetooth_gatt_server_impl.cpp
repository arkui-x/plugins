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
    }
    return ret;
}

int BluetoothGattServerImpl::NotifyClient(
    const BluetoothGattDevice& device, BluetoothGattCharacteristic* characteristic, bool needConfirm)
{
    HILOGE("Unsupport GattServer NotifyClient");
    return BT_ERR_INTERNAL_ERROR;
}

int BluetoothGattServerImpl::RespondCharacteristicRead(
    const BluetoothGattDevice& device, BluetoothGattCharacteristic* characteristic, int32_t ret)
{
    HILOGE("Unsupport GattServer RespondCharacteristicRead");
    return BT_ERR_INTERNAL_ERROR;
}

int BluetoothGattServerImpl::RespondCharacteristicWrite(
    const BluetoothGattDevice& device, const BluetoothGattCharacteristic& characteristic, int32_t ret)
{
    HILOGE("Unsupport GattServer RespondCharacteristicWrite");
    return BT_ERR_INTERNAL_ERROR;
}

int BluetoothGattServerImpl::RespondDescriptorRead(
    const BluetoothGattDevice& device, BluetoothGattDescriptor* descriptor, int32_t ret)
{
    HILOGE("Unsupport GattServer RespondDescriptorRead");
    return BT_ERR_INTERNAL_ERROR;
}

int BluetoothGattServerImpl::RespondDescriptorWrite(
    const BluetoothGattDevice& device, const BluetoothGattDescriptor& descriptor, int32_t ret)
{
    HILOGE("Unsupport GattServer RespondDescriptorWrite");
    return BT_ERR_INTERNAL_ERROR;
}

void BluetoothGattServerImpl::OnCharacteristicReadRequest(
    int appId, const BluetoothGattDevice& device, const BluetoothGattCharacteristic& characteristic)
{
    std::lock_guard<std::mutex> lock(gattServerMutex_);
    if (gattServerCallbackMap_[appId] != nullptr) {
        gattServerCallbackMap_[appId]->OnCharacteristicReadRequest(device, characteristic);
    }
}

void BluetoothGattServerImpl::OnCharacteristicWriteRequest(
    int appId, const BluetoothGattDevice& device, const BluetoothGattCharacteristic& characteristic, bool needRespones)
{
    std::lock_guard<std::mutex> lock(gattServerMutex_);
    if (gattServerCallbackMap_[appId] != nullptr) {
        gattServerCallbackMap_[appId]->OnCharacteristicWriteRequest(device, characteristic, needRespones);
    }
}

void BluetoothGattServerImpl::OnDescriptorReadRequest(
    int appId, const BluetoothGattDevice& device, const BluetoothGattDescriptor& descriptor)
{
    std::lock_guard<std::mutex> lock(gattServerMutex_);
    if (gattServerCallbackMap_[appId] != nullptr) {
        gattServerCallbackMap_[appId]->OnDescriptorReadRequest(device, descriptor);
    }
}

void BluetoothGattServerImpl::OnDescriptorWriteRequest(
    int appId, const BluetoothGattDevice& device, const BluetoothGattDescriptor& descriptor)
{
    std::lock_guard<std::mutex> lock(gattServerMutex_);
    if (gattServerCallbackMap_[appId] != nullptr) {
        gattServerCallbackMap_[appId]->OnDescriptorWriteRequest(device, descriptor);
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
    if (gattServerCallbackMap_[appId] != nullptr) {
        gattServerCallbackMap_[appId]->OnConnectionStateChanged(
            device, ret, state, static_cast<int>(GattDisconnectReason::CONN_UNKNOWN));
    }
}

void BluetoothGattServerImpl::ClearServices(int appId)
{
    HILOGE("Unsupport GattServer ClearServices");
}

int BluetoothGattServerImpl::Connect(int appId, const BluetoothGattDevice& device, bool isDirect)
{
    HILOGE("Unsupport GattServer Connect");
    return BT_ERR_INTERNAL_ERROR;
}

int BluetoothGattServerImpl::CancelConnection(int appId, const BluetoothGattDevice& device)
{
    HILOGE("Unsupport GattServer Connect");
    return BT_ERR_INTERNAL_ERROR;
}
} // namespace Bluetooth
} // namespace OHOS
