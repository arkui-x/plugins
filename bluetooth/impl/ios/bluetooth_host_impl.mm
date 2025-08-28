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

#include "bluetooth_host_impl.h"

#include <memory>

#import "BluetoothCentralManager.h"
#import "BluetoothPeripheralManager.h"
#include "bluetooth_def.h"
#include "bluetooth_errorcode.h"
#include "bluetooth_gatt_client_impl.h"
#include "bluetooth_gatt_server_impl.h"
#include "bluetooth_log.h"
#include "refbase.h"

using namespace OHOS::bluetooth;

namespace OHOS {
namespace Bluetooth {
BluetoothHostImpl::BluetoothHostImpl()
{
    [BluetoothPeripheralManager sharedInstance];
    BluetoothCentralManager* centralManager = [BluetoothCentralManager sharedInstance];
    centralManager.updataBleStateBlock = ^(int ret) {
      if (registerObserverCallBack_ != nullptr) {
          int bleState = BTStateID::STATE_TURN_OFF;
          if (ret == CBManagerStatePoweredOn) {
              bleState = BTStateID::STATE_TURN_ON;
          }
          registerObserverCallBack_->OnStateChanged(BTTransport::ADAPTER_BREDR, bleState);
      }
    };
}

BluetoothHostImpl::~BluetoothHostImpl() {}

void BluetoothHostImpl::RegisterObserver(const sptr<IBluetoothHostObserver>& observer)
{
    std::lock_guard<std::mutex> lock(observerMutex_);
    registerObserverCallBack_ = observer;
}

void BluetoothHostImpl::DeregisterObserver(const sptr<IBluetoothHostObserver>& observer) {}

int32_t BluetoothHostImpl::EnableBt()
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::DisableBt(bool isAsync, const std::string &callingName)
{
    return BT_NO_ERROR;
}

sptr<IRemoteObject> BluetoothHostImpl::GetProfile(const std::string& name)
{
    return nullptr;
}

sptr<IRemoteObject> BluetoothHostImpl::GetBleRemote(const std::string& name)
{
    return nullptr;
}

int32_t BluetoothHostImpl::BluetoothFactoryReset()
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::GetBtState(int& state)
{
    BluetoothCentralManager* centralManager = [BluetoothCentralManager sharedInstance];
    int bleState = [centralManager getBleState];
    if (bleState == CBManagerStatePoweredOn) {
        state = static_cast<int>(BluetoothState::STATE_ON);
    } else if (bleState == CBManagerStatePoweredOff) {
        state = static_cast<int>(BluetoothState::STATE_OFF);
    } else {
        return BT_ERR_INTERNAL_ERROR;
    }
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::GetLocalAddress(std::string& addr)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::DisableBle()
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::EnableBle(bool noAutoConnect, bool isAsync, const std::string &callingNam)
{
    return BT_NO_ERROR;
}

bool BluetoothHostImpl::IsBrEnabled()
{
    return [[BluetoothCentralManager sharedInstance] isBleEnabled];
}

bool BluetoothHostImpl::IsBleEnabled()
{
    return [[BluetoothPeripheralManager sharedInstance] isBleEnabled];
}

std::vector<uint32_t> BluetoothHostImpl::GetProfileList()
{
    std::vector<uint32_t> vec;
    return vec;
}

int32_t BluetoothHostImpl::GetMaxNumConnectedAudioDevices()
{
    return -1;
}

int32_t BluetoothHostImpl::GetBtConnectionState(int& state)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::GetBtProfileConnState(uint32_t profileId, int& state)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::GetLocalDeviceClass()
{
    return BT_NO_ERROR;
}

bool BluetoothHostImpl::SetLocalDeviceClass(const int32_t& deviceClass)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::GetLocalName(std::string& name)
{
    BluetoothCentralManager* central_manager = [BluetoothCentralManager sharedInstance];
    NSString* strName = [central_manager getLocalName];
    if (strName.length == 0) {
        return BT_ERR_INTERNAL_ERROR;
    }
    name = [strName cStringUsingEncoding:NSUTF8StringEncoding];
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::SetLocalName(const std::string& name)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::GetBtScanMode(int32_t& scanMode)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::SetBtScanMode(int32_t mode, int32_t duration)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::GetBondableMode(const int32_t transport)
{
    return BT_NO_ERROR;
}

bool BluetoothHostImpl::SetBondableMode(int32_t transport, int32_t mode)
{
    return false;
}

int32_t BluetoothHostImpl::StartBtDiscovery()
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::CancelBtDiscovery()
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::IsBtDiscovering(bool& isDisCovering, const int32_t transport)
{
    return BT_NO_ERROR;
}

long BluetoothHostImpl::GetBtDiscoveryEndMillis()
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::GetPairedDevices(std::vector<BluetoothRawAddress>& pairedAddr)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::RemovePair(const int32_t transport, const sptr<BluetoothRawAddress>& device)
{
    return BT_NO_ERROR;
}

bool BluetoothHostImpl::RemoveAllPairs()
{
    return false;
}

void BluetoothHostImpl::RegisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver>& observer) {}

void BluetoothHostImpl::DeregisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver>& observer) {}

int32_t BluetoothHostImpl::GetBleMaxAdvertisingDataLength()
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::GetDeviceType(int32_t transport, const std::string& address)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::GetPhonebookPermission(const std::string& address)
{
    return BT_NO_ERROR;
}

bool BluetoothHostImpl::SetPhonebookPermission(const std::string& address, int32_t permission)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::GetMessagePermission(const std::string& address)
{
    return BT_NO_ERROR;
}

bool BluetoothHostImpl::SetMessagePermission(const std::string& address, int32_t permission)
{
    return false;
}

int32_t BluetoothHostImpl::GetPowerMode(const std::string& address)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::GetDeviceName(int32_t transport, const std::string& address, std::string& name, bool alias)
{
    NSString* strDeviceId = [NSString stringWithFormat:@"%s", address.c_str()];
    CBPeripheral* peripheral = [[BluetoothCentralManager sharedInstance] getDevice:strDeviceId];
    if (peripheral && peripheral.name.length != 0) {
        name = peripheral.name.UTF8String;
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

std::string BluetoothHostImpl::GetDeviceAlias(const std::string& address)
{
    return "";
}

int32_t BluetoothHostImpl::SetDeviceAlias(const std::string& address, const std::string& aliasName)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::GetRemoteDeviceBatteryInfo(const std::string& address, BluetoothBatteryInfo& batteryInfo)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::GetPairState(int32_t transport, const std::string& address, int32_t& pairState)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::StartPair(int32_t transport, const std::string& address)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::StartCrediblePair(int32_t transport, const std::string& address)
{
    return BT_NO_ERROR;
}

bool BluetoothHostImpl::CancelPairing(int32_t transport, const std::string& address)
{
    return false;
}

bool BluetoothHostImpl::IsBondedFromLocal(int32_t transport, const std::string& address)
{
    return false;
}

bool BluetoothHostImpl::IsAclConnected(int32_t transport, const std::string& address)
{
    return false;
}

bool BluetoothHostImpl::IsAclEncrypted(int32_t transport, const std::string& address)
{
    return false;
}

int32_t BluetoothHostImpl::GetDeviceClass(const std::string& address, int& cod)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::SetDevicePin(const std::string& address, const std::string& pin)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::SetDevicePairingConfirmation(int32_t transport, const std::string& address, bool accept)
{
    return BT_NO_ERROR;
}

bool BluetoothHostImpl::SetDevicePasskey(int32_t transport, const std::string& address, int32_t passkey, bool accept)
{
    return false;
}

bool BluetoothHostImpl::PairRequestReply(int32_t transport, const std::string& address, bool accept)
{
    return false;
}

bool BluetoothHostImpl::ReadRemoteRssiValue(const std::string& address)
{
    return false;
}

void BluetoothHostImpl::GetLocalSupportedUuids(std::vector<std::string>& uuids) {}

int32_t BluetoothHostImpl::GetDeviceUuids(const std::string& address, std::vector<std::string>& uuids)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::GetLocalProfileUuids(std::vector<std::string>& uuids)
{
    return BT_NO_ERROR;
}

void BluetoothHostImpl::RegisterBleAdapterObserver(const sptr<IBluetoothHostObserver>& observer) {}

void BluetoothHostImpl::DeregisterBleAdapterObserver(const sptr<IBluetoothHostObserver>& observer) {}

void BluetoothHostImpl::RegisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver>& observer) {}

void BluetoothHostImpl::DeregisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver>& observer) {}

int32_t BluetoothHostImpl::SetFastScan(bool isEnable)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::GetRandomAddress(const std::string& realAddr, std::string& randomAddr, uint64_t tokenId)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::SyncRandomAddress(const std::string& realAddr, const std::string& randomAddr)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::ConnectAllowedProfiles(const std::string& remoteAddr)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::DisconnectAllowedProfiles(const std::string& remoteAddr)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::SetDeviceCustomType(const std::string& address, int32_t deviceType)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::GetRemoteDeviceInfo(
    const std::string& address, std::shared_ptr<BluetoothRemoteDeviceInfo>& deviceInfo, int type)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::SatelliteControl(int type, int state)
{
    return BT_NO_ERROR;
}

void BluetoothHostImpl::RegisterBtResourceManagerObserver(const sptr<IBluetoothResourceManagerObserver>& observer) {}

void BluetoothHostImpl::DeregisterBtResourceManagerObserver(const sptr<IBluetoothResourceManagerObserver>& observer) {}

int32_t BluetoothHostImpl::OnChangeStateCallBack(int state)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::OnDiscoveryResultCallBack(
    std::string address, int rssi, std::string deviceName, int deviceClass)
{
    return BT_NO_ERROR;
}

void BluetoothHostImpl::UpdateVirtualDevice(int32_t action, const std::string& address) {}

int32_t BluetoothHostImpl::IsSupportVirtualAutoConnect(const std::string& address, bool& outSupport)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::SetVirtualAutoConnectType(const std::string& address, int connType, int businessType)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::SetFastScanLevel(int level)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::OnPairStatusChangedCallBack(
    const int32_t transport, std::string address, int32_t status, int32_t cause)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::EnableBluetoothToRestrictMode(const std::string &callingName)
{
    return BT_NO_ERROR;
}

int32_t BluetoothHostImpl::ControlDeviceAction(const std::string &deviceId, uint32_t controlType,
    uint32_t controlTypeVal, uint32_t controlObject)
{
    return 0;
}

int32_t BluetoothHostImpl::GetLastConnectionTime(const std::string &address, int64_t &connectionTime)
{
    return 0;
}

int32_t BluetoothHostImpl::UpdateCloudBluetoothDevice(std::vector<BluetoothTrustPairDevice> &cloudDevices)
{
    return 0;
}

int32_t BluetoothHostImpl::GetCloudBondState(const std::string &address, int32_t &cloudBondState)
{
    return 0;
}

int32_t BluetoothHostImpl::GetDeviceTransport(const std::string &address, int32_t &transport)
{
    return 0;
}

int BluetoothHostImpl::UpdateRefusePolicy(const int32_t protocolType,
    const int32_t pid, const int64_t prohibitedSecondsTime)
{
    return 0;
}

int BluetoothHostImpl::ProcessRandomDeviceIdCommand(
    int32_t command, std::vector<std::string>& deviceIdVec, bool &isValid)
{
    return 0;
}

int BluetoothHostImpl::GetCarKeyDfxData(std::string &dfxData)
{
    return 0;
}

int BluetoothHostImpl::SetCarKeyCardData(const std::string &address, int32_t action)
{
    return 0;
}

int32_t BluetoothHostImpl::NotifyDialogResult(uint32_t dialogType, bool dialogResult)
{
    return 0;
}
} // namespace Bluetooth
} // namespace OHOS
