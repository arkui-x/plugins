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
#ifndef VIBRATOR_SERVICE_H
#define VIBRATOR_SERVICE_H

#include "i_vibrator_service.h"
#include "vibrator_plugin_jni.h"

namespace OHOS {
namespace Sensors {

class VibratorImpl : public IVibratorService {
public:
    VibratorImpl() = default;
    virtual ~VibratorImpl() = default;
    int32_t Vibrate(const VibratorIdentifierData &identifier, int32_t timeOut,
                    int32_t usage, bool systemUsage) override;
    int32_t PlayPrimitiveEffect(const VibratorIdentifierData &identifier, const std::string &effect,
                                const PrimitiveEffectData &primitiveEffect) override;
    int32_t PlayVibratorCustom(const VibratorIdentifierData &identifier, int32_t fd, int64_t offset,
                               int64_t length, const CustomHapticInfoData &customHapticInfo) override;
    int32_t StopVibrator(const VibratorIdentifierData &identifier) override;
    int32_t StopVibratorByMode(const VibratorIdentifierData &identifier, const std::string &mode) override;
    int32_t IsSupportEffect(const VibratorIdentifierData &identifier, const std::string &effect, bool &state) override;
    int32_t PlayPattern(const VibratorIdentifierData &identifier, const VibratePattern &pattern,
                        const CustomHapticInfoData &customHapticInfo) override;
    int32_t GetVibratorList(const VibratorIdentifierData &identifier,
                            std::vector<VibratorInfo> &vibratorInfo) override;
    int32_t GetEffectInfo(const VibratorIdentifierData &identifier, const std::string &effectType,
                          EffectInfoData &effectInfo) override;
    int32_t GetVibratorCapacity(const VibratorIdentifierData &identifier, VibratorCapacity &capacity) override;
    VibrateCurvePointTrans ConvertCurvePoint(const VibrateCurvePoint& src);
    VibrateEventTrans ConvertEvent(const VibrateEvent& src);
    VibratePatternTrans ConvertPattern(const VibratePattern& src);
    std::vector<VibratePatternTrans> ConvertPatterns(const std::vector<VibratePattern>& srcPatterns);
    void MergeVibratorParmeters(const VibrateParameter &parameter, VibratePackage &package);
    void MergeTransientEvent(VibrateEvent &event, float intensityScale, int32_t frequencyAdjust);
    void MergeCurvePointEvent(VibrateEvent &event, float intensityScale, int32_t frequencyAdjust);
};
} // namespace Sensors
} // namespace OHOS

#endif // VIBRATOR_SERVICE_H