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

#ifndef SENSOR_SERVICE_H
#define SENSOR_SERVICE_H

#include "i_sensor_service.h"

namespace OHOS {
namespace Sensors {

/**
 * @brief Entry point that resolves the platform implementation once.
 *
 * The returned instance lives for the whole process; callers do not manage
 * its lifetime.
 */
ISensorService &GetSensorService();

} // namespace Sensors
} // namespace OHOS

#endif // SENSOR_SERVICE_H
