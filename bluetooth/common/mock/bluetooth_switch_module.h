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

#ifndef PLUGINS_BLUETOOTH_COMMON_MOCK_BLUETOOTH_SWITCH_MODULE_H
#define PLUGINS_BLUETOOTH_COMMON_MOCK_BLUETOOTH_SWITCH_MODULE_H

#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

#include "ffrt_inner.h"

namespace OHOS {
namespace Bluetooth {
class IBluetoothSwitchAction {
public:
    IBluetoothSwitchAction() = default;
    virtual ~IBluetoothSwitchAction() = default;

    virtual int EnableBluetooth(void) = 0;
    virtual int DisableBluetooth(void) = 0;
    virtual int EnableBluetoothToRestrictMode(void) = 0;
};

enum class BluetoothSwitchEvent : int {
    NONE = -1,
    ENABLE_BLUETOOTH = 0,
    DISABLE_BLUETOOTH,
    ENABLE_BLUETOOTH_TO_RESTRICE_MODE,
    BLUETOOTH_ON,
    BLUETOOTH_OFF,
    BLUETOOTH_HALF,
};

class BluetoothSwitchModule : public std::enable_shared_from_this<BluetoothSwitchModule> {
public:
    explicit BluetoothSwitchModule(std::unique_ptr<IBluetoothSwitchAction> switchAction)
        : ffrtQueue_("bt_switch"), switchAction_(std::move(switchAction))
    {}
    ~BluetoothSwitchModule() = default;

    int ProcessBluetoothSwitchEvent(BluetoothSwitchEvent event, std::string callingName = "", bool isAsync = false);
    void SetNoAutoConnect(bool);

private:
    int ProcessEnableBluetoothEvent(void);
    int ProcessDisableBluetoothEvent(void);
    int ProcessEnableBluetoothToRestrictModeEvent(void);
    int ProcessBluetoothOnEvent(void);
    int ProcessBluetoothOffEvent(void);
    int ProcessBluetoothHalfEvent(void);
    int ProcessBluetoothSwitchAction(std::function<int(void)> action, BluetoothSwitchEvent cachedEvent);
    int ProcessBluetoothSwitchCachedEvent(BluetoothSwitchEvent event);
    int ProcessBluetoothSwitchActionEnd(
        BluetoothSwitchEvent curSwitchActionEvent, std::vector<BluetoothSwitchEvent> expectedEventVec);
    void DeduplicateCacheEvent(BluetoothSwitchEvent curEvent);
    void LogCacheEventIgnored(std::vector<BluetoothSwitchEvent> eventVec);
    void LogBluetoothSwitchEvent(BluetoothSwitchEvent event);
    void OnTaskTimeout(void);

    const uint64_t DEFAULT_TASK_TIMEOUT = 10000000; // 10s
    uint64_t taskTimeout_ = DEFAULT_TASK_TIMEOUT;
    ffrt::task_handle taskTimeoutHandle_;
    ffrt::queue ffrtQueue_;

    std::unique_ptr<IBluetoothSwitchAction> switchAction_ { nullptr };
    std::atomic_bool isBtSwitchProcessing_ { false };
    std::vector<BluetoothSwitchEvent> cachedEventVec_ {};
    std::mutex bluetoothSwitchEventMutex_ {}; // Used for ProcessBluetoothSwitchEvent function
    std::atomic_bool noAutoConnect_ { false };
};
} // namespace Bluetooth
} // namespace OHOS
#endif // PLUGINS_BLUETOOTH_COMMON_MOCK_BLUETOOTH_SWITCH_MODULE_H
