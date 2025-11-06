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

#ifndef DEFAULT_VIBRATOR_DECODER_H
#define DEFAULT_VIBRATOR_DECODER_H

#include "i_vibrator_decoder.h"

namespace OHOS {
namespace Sensors {
class DefaultVibratorDecoder : public IVibratorDecoder {
public:
    DefaultVibratorDecoder() = default;
    ~DefaultVibratorDecoder() = default;
    int32_t DecodeEffect(const RawFileDescriptor &rawFd, const JsonParser &parser,
        VibratePackage &patternPackage) override;

private:
    int32_t CheckMetadata(const JsonParser &parser);
    int32_t ParseChannel(const JsonParser &parser, VibratePattern &originPattern, VibratePackage &patternPackage);
    int32_t ParseChannelParameters(const JsonParser &parser, cJSON *channelParametersItem);
    int32_t ParsePattern(const JsonParser &parser, cJSON *patternItem, VibratePattern &originPattern);
    int32_t ParseEvent(const JsonParser &parser, cJSON *eveventItement, VibrateEvent &events);
    bool CheckEventParameters(const VibrateEvent &event);
    int32_t ParseCurve(const JsonParser &parser, cJSON *curveItem, VibrateEvent &event);
    void PatternSplit(VibratePattern &originPattern, VibratePackage &patternPackage);
    int32_t channelNumber_ = 0;
    double version_ = 0.0;
};
} // namespace Sensors
} // namespace OHOS
#endif // VIBRATOR_DECODER_H