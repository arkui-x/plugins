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

#ifndef IOS_VIBRATOR_DECODER_H
#define IOS_VIBRATOR_DECODER_H

#include "ios_ivibrator_decoder.h"
#include "ios_json_parser.h"

namespace OHOS {
namespace Sensors {

class IOSVibratorDecoder : public IosIVibratorDecoder {
public:
    int32_t DecodeEffect(const RawFileDescriptor &rawFd, const IosJsonParser &parser, VibratePackage &pkg) override;

private:
    int32_t ParseVersion(const IosJsonParser &parser);
    int32_t ParsePatternList(const IosJsonParser &parser, void* patternListJSON, VibratePackage &pkg);
    int32_t ParsePattern(const IosJsonParser &parser, void* patternJSON, VibratePattern &pattern);
    int32_t ParseEvent(const IosJsonParser &parser, void* eventJSON, VibrateEvent &event);
    int32_t ParseCurve(const IosJsonParser &parser, void* curveJSON, VibrateEvent &event);
    int32_t ParseEventParameters(const IosJsonParser &parser, void* eventJSON, const std::string &type,
        VibrateEvent &event);
    int32_t ParsePoint(const IosJsonParser &parser, const void* itemJSON, int32_t eventDuration,
        VibrateCurvePoint &point);
    bool CheckEventParameters(const VibrateEvent &event);
};

} // namespace Sensors
} // namespace OHOS

#endif //IOS_VIBRATOR_DECODER_H