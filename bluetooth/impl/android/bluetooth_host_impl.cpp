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

#include "bluetooth_errorcode.h"
#include "bluetooth_impl_utils.h"
#include "bluetooth_jni.h"
#include "bluetooth_log.h"
#include "refbase.h"

using namespace OHOS::bluetooth;
namespace OHOS {
namespace Bluetooth {
BluetoothHostImpl::BluetoothHostImpl() {}

BluetoothHostImpl::~BluetoothHostImpl() {}

void BluetoothHostImpl::RegisterObserver(const sptr<IBluetoothHostObserver>& observer)
{
    observer_ = observer;
}

void BluetoothHostImpl::DeregisterObserver(const sptr<IBluetoothHostObserver>& observer)
{
    observer_ = nullptr;
}

int32_t BluetoothHostImpl::EnableBt()
{
    return BluetoothJni::EnableBt();
}

int32_t BluetoothHostImpl::DisableBt(bool isAsync, const std::string &callingName)
{
    return BluetoothJni::DisableBt();
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
    return 0;
}

int32_t BluetoothHostImpl::GetBtState(int& state)
{
    int ret = BluetoothJni::GetBtState(state);
    if (ret == BT_NO_ERROR) {
        int stateValue = state;
        ret = BluetoothImplUtils::GetOhHostBtStateFromBluetoothAdapter(stateValue, state);
        HILOGI("GetBtState, state: %{public}d", state);
        if (ret != BT_NO_ERROR) {
            return BT_ERR_INTERNAL_ERROR;
        }
    }
    return ret;
}

int32_t BluetoothHostImpl::GetLocalAddress(std::string& addr)
{
    return 0;
}

int32_t BluetoothHostImpl::DisableBle()
{
    return DisableBt(false, "");
}

int32_t BluetoothHostImpl::EnableBle(bool noAutoConnect, bool isAsync, const std::string &callingName)
{
    return EnableBt();
}

bool BluetoothHostImpl::IsBrEnabled()
{
    return BluetoothJni::IsBtEnable();
}

bool BluetoothHostImpl::IsBleEnabled()
{
    return BluetoothJni::IsBtEnable();
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
    return BluetoothJni::GetBtProfileConnState(state);
}

int32_t BluetoothHostImpl::GetBtProfileConnState(uint32_t profileId, int& state)
{
    return BluetoothJni::GetBtProfileConnState(profileId, state);
}

int32_t BluetoothHostImpl::GetLocalDeviceClass()
{
    return 0;
}

bool BluetoothHostImpl::SetLocalDeviceClass(const int32_t& deviceClass)
{
    return 0;
}

int32_t BluetoothHostImpl::GetLocalName(std::string& name)
{
    return BluetoothJni::GetLocalName(name);
}

int32_t BluetoothHostImpl::SetLocalName(const std::string& name)
{
    return BT_ERR_API_NOT_SUPPORT;
}

int32_t BluetoothHostImpl::GetBtScanMode(int32_t& scanMode)
{
    int ret = BluetoothJni::GetBtScanMode(scanMode);
    if (ret == BT_NO_ERROR) {
        int scanModeValue = scanMode;
        ret = BluetoothImplUtils::GetOhScanModeFromBluetoothAdapter(scanModeValue, scanMode);
        HILOGI("GetBtScanMode, scanMode: %{public}d]", scanMode);
        if (ret != BT_NO_ERROR) {
            ret = BT_ERR_INTERNAL_ERROR;
        }
    }
    return ret;
}

int32_t BluetoothHostImpl::SetBtScanMode(int32_t mode, int32_t duration)
{
    return BT_ERR_API_NOT_SUPPORT;
}

int32_t BluetoothHostImpl::GetBondableMode(const int32_t transport)
{
    return 0;
}

bool BluetoothHostImpl::SetBondableMode(int32_t transport, int32_t mode)
{
    return false;
}

int32_t BluetoothHostImpl::StartBtDiscovery()
{
    return BluetoothJni::StartBtDiscovery();
}

int32_t BluetoothHostImpl::CancelBtDiscovery()
{
    return BluetoothJni::CancelBtDiscovery();
}

int32_t BluetoothHostImpl::IsBtDiscovering(bool& isDisCovering, const int32_t transport)
{
    return BluetoothJni::IsBtDiscovering(isDisCovering);
}

long BluetoothHostImpl::GetBtDiscoveryEndMillis()
{
    return 0;
}

int32_t BluetoothHostImpl::GetPairedDevices(std::vector<BluetoothRawAddress>& pairedAddr)
{
    return BluetoothJni::GetPairedDevices(pairedAddr);
}

int32_t BluetoothHostImpl::RemovePair(const int32_t transport, const sptr<BluetoothRawAddress>& device)
{
    return BT_ERR_API_NOT_SUPPORT;
}

bool BluetoothHostImpl::RemoveAllPairs()
{
    return false;
}

void BluetoothHostImpl::RegisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver>& observer)
{
    remoteDeviceObserver_ = observer;
}

void BluetoothHostImpl::DeregisterRemoteDeviceObserver(const sptr<IBluetoothRemoteDeviceObserver>& observer)
{
    remoteDeviceObserver_ = nullptr;
}

int32_t BluetoothHostImpl::GetBleMaxAdvertisingDataLength()
{
    return 0;
}

int32_t BluetoothHostImpl::GetDeviceType(int32_t transport, const std::string& address)
{
    return 0;
}

int32_t BluetoothHostImpl::GetPhonebookPermission(const std::string& address)
{
    return 0;
}

bool BluetoothHostImpl::SetPhonebookPermission(const std::string& address, int32_t permission)
{
    return 0;
}

int32_t BluetoothHostImpl::GetMessagePermission(const std::string& address)
{
    return 0;
}

bool BluetoothHostImpl::SetMessagePermission(const std::string& address, int32_t permission)
{
    return false;
}

int32_t BluetoothHostImpl::GetPowerMode(const std::string& address)
{
    return 0;
}

int32_t BluetoothHostImpl::GetDeviceName(int32_t transport, const std::string& address, std::string& name, bool alias)
{
    return BluetoothJni::GetDeviceName(address, name);
}

std::string BluetoothHostImpl::GetDeviceAlias(const std::string& address)
{
    return std::string("");
}

int32_t BluetoothHostImpl::SetDeviceAlias(const std::string& address, const std::string& aliasName)
{
    return 0;
}

int32_t BluetoothHostImpl::GetRemoteDeviceBatteryInfo(const std::string& address, BluetoothBatteryInfo& batteryInfo)
{
    return 0;
}

int32_t BluetoothHostImpl::GetPairState(int32_t transport, const std::string& address, int32_t& pairState)
{
    return BluetoothJni::GetPairState(address, pairState);
}

int32_t BluetoothHostImpl::StartPair(int32_t transport, const std::string& address)
{
    return BluetoothJni::StartPair(address);
}

int32_t BluetoothHostImpl::StartCrediblePair(int32_t transport, const std::string& address)
{
    return 0;
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
    return BluetoothJni::GetDeviceClass(address, cod);
}

int32_t BluetoothHostImpl::SetDevicePin(const std::string& address, const std::string& pin)
{
    return BT_ERR_API_NOT_SUPPORT;
}

int32_t BluetoothHostImpl::SetDevicePairingConfirmation(int32_t transport, const std::string& address, bool accept)
{
    return 0;
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
    return BluetoothJni::GetDeviceUuids(address, uuids);
}

int32_t BluetoothHostImpl::GetLocalProfileUuids(std::vector<std::string>& uuids)
{
    return 0;
}

void BluetoothHostImpl::RegisterBleAdapterObserver(const sptr<IBluetoothHostObserver>& observer) {}

void BluetoothHostImpl::DeregisterBleAdapterObserver(const sptr<IBluetoothHostObserver>& observer) {}

void BluetoothHostImpl::RegisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver>& observer) {}

