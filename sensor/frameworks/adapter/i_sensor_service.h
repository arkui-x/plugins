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

#ifndef I_SENSOR_SERVICE_H
#define I_SENSOR_SERVICE_H

#include <cstdint>
#include <functional>

namespace OHOS {
namespace Sensors {

/**
 * @brief Sensor type identifier exposed by the @ohos.sensor module.
 *
 * The values follow the OpenHarmony SensorId definition; only the types
 * carried by this adaptation are listed here.
 */
enum SensorTypeId {
    SENSOR_TYPE_ID_ACCELEROMETER = 1,
    SENSOR_TYPE_ID_GYROSCOPE = 2,
    SENSOR_TYPE_ID_AMBIENT_LIGHT = 5,
    SENSOR_TYPE_ID_BAROMETER = 8,
    SENSOR_TYPE_ID_PROXIMITY = 12,
    SENSOR_TYPE_ID_ORIENTATION = 256,
};

/**
 * @brief Data accuracy reported together with a sensor sample.
 *
 * The values are compatible with the Android ASensorEvent accuracy field,
 * which is passed through unchanged.
 */
enum SensorDataAccuracy {
    SENSOR_ACCURACY_UNRELIABLE = 0,
    SENSOR_ACCURACY_LOW = 1,
    SENSOR_ACCURACY_MEDIUM = 2,
    SENSOR_ACCURACY_HIGH = 3,
};

/**
 * @brief One sensor sample delivered by the platform implementation.
 *
 * The meaning of the value fields depends on the sensor type; unused fields
 * stay zero. Timestamp is in nanoseconds since boot.
 */
struct SensorEventData {
    int32_t sensorTypeId = SENSOR_TYPE_ID_ACCELEROMETER;
    int32_t accuracy = SENSOR_ACCURACY_UNRELIABLE;
    int64_t timestamp = 0;
    double values[3] = {0.0, 0.0, 0.0};
};

/**
 * @brief Callback invoked for every sensor sample.
 *
 * The callback is invoked on the platform event thread; the receiver is
 * responsible for thread switching.
 */
using SensorDataCallback = std::function<void(const SensorEventData &event)>;

/**
 * @brief Platform adapter of the @ohos.sensor module.
 *
 * One subscription is kept per sensor type: a second Subscribe replaces the
 * sampling parameters, Unsubscribe stops the platform event source.
 */
class ISensorService {
public:
    ISensorService() = default;
    virtual ~ISensorService() = default;

    /**
     * @brief Start the platform event source of the given sensor.
     *
     * @param sensorTypeId Sensor type to subscribe.
     * @param intervalNs   Desired sampling interval in nanoseconds; the
     *                     implementation clamps it to the hardware range.
     * @param callback     Receiver of the samples; must not be null.
     * @return 0 on success; 14500101 when the sensor is unavailable on the
     *         device; 801 when the platform provides no sensor framework.
     */
    virtual int32_t SubscribeSensor(int32_t sensorTypeId, int64_t intervalNs,
                                    const SensorDataCallback &callback) = 0;

    /**
     * @brief Stop the platform event source of the given sensor.
     *
     * @param sensorTypeId Sensor type to unsubscribe.
     * @return 0 on success, an error code otherwise.
     */
    virtual int32_t UnsubscribeSensor(int32_t sensorTypeId) = 0;
};

} // namespace Sensors
} // namespace OHOS

#endif // I_SENSOR_SERVICE_H
