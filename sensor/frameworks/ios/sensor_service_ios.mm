/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#import <Foundation/Foundation.h>
#include "sensor_service.h"

namespace OHOS {
namespace Sensors {
namespace {
constexpr int32_t IS_NOT_SUPPORTED = 801;

/**
 * @brief Placeholder adapter for the iOS platform.
 *
 * The sensor adaptation lands on Android first; on iOS every entry reports
 * the capability as unsupported so that the module loads and the callers
 * receive the documented 801 error code.
 */
class SensorServiceIos final : public ISensorService {
public:
    SensorServiceIos() = default;
    ~SensorServiceIos() override = default;

    int32_t SubscribeSensor(int32_t sensorTypeId, int64_t intervalNs,
                            const SensorDataCallback &callback) override
    {
        (void)sensorTypeId;
        (void)intervalNs;
        (void)callback;
        return IS_NOT_SUPPORTED;
    }

    int32_t UnsubscribeSensor(int32_t sensorTypeId) override
    {
        (void)sensorTypeId;
        return IS_NOT_SUPPORTED;
    }
};
} // namespace

ISensorService &GetSensorService()
{
    static SensorServiceIos instance;
    return instance;
}
} // namespace Sensors
} // namespace OHOS
