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

#ifndef PLUGINS_BLUETOOTH_IMPL_ANDROID_INCLUDE_BLUETOOTH_GATT_CLIENT_IMPL_H
#define PLUGINS_BLUETOOTH_IMPL_ANDROID_INCLUDE_BLUETOOTH_GATT_CLIENT_IMPL_H

#include <condition_variable>
#include <map>
#include <mutex>

#include "bluetooth_gatt_client_stub.h"
#include "i_bluetooth_gatt_client.h"
#include "i_bluetooth_gatt_client_callback.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Bluetooth {
constexpr size_t BLE_DEFAULT_MTU_SIZE = 23;
struct GattClientData {
    GattClientData(
        const sptr<IBluetoothGattClientCallback>& callback, const BluetoothRawAddress& rawAddress, int32_t transport)
        : callback_(callback), rawAddress_(rawAddress), transport_(transport)
    {}

    GattClientData(const GattClientData& src)
        : callback_(src.callback_), rawAddress_(src.rawAddress_), transport_(src.transport_)
    {}

    GattClientData& operator=(const GattClientData& src) = delete;

    void SetCharacteristic(const std::shared_ptr<BluetoothGattCharacteristic>& characteristic)
    {
        characteristic_ = characteristic;
    }

    sptr<IBluetoothGattClientCallback> callback_;
    BluetoothRawAddress rawAddress_;
    int32_t transport_;
    std::shared_ptr<BluetoothGattCharacteristic> characteristic_;
    std::vector<BluetoothGattService> services_;
    size_t mtu_ = BLE_DEFAULT_MTU_SIZE;
};

class BluetoothGattClientImpl : public BluetoothGattClientStub,
                                public std::enable_shared_from_this<BluetoothGattClientImpl> {
public:
    BluetoothGattClientImpl();
    ~BluetoothGattClientImpl();

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
    void CopyBluetoothGattService(
        std::vector<BluetoothGattService>& services, std::vector<BluetoothGattService>& copeServices);

    static std::shared_ptr<GattClientData> GetGattClientData(int32_t appId);
    static bool SetHandle(const int applicationId, BluetoothGattService& service);
    static void EraseHandleByAppId(const int applicationId);
    static std::string GetServiceUuidByHandle(const int appId, const int handle);
    static std::string GetCharacterUuidByHandle(const int appId, const int handle);
    static std::string GetDescriptorUuidByHandle(const int appId, const int handle);

private:
    static std::map<int32_t, std::shared_ptr<GattClientData>> gattClientDataMap_;
    static std::map<int32_t, std::list<std::pair<uint16_t, uint16_t>>> gattClientHandleMap_;
    static std::map<int32_t, std::map<std::pair<uint16_t, uint16_t>, std::string>> serviceHandleMap_;
    static std::map<int32_t, std::map<std::pair<uint16_t, uint16_t>, std::string>> characterHandleMap_;
    static std::map<int32_t, std::map<uint16_t, std::string>> descriptorHandleMap_;
    static int32_t appId_;
    static std::mutex gattClientMutex_;
};
} // namespace Bluetooth
} // namespace OHOS
#endif // PLUGINS_BLUETOOTH_IMPL_ANDROID_INCLUDE_BLUETOOTH_GATT_CLIENT_IMPL_H
