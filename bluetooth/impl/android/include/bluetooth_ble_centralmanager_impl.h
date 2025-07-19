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

#ifndef PLUGINS_BLUETOOTH_IMPL_ANDROID_INCLUDE_BLUETOOTH_BLE_CENTRALMANAGER_IMPL_H
#define PLUGINS_BLUETOOTH_IMPL_ANDROID_INCLUDE_BLUETOOTH_BLE_CENTRALMANAGER_IMPL_H

#include <set>

#include "bluetooth_ble_advertiser_data.h"
#include "bluetooth_ble_advertiser_settings.h"
#include "bluetooth_ble_scan_filter.h"
#include "bluetooth_ble_scan_settings.h"
#include "bluetooth_blecentralmanager_stub.h"
#include "bluetooth_service_ipc_interface_code.h"
#include "i_bluetooth_ble_central_manager.h"
#include "i_bluetooth_ble_central_manager_callback.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Bluetooth {

class BluetoothBleCentralManagerImpl : public BluetoothBleCentralManagerStub,
                                       public std::enable_shared_from_this<BluetoothBleCentralManagerImpl> {
public:
    BluetoothBleCentralManagerImpl();
    ~BluetoothBleCentralManagerImpl();

public:
    void RegisterBleCentralManagerCallback(int32_t& scannerId, bool enableRandomAddrMode,
        const sptr<IBluetoothBleCentralManagerCallback>& callback) override;
    void DeregisterBleCentralManagerCallback(
        int32_t scannerId, const sptr<IBluetoothBleCentralManagerCallback>& callback) override;
    int StartScan(int32_t scannerId, const BluetoothBleScanSettings& settings,
        const std::vector<BluetoothBleScanFilter>& filters, bool isNewApi) override;
    int StopScan(int32_t scannerId) override;

    void OnScanResult(int32_t scannerId, const BluetoothBleScanResult& scanResult);

    void RemoveScanFilter(int32_t scannerId) override;
    bool FreezeByRss(std::set<int> pidSet, bool isProxy) override;
    bool ResetAllProxy() override;
    int SetLpDeviceAdvParam(int duration, int maxExtAdvEvents, int window, int interval, int advHandle) override;
    int SetScanReportChannelToLpDevice(int32_t scannerId, bool enable) override;
    int EnableSyncDataToLpDevice() override;
    int DisableSyncDataToLpDevice() override;
    int SendParamsToLpDevice(const std::vector<uint8_t>& dataValue, int32_t type) override;
    bool IsLpDeviceAvailable() override;
    int SetLpDeviceParam(const BluetoothLpDeviceParamSet& paramSet) override;
    int RemoveLpDeviceParam(const bluetooth::Uuid& uuid) override;
    int ChangeScanParams(int32_t scannerId, const BluetoothBleScanSettings &settings,
        const std::vector<BluetoothBleScanFilter> &filters, uint32_t filterAction) override;
    int IsValidScannerId(int32_t scannerId, bool &isValid) override;

    int32_t GetCallBackScannerId();

private:
    int32_t scannerId_ = 0;
    std::map<int32_t, OHOS::sptr<IBluetoothBleCentralManagerCallback>> profileIdScanMap_;
    std::mutex scanMutex_;
};
} // namespace Bluetooth
} // namespace OHOS
#endif // PLUGINS_BLUETOOTH_IMPL_ANDROID_INCLUDE_BLUETOOTH_BLE_CENTRALMANAGER_IMPL_H
