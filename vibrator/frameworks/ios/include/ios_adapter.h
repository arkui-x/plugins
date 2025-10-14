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

#ifndef PLUGINS_VIBRATOR_IOS_ADAPTER_H
#define PLUGINS_VIBRATOR_IOS_ADAPTER_H

#include "i_vibrator_service.h"
#include "vibrator_infos.h"

namespace OHOS::Sensors {
class VibratorImpl final : public IVibratorService {
public:
    VibratorImpl() = default;
    virtual ~VibratorImpl() = default;
    int32_t Vibrate(const VibratorIdentifierData &identifier, int32_t timeOut,
        int32_t usage, bool systemUsage) override;
    int32_t PlayPrimitiveEffect(const VibratorIdentifierData &identifier, const std::string &effect,
        const PrimitiveEffectData &primitiveEffectData) override;
    int32_t PlayVibratorCustom(const VibratorIdentifierData &identifier, int32_t fd, int64_t offset, int64_t length,
        const CustomHapticInfoData &customHapticInfoData) override;
    int32_t StopVibrator(const VibratorIdentifierData &identifier) override;
    int32_t StopVibratorByMode(const VibratorIdentifierData &identifier, const std::string &mode) override;
    int32_t IsSupportEffect(const VibratorIdentifierData &identifier, const std::string &effect, bool &state) override;
    int32_t PlayPattern(const VibratorIdentifierData &identifier, const VibratePattern &pattern,
        const CustomHapticInfoData &customHapticInfoData) override;
    int32_t GetVibratorList(const VibratorIdentifierData &identifier,
        std::vector<VibratorInfo> &vibratorInfoData) override;
    int32_t GetEffectInfo(const VibratorIdentifierData &identifier, const std::string &effectType,
        EffectInfoData &effectInfoIPCData) override;
    int32_t GetVibratorCapacity(const VibratorIdentifierData &identifier, VibratorCapacity &capacity) override;
private:
    int32_t CheckAuthAndParam(int32_t usage, const VibrateParameter &parameter,
        const VibratorIdentifierData &identifier);
    bool CheckVibratorParmeters(const VibrateParameter &parameter);
};
} // namespace OHOS::Sensors
#endif // PLUGINS_VIBRATOR_IOS_ADAPTER_H