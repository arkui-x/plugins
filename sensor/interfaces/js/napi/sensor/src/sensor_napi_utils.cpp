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

#include "sensor_napi_utils.h"

#include <cmath>

namespace OHOS {
namespace Sensors {
namespace {
constexpr int64_t DEFAULT_INTERVAL_NS = 200000000;
constexpr const char *FIELD_TIMESTAMP = "timestamp";
constexpr const char *FIELD_ACCURACY = "accuracy";
constexpr const char *FIELD_X = "x";
constexpr const char *FIELD_Y = "y";
constexpr const char *FIELD_Z = "z";
constexpr const char *FIELD_ALPHA = "alpha";
constexpr const char *FIELD_BETA = "beta";
constexpr const char *FIELD_GAMMA = "gamma";
constexpr const char *FIELD_INTENSITY = "intensity";
constexpr const char *FIELD_DISTANCE = "distance";
constexpr const char *FIELD_PRESSURE = "pressure";

void SetDoubleField(napi_env env, napi_value object, const char *name, double value)
{
    napi_value field = nullptr;
    napi_create_double(env, value, &field);
    napi_set_named_property(env, object, name, field);
}

void SetInt64Field(napi_env env, napi_value object, const char *name, int64_t value)
{
    napi_value field = nullptr;
    napi_create_int64(env, value, &field);
    napi_set_named_property(env, object, name, field);
}

void SetInt32Field(napi_env env, napi_value object, const char *name, int32_t value)
{
    napi_value field = nullptr;
    napi_create_int32(env, value, &field);
    napi_set_named_property(env, object, name, field);
}
} // namespace

int64_t ParseSensorOptions(napi_env env, napi_value options)
{
    if (options == nullptr) {
        return DEFAULT_INTERVAL_NS;
    }
    napi_valuetype type = napi_undefined;
    napi_typeof(env, options, &type);
    if (type != napi_object) {
        return DEFAULT_INTERVAL_NS;
    }
    bool hasInterval = false;
    napi_has_named_property(env, options, "interval", &hasInterval);
    if (!hasInterval) {
        return DEFAULT_INTERVAL_NS;
    }
    napi_value value = nullptr;
    napi_get_named_property(env, options, "interval", &value);
    napi_typeof(env, value, &type);
    if (type != napi_number) {
        return DEFAULT_INTERVAL_NS;
    }
    double interval = 0.0;
    napi_get_value_double(env, value, &interval);
    if (std::isnan(interval) || interval <= 0.0) {
        return DEFAULT_INTERVAL_NS;
    }
    return static_cast<int64_t>(interval);
}

napi_value CreateSensorResponse(napi_env env, const SensorEventData &event)
{
    napi_value result = nullptr;
    napi_create_object(env, &result);
    SetInt64Field(env, result, FIELD_TIMESTAMP, event.timestamp);
    SetInt32Field(env, result, FIELD_ACCURACY, event.accuracy);
    switch (event.sensorTypeId) {
        case SENSOR_TYPE_ID_ACCELEROMETER:
        case SENSOR_TYPE_ID_GYROSCOPE:
            SetDoubleField(env, result, FIELD_X, event.values[0]);
            SetDoubleField(env, result, FIELD_Y, event.values[1]);
            SetDoubleField(env, result, FIELD_Z, event.values[2]);
            break;
        case SENSOR_TYPE_ID_ORIENTATION:
            SetDoubleField(env, result, FIELD_ALPHA, event.values[0]);
            SetDoubleField(env, result, FIELD_BETA, event.values[1]);
            SetDoubleField(env, result, FIELD_GAMMA, event.values[2]);
            break;
        case SENSOR_TYPE_ID_AMBIENT_LIGHT:
            SetDoubleField(env, result, FIELD_INTENSITY, event.values[0]);
            break;
        case SENSOR_TYPE_ID_PROXIMITY:
            SetDoubleField(env, result, FIELD_DISTANCE, event.values[0]);
            break;
        case SENSOR_TYPE_ID_BAROMETER:
            SetDoubleField(env, result, FIELD_PRESSURE, event.values[0]);
            break;
        default:
            break;
    }
    return result;
}

bool IsFunctionArg(napi_env env, napi_value arg)
{
    if (arg == nullptr) {
        return false;
    }
    napi_valuetype type = napi_undefined;
    napi_typeof(env, arg, &type);
    return type == napi_function;
}
} // namespace Sensors
} // namespace OHOS
