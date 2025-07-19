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

#include "bluetooth_ble_centralmanager_impl.h"

#include "bluetooth_ble_scan_result.h"
#include "bluetooth_jni.h"
#include "bluetooth_log.h"
#include "message_parcel.h"

using namespace OHOS::bluetooth;
namespace OHOS {
namespace Bluetooth {
int32_t BluetoothBleCentralManagerImpl::GetCallBackScannerId()
{
    std::lock_guard<std::mutex> lock(scanMutex_);
    scannerId_ += 1;
    return scannerId_;
}

BluetoothBleCentralManagerImpl::BluetoothBleCentralManagerImpl() {}

BluetoothBleCentralManagerImpl::~BluetoothBleCentralManagerImpl() {}

void BluetoothBleCentralManagerImpl::RegisterBleCentralManagerCallback(
    int32_t& scannerId, bool enableRandomAddrMode, const sptr<IBluetoothBleCentralManagerCallback>& callback)
{
    if (callback == nullptr) {
        HILOGE("callback is nullptr");
        return;
    }
    scannerId = GetCallBackScannerId();
    profileIdScanMap_.emplace(scannerId, callback);
}

void BluetoothBleCentralManagerImpl::DeregisterBleCentralManagerCallback(
    int32_t scannerId, const sptr<IBluetoothBleCentralManagerCallback>& callback)
{
    HILOGD("scannerId is %d ", scannerId);
    std::lock_guard<std::mutex> lock(scanMutex_);
    auto it = profileIdScanMap_.find(scannerId);
    if (it != profileIdScanMap_.end()) {
        profileIdScanMap_.erase(it);
    }
}

int BluetoothBleCentralManagerImpl::StartScan(int32_t scannerId, const BluetoothBleScanSettings& settings,
    const std::vector<BluetoothBleScanFilter>& filters, bool isNewApi)
{
    HILOGD("scannerId is %d ", scannerId);
    return BluetoothJni::StartScan(scannerId, settings, filters);
}

int BluetoothBleCentralManagerImpl::StopScan(int32_t scannerId)
{
    HILOGD("scannerId is %d ", scannerId);
    return BluetoothJni::StopScan(scannerId);
}

void BluetoothBleCentralManagerImpl::OnScanResult(int32_t scannerId, const BluetoothBleScanResult& scanResult)
{
    std::lock_guard<std::mutex> lock(scanMutex_);
    if (profileIdScanMap_[scannerId] != nullptr) {
        profileIdScanMap_[scannerId]->OnScanCallback(scanResult, OHOS::bluetooth::BLE_SCAN_CALLBACK_TYPE_ALL_MATCH);
    }
}

void BluetoothBleCentralManagerImpl::RemoveScanFilter(int32_t scannerId)
{
    HILOGE("Not Supported");
    return;
}

bool BluetoothBleCentralManagerImpl::FreezeByRss(std::set<int> pidSet, bool isProxy)
{
    HILOGE("Not Supported");
    return 0;
}

bool BluetoothBleCentralManagerImpl::ResetAllProxy()
{
    HILOGE("Not Supported");
    return 0;
}

int BluetoothBleCentralManagerImpl::SetLpDeviceAdvParam(
    int duration, int maxExtAdvEvents, int window, int interval, int advHandle)
{
    HILOGE("Not Supported");
    return 0;
}

int BluetoothBleCentralManagerImpl::SetScanReportChannelToLpDevice(int32_t scannerId, bool enable)
{
    HILOGE("Not Supported");
    return 0;
}

int BluetoothBleCentralManagerImpl::EnableSyncDataToLpDevice()
{
    HILOGE("Not Supported");
    return 0;
}

int BluetoothBleCentralManagerImpl::DisableSyncDataToLpDevice()
{
    HILOGE("Not Supported");
    return 0;
}

int BluetoothBleCentralManagerImpl::SendParamsToLpDevice(const std::vector<uint8_t>& dataValue, int32_t type)
{
    HILOGE("Not Supported");
    return 0;
}

bool BluetoothBleCentralManagerImpl::IsLpDeviceAvailable()
{
    HILOGE("Not Supported");
    return 0;
}

int BluetoothBleCentralManagerImpl::SetLpDeviceParam(const BluetoothLpDeviceParamSet& paramSet)
{
    HILOGE("Not Supported");
    return 0;
}

int BluetoothBleCentralManagerImpl::RemoveLpDeviceParam(const bluetooth::Uuid& uuid)
{
    HILOGE("Not Supported");
    return 0;
}

int BluetoothBleCentralManagerImpl::ChangeScanParams(int32_t scannerId, const BluetoothBleScanSettings& settings,
    const std::vector<BluetoothBleScanFilter>& filters, uint32_t filterAction)
{
    HILOGE("Not Supported");
    return 0;
}

int BluetoothBleCentralManagerImpl::IsValidScannerId(int32_t scannerId, bool &isValid)
{
    HILOGE("Not Supported");
    return BT_NO_ERROR;
}
} // namespace Bluetooth
} // namespace OHOS
