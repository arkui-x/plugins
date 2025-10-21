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

#import <Foundation/Foundation.h>
#include <algorithm>
#include "ios_default_vibrator_decoder.h"

namespace OHOS {
namespace Sensors {
namespace {
constexpr int32_t STARTTIME_MIN = 0;
constexpr int32_t STARTTIME_MAX = 1800000;
constexpr int32_t TRANSIENT_VIBRATION_DURATION = 48;
constexpr int32_t INTENSITY_MIN = 0;
constexpr int32_t INTENSITY_MAX = 100;
constexpr int32_t FREQUENCY_MIN = 0;
constexpr int32_t FREQUENCY_MAX = 100;
constexpr int32_t EVENT_NUM_MAX = 128;
constexpr double SUPPORT_JSON_VERSION = 1.0;
constexpr int32_t SUPPORT_CHANNEL_NUMBER = 3;
constexpr uint32_t PATTERN_CAPACITY = 16;
constexpr size_t CURVE_POINT_MIN = 4;
constexpr size_t CURVE_POINT_MAX = 16;
constexpr double CURVE_INTENSITY_SCALE = 100.0;
constexpr int32_t CURVE_INTENSITY_MIN = 0;
constexpr int32_t CURVE_INTENSITY_MAX = 100;
constexpr int32_t CURVE_FREQUENCY_MIN = 0;
constexpr int32_t CURVE_FREQUENCY_MAX = 100;

constexpr int32_t ERROR = -1;
constexpr int32_t SUCCESS = 0;
} // namespace

int32_t IOSDefaultVibratorDecoder::DecodeEffect(const RawFileDescriptor &rawFd, const IosJsonParser &parser, VibratePackage &pkg)
{
    NSLog(@"Starting default format decoding");
    double version;
    int32_t channelNumber;
    int32_t ret = CheckMetadata(parser, version, channelNumber);
    if (ret != SUCCESS) {
        NSLog(@"Check metadata failed");
        return ret;
    }
    
    VibratePattern originPattern;
    ret = ParseChannel(parser, originPattern, pkg);
    if (ret != SUCCESS) {
        NSLog(@"Parse channel failed");
        return ret;
    }
    PatternSplit(originPattern, pkg);
    NSLog(@"Default format decoding completed, %lu patterns created", pkg.patterns.size());
    return SUCCESS;
}

int32_t IOSDefaultVibratorDecoder::CheckMetadata(const IosJsonParser &parser, double &version, int32_t &channelNumber)
{
    void* metadataItem = parser.GetObjectItem("MetaData");
    if (!metadataItem) {
        NSLog(@"MetaData not found");
        return ERROR;
    }
    
    void* versionItem = parser.GetObjectItem(metadataItem, "Version");
    if (!versionItem) {
        NSLog(@"Version not found in MetaData");
        return ERROR;
    }
    
    version = parser.GetDoubleValue(versionItem, -1.0);
    if (version != SUPPORT_JSON_VERSION) {
        NSLog(@"Unsupported JSON version: %f, expected: %f", version, SUPPORT_JSON_VERSION);
        return ERROR;
    }
    
    void* channelNumberItem = parser.GetObjectItem(metadataItem, "ChannelNumber");
    if (!channelNumberItem) {
        NSLog(@"ChannelNumber not found in MetaData");
        return ERROR;
    }
    
    channelNumber = parser.GetIntValue(channelNumberItem, -1);
    if (channelNumber <= 0 || channelNumber > SUPPORT_CHANNEL_NUMBER) {
        NSLog(@"Invalid ChannelNumber: %d", channelNumber);
        return ERROR;
    }
    
    channelNumber_ = channelNumber;
    version_ = version;
    
    NSLog(@"Metadata check passed: Version=%f, ChannelNumber=%d", version, channelNumber);
    return SUCCESS;
}

int32_t IOSDefaultVibratorDecoder::ParseChannel(const IosJsonParser &parser, VibratePattern &originPattern, VibratePackage &patternPackage)
{
    void* channelsItem = parser.GetObjectItem("Channels");
    if (!channelsItem || !parser.IsArray(channelsItem)) {
        NSLog(@"Channels not found or Channels is not an array");
        return ERROR;
    }
    int32_t size = parser.GetArraySize(channelsItem);
    if (size != channelNumber_) {
        NSLog(@"Channels size mismatch: %d vs %d", size, channelNumber_);
        return ERROR;
    }

    int32_t parseDuration = 0;
    for (int32_t i = 0; i < size; i++) {
        void* channelItem = parser.GetArrayItem(channelsItem, i);
        if (!channelItem) {
            NSLog(@"Failed to get channel item at index %d", i);
            return ERROR;
        }
        void* channelParametersItem = parser.GetObjectItem(channelItem, "Parameters");
        if (!channelParametersItem) {
            NSLog(@"Parameters not found in channel");
            return ERROR;
        }
        int32_t ret = ParseChannelParameters(parser, channelParametersItem);
        if (ret != SUCCESS) {
            NSLog(@"Parse channel parameters failed");
            return ret;
        }
        void* patternItem = parser.GetObjectItem(channelItem, "Pattern");
        if (!patternItem) {
            NSLog(@"Pattern not found in channel");
            return ERROR;
        }

        ret = ParsePattern(parser, patternItem, originPattern);
        if (ret != SUCCESS) {
            NSLog(@"Parse pattern failed");
            return ret;
        }

        parseDuration += originPattern.patternDuration;
    }

    patternPackage.packageDuration = parseDuration;

    std::sort(originPattern.events.begin(), originPattern.events.end(), 
            [](const VibrateEvent &a, const VibrateEvent &b) {
                return a.time < b.time;
            });
    return SUCCESS;
}

int32_t IOSDefaultVibratorDecoder::ParseChannelParameters(const IosJsonParser &parser, void* channelParametersItem)
{
    void* indexItem = parser.GetObjectItem(channelParametersItem, "Index");
    if (!indexItem) {
        NSLog(@"Index not found in channel parameters");
        return ERROR;
    }

    int32_t indexVal = parser.GetIntValue(indexItem, -1);
    if (indexVal < 0 || indexVal >= SUPPORT_CHANNEL_NUMBER) {
        NSLog(@"Invalid channel index: %d", indexVal);
        return ERROR;
    }

    return SUCCESS;
}

int32_t IOSDefaultVibratorDecoder::ParsePattern(const IosJsonParser &parser, void* patternItem, VibratePattern &originPattern)
{
    if (!parser.IsArray(patternItem)) {
        NSLog(@"Pattern is not an array");
        return ERROR;
    }

    int32_t size = parser.GetArraySize(patternItem);
    if (size > EVENT_NUM_MAX) {
        NSLog(@"Pattern size too large: %d", size);
        return ERROR;
    }

    int32_t vibratorDuration = 0;
    for (int32_t i = 0; i < size; i++) {
        void* item = parser.GetArrayItem(patternItem, i);
        if (!item) {
            NSLog(@"Failed to get pattern item at index %d", i);
            return ERROR;
        }

        void* eventItem = parser.GetObjectItem(item, "Event");
        if (!eventItem) {
            NSLog(@"Event not found in pattern item");
            return ERROR;
        }

        VibrateEvent event;
        int32_t ret = ParseEvent(parser, eventItem, event);
        if (ret != SUCCESS) {
            NSLog(@"Parse event failed");
            return ret;
        }
        vibratorDuration += event.duration;
        originPattern.events.emplace_back(event);
    }
    
    originPattern.patternDuration = vibratorDuration;
    return SUCCESS;
}

int32_t IOSDefaultVibratorDecoder::ParseEvent(const IosJsonParser &parser, void* eventItem, VibrateEvent &event)
{
    void* typeItem = parser.GetObjectItem(eventItem, "Type");
    if (!typeItem) {
        NSLog(@"Type not found in event");
        return ERROR;
    }

    std::string curType = parser.GetStringValue(typeItem);
    if (curType == "transient") {
        event.tag = EVENT_TAG_TRANSIENT;
        event.duration = TRANSIENT_VIBRATION_DURATION;
    } else if (curType == "continuous") {
        event.tag = EVENT_TAG_CONTINUOUS;
        void* durationItem = parser.GetObjectItem(eventItem, "Duration");
        if (!durationItem) {
            NSLog(@"Duration not found in continuous event");
            return ERROR;
        }
        event.duration = parser.GetIntValue(durationItem, -1);
        if (event.duration <= 0) {
            NSLog(@"Invalid duration for continuous event: %d", event.duration);
            return ERROR;
        }
    } else {
        NSLog(@"Unsupported event type: %s", curType.c_str());
        return ERROR;
    }

    int32_t ret = ParseEventParameters(parser, eventItem, curType, event);
    if (ret != SUCCESS) {
        NSLog(@"Parse curve failed");
        return ret;
    }

    if (!CheckEventParameters(event)) {
        NSLog(@"Event parameter check failed");
        return ERROR;
    }
    return SUCCESS;
}

int32_t IOSDefaultVibratorDecoder::ParseEventParameters(const IosJsonParser &parser, void* eventItem,
    const std::string &curType, VibrateEvent &event)
{
    void* startTimeItem = parser.GetObjectItem(eventItem, "StartTime");
    if (!startTimeItem) {
        NSLog(@"StartTime not found in event");
        return ERROR;
    }

    event.time = parser.GetIntValue(startTimeItem, -1);
    if (event.time < 0) {
        NSLog(@"Invalid start time: %d", event.time);
        return ERROR;
    }

    void* eventParametersItem = parser.GetObjectItem(eventItem, "Parameters");
    if (!eventParametersItem) {
        NSLog(@"Parameters not found in event");
        return ERROR;
    }
    void* intensityItem = parser.GetObjectItem(eventParametersItem, "Intensity");
    if (!intensityItem) {
        NSLog(@"Intensity not found in parameters");
        return ERROR;
    }
    event.intensity = parser.GetIntValue(intensityItem, -1);
    void* frequencyItem = parser.GetObjectItem(eventParametersItem, "Frequency");
    if (!frequencyItem) {
        return ERROR;
    }
    event.frequency = parser.GetIntValue(frequencyItem, -1);
    event.index = 0;
    if (curType == "continuous") {
        void* curveItem = parser.GetObjectItem(eventParametersItem, "Curve");
        if (curveItem) {
            int32_t ret = ParseCurve(parser, curveItem, event);
            if (ret != SUCCESS) {
                NSLog(@"Parse curve failed");
                return ret;
            }
        }
    }
    return SUCCESS;
}

bool IOSDefaultVibratorDecoder::CheckEventParameters(const VibrateEvent &event)
{
    if (event.time < STARTTIME_MIN || event.time > STARTTIME_MAX) {
        NSLog(@"Event time out of range: %d", event.time);
        return false;
    }

    if (event.intensity < INTENSITY_MIN || event.intensity > INTENSITY_MAX) {
        NSLog(@"Event intensity out of range: %d", event.intensity);
        return false;
    }

    if (event.frequency < FREQUENCY_MIN || event.frequency > FREQUENCY_MAX) {
        NSLog(@"Event frequency out of range: %d", event.frequency);
        return false;
    }

    if (event.duration <= 0) {
        NSLog(@"Event duration invalid: %d", event.duration);
        return false;
    }

    return true;
}

void IOSDefaultVibratorDecoder::PatternSplit(VibratePattern &originPattern, VibratePackage &patternPackage)
{
    if (originPattern.events.empty()) {
        NSLog(@"Origin pattern is empty");
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

int32_t IOSDefaultVibratorDecoder::ParseCurve(const IosJsonParser &parser, void* curveItem, VibrateEvent &event)
{
    if (!parser.IsArray(curveItem)) {
        NSLog(@"The value of curve is not array");
        return ERROR;
    }
    size_t size = static_cast<size_t>(parser.GetArraySize(curveItem));
    if ((size < CURVE_POINT_MIN) || (size > CURVE_POINT_MAX)) {
        NSLog(@"The size of curve point is out of bounds, size:%zu", size);
        return ERROR;
    }
    for (size_t i = 0; i < size; i++) {
        VibrateCurvePoint point;
        void* item = parser.GetArrayItem(curveItem, i);
        if (!item) {
            NSLog(@"Failed to get curve point item at index %zu", i);
            return ERROR;
        }

        int32_t ret = ParsePoint(parser, item, event.duration, point);
        if (ret != SUCCESS) {
            NSLog(@"ParsePoint is error");
            return ret;
        }
        event.points.emplace_back(point);
    }
    std::sort(event.points.begin(), event.points.end(), 
            [](const VibrateCurvePoint &a, const VibrateCurvePoint &b) {
                return a.time < b.time;
            });
    return SUCCESS;
}

int32_t IOSDefaultVibratorDecoder::ParsePoint(const IosJsonParser &parser, const void* item,
    int32_t eventDuration, VibrateCurvePoint &point)
{
    void* timeItem = parser.GetObjectItem(item, "Time");
    if (!timeItem) {
        NSLog(@"Time not found in curve point");
        return ERROR;
    }
    point.time = parser.GetIntValue(timeItem, -1);
    if ((point.time < 0) || (point.time > eventDuration)) {
        NSLog(@"The time of curve point is invalid, time:%d, duration:%d", point.time, eventDuration);
        return ERROR;
    }
    void* intensityItem = parser.GetObjectItem(item, "Intensity");
    if (!intensityItem) {
        NSLog(@"Intensity not found in curve point");
        return ERROR;
    }
    double rawIntensity = parser.GetDoubleValue(intensityItem, -1.0);
    if (rawIntensity < 0.0 || rawIntensity > 1.0) {
        return ERROR;
    }
    point.intensity = static_cast<int32_t>(rawIntensity * CURVE_INTENSITY_SCALE);
    void* frequencyItem = parser.GetObjectItem(item, "Frequency");
    if (!frequencyItem) {
        NSLog(@"Frequency not found in curve point");
        return ERROR;
    }
    point.frequency = parser.GetIntValue(frequencyItem, -1);
    return SUCCESS;
}
} // namespace Sensors
} // namespace OHOS