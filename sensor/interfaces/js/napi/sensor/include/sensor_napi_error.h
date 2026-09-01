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

#ifndef SENSOR_NAPI_ERROR_H
#define SENSOR_NAPI_ERROR_H

#include <string>

#include "napi/native_node_api.h"

namespace OHOS {
namespace Sensors {

/** Parameter check failed: mandatory parameter missing or wrong type. */
constexpr int32_t SENSOR_ERR_PARAM = 401;
/** The running platform does not provide this capability. */
constexpr int32_t SENSOR_ERR_NOT_SUPPORTED = 801;
/** The sensor service is unavailable or the device lacks the sensor. */
constexpr int32_t SENSOR_ERR_SERVICE = 14500101;

/**
 * @brief Throws a BusinessError carrying the given code and message.
 *
 * The messages follow the wording of the @ohos.sensor reference document.
 */
void ThrowSensorErr(napi_env env, int32_t errCode, const std::string &errMessage);

/**
 * @brief Creates the JS object handed to on* callbacks on failure.
 */
napi_value CreateSensorError(napi_env env, int32_t errCode, const std::string &errMessage);

/**
 * @brief Maps an internal return code onto the public error space.
 */
int32_t ToPublicErrCode(int32_t innerCode);

} // namespace Sensors
} // namespace OHOS

#endif // SENSOR_NAPI_ERROR_H
