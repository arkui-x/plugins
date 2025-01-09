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

#ifndef OHOS_BLUETOOTH_STANDARD_GATT_CLIENT_IMPL_H
#define OHOS_BLUETOOTH_STANDARD_GATT_CLIENT_IMPL_H

#include <map>

#include "bluetooth_gatt_client_stub.h"
#include "i_bluetooth_gatt_client.h"
#include "i_bluetooth_gatt_client_callback.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothGattClientImpl : public BluetoothGattClientStub,
                                public std::enable_shared_from_this<BluetoothGattClientImpl> {
public:
    BluetoothGattClientImpl();
    ~BluetoothGattClientImpl() = default;

    int RegisterApplication(const sptr<IBluetoothGattClientCallback>& callback, const BluetoothRawAddress& addr,
        int32_t transport) override;
    int RegisterApplication(const sptr<IBluetoothGattClientCallback>& callback, const BluetoothRawAddress& addr,
        int32_t transport, int& appId) override;
    int DeregisterApplication(int32_t appId) override;
    int Connect(int32_t appId, bool autoConnect) override;
    int Disconnect(int32_t appId) override;
    int DiscoveryServices(int32_t appId) override;
    int ReadCharacteristic(int32_t appId, const BluetoothGattCharacteristic& characteristic) override;
    int WriteCharacteristic(int32_t appId, BluetoothGattCharacteristic* characteristic, bool withoutRespond) override;
    int SignedWriteCharacteristic(int32_t appId, BluetoothGattCharacteristic* characteristic) override;
    int ReadDescriptor(int32_t appId, const BluetoothGattDescriptor& descriptor) override;
    int WriteDescriptor(int32_t appId, BluetoothGattDescriptor* descriptor) override;
    int RequestExchangeMtu(int32_t appId, int32_t mtu) override;
    void GetAllDevice(std::vector<BluetoothGattDevice>& device) override;
    int RequestConnectionPriority(int32_t appId, int32_t connPriority) override;
    int GetServices(int32_t appId, std::vector<BluetoothGattService>& service) override;
    int RequestFastestConn(const BluetoothRawAddress& addr) override;
    int ReadRemoteRssiValue(int32_t appId) override;
    int RequestNotification(int32_t appId, uint16_t characterHandle, bool enable) override;

private:
    int32_t appId_;
    std::map<int32_t, OHOS::sptr<IBluetoothGattClientCallback>> gattClientCallBackMap_;
    std::map<int32_t, std::pair<uint16_t, uint16_t>> gattClientHandleMap_;
    std::map<uint16_t, BluetoothGattService> gattServiceMap_;
    std::map<uint16_t, BluetoothGattDescriptor> gattDescriptorMap_;
    std::mutex gattClientMutex_;
    int GetGattClientAppId();
    void ChangeConnectState(int32_t appId, int ret, int state);
    OHOS::sptr<IBluetoothGattClientCallback> GetCallBack(const int32_t appId);
    BluetoothGattService GetServiceWithCharacterHandle(uint16_t characterHandle);
    bool CalculateAndAssignHandle(const int32_t applicationId, BluetoothGattService& service);
    void GetGattCharacteristic(std::string serviceUUID, std::string charaUUID, int length, const uint8_t* bytesValue,
        BluetoothGattCharacteristic& character);
};
} // namespace Bluetooth
} // namespace OHOS
#endif // OHOS_BLUETOOTH_STANDARD_GATT_CLIENT_IMPL_H