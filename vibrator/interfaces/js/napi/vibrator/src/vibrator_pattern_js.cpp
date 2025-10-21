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

#include "vibrator_pattern_js.h"

#include <optional>
#include <string>

#include "hilog/log.h"
#include "securec.h"

#include "miscdevice_log.h"
#include "vibrator_napi_error.h"
#include "vibrator_napi_utils.h"

#undef LOG_TAG
#define LOG_TAG "VibratorPatternJs"

namespace OHOS {
namespace Sensors {
namespace {
constexpr int32_t EVENT_START_TIME_MAX = 1800000;
constexpr int32_t EVENT_NUM_MAX = 128;
constexpr int32_t TRANSIENT_VIBRATION_DURATION = 48;
constexpr int32_t INTENSITY_MIN = 0;
constexpr int32_t INTENSITY_MAX = 100;
constexpr int32_t FREQUENCY_MIN = 0;
constexpr int32_t FREQUENCY_MAX = 100;
constexpr int32_t CURVE_POINT_INTENSITY_MAX = 100;
constexpr int32_t CURVE_POINT_NUM_MIN = 4;
constexpr int32_t CURVE_POINT_NUM_MAX = 16;
constexpr int32_t CURVE_FREQUENCY_MIN = -100;
constexpr int32_t CURVE_FREQUENCY_MAX = 100;
constexpr int32_t CONTINUOUS_DURATION_MAX = 5000;
constexpr double CURVE_INTENSITY_SCALE = 100.0;
constexpr int32_t DEFAULT_EVENT_INTENSITY = 100;
constexpr int32_t DEFAULT_EVENT_FREQUENCY = 50;
constexpr int32_t DEFAULT_POINT_INTENSITY = 100;
constexpr int32_t EVENT_INDEX_MAX = 2;
constexpr int32_t PARAMETER_TWO = 2;
constexpr int32_t PARAMETER_THREE = 3;
} // namespace

napi_value VibratorPatternBuilder::VibratorPatternConstructor(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    size_t argc = 0;
    napi_value args[1] = {0};
    napi_value res = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &res, &data));

    auto vibratorPatternBuilder = new (std::nothrow) VibratorPatternBuilder();
    CHKPP(vibratorPatternBuilder);
    napi_wrap(
        env, res, vibratorPatternBuilder,
        [](napi_env env, void* data, void* hint) {
            auto vibratorPatternBuilder = static_cast<VibratorPatternBuilder*>(data);
            if (vibratorPatternBuilder != nullptr) {
                delete vibratorPatternBuilder;
            }
        },
        nullptr, nullptr);
    return res;
}

bool VibratorPatternBuilder::ParsePoints(const napi_env &env, const napi_value &value,
    VibrateEvent &event)
{
    CALL_LOG_ENTER;
    napi_value points = nullptr;
    CHKCF((napi_get_named_property(env, value, "points", &points) == napi_ok), "napi get points fail");
    CHKCF(IsMatchArrayType(env, points), "The points parameter type is incorrect. Napi array expected");
    uint32_t length = 0;
    CHKCF((napi_get_array_length(env, points, &length) == napi_ok), "napi_get_array_length fail");
    for (size_t i = 0; i < length; ++i) {
        napi_value element = nullptr;
        CHKCF((napi_get_element(env, points, i, &element) == napi_ok), "napi_get_element fail");
        VibrateCurvePoint point;
        napi_value timeValue = nullptr;
        CHKCF((napi_get_named_property(env, element, "time", &timeValue) == napi_ok), "napi get time fail");
        CHKCF(IsMatchType(env, timeValue, napi_number),
            "The time parameter type is incorrect. napi_number expected");
        CHKCF(GetInt32Value(env, timeValue, point.time), "Get int number fail");
        bool exist = false;
        napi_status status = napi_has_named_property(env, element, "intensity", &exist);
        if ((status == napi_ok) && exist) {
            napi_value intensityValue = nullptr;
            CHKCF((napi_get_named_property(env, element, "intensity", &intensityValue) == napi_ok),
                "napi get intensity fail");
            CHKCF(IsMatchType(env, intensityValue, napi_number),
                "The points intensity parameter type is incorrect. napi_number expected");
            double intensityResult = 0.0;
            CHKCF(GetDoubleValue(env, intensityValue, intensityResult), "Get double number fail");
            point.intensity = static_cast<int32_t>(intensityResult * CURVE_INTENSITY_SCALE);
        } else {
            point.intensity = DEFAULT_POINT_INTENSITY;
        }
        status = napi_has_named_property(env, element, "frequency", &exist);
        if ((status == napi_ok) && exist) {
            napi_value frequencyValue = nullptr;
            CHKCF((napi_get_named_property(env, element, "frequency", &frequencyValue) == napi_ok),
                "napi get frequency fail");
            CHKCF(IsMatchType(env, frequencyValue, napi_number),
                "The points frequency parameter type is incorrect. napi_number expected");
            CHKCF(GetInt32Value(env, frequencyValue, point.frequency), "Get int number fail");
        } else {
            point.frequency = 0;
        }
        event.points.push_back(point);
    }
    return true;
}

