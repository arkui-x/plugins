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

#ifndef IBLUETOOTH_SCAN_RESULT_CALLBACK_H
#define IBLUETOOTH_SCAN_RESULT_CALLBACK_H

#include "iremote_broker.h"

#include "bluetooth_scan_result.h"

namespace OHOS {
namespace Location {
class IBluetoothScanResultCallback : public IRemoteBroker {
public:
enum {
        RECEIVE_INFO_EVENT = 1,
    };
    DECLARE_INTERFACE_DESCRIPTOR(u"location.IBluetoothScanResultCallback");
    virtual void OnBluetoothScanResultChange(const std::unique_ptr<BluetoothScanResult>& bluetoothScanResult) = 0;
};
} // namespace Location
} // namespace OHOS
#endif // IBLUETOOTH_SCAN_RESULT_CALLBACK_H
