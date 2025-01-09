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

#ifndef PLUGINS_BLUETOOTH_COMMON_INCLUDE_BLUETOOTH_BLECENTRALMANAGER_STUB_H
#define PLUGINS_BLUETOOTH_COMMON_INCLUDE_BLUETOOTH_BLECENTRALMANAGER_STUB_H

#include <set>

#include "bluetooth_ble_advertiser_data.h"
#include "bluetooth_ble_advertiser_settings.h"
#include "bluetooth_ble_scan_filter.h"
#include "bluetooth_ble_scan_settings.h"
#include "bluetooth_service_ipc_interface_code.h"
#include "i_bluetooth_ble_central_manager.h"
#include "i_bluetooth_ble_central_manager_callback.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothBleCentralManagerStub : public IRemoteStub<IBluetoothBleCentralManager> {
public:
    BluetoothBleCentralManagerStub() {}
    ~BluetoothBleCentralManagerStub() {}
};
} // namespace Bluetooth
} // namespace OHOS
#endif // PLUGINS_BLUETOOTH_COMMON_INCLUDE_BLUETOOTH_BLECENTRALMANAGER_STUB_H