bool VibratorPatternBuilder::ParseOptions(const napi_env &env, const napi_value &value,
    VibrateEvent &event)
{
    CALL_LOG_ENTER;
    bool exist = false;
    if (!ParseParameterOptions(env, value, event, exist)) {
        return false;
    }
    napi_status status = napi_has_named_property(env, value, "points", &exist);
    if ((status == napi_ok) && exist) {
        if (!ParsePoints(env, value, event)) {
            MISC_HILOGE("ParsePoints fail");
            return false;
        }
    }
    status = napi_has_named_property(env, value, "index", &exist);
    if ((status == napi_ok) && exist) {
        napi_value index = nullptr;
        CHKCF((napi_get_named_property(env, value, "index", &index) == napi_ok), "napi get frequency fail");
        CHKCF(IsMatchType(env, index, napi_number), "The index parameter type is incorrect. napi_number expected");
        CHKCF(GetInt32Value(env, index, event.index), "Get int number frequency fail");
    } else {
        event.index = 0;
    }
    return true;
}

bool VibratorPatternBuilder::ParseParameterOptions(const napi_env &env, const napi_value &value,
    VibrateEvent &event, bool &exist)
{
    CALL_LOG_ENTER;
    napi_status status = napi_has_named_property(env, value, "intensity", &exist);
    if ((status == napi_ok) && exist) {
        napi_value intensity = nullptr;
        CHKCF((napi_get_named_property(env, value, "intensity", &intensity) == napi_ok), "napi get intensity fail");
        CHKCF(IsMatchType(env, intensity, napi_number),
            "The intensity parameter type is incorrect. napi_number expected");
        CHKCF(GetInt32Value(env, intensity, event.intensity), "Get int number intensity fail");
    } else {
        event.intensity = DEFAULT_EVENT_INTENSITY;
    }
    status = napi_has_named_property(env, value, "frequency", &exist);
    if ((status == napi_ok) && exist) {
        napi_value frequency = nullptr;
        CHKCF((napi_get_named_property(env, value, "frequency", &frequency) == napi_ok), "napi get frequency fail");
        CHKCF(IsMatchType(env, frequency, napi_number),
            "The frequency parameter type is incorrect. napi_number expected");
        CHKCF(GetInt32Value(env, frequency, event.frequency), "Get int number frequency fail");
    } else {
        event.frequency = DEFAULT_EVENT_FREQUENCY;
    }
    return true;
}

bool VibratorPatternBuilder::ParseTransientOptions(const napi_env &env, const napi_value &value,
    VibrateEvent &event)
{
    CALL_LOG_ENTER;
    bool exist = false;
    if (!ParseParameterOptions(env, value, event, exist)) {
        return false;
    }
    napi_status status = napi_has_named_property(env, value, "index", &exist);
    if ((status == napi_ok) && exist) {
        napi_value index = nullptr;
        CHKCF((napi_get_named_property(env, value, "index", &index) == napi_ok), "napi get frequency fail");
        CHKCF(IsMatchType(env, index, napi_number), "The index parameter type is incorrect. napi_number expected");
        CHKCF(GetInt32Value(env, index, event.index), "Get int number frequency fail");
    } else {
        event.index = 0;
    }
    return true;
}

