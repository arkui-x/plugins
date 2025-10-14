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
#include "ios_vibrator_decoder.h"

namespace OHOS {
namespace Sensors {
namespace {
constexpr int32_t EVENT_NUM_MAX = 16;
constexpr int32_t SUPPORTED_HE_VERSION_1 = 1;
constexpr int32_t SUPPORTED_HE_VERSION_2 = 2;
constexpr int32_t TRANSIENT_VIBRATION_DURATION = 48;
constexpr int32_t INTENSITY_MIN = 0;
constexpr int32_t INTENSITY_MAX = 100;
constexpr int32_t CONTINUOUS_FREQUENCY_MIN = 0;
constexpr int32_t CONTINUOUS_FREQUENCY_MAX = 100;
constexpr int32_t ERROR = -1;
constexpr int32_t SUCCESS = 0;
constexpr double CURVE_INTENSITY_SCALE = 100.0;
constexpr int32_t CURVE_INTENSITY_MIN = 0;
constexpr int32_t CURVE_INTENSITY_MAX = 100;
constexpr int32_t CURVE_FREQUENCY_MIN = 0;
constexpr int32_t CURVE_FREQUENCY_MAX = 100;
constexpr int32_t CURVE_POINT_NUM_MIN = 2;
constexpr int32_t CURVE_POINT_NUM_MAX = 16;
} // namespace

int32_t IOSVibratorDecoder::DecodeEffect(const RawFileDescriptor &rawFd, const IosJsonParser &parser, VibratePackage &pkg)
{
    int32_t version = ParseVersion(parser);
    pkg.patterns.clear();
    switch (version) {
        case SUPPORTED_HE_VERSION_1: {
            void* patternJSON = parser.GetObjectItem("Pattern");
            if (patternJSON == nullptr) {
                NSLog(@"Pattern not found in JSON");
                return ERROR;
            }
            VibratePattern pattern;
            pattern.startTime = 0;
            int32_t ret = ParsePattern(parser, patternJSON, pattern);
            if (ret != SUCCESS) {
                NSLog(@"Parse pattern failed");
                return ERROR;
            }
            pkg.patterns.emplace_back(pattern);
            break;
        }
        case SUPPORTED_HE_VERSION_2: {
            void* patternListJSON = parser.GetObjectItem("PatternList");
            if (patternListJSON == nullptr) {
                NSLog(@"PatternList not found in JSON");
                return ERROR;
            }

            int32_t ret = ParsePatternList(parser, patternListJSON, pkg);
            if (ret != SUCCESS) {
                NSLog(@"Parse pattern list failed");
                return ERROR;
            }
            break;
        }
        default: {
            NSLog(@"Unsupported version: %d", version);
            return ERROR;
        }
    }

    NSLog(@"Successfully decoded %lu patterns", pkg.patterns.size());
    return SUCCESS;
}

int32_t IOSVibratorDecoder::ParseVersion(const IosJsonParser &parser)
{
    void* metadataJSON = parser.GetObjectItem("MetaData");
    if (metadataJSON == nullptr) {
        NSLog(@"Metadata not found");
        return ERROR;
    }

    void* versionJSON = parser.GetObjectItem(metadataJSON, "Version");
    if (versionJSON == nullptr) {
        NSLog(@"Version not found in Metadata");
        return ERROR;
    }

    return parser.GetIntValue(versionJSON, ERROR);
}

int32_t IOSVibratorDecoder::ParsePatternList(const IosJsonParser &parser, void* patternListJSON, VibratePackage &pkg)
{
    if (!parser.IsArray(patternListJSON)) {
        NSLog(@"PatternList is not an array");
        return ERROR;
    }

    int32_t size = parser.GetArraySize(patternListJSON);
    if (size <= 0) {
        NSLog(@"PatternList size invalid: %d", size);
        return ERROR;
    }

    int32_t previousPatternTime = -1;
    for (int32_t i = 0; i < size; i++) {
        void* patternListItemJSON = parser.GetArrayItem(patternListJSON, i);
        if (patternListItemJSON == nullptr) {
            NSLog(@"Failed to get pattern list item at index %d", i);
            return ERROR;
        }

        void* timeJSON = parser.GetObjectItem(patternListItemJSON, "AbsoluteTime");
        if (timeJSON == nullptr) {
            NSLog(@"AbsoluteTime not found in pattern list item");
            return ERROR;
        }

        int32_t time = parser.GetIntValue(timeJSON, -1);
        if (time <= previousPatternTime) {
            NSLog(@"Invalid absolute time: %d, previous: %d", time, previousPatternTime);
            return ERROR;
        }
        previousPatternTime = time;

        void* patternJSON = parser.GetObjectItem(patternListItemJSON, "Pattern");
        if (patternJSON == nullptr) {
            NSLog(@"Pattern not found in pattern list item");
            return ERROR;
        }

        VibratePattern pattern;
        pattern.startTime = time;
        int32_t ret = ParsePattern(parser, patternJSON, pattern);
        if (ret != SUCCESS) {
            NSLog(@"Parse pattern failed in pattern list");
            return ERROR;
        }
        pkg.patterns.emplace_back(pattern);
    }
    return SUCCESS;
}

int32_t IOSVibratorDecoder::ParsePattern(const IosJsonParser &parser, void* patternJSON, VibratePattern &pattern)
{
    if (!parser.IsArray(patternJSON)) {
        NSLog(@"Pattern is not an array");
        return ERROR;
    }

    int32_t size = parser.GetArraySize(patternJSON);
    if (size <= 0 || size > EVENT_NUM_MAX) {
        NSLog(@"Pattern size out of bounds: %d", size);
        return ERROR;
    }

    int32_t previousEventTime = 0;
    for (int32_t i = 0; i < size; i++) {
        void* patternItemJSON = parser.GetArrayItem(patternJSON, i);
        if (patternItemJSON == nullptr) {
            NSLog(@"Failed to get pattern item at index %d", i);
            return ERROR;
        }

        void* eventJSON = parser.GetObjectItem(patternItemJSON, "Event");
        if (eventJSON == nullptr) {
            NSLog(@"Event not found in pattern item");
            return ERROR;
        }

        VibrateEvent event;
        int32_t ret = ParseEvent(parser, eventJSON, event);
        if (ret != SUCCESS) {
            NSLog(@"Parse event failed");
            return ERROR;
        }

        if (event.time < previousEventTime) {
            NSLog(@"Invalid event time: %d, previous: %d", event.time, previousEventTime);
            return ERROR;
        }
        previousEventTime = event.time;
        pattern.events.emplace_back(event);
    }
    return SUCCESS;
}

int32_t IOSVibratorDecoder::ParseEvent(const IosJsonParser &parser, void* eventJSON, VibrateEvent &event)
{
    void* typeJSON = parser.GetObjectItem(eventJSON, "Type");
    if (typeJSON == nullptr) {
        NSLog(@"Type not found in event");
        return ERROR;
    }

    std::string type = parser.GetStringValue(typeJSON);
    if (type == "transient") {
        event.tag = EVENT_TAG_TRANSIENT;
        event.duration = TRANSIENT_VIBRATION_DURATION;
    } else if (type == "continuous") {
        event.tag = EVENT_TAG_CONTINUOUS;
        void* durationJSON = parser.GetObjectItem(eventJSON, "Duration");
        if (durationJSON == nullptr) {
            NSLog(@"Duration not found in continuous event");
            return ERROR;
        }
        event.duration = parser.GetIntValue(durationJSON, -1);
        if (event.duration <= 0) {
            NSLog(@"Invalid duration: %d", event.duration);
            return ERROR;
        }
    } else {
        NSLog(@"Unknown event type: %s", type.c_str());
        return ERROR;
    }

    int32_t ret = ParseEventParameters(parser, eventJSON, type, event);
    if (ret != SUCCESS) {
        NSLog(@"ParseEventParameters is failed");
        return ret;
    }

    if (!CheckEventParameters(event)) {
        NSLog(@"Event parameter check failed");
        return ERROR;
    }
    return SUCCESS;
}

int32_t IOSVibratorDecoder::ParseEventParameters(const IosJsonParser &parser, void* eventJSON,
    const std::string &type, VibrateEvent &event)
{
    void* timeJSON = parser.GetObjectItem(eventJSON, "RelativeTime");
    if (timeJSON == nullptr) {
        NSLog(@"RelativeTime not found in event");
        return ERROR;
    }
    event.time = parser.GetIntValue(timeJSON, -1);

    void* paramJSON = parser.GetObjectItem(eventJSON, "Parameters");
    if (paramJSON == nullptr) {
        NSLog(@"Parameters not found in event");
        return ERROR;
    }

    void* intensityJSON = parser.GetObjectItem(paramJSON, "Intensity");
    if (intensityJSON == nullptr) {
        NSLog(@"Intensity not found in parameters");
        return ERROR;
    }
    event.intensity = parser.GetIntValue(intensityJSON, -1);
    void* frequencyJSON = parser.GetObjectItem(paramJSON, "Frequency");
    if (frequencyJSON == nullptr) {
        NSLog(@"Frequency not found in parameters");
        return ERROR;
    }
    event.frequency = parser.GetIntValue(frequencyJSON, -1);
    event.index = 0;
    if (type == "continuous") {
        void* curveItem = parser.GetObjectItem(paramJSON, "Curve");
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

bool IOSVibratorDecoder::CheckEventParameters(const VibrateEvent &event)
{
    if (event.time < 0) {
        NSLog(@"Invalid event time: %d", event.time);
        return false;
    }
    if (event.intensity < INTENSITY_MIN || event.intensity > INTENSITY_MAX) {
        NSLog(@"Invalid intensity: %d", event.intensity);
        return false;
    }
    if (event.tag == EVENT_TAG_TRANSIENT) {
        return true;
    } else if (event.tag == EVENT_TAG_CONTINUOUS) {
        if (event.duration <= 0) {
            NSLog(@"Invalid duration for continuous event: %d", event.duration);
            return false;
        }
        if (event.frequency < CONTINUOUS_FREQUENCY_MIN || event.frequency > CONTINUOUS_FREQUENCY_MAX) {
            NSLog(@"Invalid frequency for continuous event: %d", event.frequency);
            return false;
        }
        return true;
    }
    NSLog(@"Unknown event tag: %d", event.tag);
    return false;
}

int32_t IOSVibratorDecoder::ParseCurve(const IosJsonParser &parser, void* curveJSON, VibrateEvent &event)
{
    if (!parser.IsArray(curveJSON)) {
        NSLog(@"The value of curve is not array");
        return ERROR;
    }
    int32_t size = parser.GetArraySize(curveJSON);
    if (size < CURVE_POINT_NUM_MIN || size > CURVE_POINT_NUM_MAX) {
        NSLog(@"The size of curve point is out of bounds, size:%d", size);
        return ERROR;
    }

    for (int32_t i = 0; i < size; i++) {
        void* itemJSON = parser.GetArrayItem(curveJSON, i);
        if (itemJSON == nullptr) {
            NSLog(@"Failed to get curve item at index %d", i);
            return ERROR;
        }
        VibrateCurvePoint point;
        int32_t ret = ParsePoint(parser, itemJSON, event.duration, point);
        if (ret != SUCCESS) {
            NSLog(@"ParsePoin is error");
            return ret;
        }
        event.points.emplace_back(point);
    }
    return SUCCESS;
}

int32_t IOSVibratorDecoder::ParsePoint(const IosJsonParser &parser, const void* itemJSON, int32_t eventDuration,
    VibrateCurvePoint &point)
{
    void* timeJSON = parser.GetObjectItem(itemJSON, "Time");
    if (timeJSON == nullptr) {
        NSLog(@"Time not found in curve point");
        return ERROR;
    }
    point.time = parser.GetIntValue(timeJSON, -1);
    if (point.time < 0 || point.time > eventDuration) {
        NSLog(@"The time of curve point is invalid, time:%d, duration:%d", point.time, eventDuration);
        return ERROR;
    }
    void* intensityJSON = parser.GetObjectItem(itemJSON, "Intensity");
    if (intensityJSON == nullptr) {
        NSLog(@"Intensity not found in curve point");
        return ERROR;
    }
    point.intensity = static_cast<int32_t>(parser.GetDoubleValue(intensityJSON, -1.0) * CURVE_INTENSITY_SCALE);
    if (point.intensity < CURVE_INTENSITY_MIN || point.intensity > CURVE_INTENSITY_MAX) {
        NSLog(@"The intensity of curve point is invalid, intensity:%d", point.intensity);
        return ERROR;
    }
    void* frequencyJSON = parser.GetObjectItem(itemJSON, "Frequency");
    if (frequencyJSON == nullptr) {
        NSLog(@"Frequency not found in curve point");
        return ERROR;
    }
    point.frequency = parser.GetIntValue(frequencyJSON, -1);
    if (point.frequency < CURVE_FREQUENCY_MIN || point.frequency > CURVE_FREQUENCY_MAX) {
        NSLog(@"The freq of curve point is invalid, freq:%d", point.frequency);
        return ERROR;
    }
}

} // namespace Sensors
} // namespace OHOS