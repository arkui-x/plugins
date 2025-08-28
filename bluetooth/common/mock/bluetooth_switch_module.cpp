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

#include "bluetooth_switch_module.h"

#include <algorithm>

#include "bluetooth_errorcode.h"
#include "bluetooth_log.h"

namespace OHOS {
namespace Bluetooth {
static const char* ToString(BluetoothSwitchEvent event)
{
    switch (event) {
        case BluetoothSwitchEvent::ENABLE_BLUETOOTH:
            return "ENABLE_BLUETOOTH";
        case BluetoothSwitchEvent::DISABLE_BLUETOOTH:
            return "DISABLE_BLUETOOTH";
        case BluetoothSwitchEvent::ENABLE_BLUETOOTH_TO_RESTRICE_MODE:
            return "ENABLE_BLUETOOTH_TO_RESTRICE_MODE";
        case BluetoothSwitchEvent::BLUETOOTH_ON:
            return "BLUETOOTH_ON";
        case BluetoothSwitchEvent::BLUETOOTH_OFF:
            return "BLUETOOTH_OFF";
        case BluetoothSwitchEvent::BLUETOOTH_HALF:
            return "BLUETOOTH_HALF";
        default:
            break;
    }
    return "Unknown";
}

void BluetoothSwitchModule::LogBluetoothSwitchEvent(BluetoothSwitchEvent event) {}

int BluetoothSwitchModule::ProcessBluetoothSwitchEvent(BluetoothSwitchEvent event, std::string callingName, bool isAsyn)
{
    CHECK_AND_RETURN_LOG_RET(switchAction_, BT_ERR_INTERNAL_ERROR, "switchAction is nullptr");

    std::lock_guard<std::mutex> lock(bluetoothSwitchEventMutex_);
    LogBluetoothSwitchEvent(event);
    switch (event) {
        case BluetoothSwitchEvent::ENABLE_BLUETOOTH:
            return ProcessEnableBluetoothEvent();
        case BluetoothSwitchEvent::DISABLE_BLUETOOTH:
            return ProcessDisableBluetoothEvent();
        default:
            break;
    }
    HILOGI("Invalid event: %{public}s", ToString(event));
    return BT_ERR_INTERNAL_ERROR;
}

void BluetoothSwitchModule::OnTaskTimeout(void) {}

int BluetoothSwitchModule::ProcessBluetoothSwitchAction(
    std::function<int(void)> action, BluetoothSwitchEvent cachedEvent)
{
    return BT_NO_ERROR;
}

int BluetoothSwitchModule::ProcessEnableBluetoothEvent(void)
{
    return switchAction_->EnableBluetooth();
}

int BluetoothSwitchModule::ProcessDisableBluetoothEvent(void)
{
    return switchAction_->DisableBluetooth();
}

int BluetoothSwitchModule::ProcessEnableBluetoothToRestrictModeEvent(void)
{
    return BT_NO_ERROR;
}

int BluetoothSwitchModule::ProcessBluetoothOnEvent(void)
{
    return BT_NO_ERROR;
}

int BluetoothSwitchModule::ProcessBluetoothOffEvent(void)
{
    return BT_NO_ERROR;
}

int BluetoothSwitchModule::ProcessBluetoothHalfEvent(void)
{
    return BT_NO_ERROR;
}

int BluetoothSwitchModule::ProcessBluetoothSwitchActionEnd(
    BluetoothSwitchEvent curSwitchActionEvent, std::vector<BluetoothSwitchEvent> expectedEventVec)
{
    return BT_NO_ERROR;
}

int BluetoothSwitchModule::ProcessBluetoothSwitchCachedEvent(BluetoothSwitchEvent event)
{
    return BT_NO_ERROR;
}

void BluetoothSwitchModule::DeduplicateCacheEvent(BluetoothSwitchEvent curEvent) {}

void BluetoothSwitchModule::LogCacheEventIgnored(std::vector<BluetoothSwitchEvent> eventVec) {}

void BluetoothSwitchModule::SetNoAutoConnect(bool noAutoConnect)
{
    noAutoConnect_ = noAutoConnect;
}
} // namespace Bluetooth
} // namespace OHOS
