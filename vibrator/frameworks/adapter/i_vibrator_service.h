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

#ifndef I_VIBRATOR_SERVICE_H
#define I_VIBRATOR_SERVICE_H

#include "vibrator_infos.h"

namespace OHOS {
namespace Sensors {

class IVibratorService {
public:
    IVibratorService() = default;
    virtual ~IVibratorService() = default;
    virtual int32_t Vibrate(const VibratorIdentifierData &identifier, int32_t timeOut, int32_t usage,
        bool systemUsage) = 0;
    virtual int32_t PlayPrimitiveEffect(const VibratorIdentifierData &identifier, const std::string &effect,
        const PrimitiveEffectData &primitiveEffectData) = 0;
    virtual int32_t PlayVibratorCustom(const VibratorIdentifierData &identifier, int32_t fd, int64_t offset,
        int64_t length, const CustomHapticInfoData &customHapticInfoData) = 0;
    virtual int32_t StopVibrator(const VibratorIdentifierData &identifier) = 0;
    virtual int32_t StopVibratorByMode(const VibratorIdentifierData &identifier, const std::string &mode) = 0;
    virtual int32_t IsSupportEffect(const VibratorIdentifierData &identifier, const std::string &effect,
        bool &state) = 0;
    virtual int32_t PlayPattern(const VibratorIdentifierData &identifier, const VibratePattern &pattern,
        const CustomHapticInfoData &customHapticInfoData) = 0;
    virtual int32_t GetVibratorList(const VibratorIdentifierData &identifier,
        std::vector<VibratorInfo> &vibratorInfoData) = 0;
    virtual int32_t GetEffectInfo(const VibratorIdentifierData &identifier, const std::string &effectType,
        EffectInfoData &effectInfoIPCData) = 0;
    virtual int32_t GetVibratorCapacity(const VibratorIdentifierData &identifier, VibratorCapacity &capacity) = 0;
};
} // namespace Sensors
} // namespace OHOS

#endif // I_VIBRATOR_SERVICE_H