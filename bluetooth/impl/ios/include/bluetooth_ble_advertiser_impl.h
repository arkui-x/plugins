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

#ifndef OHOS_BLUETOOTH_STANDARD_BLE_ADVERTISER_IMPL_H
#define OHOS_BLUETOOTH_STANDARD_BLE_ADVERTISER_IMPL_H

#include "bluetooth_ble_advertiser_stub.h"
#include "bluetooth_errorcode.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothBleAdvertiserImpl : public BluetoothBleAdvertiserStub,
                                public std::enable_shared_from_this<BluetoothBleAdvertiserImpl> {
public:
    BluetoothBleAdvertiserImpl();
    ~BluetoothBleAdvertiserImpl();

public:
    int StartAdvertising(const BluetoothBleAdvertiserSettings& settings, const BluetoothBleAdvertiserData& advData,
        const BluetoothBleAdvertiserData& scanResponse, int32_t advHandle, uint16_t duration, bool isRawData) override;
    void RegisterBleAdvertiserCallback(const sptr<IBluetoothBleAdvertiseCallback>& callback) override;
    void DeregisterBleAdvertiserCallback(const sptr<IBluetoothBleAdvertiseCallback>& callback) override;
    int EnableAdvertising(uint8_t advHandle, uint16_t duration) override;
    int DisableAdvertising(uint8_t advHandle) override;
    int ChangeAdvertisingParams(uint8_t advHandle, const BluetoothBleAdvertiserSettings &settings) override;
    int StopAdvertising(int32_t advHandle) override;
    void Close(int32_t advHandle) override;
    void SetAdvertisingData(const BluetoothBleAdvertiserData& advData, const BluetoothBleAdvertiserData& scanResponse,
        int32_t advHandle) override;
    int32_t GetAdvertiserHandle(int32_t& advHandle, const sptr<IBluetoothBleAdvertiseCallback> &callback) override;

private:
    std::mutex advertiserMutex_;
    OHOS::sptr<IBluetoothBleAdvertiseCallback> advertiseCallBack_;
};
} // namespace Bluetooth
} // namespace OHOS
#endif // OHOS_BLUETOOTH_STANDARD_BLE_ADVERTISER_IMPL_H