bool VibratorPatternBuilder::CheckCurvePoints(const VibrateEvent &event)
{
    int32_t pointNum = static_cast<int32_t>(event.points.size());
    if ((pointNum < CURVE_POINT_NUM_MIN) || (pointNum > CURVE_POINT_NUM_MAX)) {
        MISC_HILOGE("The points size is out of range, size:%{public}d", pointNum);
        return false;
    }
    for (int32_t i = 0; i < pointNum; ++i) {
        if ((event.points[i].time < 0) || (event.points[i].time > event.duration)) {
            MISC_HILOGE("time in points is out of range, time:%{public}d", event.points[i].time);
            return false;
        }
        if ((event.points[i].intensity < 0) || (event.points[i].intensity > CURVE_POINT_INTENSITY_MAX)) {
            MISC_HILOGE("intensity in points is out of range, intensity:%{public}d", event.points[i].intensity);
            return false;
        }
        if ((event.points[i].frequency < CURVE_FREQUENCY_MIN) || (event.points[i].frequency > CURVE_FREQUENCY_MAX)) {
            MISC_HILOGE("frequency in points is out of range, frequency:%{public}d", event.points[i].frequency);
            return false;
        }
    }
    return true;
}

bool VibratorPatternBuilder::CheckParameters(const VibrateEvent &event)
{
    CALL_LOG_ENTER;
    if ((event.time < 0) || (event.time > EVENT_START_TIME_MAX)) {
        MISC_HILOGE("The event time is out of range, time:%{public}d", event.time);
        return false;
    }
    if ((event.frequency < FREQUENCY_MIN) || (event.frequency > FREQUENCY_MAX)) {
        MISC_HILOGE("The event frequency is out of range, frequency:%{public}d", event.frequency);
        return false;
    }
    if ((event.intensity < INTENSITY_MIN) || (event.intensity > INTENSITY_MAX)) {
        MISC_HILOGE("The event intensity is out of range, intensity:%{public}d", event.intensity);
        return false;
    }
    if ((event.duration <= 0) || (event.duration > CONTINUOUS_DURATION_MAX)) {
        MISC_HILOGE("The event duration is out of range, duration:%{public}d", event.duration);
        return false;
    }
    if ((event.index < 0) || (event.index > EVENT_INDEX_MAX)) {
        MISC_HILOGE("The event index is out of range, index:%{public}d", event.index);
        return false;
    }
    if ((event.tag == VibrateTag::EVENT_TAG_CONTINUOUS) && !event.points.empty()) {
        if (!CheckCurvePoints(event)) {
            MISC_HILOGE("CheckCurvePoints failed");
            return false;
        }
    }
    return true;
}

napi_value VibratorPatternBuilder::ConvertVibrateCurvePoint(napi_env env, const VibrateCurvePoint &point)
{
    CALL_LOG_ENTER;
    napi_value pointObj;
    napi_status status = napi_create_object(env, &pointObj);
    if (status != napi_ok) {
        MISC_HILOGE("napi_create_object fail");
        return nullptr;
    }
    CHKCP(CreateInt32Property(env, pointObj, "time", point.time), "CreateInt32Property time fail");
    CHKCP(CreateInt32Property(env, pointObj, "intensity", point.intensity), "CreateInt32Property intensity fail");
    CHKCP(CreateInt32Property(env, pointObj, "frequency", point.frequency), "CreateInt32Property frequency fail");
    return pointObj;
}

