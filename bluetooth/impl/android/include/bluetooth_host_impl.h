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

#ifndef PLUGINS_BLUETOOTH_IMPL_ANDROID_INCLUDE_BLUETOOTH_HOST_IMPL_H
#define PLUGINS_BLUETOOTH_IMPL_ANDROID_INCLUDE_BLUETOOTH_HOST_IMPL_H

#include "bluetooth_host_stub.h"
#include "bluetooth_raw_address.h"
#include "bt_def.h"
#include "i_bluetooth_host.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothHostImpl : public BluetoothHostStub, public std::enable_shared_from_this<BluetoothHostImpl> {
public:
    BluetoothHostImpl();
    ~BluetoothHostImpl();

    void RegisterObserver(const sptr<IBluetoothHostObserver>& observer) override;
    void DeregisterObserver(const sptr<IBluetoothHostObserver>& observer) override;
    int32_t EnableBt() override;
    int32_t DisableBt(bool isAsync, const std::string &callingName) override;
    sptr<IRemoteObject> GetProfile(const std::string& name) override;
    sptr<IRemoteObject> GetBleRemote(const std::string& name) override;
    int32_t BluetoothFactoryReset() override;
    int32_t GetBtState(int& state) override;
    int32_t GetLocalAddress(std::string& addr) override;
    int32_t DisableBle() override;
    int32_t EnableBle(bool noAutoConnect, bool isAsync, const std::string &callingName) override;
    bool IsBrEnabled();
    bool IsBleEnabled();
    std::vector<uint32_t> GetProfileList() override;
    int32_t GetMaxNumConnectedAudioDevices() override;
    int32_t GetBtConnectionState(int& state) override;
    int32_t GetBtProfileConnState(uint32_t profileId, int& state) override;
    int32_t GetLocalDeviceClass() override;
    bool SetLocalDeviceClass(const int32_t& deviceClass) override;
    int32_t GetLocalName(std::string& name) override;
    int32_t SetLocalName(const std::string& name) override;
    int32_t GetBtScanMode(int32_t& scanMode) override;
    int32_t SetBtScanMode(int32_t mode, int32_t duration) override;
    int32_t GetBondableMode(const int32_t transport) override;
    bool SetBondableMode(int32_t transport, int32_t mode) override;
    int32_t StartBtDiscovery() override;
    int32_t CancelBtDiscovery() override;
    int32_t IsBtDiscovering(bool& isDisCovering, const int32_t transport) override;
    long GetBtDiscoveryEndMillis() override;
    int32_t GetPairedDevices(std::vector<BluetoothRawAddress>& pairedAddr) override;
    int32_t RemovePair(const int32_t transport, const sptr<BluetoothRawAddress>& device) override;
    bool RemoveAllPairs() override;
    void RegisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver>& observer) override;
    void DeregisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver>& observer) override;
    int32_t GetBleMaxAdvertisingDataLength() override;
    int32_t GetDeviceType(int32_t transport, const std::string& address) override;
    int32_t GetPhonebookPermission(const std::string& address) override;
    bool SetPhonebookPermission(const std::string& address, int32_t permission) override;
    int32_t GetMessagePermission(const std::string& address) override;
    bool SetMessagePermission(const std::string& address, int32_t permission) override;
    int32_t GetPowerMode(const std::string& address) override;
    int32_t GetDeviceName(int32_t transport, const std::string& address, std::string& name,
        bool alias = false) override;
    std::string GetDeviceAlias(const std::string& address) override;
    int32_t SetDeviceAlias(const std::string& address, const std::string& aliasName) override;
    int32_t GetRemoteDeviceBatteryInfo(const std::string& address, BluetoothBatteryInfo& info) override;
    int32_t GetPairState(int32_t transport, const std::string& address, int32_t& pairState) override;
    int32_t StartPair(int32_t transport, const std::string& address) override;
    int32_t StartCrediblePair(int32_t transport, const std::string& address) override;
    bool CancelPairing(int32_t transport, const std::string& address) override;
    bool IsBondedFromLocal(int32_t transport, const std::string& address) override;
    bool IsAclConnected(int32_t transport, const std::string& address) override;
    bool IsAclEncrypted(int32_t transport, const std::string& address) override;
    int32_t GetDeviceClass(const std::string& address, int& cod) override;
    int32_t SetDevicePin(const std::string& address, const std::string& pin) override;
    int32_t SetDevicePairingConfirmation(int32_t transport, const std::string& address, bool accept) override;
    bool SetDevicePasskey(int32_t transport, const std::string& address, int32_t passkey, bool accept) override;
    bool PairRequestReply(int32_t transport, const std::string& address, bool accept) override;
    bool ReadRemoteRssiValue(const std::string& address) override;
    void GetLocalSupportedUuids(std::vector<std::string>& uuids) override;
    int32_t GetDeviceUuids(const std::string& address, std::vector<std::string>& uuids) override;
    int32_t GetLocalProfileUuids(std::vector<std::string>& uuids) override;
    void RegisterBleAdapterObserver(const sptr<IBluetoothHostObserver>& observer) override;
    void DeregisterBleAdapterObserver(const sptr<IBluetoothHostObserver>& observer) override;
    void RegisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver>& observer) override;
    void DeregisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver>& observer) override;
    int32_t SetFastScan(bool isEnable) override;
    int32_t GetRandomAddress(const std::string& realAddr, std::string& randomAddr, uint64_t tokenId = 0) override;
    int32_t SyncRandomAddress(const std::string& realAddr, const std::string& randomAddr) override;
    int32_t ConnectAllowedProfiles(const std::string& remoteAddr) override;
    int32_t DisconnectAllowedProfiles(const std::string& remoteAddr) override;
    int32_t SetDeviceCustomType(const std::string& address, int32_t deviceType) override;
    int32_t GetRemoteDeviceInfo(
        const std::string& address, std::shared_ptr<BluetoothRemoteDeviceInfo>& deviceInfo, int type) override;

    void UpdateVirtualDevice(int32_t action, const std::string& address) override;
    int32_t SatelliteControl(int type, int state) override;
    void RegisterBtResourceManagerObserver(const sptr<IBluetoothResourceManagerObserver>& observer) override;
    void DeregisterBtResourceManagerObserver(const sptr<IBluetoothResourceManagerObserver>& observer) override;
    int32_t OnChangeStateCallBack(int state);
    int32_t OnDiscoveryResultCallBack(std::string address, int rssi, std::string deviceName, int deviceClass);
    int32_t OnPairStatusChangedCallBack(const int32_t transport, std::string address, int32_t status, int32_t cause);

    int32_t IsSupportVirtualAutoConnect(const std::string& address, bool& outSupport) override;
    int32_t SetVirtualAutoConnectType(const std::string& address, int connType, int businessType) override;
    int32_t SetFastScanLevel(int level) override;
    int32_t EnableBluetoothToRestrictMode(const std::string &callingName) override;
    int32_t ControlDeviceAction(const std::string &deviceId, uint32_t controlType,
        uint32_t controlTypeVal, uint32_t controlObject) override;
    int32_t GetLastConnectionTime(const std::string &address, int64_t &connectionTime) override;
    int32_t UpdateCloudBluetoothDevice(std::vector<BluetoothTrustPairDevice> &cloudDevices) override;
    int32_t GetCloudBondState(const std::string &address, int32_t &cloudBondState) override;
    int32_t GetDeviceTransport(const std::string &address, int32_t &transport) override;
    int32_t UpdateRefusePolicy(const int32_t protocolType,
        const int32_t pid, const int64_t prohibitedSecondsTime) override;
    int32_t ProcessRandomDeviceIdCommand(
        int32_t command, std::vector<std::string> &deviceIdVec, bool &isValid) override;
    int32_t GetCarKeyDfxData(std::string &dfxData) override;
    int32_t SetCarKeyCardData(const std::string &address, int32_t action) override;
    int32_t NotifyDialogResult(uint32_t dialogType, bool dialogResult) override;

private:
    sptr<IBluetoothHostObserver> observer_ = nullptr;
    sptr<IBluetoothRemoteDeviceObserver> remoteDeviceObserver_ = nullptr;
    std::mutex remoteDeviceObserverMutex_;
    std::mutex observerMutex_;
};
} // namespace Bluetooth
} // namespace OHOS
#endif // PLUGINS_BLUETOOTH_IMPL_ANDROID_INCLUDE_BLUETOOTH_HOST_IMPL_H
