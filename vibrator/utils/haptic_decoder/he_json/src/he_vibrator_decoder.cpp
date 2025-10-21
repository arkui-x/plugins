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

#include "he_vibrator_decoder.h"

#include "sensors_errors.h"

#undef LOG_TAG
#define LOG_TAG "HEVibratorDecoder"

namespace OHOS {
namespace Sensors {
namespace {
constexpr int32_t EVENT_NUM_MAX = 16;
constexpr int32_t SUPPORTED_HE_VERSION_1 = 1;
constexpr int32_t SUPPORTED_HE_VERSION_2 = 2;
constexpr int32_t INDEX_MIN = 0;
constexpr int32_t INDEX_MAX = 2;
constexpr int32_t TRANSIENT_VIBRATION_DURATION = 48;
constexpr int32_t INTENSITY_MIN = 0;
constexpr int32_t INTENSITY_MAX = 100;
constexpr int32_t CONTINUOUS_FREQUENCY_MIN = 0;
constexpr int32_t CONTINUOUS_FREQUENCY_MAX = 100;
constexpr int32_t TRANSIENT_FREQUENCY_MIN = -50;
constexpr int32_t TRANSIENT_FREQUENCY_MAX = 150;
constexpr int32_t CURVE_POINT_NUM_MIN = 4;
constexpr int32_t CURVE_POINT_NUM_MAX = 16;
constexpr int32_t CURVE_INTENSITY_MIN = 0;
constexpr int32_t CURVE_INTENSITY_MAX = 100;
constexpr int32_t CURVE_INTENSITY_SCALE = 100;
constexpr int32_t CURVE_FREQUENCY_MIN = -100;
constexpr int32_t CURVE_FREQUENCY_MAX = 100;
constexpr int32_t CONTINUOUS_DURATION_MAX = 5000;
} // namespace

int32_t HEVibratorDecoder::DecodeEffect(const RawFileDescriptor &rawFd, const JsonParser &parser,
    VibratePackage &pkg)
{
    int32_t version = ParseVersion(parser);
    pkg.patterns.clear();
    switch (version) {
        case SUPPORTED_HE_VERSION_1: {
            cJSON *patternJSON = parser.GetObjectItem("Pattern");
            CHKPR(patternJSON, ERROR);
            VibratePattern pattern;
            pattern.startTime = 0;
            int32_t ret = ParsePattern(parser, patternJSON, pattern);
            CHKCR((ret == SUCCESS), ERROR, "parse pattern fail!");
            pkg.patterns.emplace_back(pattern);
            break;
        }
        case SUPPORTED_HE_VERSION_2: {
            cJSON *patternListJSON = parser.GetObjectItem("PatternList");
            CHKPR(patternListJSON, ERROR);
            int32_t ret = ParsePatternList(parser, patternListJSON, pkg);
            CHKCR((ret == SUCCESS), ERROR, "parse pattern list fail!");
            break;
        }
        default: {
            MISC_HILOGE("unsupported version %{public}d", version);
            return ERROR;
        }
    }
    return SUCCESS;
}

int32_t HEVibratorDecoder::ParseVersion(const JsonParser &parser)
{
    cJSON *metadataJSON = parser.GetObjectItem("Metadata");
    CHKPR(metadataJSON, ERROR);
    cJSON *versionJSON = parser.GetObjectItem(metadataJSON, "Version");
    CHKPR(versionJSON, ERROR);
    return cJSON_IsNumber(versionJSON) ? versionJSON->valueint : -1;
}

int32_t HEVibratorDecoder::ParsePatternList(const JsonParser &parser, cJSON *patternListJSON, VibratePackage &pkg)
{
    if (!parser.IsArray(patternListJSON)) {
        MISC_HILOGE("The value of pattern list is not array!");
        return ERROR;
    }
    int32_t size = parser.GetArraySize(patternListJSON);
    if (size <= 0) {
        MISC_HILOGE("The size of pattern list  %{public}d is invalid!", size);
        return ERROR;
    }
    int32_t previousPattternTime = -1;
    for (int32_t i = 0; i < size; i++) {
        cJSON *patternListItemJSON = parser.GetArrayItem(patternListJSON, i);
        CHKPR(patternListItemJSON, ERROR);
        cJSON *timeJSON = parser.GetObjectItem(patternListItemJSON, "AbsoluteTime");
        CHKPR(timeJSON, ERROR);
        int32_t time = cJSON_IsNumber(timeJSON) ? timeJSON->valueint : -1;
        if (time <= previousPattternTime) {
            MISC_HILOGE("The value of absolute time %{public}d is invalid!", time);
            return ERROR;
        }
        previousPattternTime = time;
        cJSON *patternJSON = parser.GetObjectItem(patternListItemJSON, "Pattern");
        CHKPR(patternJSON, ERROR);
        VibratePattern pattern;
        pattern.startTime = time;
        int32_t ret = ParsePattern(parser, patternJSON, pattern);
        CHKCR((ret == SUCCESS), ERROR, "parse pattern fail!");
        pkg.patterns.emplace_back(pattern);
    }
    return SUCCESS;
}

int32_t HEVibratorDecoder::ParsePattern(const JsonParser &parser, cJSON *patternJSON, VibratePattern &pattern)
{
    if (!parser.IsArray(patternJSON)) {
        MISC_HILOGE("The value of pattern is not array");
        return ERROR;
    }
    int32_t size = parser.GetArraySize(patternJSON);
    if (size <= 0 || size > EVENT_NUM_MAX) {
        MISC_HILOGE("The size of pattern is out of bounds, size:%{public}d", size);
        return ERROR;
    }
    int32_t previousEventTime = 0;
    for (int32_t i = 0; i < size; i++) {
        cJSON *patternItemJSON = parser.GetArrayItem(patternJSON, i);
        CHKPR(patternItemJSON, ERROR);
        cJSON *eventJSON = parser.GetObjectItem(patternItemJSON, "Event");
        CHKPR(eventJSON, ERROR);
        VibrateEvent event;
        int32_t ret = ParseEvent(parser, eventJSON, event);
        CHKCR((ret == SUCCESS), ERROR, "parse event fail!");
        if (event.time < previousEventTime) {
            MISC_HILOGE("The value of absolute time %{public}d is invalid!", event.time);
            return ERROR;
        }
        previousEventTime = event.time;
        pattern.events.emplace_back(event);
    }
    return SUCCESS;
}

int32_t HEVibratorDecoder::ParseEvent(const JsonParser &parser, cJSON *eventJSON, VibrateEvent &event)
{
    cJSON *typeJSON = parser.GetObjectItem(eventJSON, "Type");
    CHKPR(typeJSON, ERROR);
    std::string type = cJSON_IsString(typeJSON) ? typeJSON->valuestring : "";
    if (type == "transient") {
        event.tag = EVENT_TAG_TRANSIENT;
        event.duration = TRANSIENT_VIBRATION_DURATION;
    } else if (type == "continuous") {
        event.tag = EVENT_TAG_CONTINUOUS;
        cJSON *durationJSON = parser.GetObjectItem(eventJSON, "Duration");
        CHKPR(durationJSON, ERROR);
        event.duration = cJSON_IsNumber(durationJSON) ? durationJSON->valueint : -1;
    } else {
        MISC_HILOGE("Unknown event type %{public}s", type.c_str());
        return ERROR;
    }
    if (parser.HasObjectItem(eventJSON, "Index")) {
        cJSON *indexJSON = parser.GetObjectItem(eventJSON, "Index");
        CHKPR(indexJSON, ERROR);
        event.index = cJSON_IsNumber(indexJSON) ? indexJSON->valueint : -1;
    } else {
        event.index = 0;
    }
    cJSON *timeJSON = parser.GetObjectItem(eventJSON, "RelativeTime");
    CHKPR(timeJSON, ERROR);
    event.time = cJSON_IsNumber(timeJSON) ? timeJSON->valueint : -1;
    cJSON *paramJSON = parser.GetObjectItem(eventJSON, "Parameters");
    CHKPR(paramJSON, ERROR);
    cJSON *intensityJSON = parser.GetObjectItem(paramJSON, "Intensity");
    CHKPR(intensityJSON, ERROR);
    event.intensity = cJSON_IsNumber(intensityJSON) ? intensityJSON->valueint : -1;
    cJSON *frequencyJSON = parser.GetObjectItem(paramJSON, "Frequency");
    CHKPR(frequencyJSON, ERROR);
    event.frequency = cJSON_IsNumber(frequencyJSON) ? frequencyJSON->valueint : -1;
    if (event.tag == EVENT_TAG_CONTINUOUS) {
        cJSON *curveJSON = parser.GetObjectItem(paramJSON, "Curve");
        CHKPR(curveJSON, ERROR);
        int32_t ret = ParseCurve(parser, curveJSON, event);
        if (ret != SUCCESS) {
            MISC_HILOGE("Parse curve fail, startTime:%{public}d", event.time);
            return ERROR;
        }
    }
    if (!CheckEventParameters(event)) {
        MISC_HILOGE("Parameter check of vibration event failed, startTime:%{public}d", event.time);
        return ERROR;
    }
    return SUCCESS;
}

int32_t HEVibratorDecoder::ParseCurve(const JsonParser &parser, cJSON *curveJSON, VibrateEvent &event)
{
    if (!parser.IsArray(curveJSON)) {
        MISC_HILOGE("The value of curve is not array");
        return ERROR;
    }
    int32_t size = parser.GetArraySize(curveJSON);
    if ((size < CURVE_POINT_NUM_MIN) || (size > CURVE_POINT_NUM_MAX)) {
        MISC_HILOGE("The size of curve point is out of bounds, size:%{public}d", size);
        return ERROR;
    }
    int32_t previousCurveTime = -1;
    for (int32_t i = 0; i < size; i++) {
        VibrateCurvePoint point;
        cJSON *itemJSON = parser.GetArrayItem(curveJSON, i);
        CHKPR(itemJSON, ERROR);
        cJSON *timeJSON = parser.GetObjectItem(itemJSON, "Time");
        CHKPR(timeJSON, ERROR);
        point.time = cJSON_IsNumber(timeJSON) ? timeJSON->valueint : -1;
        if (point.time <= previousCurveTime || point.time > event.duration) {
            MISC_HILOGE("The time of curve point is invalid, time:%{public}d", point.time);
            return ERROR;
        }
        previousCurveTime = point.time;
        cJSON *intensityJSON = parser.GetObjectItem(itemJSON, "Intensity");
        CHKPR(intensityJSON, ERROR);
        point.intensity = cJSON_IsNumber(intensityJSON) ? (intensityJSON->valuedouble) * CURVE_INTENSITY_SCALE : -1;
        if (point.intensity < CURVE_INTENSITY_MIN || point.intensity > CURVE_INTENSITY_MAX) {
            MISC_HILOGE("The intensity of curve point is invalid, intensity:%{public}d", point.intensity);
            return ERROR;
        }
        cJSON *frequencyJSON = parser.GetObjectItem(itemJSON, "Frequency");
        CHKPR(frequencyJSON, ERROR);
        point.frequency = cJSON_IsNumber(frequencyJSON) ? frequencyJSON->valueint : -1;
        if (point.frequency < CURVE_FREQUENCY_MIN || point.frequency > CURVE_FREQUENCY_MAX) {
            MISC_HILOGE("The freq of curve point is invalid, freq:%{public}d", point.frequency);
            return ERROR;
        }
        event.points.emplace_back(point);
    }
    return SUCCESS;
}

bool HEVibratorDecoder::CheckEventParameters(const VibrateEvent &event)
{
    if (!CheckCommonParameters(event)) {
        return false;
    }
    if (event.tag == EVENT_TAG_TRANSIENT) {
        return CheckTransientParameters(event);
    } else if (event.tag == EVENT_TAG_CONTINUOUS) {
        return CheckContinuousParameters(event);
    } else {
        MISC_HILOGE("The event tag is unknown, tag:%{public}d", event.tag);
        return false;
    }
    return true;
}

bool HEVibratorDecoder::CheckCommonParameters(const VibrateEvent &event)
{
    if (event.time < 0) {
        MISC_HILOGE("The event startTime is out of range, startTime:%{public}d", event.time);
        return false;
    }
    if (event.index < INDEX_MIN || event.index > INDEX_MAX) {
        MISC_HILOGE("The event index is out of range, index:%{public}d", event.index);
        return false;
    }
    if (event.intensity < INTENSITY_MIN || event.intensity > INTENSITY_MAX) {
        MISC_HILOGE("The event intensity is out of range, intensity:%{public}d", event.intensity);
        return false;
    }
    return true;
}

bool HEVibratorDecoder::CheckTransientParameters(const VibrateEvent &event)
{
    if (event.frequency < TRANSIENT_FREQUENCY_MIN || event.frequency > TRANSIENT_FREQUENCY_MAX) {
        MISC_HILOGE("The event frequency is out of range, frequency:%{public}d", event.frequency);
        return false;
    }
    return true;
}

bool HEVibratorDecoder::CheckContinuousParameters(const VibrateEvent &event)
{
    if (event.frequency < CONTINUOUS_FREQUENCY_MIN || event.frequency > CONTINUOUS_FREQUENCY_MAX) {
        MISC_HILOGE("The event frequency is out of range, frequency:%{public}d", event.frequency);
        return false;
    }
    if (event.duration <= 0 || event.duration > CONTINUOUS_DURATION_MAX) {
        MISC_HILOGE("The event duration is out of range, duration:%{public}d", event.duration);
        return false;
    }
    int32_t pointNum = static_cast<int32_t>(event.points.size());
    if (pointNum < CURVE_POINT_NUM_MIN || pointNum > CURVE_POINT_NUM_MAX) {
        MISC_HILOGE("The points size is out of range, size:%{public}d", pointNum);
        return false;
    }
    if (event.points[0].time != 0 || event.points[0].intensity != 0) {
        MISC_HILOGE("The fist curve point is invalivd, time %{public}d, insentsity %{public}d", event.points[0].time,
            event.points[0].intensity);
        return false;
    }
    if (event.points[pointNum - 1].time != event.duration || event.points[pointNum - 1].intensity != 0) {
        MISC_HILOGE("The last curve point is invalivd, time %{public}d, insentsity %{public}d",
            event.points[pointNum - 1].time, event.points[pointNum - 1].intensity);
        return false;
    }
    return true;
}
} // namespace Sensors
} // namespace OHOS