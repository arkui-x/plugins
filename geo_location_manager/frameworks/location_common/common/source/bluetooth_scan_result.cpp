/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#include "common_utils.h"
#include "constant_definition.h"
#include "bluetooth_scan_result.h"

namespace OHOS {
namespace Location {
BluetoothScanResult::BluetoothScanResult()
{
    deviceId_ = "";
    deviceName_ = "";
    rssi_ = 0;
    connectable_ = false;
}

BluetoothScanResult::BluetoothScanResult(BluetoothScanResult& bluetoothScanResult)
{
    deviceId_ = bluetoothScanResult.GetDeviceId();
    deviceName_ = bluetoothScanResult.GetDeviceName();
    rssi_ = bluetoothScanResult.GetRssi();
    connectable_ = bluetoothScanResult.GetConnectable();
    data_ = bluetoothScanResult.GetData();
}

BluetoothScanResult::~BluetoothScanResult() {}

bool BluetoothScanResult::Marshalling(Parcel& parcel) const
{
    return true;
}
} // namespace Location
} // namespace OHOS