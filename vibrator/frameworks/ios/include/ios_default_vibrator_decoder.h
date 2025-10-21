/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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

#ifndef IOS_DEFAULT_VIBRATOR_DECODER_H
#define IOS_DEFAULT_VIBRATOR_DECODER_H

#include "ios_ivibrator_decoder.h"
#include "ios_json_parser.h"

namespace OHOS {
namespace Sensors {

class IOSDefaultVibratorDecoder : public IosIVibratorDecoder {
public:
    int32_t DecodeEffect(const RawFileDescriptor &rawFd, const IosJsonParser &parser, VibratePackage &pkg) override;
    
private:
    int32_t CheckMetadata(const IosJsonParser &parser, double &version, int32_t &channelNumber);
    int32_t ParseChannel(const IosJsonParser &parser, VibratePattern &originPattern, VibratePackage &patternPackage);
    int32_t ParseChannelParameters(const IosJsonParser &parser, void* channelParametersItem);
    int32_t ParsePattern(const IosJsonParser &parser, void* patternItem, VibratePattern &originPattern);
    int32_t ParseEvent(const IosJsonParser &parser, void* eventItem, VibrateEvent &event);
    bool CheckEventParameters(const VibrateEvent &event);
    void PatternSplit(VibratePattern &originPattern, VibratePackage &patternPackage);
    int32_t ParseCurve(const IosJsonParser &parser, void* curveItem, VibrateEvent &event);
    int32_t ParseEventParameters(const IosJsonParser &parser, void* eventItem, const std::string &curType,
        VibrateEvent &event);
    int32_t ParsePoint(const IosJsonParser &parser, const void* item, int32_t eventDuration, VibrateCurvePoint &point);
    
    double version_ = 0.0;
    int32_t channelNumber_ = 0;
};

} // namespace Sensors
} // namespace OHOS

#endif // IOS_DEFAULT_VIBRATOR_DECODER_H