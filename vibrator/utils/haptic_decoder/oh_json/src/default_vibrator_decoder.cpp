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

#include "default_vibrator_decoder.h"

#include <cinttypes>

#include "sensors_errors.h"

#undef LOG_TAG
#define LOG_TAG "DefaultVibratorDecoder"

namespace OHOS {
namespace Sensors {
namespace {
constexpr int32_t STARTTMIE_MIN = 0;
constexpr int32_t STARTTMIE_MAX = 1800000;
constexpr int32_t CONTINUOUS_VIBRATION_DURATION_MIN = 0;
constexpr int32_t CONTINUOUS_VIBRATION_DURATION_MAX = 5000;
constexpr int32_t TRANSIENT_VIBRATION_DURATION = 48;
constexpr int32_t INTENSITY_MIN = 0;
constexpr int32_t INTENSITY_MAX = 100;
constexpr int32_t FREQUENCY_MIN = 0;
constexpr int32_t FREQUENCY_MAX = 100;
constexpr int32_t EVENT_NUM_MAX = 128;
constexpr double SUPPORT_JSON_VERSION = 1.0;
constexpr int32_t SUPPORT_CHANNEL_NUMBER = 3;
constexpr uint32_t PATTERN_CAPACITY = 16;
constexpr uint32_t CURVE_POINT_MIN = 4;
constexpr uint32_t CURVE_POINT_MAX = 16;
constexpr int32_t CURVE_INTENSITY_MIN = 0;
constexpr int32_t CURVE_INTENSITY_MAX = 100;
constexpr int32_t CURVE_INTENSITY_SCALE = 100;
constexpr int32_t CURVE_FREQUENCY_MIN = -100;
constexpr int32_t CURVE_FREQUENCY_MAX = 100;
constexpr int32_t MAX_JSON_FILE_SIZE = 64 * 1024;
} // namespace

int32_t DefaultVibratorDecoder::DecodeEffect(const RawFileDescriptor &rawFd, const JsonParser &parser,
    VibratePackage &patternPackage) __attribute__((no_sanitize("cfi")))
{
    if ((rawFd.fd < 0) || (rawFd.offset < 0) || (rawFd.length <= 0) || (rawFd.length > MAX_JSON_FILE_SIZE)) {
        MISC_HILOGE("Invalid file descriptor, fd:%{public}d, offset:%{public}" PRId64 ", length:%{public}" PRId64,
            rawFd.fd, rawFd.offset, rawFd.length);
        return PARAMETER_ERROR;
    }
    int32_t ret = CheckMetadata(parser);
    if (ret != SUCCESS) {
        MISC_HILOGE("Check metadata fail");
        return ret;
    }
    VibratePattern originPattern;
    ret = ParseChannel(parser, originPattern, patternPackage);
    if (ret != SUCCESS) {
        MISC_HILOGE("Parse channel fail");
        return ret;
    }
    MISC_HILOGD("packageDuration:%{public}d", patternPackage.packageDuration);
    PatternSplit(originPattern, patternPackage);
    return SUCCESS;
}

int32_t DefaultVibratorDecoder::CheckMetadata(const JsonParser &parser)
{
    cJSON *metadataItem = parser.GetObjectItem("MetaData");
    CHKPR(metadataItem, ERROR);
    cJSON *versionItem = parser.GetObjectItem(metadataItem, "Version");
    CHKPR(versionItem, ERROR);
    version_ = parser.GetDoubleValue(versionItem);
    if (version_ != SUPPORT_JSON_VERSION) {
        MISC_HILOGE("Json file version is not supported, version:%{public}f", version_);
        return ERROR;
    }
    cJSON *channelNumberItem = parser.GetObjectItem(metadataItem, "ChannelNumber");
    CHKPR(channelNumberItem, ERROR);
    channelNumber_ = parser.GetIntValue(channelNumberItem);
    if ((channelNumber_ <= 0)|| (channelNumber_ >= SUPPORT_CHANNEL_NUMBER)) {
        MISC_HILOGE("Json file channelNumber is not supported, channelNumber:%{public}d", channelNumber_);
        return ERROR;
    }
    return SUCCESS;
}

int32_t DefaultVibratorDecoder::ParseChannel(const JsonParser &parser, VibratePattern &originPattern,
    VibratePackage &patternPackage) __attribute__((no_sanitize("cfi")))
{
    cJSON *channelsItem = parser.GetObjectItem("Channels");
    CHKPR(channelsItem, ERROR);
    if (!parser.IsArray(channelsItem)) {
        MISC_HILOGE("The value of channels is not array");
        return ERROR;
    }
    int32_t size = parser.GetArraySize(channelsItem);
    if (size != channelNumber_) {
        MISC_HILOGE("The size of channels conflicts with channelNumber, size:%{public}d", size);
        return ERROR;
    }
    int32_t parseDuration = 0;
    for (int32_t i = 0; i < size; i++) {
        cJSON *channelItem = parser.GetArrayItem(channelsItem, i);
        CHKPR(channelItem, ERROR);
        cJSON *channelParametersItem = parser.GetObjectItem(channelItem, "Parameters");
        CHKPR(channelParametersItem, ERROR);
        int32_t ret = ParseChannelParameters(parser, channelParametersItem);
        CHKCR((ret == SUCCESS), ERROR, "parse channel parameters fail");
        cJSON *patternItem = parser.GetObjectItem(channelItem, "Pattern");
        CHKPR(patternItem, ERROR);
        ret = ParsePattern(parser, patternItem, originPattern);
        parseDuration += originPattern.patternDuration;
        CHKCR((ret == SUCCESS), ERROR, "parse pattern fail");
    }
    patternPackage.packageDuration = parseDuration;
    std::sort(originPattern.events.begin(), originPattern.events.end());
    return SUCCESS;
}

int32_t DefaultVibratorDecoder::ParseChannelParameters(const JsonParser &parser,
    cJSON *channelParametersItem) __attribute__((no_sanitize("cfi")))
{
    cJSON *indexItem = parser.GetObjectItem(channelParametersItem, "Index");
    CHKPR(indexItem, ERROR);
    int32_t indexVal = parser.GetIntValue(indexItem);
    CHKCR((indexVal >= 0) && (indexVal < SUPPORT_CHANNEL_NUMBER), ERROR, "invalid channel index");
    return SUCCESS;
}

int32_t DefaultVibratorDecoder::ParsePattern(const JsonParser &parser, cJSON *patternItem,
    VibratePattern &originPattern)
{
    if (!parser.IsArray(patternItem)) {
        MISC_HILOGE("The value of pattern is not array");
        return ERROR;
    }
    int32_t size = parser.GetArraySize(patternItem);
    if (size > EVENT_NUM_MAX) {
        MISC_HILOGE("The size of pattern is out of bounds, size:%{public}d", size);
        return ERROR;
    }
    int32_t vibratorDuration = 0;
    for (int32_t i = 0; i < size; i++) {
        cJSON *item = parser.GetArrayItem(patternItem, i);
        CHKPR(item, ERROR);
        cJSON *eventItem = parser.GetObjectItem(item, "Event");
        CHKPR(eventItem, ERROR);
        VibrateEvent event;
        int32_t ret = ParseEvent(parser, eventItem, event);
        CHKCR((ret == SUCCESS), ERROR, "parse event fail");
        vibratorDuration += event.duration;
        originPattern.events.emplace_back(event);
    }
    originPattern.patternDuration = vibratorDuration;
    return SUCCESS;
}

int32_t DefaultVibratorDecoder::ParseEvent(const JsonParser &parser, cJSON *eventItem, VibrateEvent &event)
{
    cJSON *typeItem = parser.GetObjectItem(eventItem, "Type");
    CHKPR(typeItem, ERROR);
    std::string curType = parser.GetStringValue(typeItem);
    if (curType == "continuous") {
        event.tag = EVENT_TAG_CONTINUOUS;
        cJSON *durationItem = parser.GetObjectItem(eventItem, "Duration");
        CHKPR(durationItem, ERROR);
        event.duration = parser.GetIntValue(durationItem);
    } else if (curType == "transient") {
        event.tag = EVENT_TAG_TRANSIENT;
        event.duration = TRANSIENT_VIBRATION_DURATION;
    } else {
        MISC_HILOGE("Unknown event type, curType:%{public}s", curType.c_str());
        return ERROR;
    }
    cJSON *startTimeItem = parser.GetObjectItem(eventItem, "StartTime");
    CHKPR(startTimeItem, ERROR);
    event.time = parser.GetIntValue(startTimeItem);
    cJSON *eventParametersItem = parser.GetObjectItem(eventItem, "Parameters");
    CHKPR(eventParametersItem, ERROR);
    cJSON *intensityItem = parser.GetObjectItem(eventParametersItem, "Intensity");
    CHKPR(intensityItem, ERROR);
    event.intensity = parser.GetIntValue(intensityItem);
    cJSON *frequencyItem = parser.GetObjectItem(eventParametersItem, "Frequency");
    CHKPR(frequencyItem, ERROR);
    event.frequency = parser.GetIntValue(frequencyItem);
    if (!CheckEventParameters(event)) {
        MISC_HILOGE("Parameter check of vibration event failed, startTime:%{public}d", event.time);
        return ERROR;
    }
    if ((curType == "continuous") && parser.HasObjectItem(eventParametersItem, "Curve")) {
        cJSON *curveItem = parser.GetObjectItem(eventParametersItem, "Curve");
        CHKPR(curveItem, ERROR);
        int32_t ret = ParseCurve(parser, curveItem, event);
        if (ret != SUCCESS) {
            MISC_HILOGE("Parse curve fail, startTime:%{public}d", event.time);
            return ERROR;
        }
    }
    return SUCCESS;
}

bool DefaultVibratorDecoder::CheckEventParameters(const VibrateEvent &event)
{
    if (event.time < STARTTMIE_MIN || event.time > STARTTMIE_MAX) {
        MISC_HILOGE("The event startTime is out of range, startTime:%{public}d", event.time);
        return false;
    }
    if (event.duration < CONTINUOUS_VIBRATION_DURATION_MIN ||
        event.duration > CONTINUOUS_VIBRATION_DURATION_MAX) {
        MISC_HILOGE("The event duration is out of range, duration:%{public}d", event.duration);
        return false;
    }
    if (event.intensity < INTENSITY_MIN || event.intensity > INTENSITY_MAX) {
        MISC_HILOGE("The event intensity is out of range, intensity:%{public}d", event.intensity);
        return false;
    }
    if (event.frequency < FREQUENCY_MIN || event.frequency > FREQUENCY_MAX) {
        MISC_HILOGE("The event frequency is out of range, frequency:%{public}d", event.frequency);
        return false;
    }
    return true;
}

int32_t DefaultVibratorDecoder::ParseCurve(const JsonParser &parser, cJSON *curveItem, VibrateEvent &event)
{
    if (!parser.IsArray(curveItem)) {
        MISC_HILOGE("The value of curve is not array");
        return ERROR;
    }
    size_t size = static_cast<size_t>(parser.GetArraySize(curveItem));
    if ((size < CURVE_POINT_MIN) || (size > CURVE_POINT_MAX)) {
        MISC_HILOGE("The size of curve point is out of bounds, size:%{public}zu", size);
        return ERROR;
    }
    for (size_t i = 0; i < size; i++) {
        VibrateCurvePoint point;
        cJSON *item = parser.GetArrayItem(curveItem, i);
        CHKPR(item, ERROR);
        cJSON *timeItem = parser.GetObjectItem(item, "Time");
        CHKPR(timeItem, ERROR);
        point.time = parser.GetIntValue(timeItem);
        if ((point.time < 0) || (point.time > event.duration)) {
            MISC_HILOGE("The time of curve point is out of bounds, time:%{public}d", point.time);
            return ERROR;
        }
        cJSON *intensityItem = parser.GetObjectItem(item, "Intensity");
        CHKPR(intensityItem, ERROR);
        point.intensity = (parser.GetDoubleValue(intensityItem)) * CURVE_INTENSITY_SCALE;
        if ((point.intensity < CURVE_INTENSITY_MIN) || (point.intensity > CURVE_INTENSITY_MAX)) {
            MISC_HILOGE("The intensity of curve point is out of bounds, intensity:%{public}d", point.intensity);
            return ERROR;
        }
        cJSON *frequencyItem = parser.GetObjectItem(item, "Frequency");
        CHKPR(frequencyItem, ERROR);
        point.frequency = parser.GetIntValue(frequencyItem);
        if ((point.frequency < CURVE_FREQUENCY_MIN) || (point.frequency > CURVE_FREQUENCY_MAX)) {
            MISC_HILOGE("The frequency of curve point is out of bounds, frequency:%{public}d", point.frequency);
            return ERROR;
        }
        event.points.emplace_back(point);
    }
    std::sort(event.points.begin(), event.points.end());
    return SUCCESS;
}

void DefaultVibratorDecoder::PatternSplit(VibratePattern &originPattern,
    VibratePackage &patternPackage) __attribute__((no_sanitize("cfi")))
{
    if (originPattern.events.empty()) {
        MISC_HILOGI("The origin pattern is empty");
        return;
    }
    VibratePattern slicePattern;
    slicePattern.startTime = originPattern.events[0].time;
    size_t size = originPattern.events.size();
    for (size_t i = 0; i < size; ++i) {
        originPattern.events[i].time -= slicePattern.startTime;
        slicePattern.events.emplace_back(originPattern.events[i]);
        if ((slicePattern.events.size() >= PATTERN_CAPACITY) || (i == (size - 1))) {
            patternPackage.patterns.emplace_back(slicePattern);
            slicePattern.events.clear();
            slicePattern.startTime = ((i < size - 1) ? originPattern.events[i + 1].time : 0);
        }
    }
}
} // namespace Sensors
} // namespace OHOS