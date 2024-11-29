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

#ifndef PLUGINS_BLUETOOTH_COMMON_MOCK_NAPI_TIMER_H
#define PLUGINS_BLUETOOTH_COMMON_MOCK_NAPI_TIMER_H

#include <functional>
#include <memory>

namespace OHOS {
namespace Bluetooth {
class NapiTimer {
public:
    using TimerCallback = std::function<void()>;
    static constexpr uint32_t DEFAULT_TIMEOUT = 10000; // 10s

    static NapiTimer* GetInstance(void);

    NapiTimer();
    ~NapiTimer();

    int Register(const TimerCallback& callback, uint32_t& outTimerId, uint32_t interval = DEFAULT_TIMEOUT);
    void Unregister(uint32_t timerId);
};
} // namespace Bluetooth
} // namespace OHOS
#endif // PLUGINS_BLUETOOTH_COMMON_MOCK_NAPI_TIMER_H