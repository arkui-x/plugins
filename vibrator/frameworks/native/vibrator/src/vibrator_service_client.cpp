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

#include "vibrator_service_client.h"

#include <algorithm>
#include <climits>
#include <set>
#include <thread>
#include <vector>

#ifdef HIVIEWDFX_HISYSEVENT_ENABLE
#include "hisysevent.h"
#endif // HIVIEWDFX_HISYSEVENT_ENABLE
#ifdef HIVIEWDFX_HITRACE_ENABLE
#include "hitrace_meter.h"
#endif // HIVIEWDFX_HITRACE_ENABLE

#include "sensors_errors.h"

#undef LOG_TAG
#define LOG_TAG "VibratorServiceClient"

namespace OHOS {
namespace Sensors {
static constexpr int32_t MIN_VIBRATOR_EVENT_TIME = 100;
static constexpr int32_t FREQUENCY_UPPER_BOUND = 100;
static constexpr int32_t FREQUENCY_LOWER_BOUND = -100;
static constexpr int32_t INTENSITY_UPPER_BOUND = 100;
static constexpr int32_t INTENSITY_LOWER_BOUND = 0;
static constexpr int32_t TAKE_AVERAGE = 2;
static constexpr int32_t MAX_PATTERN_EVENT_NUM = 1000;
static constexpr int32_t MAX_PATTERN_NUM = 1000;
static constexpr int32_t CURVE_POINT_NUM_MIN = 4;
static constexpr int32_t CURVE_POINT_NUM_MAX = 16;
static constexpr int32_t EVENT_NUM_MAX = 16;
VibratorServiceClient::~VibratorServiceClient()
{
}

void VibratorServiceClient::InitServiceClient()
{
    if (miscdeviceProxy_ == nullptr) {
        miscdeviceProxy_ = std::make_shared<VibratorImpl>();
    }
}

int32_t VibratorServiceClient::Vibrate(const VibratorIdentifier &identifier, int32_t timeOut, int32_t usage,
    bool systemUsage)
{
    MISC_HILOGI("Vibrate begin, time:%{public}d, usage:%{public}d, deviceId:%{public}d, vibratorId:%{public}d",
        timeOut, usage, identifier.deviceId, identifier.vibratorId);
    InitServiceClient();
    VibratorIdentifierData vibrateIdentifier;
    vibrateIdentifier.deviceId = identifier.deviceId;
    vibrateIdentifier.vibratorId = identifier.vibratorId;
    int32_t ret = miscdeviceProxy_->Vibrate(vibrateIdentifier, timeOut, usage, systemUsage);
    if (ret != ERR_OK) {
        MISC_HILOGE("Vibrate time failed, ret:%{public}d, time:%{public}d, usage:%{public}d", ret, timeOut, usage);
    }
    return ret;
}

int32_t VibratorServiceClient::Vibrate(const VibratorIdentifier &identifier, const std::string &effect,
    int32_t loopCount, int32_t usage, bool systemUsage)
{
    MISC_HILOGI("Vibrate begin, effect:%{public}s, loopCount:%{public}d, usage:%{public}d",
        effect.c_str(), loopCount, usage);
    InitServiceClient();
    VibratorIdentifierData vibrateIdentifier;
    vibrateIdentifier.deviceId = identifier.deviceId;
    vibrateIdentifier.vibratorId = identifier.vibratorId;
    PrimitiveEffectData primitiveEffectData;
    primitiveEffectData.usage = usage;
    primitiveEffectData.systemUsage = systemUsage;
    primitiveEffectData.count = loopCount;
    int32_t ret = miscdeviceProxy_->PlayPrimitiveEffect(vibrateIdentifier, effect, primitiveEffectData);
    if (ret != ERR_OK) {
        MISC_HILOGE("Vibrate effect failed, ret:%{public}d, effect:%{public}s, loopCount:%{public}d, usage:%{public}d",
            ret, effect.c_str(), loopCount, usage);
    }
    return ret;
}

int32_t VibratorServiceClient::PlayVibratorCustom(const VibratorIdentifier &identifier, const RawFileDescriptor &rawFd,
    int32_t usage, bool systemUsage, const VibratorParameter &parameter)
{
    MISC_HILOGI("Vibrate begin, fd:%{public}d, offset:%{public}lld, length:%{public}lld, usage:%{public}d",
        rawFd.fd, static_cast<long long>(rawFd.offset), static_cast<long long>(rawFd.length), usage);
    InitServiceClient();
    CustomHapticInfoData customHapticInfoData;
    customHapticInfoData.usage = usage;
    customHapticInfoData.systemUsage = systemUsage;
    customHapticInfoData.parameter.intensity = parameter.intensity;
    customHapticInfoData.parameter.frequency = parameter.frequency;
    VibratorIdentifierData vibrateIdentifier;
    vibrateIdentifier.deviceId = identifier.deviceId;
    vibrateIdentifier.vibratorId = identifier.vibratorId;
    int32_t ret = miscdeviceProxy_->PlayVibratorCustom(vibrateIdentifier, rawFd.fd, rawFd.offset,
        rawFd.length, customHapticInfoData);
    if (ret != ERR_OK) {
        MISC_HILOGE("PlayVibratorCustom failed, ret:%{public}d, usage:%{public}d", ret, usage);
    }
    return ret;
}

int32_t VibratorServiceClient::StopVibrator(const VibratorIdentifier &identifier, const std::string &mode)
{
    MISC_HILOGI("StopVibrator begin, deviceId:%{public}d, vibratorId:%{public}d, mode:%{public}s", identifier.deviceId,
        identifier.vibratorId, mode.c_str());
    InitServiceClient();
    VibratorIdentifierData vibrateIdentifier;
    vibrateIdentifier.deviceId = identifier.deviceId;
    vibrateIdentifier.vibratorId = identifier.vibratorId;
    int32_t ret = miscdeviceProxy_->StopVibratorByMode(vibrateIdentifier, mode);
    if (ret != ERR_OK) {
        MISC_HILOGI("StopVibrator by mode failed, ret:%{public}d, mode:%{public}s", ret, mode.c_str());
    }
    return ret;
}

int32_t VibratorServiceClient::StopVibrator(const VibratorIdentifier &identifier)
{
    MISC_HILOGI("StopVibrator begin, deviceId:%{public}d, vibratorId:%{public}d", identifier.deviceId,
        identifier.vibratorId);
    InitServiceClient();
    VibratorIdentifierData vibrateIdentifier;
    vibrateIdentifier.deviceId = identifier.deviceId;
    vibrateIdentifier.vibratorId = identifier.vibratorId;
    int32_t ret = miscdeviceProxy_->StopVibrator(vibrateIdentifier);
    if (ret != ERR_OK) {
        MISC_HILOGI("StopVibrator failed, ret:%{public}d", ret);
    }
    return ret;
}

bool VibratorServiceClient::IsHdHapticSupported(const VibratorIdentifier &identifier)
{
    MISC_HILOGI("VibratorServiceClient::IsHdHapticSupported begin");
    InitServiceClient();
    VibratorCapacity capacity_;
    int32_t ret = GetVibratorCapacity(identifier, capacity_);
    if (ret != ERR_OK) {
        MISC_HILOGE("IsHdHapticSupported failed, ret:%{public}d", ret);
    }
    return capacity_.isSupportHdHaptic;
}

int32_t VibratorServiceClient::IsSupportEffect(const VibratorIdentifier &identifier, const std::string &effect,
    bool &state)
{
    MISC_HILOGI("IsSupportEffect begin, effect:%{public}s", effect.c_str());
    InitServiceClient();
    VibratorIdentifierData vibrateIdentifier;
    vibrateIdentifier.deviceId = identifier.deviceId;
    vibrateIdentifier.vibratorId = identifier.vibratorId;
    int32_t ret = miscdeviceProxy_->IsSupportEffect(vibrateIdentifier, effect, state);
    if (ret != ERR_OK) {
        MISC_HILOGE("Query effect support failed, ret:%{public}d, effect:%{public}s", ret, effect.c_str());
    }
    return ret;
}

int32_t VibratorServiceClient::InitPlayPattern(const VibratorIdentifier &identifier, const VibratorPattern &pattern,
    int32_t usage, bool systemUsage, const VibratorParameter &parameter)
{
    MISC_HILOGI("VibratorServiceClient::InitPlayPattern enter");
    VibratePattern vibratePattern = {};
    vibratePattern.startTime = pattern.time;
    for (int32_t i = 0; i < pattern.eventNum; ++i) {
        if (pattern.events == nullptr) {
            MISC_HILOGE("VibratorPattern's events is null");
            return ERROR;
        }
        VibrateEvent event;
        event.tag = static_cast<VibrateTag>(pattern.events[i].type);
        event.time = pattern.events[i].time;
        event.duration = pattern.events[i].duration;
        event.intensity = pattern.events[i].intensity;
        event.frequency = pattern.events[i].frequency;
        event.index = pattern.events[i].index;
        for (int32_t j = 0; j < pattern.events[i].pointNum; ++j) {
            if (pattern.events[i].points == nullptr) {
                MISC_HILOGE("VibratorEvent's points is null");
                continue;
            }
            VibrateCurvePoint point;
            point.time = pattern.events[i].points[j].time;
            point.intensity = pattern.events[i].points[j].intensity;
            point.frequency = pattern.events[i].points[j].frequency;
            event.points.emplace_back(point);
        }
        vibratePattern.events.emplace_back(event);
        vibratePattern.patternDuration = pattern.patternDuration;
    }
    CustomHapticInfoData customHapticInfoData;
    customHapticInfoData.usage = usage;
    customHapticInfoData.systemUsage = systemUsage;
    customHapticInfoData.parameter.intensity = parameter.intensity;
    customHapticInfoData.parameter.frequency = parameter.frequency;
    customHapticInfoData.parameter.sessionId = parameter.sessionId;
    VibratorIdentifierData vibrateIdentifier;
    vibrateIdentifier.deviceId = identifier.deviceId;
    vibrateIdentifier.vibratorId = identifier.vibratorId;
    int32_t ret = miscdeviceProxy_->PlayPattern(vibrateIdentifier, vibratePattern, customHapticInfoData);
    MISC_HILOGI("VibratorServiceClient::InitPlayPattern ret=%{public}d", ret);
    return ret;
}

int32_t VibratorServiceClient::PlayPattern(const VibratorIdentifier &identifier, const VibratorPattern &pattern,
    int32_t usage, bool systemUsage, const VibratorParameter &parameter)
{
    MISC_HILOGI("Vibrate begin, usage:%{public}d", usage);
    InitServiceClient();
    int32_t ret = InitPlayPattern(identifier, pattern, usage, systemUsage, parameter);
    if (ret != ERR_OK) {
        MISC_HILOGE("PlayPattern failed, ret:%{public}d, usage:%{public}d", ret, usage);
    }
    return ret;
}

int32_t VibratorServiceClient::PlayPrimitiveEffect(const VibratorIdentifier &identifier, const std::string &effect,
    const PrimitiveEffect &primitiveEffect)
{
    MISC_HILOGI("Vibrate begin, effect:%{public}s, intensity:%{public}d, usage:%{public}d, count:%{public}d",
        effect.c_str(), primitiveEffect.intensity, primitiveEffect.usage, primitiveEffect.count);
    InitServiceClient();
    PrimitiveEffectData primitiveEffectData;
    primitiveEffectData.intensity = primitiveEffect.intensity;
    primitiveEffectData.usage = primitiveEffect.usage;
    primitiveEffectData.systemUsage = primitiveEffect.systemUsage;
    primitiveEffectData.count = primitiveEffect.count;
    VibratorIdentifierData vibrateIdentifier;
    vibrateIdentifier.deviceId = identifier.deviceId;
    vibrateIdentifier.vibratorId = identifier.vibratorId;
    int32_t ret = miscdeviceProxy_->PlayPrimitiveEffect(vibrateIdentifier, effect, primitiveEffectData);
    if (ret != ERR_OK) {
        MISC_HILOGE("Play primitive effect failed, ret:%{public}d, effect:%{public}s, intensity:%{public}d,"
            "usage:%{public}d, count:%{public}d", ret, effect.c_str(), primitiveEffect.intensity,
            primitiveEffect.usage, primitiveEffect.count);
    }
    return ret;
}

int32_t VibratorServiceClient::GetVibratorCapacity(const VibratorIdentifier &identifier, VibratorCapacity &capacity)
{
    CHKPR(miscdeviceProxy_, ERROR);
    VibratorIdentifierData vibrateIdentifier;
    vibrateIdentifier.deviceId = identifier.deviceId;
    vibrateIdentifier.vibratorId = identifier.vibratorId;
    int32_t ret = miscdeviceProxy_->GetVibratorCapacity(vibrateIdentifier, capacity);
    return ret;
}

bool VibratorServiceClient::IsSupportVibratorCustom(const VibratorIdentifier &identifier)
{
    MISC_HILOGI("VibratorServiceClient::IsSupportVibratorCustom enter");
    InitServiceClient();
    VibratorCapacity capacity_;
    int32_t ret = GetVibratorCapacity(identifier, capacity_);
    if (ret != ERR_OK) {
        MISC_HILOGE("Is support vibrator custom, ret:%{public}d", ret);
    }
    return (capacity_.isSupportHdHaptic || capacity_.isSupportPresetMapping || capacity_.isSupportTimeDelay);
}

void VibratorServiceClient::SetUsage(const VibratorIdentifier &identifier, int32_t usage, bool systemUsage)
{
    MISC_HILOGI("VibratorServiceClient::SetUsage enter usage=%{public}d, systemUsage=%{public}d", usage, systemUsage);
    std::lock_guard<std::mutex> VibratorEffectLock(vibratorEffectMutex_);
    auto it = vibratorEffectMap_.find(identifier);
    if (it != vibratorEffectMap_.end()) {
        it->second.usage = usage;
        it->second.systemUsage = systemUsage;
    } else {
        VibratorEffectParameter param = {
            .usage = usage,
            .systemUsage = systemUsage,
        };
        vibratorEffectMap_[identifier] = param;
    }
}

void VibratorServiceClient::SetLoopCount(const VibratorIdentifier &identifier, int32_t count)
{
    MISC_HILOGI("VibratorServiceClient::SetLoopCount enter");
    std::lock_guard<std::mutex> VibratorEffectLock(vibratorEffectMutex_);
    auto it = vibratorEffectMap_.find(identifier);
    if (it != vibratorEffectMap_.end()) {
        it->second.loopCount = count;
    } else {
        VibratorEffectParameter param = {
            .loopCount = count,
        };
        vibratorEffectMap_[identifier] = param;
    }
}

void VibratorServiceClient::SetParameters(const VibratorIdentifier &identifier, const VibratorParameter &parameter)
{
    MISC_HILOGI("VibratorServiceClient::SetParameters enter");
    std::lock_guard<std::mutex> VibratorEffectLock(vibratorEffectMutex_);
    auto it = vibratorEffectMap_.find(identifier);
    if (it != vibratorEffectMap_.end()) {
        it->second.vibratorParameter = parameter;
    } else {
        VibratorEffectParameter param = {
            .vibratorParameter = parameter,
        };
        vibratorEffectMap_[identifier] = param;
    }
}

VibratorEffectParameter VibratorServiceClient::GetVibratorEffectParameter(const VibratorIdentifier &identifier)
{
    MISC_HILOGI("VibratorServiceClient::GetVibratorEffectParameter ");
    std::lock_guard<std::mutex> VibratorEffectLock(vibratorEffectMutex_);
    auto it = vibratorEffectMap_.find(identifier);
    if (it != vibratorEffectMap_.end()) {
        MISC_HILOGI("VibratorServiceClient::GetVibratorEffectParameter usage=%{public}d", it->second.usage);
        return it->second;
    }
    return VibratorEffectParameter();
}

int32_t VibratorServiceClient::GetVibratorList(const VibratorIdentifier &identifier,
    std::vector<VibratorInfos> &vibratorInfo)
{
    MISC_HILOGI("VibratorIdentifier = [deviceId = %{public}d, vibratorId = %{public}d]", identifier.deviceId,
        identifier.vibratorId);
    InitServiceClient();
    VibratorIdentifierData param;
    param.deviceId = identifier.deviceId;
    param.vibratorId = identifier.vibratorId;
    std::vector<VibratorInfo> vibratorInfoList;
    int32_t ret = miscdeviceProxy_->GetVibratorList(param, vibratorInfoList);
    for (auto &info : vibratorInfoList) {
        VibratorInfos resInfo;
        resInfo.deviceId = info.deviceId;
        resInfo.vibratorId = info.vibratorId;
        resInfo.deviceName = info.deviceName;
        resInfo.isSupportHdHaptic = info.isSupportHdHaptic;
        resInfo.isLocalVibrator = info.isLocalVibrator;
        vibratorInfo.push_back(resInfo);
    }
    return OHOS::Sensors::SUCCESS;
}

int32_t VibratorServiceClient::GetEffectInfo(const VibratorIdentifier &identifier,
    const std::string &effectType, EffectInfo &effectInfo)
{
    MISC_HILOGI("VibratorIdentifier = [deviceId = %{public}d, vibratorId = %{public}d, effectType = %{public}s]",
        identifier.deviceId, identifier.vibratorId, effectType.c_str());
    InitServiceClient();
    VibratorIdentifierData param;
    param.deviceId = identifier.deviceId;
    param.vibratorId = identifier.vibratorId;
    EffectInfoData resInfo;
    int32_t ret = miscdeviceProxy_->GetEffectInfo(param, effectType, resInfo);
    effectInfo.isSupportEffect = resInfo.isSupportEffect;
    return OHOS::Sensors::SUCCESS;
}

} // namespace Sensors
} // namespace OHOS
