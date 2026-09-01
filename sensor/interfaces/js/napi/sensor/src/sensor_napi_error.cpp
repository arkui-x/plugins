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

#include "sensor_napi_error.h"

namespace OHOS {
namespace Sensors {
namespace {
const char *GetErrorMessage(int32_t errCode)
{
    switch (errCode) {
        case SENSOR_ERR_PARAM:
            return "Parameter error. Possible causes: 1. Mandatory parameters are left unspecified; "
                   "2. Incorrect parameter types; 3. Parameter verification failed.";
        case SENSOR_ERR_NOT_SUPPORTED:
            return "Capability not supported.";
        case SENSOR_ERR_SERVICE:
            return "Service exception. Possible causes: 1. Sensor hdf service exception; "
                   "2. Sensor service ipc exception; 3. Sensor data channel exception.";
        default:
            return "Unknown sensor error.";
    }
}

napi_value CreateErrorObject(napi_env env, int32_t errCode, const std::string &errMessage)
{
    napi_value result = nullptr;
    napi_value code = nullptr;
    napi_value message = nullptr;
    napi_create_object(env, &result);
    napi_create_int32(env, errCode, &code);
    napi_create_string_utf8(env, errMessage.c_str(), NAPI_AUTO_LENGTH, &message);
    napi_set_named_property(env, result, "code", code);
    napi_set_named_property(env, result, "message", message);
    return result;
}
} // namespace

void ThrowSensorErr(napi_env env, int32_t errCode, const std::string &errMessage)
{
    napi_value error = CreateErrorObject(env, errCode, errMessage.empty() ? GetErrorMessage(errCode) : errMessage);
    napi_throw(env, error);
}

napi_value CreateSensorError(napi_env env, int32_t errCode, const std::string &errMessage)
{
    return CreateErrorObject(env, errCode, errMessage.empty() ? GetErrorMessage(errCode) : errMessage);
}

int32_t ToPublicErrCode(int32_t innerCode)
{
    switch (innerCode) {
        case SENSOR_ERR_PARAM:
        case SENSOR_ERR_NOT_SUPPORTED:
            return innerCode;
        default:
            return SENSOR_ERR_SERVICE;
    }
}
} // namespace Sensors
} // namespace OHOS
