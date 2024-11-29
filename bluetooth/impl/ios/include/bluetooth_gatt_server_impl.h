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

#ifndef BLUETOOTH_GATT_SERVER_IMPL_H
#define BLUETOOTH_GATT_SERVER_IMPL_H

#include <map>

#include "bluetooth_errorcode.h"
#include "bluetooth_gatt_server_stub.h"
#include "i_bluetooth_gatt_server.h"
#include "i_bluetooth_gatt_server_callback.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothGattServerImpl : public BluetoothGattServerStub,
                                public std::enable_shared_from_this<BluetoothGattServerImpl> {
public:
    BluetoothGattServerImpl();
    ~BluetoothGattServerImpl() = default;
    int AddService(int32_t appId, BluetoothGattService* services) override;
    void ClearServices(int appId) override;
    int RegisterApplication(const sptr<IBluetoothGattServerCallback>& callback) override;
    int DeregisterApplication(int appId) override;
    int Connect(int appId, const BluetoothGattDevice& device, bool isDirect) override;
    int CancelConnection(int appId, const BluetoothGattDevice& device) override;
    int NotifyClient(
        const BluetoothGattDevice& device, BluetoothGattCharacteristic* characteristic, bool needConfirm) override;
    int RemoveService(int32_t appId, const BluetoothGattService& services) override;
    int RespondCharacteristicRead(
        const BluetoothGattDevice& device, BluetoothGattCharacteristic* characteristic, int32_t ret) override;
    int RespondCharacteristicWrite(
        const BluetoothGattDevice& device, const BluetoothGattCharacteristic& characteristic, int32_t ret) override;
    int RespondDescriptorRead(
        const BluetoothGattDevice& device, BluetoothGattDescriptor* descriptor, int32_t ret) override;
    int RespondDescriptorWrite(
        const BluetoothGattDevice& device, const BluetoothGattDescriptor& descriptor, int32_t ret) override;

private:
    int32_t appId_;
    OHOS::sptr<IBluetoothGattServerCallback> serviceCallBack_;
    std::mutex gattServerMutex_;
    std::map<int32_t, std::list<std::pair<uint16_t, uint16_t>>> gattServiceHandleMap_;
    std::map<uint16_t, BluetoothGattService> gattServiceMap_;
    int32_t GetGattServerAppId();
    void GetGattCharacteristic(std::string serviceUUID, std::string charaUUID, int length, const uint8_t* bytesValue,
        BluetoothGattCharacteristic& character);
    void GattCharacteristicNotifyBlock();
    void GattCharacteristicWriteBlock();
    void GattCharacteristicReadBlock();
    bool CalculateAndAssignHandle(const int applicationId, BluetoothGattService& service);
    void ReleaseHandle(const int applicationId, const BluetoothGattService& service);
    int RespondCharacteristic(
        const uint16_t type, const BluetoothGattDevice& device, const BluetoothGattCharacteristic& gattCharacter);
};
} // namespace Bluetooth
} // namespace OHOS
#endif // BLUETOOTH_GATT_SERVER_IMPL_H