void BluetoothHostImpl::DeregisterBlePeripheralCallback(const sptr<IBluetoothBlePeripheralObserver>& observer) {}

int32_t BluetoothHostImpl::SetFastScan(bool isEnable)
{
    return 0;
}

int32_t BluetoothHostImpl::GetRandomAddress(const std::string& realAddr, std::string& randomAddr, uint64_t tokenId)
{
    return 0;
}

int32_t BluetoothHostImpl::SyncRandomAddress(const std::string& realAddr, const std::string& randomAddr)
{
    return 0;
}

int32_t BluetoothHostImpl::ConnectAllowedProfiles(const std::string& remoteAddr)
{
    return 0;
}

int32_t BluetoothHostImpl::DisconnectAllowedProfiles(const std::string& remoteAddr)
{
    return 0;
}

int32_t BluetoothHostImpl::SetDeviceCustomType(const std::string& address, int32_t deviceType)
{
    return 0;
}

int32_t BluetoothHostImpl::GetRemoteDeviceInfo(
    const std::string& address, std::shared_ptr<BluetoothRemoteDeviceInfo>& deviceInfo, int type)
{
    return BT_ERR_API_NOT_SUPPORT;
}

int32_t BluetoothHostImpl::OnChangeStateCallBack(int state)
{
    std::lock_guard<std::mutex> lock(observerMutex_);
    if (observer_ == nullptr) {
        HILOGE("OnChangeStateCallBack, observer_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }

    int btState = 0;
    int ret = BluetoothImplUtils::GetOhHostBtStateIDFromBluetoothAdapter(state, btState);
    if (ret == BT_NO_ERROR) {
        observer_->OnStateChanged(BTTransport::ADAPTER_BREDR, btState);
        return BT_NO_ERROR;
    }

    return BT_ERR_INTERNAL_ERROR;
}

int32_t BluetoothHostImpl::OnDiscoveryResultCallBack(
    std::string address, int rssi, std::string deviceName, int deviceClass)
{
    std::lock_guard<std::mutex> lock(observerMutex_);
    if (observer_ == nullptr) {
        HILOGE("OnDiscoveryResultCallBack, observer_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!address.empty() && !deviceName.empty()) {
        BluetoothRawAddress device(address);
        observer_->OnDiscoveryResult(device, rssi, deviceName, deviceClass);
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

int32_t BluetoothHostImpl::OnPairStatusChangedCallBack(
    const int32_t transport, std::string address, int32_t status, int32_t cause)
{
    std::lock_guard<std::mutex> lock(remoteDeviceObserverMutex_);
    if (remoteDeviceObserver_ == nullptr) {
        HILOGE("OnPairStatusChangedCallBack, remoteDeviceObserver_ is null");
        return BT_ERR_INTERNAL_ERROR;
    }

    if (!address.empty()) {
        BluetoothRawAddress device(address);
        int statusValue = status;
        int ret = BluetoothImplUtils::GetOhPairStateFromBluetoothAdapter(statusValue, status);
        if (ret != BT_NO_ERROR) {
            return BT_ERR_INTERNAL_ERROR;
        }
        cause = UnbondCause::PAIR_COMMON_BOND_CAUSE;
        remoteDeviceObserver_->OnPairStatusChanged(transport, device, status, cause);
        return BT_NO_ERROR;
    }
    return BT_ERR_INTERNAL_ERROR;
}

int32_t BluetoothHostImpl::SatelliteControl(int type, int state)
{
    return 0;
}

void BluetoothHostImpl::RegisterBtResourceManagerObserver(const sptr<IBluetoothResourceManagerObserver>& observer) {}

void BluetoothHostImpl::DeregisterBtResourceManagerObserver(const sptr<IBluetoothResourceManagerObserver>& observer) {}

void BluetoothHostImpl::UpdateVirtualDevice(int32_t action, const std::string& address) {}

int32_t BluetoothHostImpl::IsSupportVirtualAutoConnect(const std::string& address, bool& outSupport)
{
    return 0;
}

int32_t BluetoothHostImpl::SetVirtualAutoConnectType(const std::string& address, int connType, int businessType)
{
    return 0;
}

int32_t BluetoothHostImpl::SetFastScanLevel(int level)
{
    return 0;
}

int32_t BluetoothHostImpl::EnableBluetoothToRestrictMode(const std::string &callingName)
{
    return 0;
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
