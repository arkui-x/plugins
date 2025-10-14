/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef VIBRATOR_PATTERN_BUILDER_H
#define VIBRATOR_PATTERN_BUILDER_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "vibrator_agent_type.h"

namespace OHOS {
namespace Sensors {
enum VibrateTag {
    EVENT_TAG_UNKNOWN = -1,
    EVENT_TAG_CONTINUOUS = 0,
    EVENT_TAG_TRANSIENT = 1,
};

struct VibrateCurvePoint {
    int32_t time = 0;
    int32_t intensity = 0;
    int32_t frequency = 0;
};

struct VibrateEvent {
    VibrateTag tag;
    int32_t time = 0;
    int32_t duration = 0;
    int32_t intensity = 0;
    int32_t frequency = 0;
    int32_t index = 0;
    std::vector<VibrateCurvePoint> points;
};

class VibratorPatternBuilder {
public:
    VibratorPatternBuilder() = default;
    ~VibratorPatternBuilder() = default;
    static napi_value AddContinuousEvent(napi_env env, napi_callback_info info);
    static napi_value AddTransientEvent(napi_env env, napi_callback_info info);
    static napi_value Build(napi_env env, napi_callback_info info);
    static napi_value VibratorPatternConstructor(napi_env env, napi_callback_info info);

private:
    static bool ParseOptions(const napi_env &env, const napi_value &value, VibrateEvent &event);
    static bool ParseTransientOptions(const napi_env &env, const napi_value &value, VibrateEvent &event);
    static bool ParsePoints(const napi_env &env, const napi_value &value, VibrateEvent &event);
    static napi_value ConvertVibrateEvent(napi_env env, const VibrateEvent &event);
    static napi_value ConvertVibrateCurvePoint(napi_env env, const VibrateCurvePoint &point);
    static bool CheckParameters(const VibrateEvent &event);
    static bool CheckCurvePoints(const VibrateEvent &event);
    static napi_value ConvertEventArrayToNapiValue(napi_env env, const std::vector<VibrateEvent> &vibrateEvents);
    static bool ParseParameterOptions(const napi_env &env, const napi_value &value,
        VibrateEvent &event, bool &exist);

private:
    std::vector<VibrateEvent> events_;
};
}  // namespace Sensors
}  // namespace OHOS
#endif  // VIBRATOR_PATTERN_BUILDER_H