napi_value VibratorPatternBuilder::ConvertVibrateEvent(napi_env env, const VibrateEvent &event)
{
    CALL_LOG_ENTER;
    napi_value eventObj = nullptr;
    CHKCP((napi_create_object(env, &eventObj) == napi_ok), "napi_create_object fail");
    CHKCP(CreateInt32Property(env, eventObj, "eventType", static_cast<int32_t>(event.tag)),
        "CreateInt32Property type fail");
    CHKCP(CreateInt32Property(env, eventObj, "time", event.time), "CreateInt32Property time fail");
    CHKCP(CreateInt32Property(env, eventObj, "duration", event.duration), "CreateInt32Property duration fail");
    CHKCP(CreateInt32Property(env, eventObj, "intensity", event.intensity), "CreateInt32Property intensity fail");
    CHKCP(CreateInt32Property(env, eventObj, "frequency", event.frequency), "CreateInt32Property frequency fail");
    CHKCP(CreateInt32Property(env, eventObj, "index", event.index), "CreateInt32Property index fail");
    int32_t pointNum = static_cast<int32_t>(event.points.size());
    napi_value pointsArray = nullptr;
    CHKCP((napi_create_array_with_length(env, pointNum, &pointsArray) == napi_ok),
        "napi_create_array_with_length fail");
    for (int i = 0; i < pointNum; ++i) {
        napi_value pointObj = ConvertVibrateCurvePoint(env, event.points[i]);
        if (pointObj == nullptr) {
            MISC_HILOGE("ConvertVibrateCurvePoint fail");
            return nullptr;
        }
        CHKCP((napi_set_element(env, pointsArray, i, pointObj) == napi_ok), "napi_set_element fail");
    }
    CHKCP((napi_set_named_property(env, eventObj, "points", pointsArray) == napi_ok),
        "napi_set_named_property fail");
    return eventObj;
}

napi_value VibratorPatternBuilder::ConvertEventArrayToNapiValue(napi_env env,
    const std::vector<VibrateEvent> &vibrateEvents)
{
    CALL_LOG_ENTER;
    napi_value result = nullptr;
    CHKCP((napi_create_object(env, &result) == napi_ok), "napi_create_object fail");
    int32_t absoluteTime = 0;
    napi_value time = nullptr;
    CHKCP((napi_create_int32(env, absoluteTime, &time) == napi_ok), "napi_create_int32 time fail");
    CHKCP((napi_set_named_property(env, result, "time", time) == napi_ok), "napi_set_named_property time fail");

    int32_t vibrateEventsNum = static_cast<int32_t>(vibrateEvents.size());
    napi_value eventsArray = nullptr;
    CHKCP((napi_create_array_with_length(env, vibrateEventsNum, &eventsArray) == napi_ok),
        "napi_create_array_with_length fail");
    for (int32_t i = 0; i < vibrateEventsNum; ++i) {
        napi_value eventObj = ConvertVibrateEvent(env, vibrateEvents[i]);
        if (eventObj == nullptr) {
            MISC_HILOGE("ConvertToNapiValue fail");
            return nullptr;
        }
        CHKCP((napi_set_element(env, eventsArray, i, eventObj) == napi_ok), "napi_set_element fail");
    }
    CHKCP((napi_set_named_property(env, result, "events", eventsArray) == napi_ok),
        "napi_setnapi_set_named_property_element fail");
    return result;
}

napi_value VibratorPatternBuilder::AddContinuousEvent(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    size_t argc = 3;
    napi_value args[PARAMETER_THREE] = {};
    napi_value thisArg = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &thisArg, nullptr);
    if ((status != napi_ok) || (argc < PARAMETER_TWO) || !IsMatchType(env, args[0], napi_number) ||
        !IsMatchType(env, args[1], napi_number)) {
        ThrowErr(env, PARAMETER_ERROR, "napi_get_cb_info fail or number of parameter invalid");
        return nullptr;
    }
    VibrateEvent event;
    event.tag = VibrateTag::EVENT_TAG_CONTINUOUS;
    CHKCP(GetInt32Value(env, args[0], event.time), "Get int number time fail");
    CHKCP(GetInt32Value(env, args[1], event.duration), "Get int number duration fail");
    if (argc > PARAMETER_TWO) {
        if (!IsMatchType(env, args[PARAMETER_TWO], napi_object)) {
            ThrowErr(env, PARAMETER_ERROR, "parameter invalid");
            return nullptr;
        }
        if (!ParseOptions(env, args[PARAMETER_TWO], event)) {
            ThrowErr(env, PARAMETER_ERROR, "ParseOptions fail");
            return nullptr;
        }
    } else {
        event.intensity = DEFAULT_EVENT_INTENSITY;
        event.frequency = DEFAULT_EVENT_FREQUENCY;
        event.index = 0;
    }
    if (!CheckParameters(event)) {
        ThrowErr(env, PARAMETER_ERROR, "Invalid parameter");
        return nullptr;
    }
    VibratorPatternBuilder *vibratorPattern = nullptr;
    CHKCP((napi_unwrap(env, thisArg, (void **)(&vibratorPattern)) == napi_ok), "napi_unwrap fail");
    if (vibratorPattern == nullptr) {
        MISC_HILOGE("vibratorPattern is nullptr");
        return nullptr;
    }
    vibratorPattern->events_.push_back(event);
    return thisArg;
}

