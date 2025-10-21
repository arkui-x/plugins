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

#ifndef VIBRATOR_SERVICE_CLIENT_H
#define VIBRATOR_SERVICE_CLIENT_H

#include <dlfcn.h>
#include <time.h>
#include <mutex>
#include <map>
#include <set>
#include <vector>

#include "singleton.h"
#include "vibrator_infos.h"
#include "vibrator_agent_type.h"
#include "miscdevice_common.h"

#ifdef ANDROID_PLATFORM
#include "android_service.h"
#endif // ANDROID_PLATFORM

#ifdef IOS_PLATFORM
#include "ios_adapter.h"
#endif // IOS_PLATFORM

#include "raw_file_descriptor.h"

namespace OHOS {
namespace Sensors {
struct VibratorCurveInterval {
    int32_t beginTime;
    int32_t endTime;
    int32_t intensity;
    int32_t frequency;
};

class VibratorServiceClient : public Singleton<VibratorServiceClient> {
public:
    ~VibratorServiceClient() override;
    int32_t Vibrate(const VibratorIdentifier &identifier, int32_t timeOut, int32_t usage, bool systemUsage);
    int32_t Vibrate(const VibratorIdentifier &identifier, const std::string &effect,
        int32_t loopCount, int32_t usage, bool systemUsage);
    int32_t PlayVibratorCustom(const VibratorIdentifier &identifier, const RawFileDescriptor &rawFd,
        int32_t usage, bool systemUsage, const VibratorParameter &parameter);
    int32_t StopVibrator(const VibratorIdentifier &identifier, const std::string &mode);
    int32_t StopVibrator(const VibratorIdentifier &identifier);
    bool IsHdHapticSupported(const VibratorIdentifier &identifier);
    int32_t IsSupportEffect(const VibratorIdentifier &identifier, const std::string &effect, bool &state);
    int32_t InitPlayPattern(const VibratorIdentifier &identifier, const VibratorPattern &pattern, int32_t usage,
        bool systemUsage, const VibratorParameter &parameter);
    int32_t PlayPattern(const VibratorIdentifier &identifier, const VibratorPattern &pattern, int32_t usage,
        bool systemUsage, const VibratorParameter &parameter);
    int32_t PlayPrimitiveEffect(const VibratorIdentifier &identifier, const std::string &effect,
        const PrimitiveEffect &primitiveEffect);
    bool IsSupportVibratorCustom(const VibratorIdentifier &identifier);
    void SetUsage(const VibratorIdentifier &identifier, int32_t usage, bool systemUsage);
    void SetLoopCount(const VibratorIdentifier &identifier, int32_t count);
    void SetParameters(const VibratorIdentifier &identifier, const VibratorParameter &parameter);
    VibratorEffectParameter GetVibratorEffectParameter(const VibratorIdentifier &identifier);
    int32_t GetVibratorList(const VibratorIdentifier &identifier, std::vector<VibratorInfos> &vibratorInfo);
    int32_t GetEffectInfo(const VibratorIdentifier &identifier, const std::string &effectType, EffectInfo &effectInfo);

private:
    void InitServiceClient();
    int32_t GetVibratorCapacity(const VibratorIdentifier &identifier, VibratorCapacity &capacity);
    std::shared_ptr<VibratorImpl> miscdeviceProxy_ = nullptr;
    std::mutex vibratorEffectMutex_;
    std::map<VibratorIdentifier, VibratorEffectParameter> vibratorEffectMap_;
};
} // namespace Sensors
} // namespace OHOS
#endif // VIBRATOR_SERVICE_CLIENT_H
