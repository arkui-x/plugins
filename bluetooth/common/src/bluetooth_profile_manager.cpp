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

#include "bluetooth_profile_manager.h"

#include <atomic>
#include <mutex>

#include "bluetooth_a2dp_impl.h"
#include "bluetooth_ble_advertiser_impl.h"
#include "bluetooth_ble_centralmanager_impl.h"
#include "bluetooth_def.h"
#include "bluetooth_gatt_client_impl.h"
#include "bluetooth_gatt_server_impl.h"
#include "bluetooth_host.h"
#include "bluetooth_host_impl.h"
#include "bluetooth_log.h"
#include "bluetooth_no_destructor.h"
#include "i_bluetooth_ble_advertise_callback.h"
#include "i_bluetooth_ble_advertiser.h"
#include "i_bluetooth_host.h"

#ifdef ANDROID_PLATFORM
#include "bluetooth_jni.h"
#include "ohos_bt_gatt.h"
#endif

namespace OHOS {
namespace Bluetooth {
std::atomic_int32_t id = BEGIN_ID;
BluetoothProfileManager& BluetoothProfileManager::GetInstance()
{
    static BluetoothNoDestructor<BluetoothProfileManager> instance;
    return *instance;
}

BluetoothProfileManager::BluetoothProfileManager()
{
    isBluetoothServiceOn_ = true;
    std::lock_guard<std::mutex> lock(getProfileRemoteMutex_);
    sptr<IRemoteObject> bluetoothHostProxy = new (std::nothrow) BluetoothHostImpl();
    if (bluetoothHostProxy != nullptr) {
        profileRemoteMap_.Insert(BLUETOOTH_HOST, bluetoothHostProxy);
    }
    sptr<IRemoteObject> bluetoothBleCentralManagerProxy = new (std::nothrow) BluetoothBleCentralManagerImpl();
    if (bluetoothBleCentralManagerProxy != nullptr) {
        profileRemoteMap_.Insert(BLE_CENTRAL_MANAGER_SERVER, bluetoothBleCentralManagerProxy);
    }
    sptr<IRemoteObject> bluetoothBleAdvertiserProxy = new (std::nothrow) BluetoothBleAdvertiserImpl();
    if (bluetoothBleAdvertiserProxy != nullptr) {
        profileRemoteMap_.Insert(BLE_ADVERTISER_SERVER, bluetoothBleAdvertiserProxy);
    }
    sptr<IRemoteObject> bluetoothGattServerProxy = new (std::nothrow) BluetoothGattServerImpl();
    if (bluetoothGattServerProxy != nullptr) {
        profileRemoteMap_.Insert(PROFILE_GATT_SERVER, bluetoothGattServerProxy);
    }
    sptr<IRemoteObject> bluetoothGattClientProxy = new (std::nothrow) BluetoothGattClientImpl();
    if (bluetoothGattClientProxy != nullptr) {
        profileRemoteMap_.Insert(PROFILE_GATT_CLIENT, bluetoothGattClientProxy);
    }
    sptr<IRemoteObject> bluetoothA2DPProxy = new (std::nothrow) BluetoothA2DPImpl();
    if (bluetoothA2DPProxy != nullptr) {
        profileRemoteMap_.Insert(PROFILE_A2DP_SRC, bluetoothA2DPProxy);
    }
}

BluetoothProfileManager::~BluetoothProfileManager()
{
    std::lock_guard<std::mutex> lock(getProfileRemoteMutex_);
    profileRemoteMap_.Clear();
}

sptr<IRemoteObject> BluetoothProfileManager::GetHostRemote()
{
    sptr<IRemoteObject> value = nullptr;
    profileRemoteMap_.Find(BLUETOOTH_HOST, value);
    return value;
}

sptr<IRemoteObject> BluetoothProfileManager::GetProfileRemote(const std::string& objectName)
{
    sptr<IRemoteObject> remote = nullptr;
    profileRemoteMap_.Find(objectName, remote);
    return remote;
}

void BluetoothProfileManager::NotifyBluetoothStateChange(int32_t transport, int32_t status) {}

int32_t BluetoothProfileManager::GetValidId()
{
    ProfileIdProperty value;
    while (profileIdFuncMap_.Find(id, value)) {
        id++;
        if (id == INT32_MAX) {
            id = BEGIN_ID;
        }
    }

    return id;
}

int32_t BluetoothProfileManager::RegisterFunc(
    const std::string& objectName, std::function<void(sptr<IRemoteObject>)> func)
{
    GetValidId();
    ProfileIdProperty value;
    ProfileIdProperty idProperties;
    idProperties.objectName = objectName;
    idProperties.functions.bluetoothLoadedfunc = func;
    int32_t idForPrint = id;
    HILOGI("objectname: %{public}s, id: %{public}d", objectName.c_str(), idForPrint);
    profileIdFuncMap_.Insert(id, idProperties);
    if (isBluetoothServiceOn_) {
        sptr<IRemoteObject> remote = GetProfileRemote(objectName);
        CHECK_AND_RETURN_LOG_RET(remote != nullptr, id, "remote is nullptr");
        func(remote);
    }
    return id;
}

int32_t BluetoothProfileManager::RegisterFunc(const std::string& objectName, ProfileFunctions profileFunctions)
{
    GetValidId();
    ProfileIdProperty value;
    ProfileIdProperty idProperties;
    idProperties.objectName = objectName;
    idProperties.functions = profileFunctions;
    int32_t idForPrint = id;
    HILOGI("objectname: %{public}s, id: %{public}d", objectName.c_str(), idForPrint);
    profileIdFuncMap_.Insert(id, idProperties);
    if (isBluetoothServiceOn_) {
        sptr<IRemoteObject> remote = GetProfileRemote(objectName);
        CHECK_AND_RETURN_LOG_RET(remote != nullptr, id, "remote is nullptr");
        if (profileFunctions.bluetoothLoadedfunc) {
            profileFunctions.bluetoothLoadedfunc(remote);
        }
        if (profileFunctions.bleTurnOnFunc && IS_BLE_ENABLED()) {
            profileFunctions.bleTurnOnFunc(remote);
        }
    }
    return id;
}

void BluetoothProfileManager::DeregisterFunc(int32_t id)
{
    ProfileIdProperty value;
    CHECK_AND_RETURN_LOG(profileIdFuncMap_.Find(id, value), "id is not exist");
    profileIdFuncMap_.Erase(id);
}

bool BluetoothProfileManager::IsBluetoothServiceOn()
{
    return isBluetoothServiceOn_.load();
}

void BluetoothProfileManager::ClearSystemAbility() {}

} // namespace Bluetooth
} // namespace OHOS