napi_value VibratorPatternBuilder::AddTransientEvent(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    size_t argc = 3;
    napi_value args[3] = {};
    napi_value thisArg = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &thisArg, nullptr);
    if ((status != napi_ok) || (argc < 1) || !IsMatchType(env, args[0], napi_number)) {
        ThrowErr(env, PARAMETER_ERROR, "napi_get_cb_info fail or number of parameter invalid");
        return nullptr;
    }
    VibrateEvent event;
    event.tag = VibrateTag::EVENT_TAG_TRANSIENT;
    event.duration = TRANSIENT_VIBRATION_DURATION;
    CHKCP(GetInt32Value(env, args[0], event.time), "Get int number time fail");
    if (argc > 1) {
        if (!IsMatchType(env, args[1], napi_object)) {
            ThrowErr(env, PARAMETER_ERROR, "parameter invalid");
            return nullptr;
        }
        if (!ParseTransientOptions(env, args[1], event)) {
            ThrowErr(env, PARAMETER_ERROR, "ParseTransientOptions fail");
            return nullptr;
        }
    } else {
        event.intensity = DEFAULT_EVENT_INTENSITY;
        event.frequency = DEFAULT_EVENT_FREQUENCY;
        event.index = 0;
    }
    if (!CheckParameters(event)) {
        ThrowErr(env, PARAMETER_ERROR, "Invalid parameter");
        return nullptr;
    }
    VibratorPatternBuilder *vibratorPattern = nullptr;
    CHKCP((napi_unwrap(env, thisArg, (void **)(&vibratorPattern)) == napi_ok), "napi_unwrap fail");
    if (vibratorPattern == nullptr) {
        MISC_HILOGE("vibratorPattern is nullptr");
        return nullptr;
    }
    vibratorPattern->events_.push_back(event);
    return thisArg;
}

napi_value VibratorPatternBuilder::Build(napi_env env, napi_callback_info info)
{
    CALL_LOG_ENTER;
    size_t argc = 1;
    napi_value args[1] = {};
    napi_value thisArg = nullptr;
    napi_status status = napi_get_cb_info(env, info, &argc, args, &thisArg, nullptr);
    if (status != napi_ok) {
        ThrowErr(env, PARAMETER_ERROR, "napi_get_cb_info fail");
        return nullptr;
    }
    VibratorPatternBuilder *vibratorPattern = nullptr;
    CHKCP((napi_unwrap(env, thisArg, (void **)(&vibratorPattern)) == napi_ok), "napi_unwrap fail");
    if (vibratorPattern == nullptr) {
        MISC_HILOGE("vibratorPattern is nullptr");
        return nullptr;
    }
    int32_t eventNum = static_cast<int32_t>(vibratorPattern->events_.size());
    if ((eventNum <= 0) || (eventNum > EVENT_NUM_MAX)) {
        ThrowErr(env, PARAMETER_ERROR, "The number of events exceeds the range");
        return nullptr;
    }
    napi_value result = ConvertEventArrayToNapiValue(env, vibratorPattern->events_);
    if (result == nullptr) {
        MISC_HILOGE("ConvertEventArrayToNapiValue fail");
    }
    return result;
}
} // namespace Sensors
} // namespace OHOS