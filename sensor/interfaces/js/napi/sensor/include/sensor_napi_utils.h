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

#ifndef SENSOR_NAPI_UTILS_H
#define SENSOR_NAPI_UTILS_H

#include <string>

#include "napi/native_node_api.h"

#include "i_sensor_service.h"

namespace OHOS {
namespace Sensors {

/**
 * @brief Reads the optional Options argument of the on* APIs.
 *
 * @return The sampling interval in nanoseconds; DEFAULT_INTERVAL_NS of the
 *         reference document (200,000,000) when the option is absent.
 */
int64_t ParseSensorOptions(napi_env env, napi_value options);

/**
 * @brief Builds the JS response object of one sensor sample.
 *
 * The field set depends on the sensor type: x/y/z for accelerometer and
 * gyroscope, alpha/beta/gamma for orientation, and one scalar field for
 * light, proximity and barometer. Every response carries timestamp and
 * accuracy.
 */
napi_value CreateSensorResponse(napi_env env, const SensorEventData &event);

/**
 * @brief Checks that the argument at the given index is a function.
 */
bool IsFunctionArg(napi_env env, napi_value arg);

} // namespace Sensors
} // namespace OHOS

#endif // SENSOR_NAPI_UTILS_H
