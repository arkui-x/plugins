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

#ifndef PLUGINS_BLUETOOTH_COMMON_INCLUDE_BLUETOOTH_GATT_CLIENT_STUB_H
#define PLUGINS_BLUETOOTH_COMMON_INCLUDE_BLUETOOTH_GATT_CLIENT_STUB_H

#include "i_bluetooth_gatt_client.h"
#include "i_bluetooth_gatt_client_callback.h"
#include "iremote_stub.h"

namespace OHOS {
namespace Bluetooth {
class BluetoothGattClientStub : public IRemoteStub<IBluetoothGattClient> {
public:
    BluetoothGattClientStub() {}
    ~BluetoothGattClientStub() {}
};
} // namespace Bluetooth
} // namespace OHOS
#endif // PLUGINS_BLUETOOTH_COMMON_INCLUDE_BLUETOOTH_GATT_CLIENT_STUB_H