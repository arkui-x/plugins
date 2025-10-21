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
#include <unistd.h>
#include "android_service.h"
#include "miscdevice_log.h"
#include "raw_file_descriptor.h"
#include "json_parser.h"
#include "vibrator_decoder_creator.h"

namespace OHOS {
namespace Sensors {

int32_t VibratorImpl::Vibrate(const VibratorIdentifierData &identifier, int32_t timeOut,
                              int32_t usage, bool systemUsage)
{
    VibratorIdentifierTrans info;
    info.deviceId = identifier.deviceId;
    info.isLocalVibrator = identifier.isLocalVibrator;
    info.position = identifier.position;
    info.vibratorId = identifier.vibratorId;
    return VibratorJni::Vibrate(info, timeOut, usage, systemUsage);
}

int32_t VibratorImpl::PlayPrimitiveEffect(const VibratorIdentifierData& identifier,
                                          const std::string &effect, const PrimitiveEffectData& primitiveEffectIn)
{
    VibratorIdentifierTrans info;
    info.deviceId = identifier.deviceId;
    info.isLocalVibrator = identifier.isLocalVibrator;
    info.position = identifier.position;
    info.vibratorId = identifier.vibratorId;
    PrimitiveEffectTrans primitiveEffect;
    primitiveEffect.count = primitiveEffectIn.count;
    primitiveEffect.intensity = primitiveEffectIn.intensity;
    primitiveEffect.systemUsage = primitiveEffectIn.systemUsage;
    primitiveEffect.usage = primitiveEffectIn.usage;
    return VibratorJni::PlayPrimitiveEffect(info, effect, primitiveEffect);
}

void VibratorImpl::MergeVibratorParmeters(const VibrateParameter &parameter, VibratePackage &package)
{
    if ((parameter.intensity == OH_INTENSITY_MAX) && (parameter.frequency == 0)) {
        MISC_HILOGD("The adjust parameter is not need to merge");
        return;
    }
    float intensityScale = static_cast<float>(parameter.intensity) / OH_INTENSITY_MAX;
    for (VibratePattern &pattern : package.patterns) {
        for (VibrateEvent &event : pattern.events) {
            if ((event.tag == EVENT_TAG_TRANSIENT) || (event.points.empty())) {
                MergeTransientEvent(event, intensityScale, parameter.frequency);
            } else {
                MergeCurvePointEvent(event, intensityScale, parameter.frequency);
            }
        }
    }
}

void VibratorImpl::MergeTransientEvent(VibrateEvent &event, float intensityScale, int32_t frequencyAdjust)
{
    event.intensity = static_cast<int32_t>(event.intensity * intensityScale);
    event.intensity = std::max(std::min(event.intensity, OH_INTENSITY_MAX), 0);
    event.frequency += frequencyAdjust;
    event.frequency = std::max(std::min(event.frequency, OH_INTENSITY_MAX), 0);
}

void VibratorImpl::MergeCurvePointEvent(VibrateEvent &event, float intensityScale, int32_t frequencyAdjust)
{
    for (VibrateCurvePoint &point : event.points) {
        point.intensity = static_cast<int32_t>(point.intensity * intensityScale);
        point.intensity = std::max(std::min(point.intensity, OH_INTENSITY_MAX), 0);
        point.frequency += frequencyAdjust;
        point.frequency = std::max(std::min(point.frequency, OH_INTENSITY_MAX), OH_INTENSITY_MIN);
    }
}

int32_t VibratorImpl::PlayVibratorCustom(const VibratorIdentifierData &identifier, int32_t fd, int64_t offset,
                                         int64_t length, const CustomHapticInfoData &customHapticInfo)
{
    RawFileDescriptor rawFd;
    rawFd.fd = fd;
    rawFd.offset = offset;
    rawFd.length = length;
    JsonParser parser(rawFd);
    VibratorDecoderCreator creator;
    std::unique_ptr<IVibratorDecoder> decoder(creator.CreateDecoder(parser));
    if (decoder == nullptr) {
        MISC_HILOGI("decoder is nullptr");
        return ERROR;
    }

    VibratePackage package;
    int32_t ret = decoder->DecodeEffect(rawFd, parser, package);
    if (ret != SUCCESS || package.patterns.empty()) {
        MISC_HILOGI("Decode effect error");
        return ERROR;
    }
    MergeVibratorParmeters(customHapticInfo.parameter, package);
    VibrateInfo info = {
        .usage = customHapticInfo.usage,
        .systemUsage = customHapticInfo.systemUsage,
        .package = package,
    };
    for (auto pattern : package.patterns) {
        PlayPattern(identifier, pattern, customHapticInfo);
    }
    return 0;
}

int32_t VibratorImpl::StopVibrator(const VibratorIdentifierData &identifier)
{
    VibratorIdentifierTrans info;
    info.deviceId = identifier.deviceId;
    info.isLocalVibrator = identifier.isLocalVibrator;
    info.position = identifier.position;
    info.vibratorId = identifier.vibratorId;
    return VibratorJni::StopVibrator(info);
}

int32_t VibratorImpl::StopVibratorByMode(const VibratorIdentifierData &identifier, const std::string &mode)
{
    VibratorIdentifierTrans info;
    info.deviceId = identifier.deviceId;
    info.isLocalVibrator = identifier.isLocalVibrator;
    info.position = identifier.position;
    info.vibratorId = identifier.vibratorId;
    return VibratorJni::StopVibratorByMode(info, mode);
}

int32_t VibratorImpl::IsSupportEffect(const VibratorIdentifierData &identifier, const std::string &effect, bool &state)
{
    VibratorIdentifierTrans info;
    info.deviceId = identifier.deviceId;
    info.isLocalVibrator = identifier.isLocalVibrator;
    info.position = identifier.position;
    info.vibratorId = identifier.vibratorId;
    return VibratorJni::IsSupportEffect(info, effect, state);
}

int32_t VibratorImpl::PlayPattern(const VibratorIdentifierData &identifier, const VibratePattern &pattern,
                                  const CustomHapticInfoData &customHapticInfo)
{
    VibratorIdentifierTrans info;
    info.deviceId = identifier.deviceId;
    info.isLocalVibrator = identifier.isLocalVibrator;
    info.position = identifier.position;
    info.vibratorId = identifier.vibratorId;
    VibratePatternTrans vibratePattern = ConvertPattern(pattern);
    CustomHapticInfoTrans hapticInfo;
    hapticInfo.usage = customHapticInfo.usage;
    hapticInfo.systemUsage = customHapticInfo.systemUsage;
    hapticInfo.parameter.frequency = customHapticInfo.parameter.frequency;
    hapticInfo.parameter.intensity = customHapticInfo.parameter.intensity;
    hapticInfo.parameter.reserved = customHapticInfo.parameter.reserved;
    hapticInfo.parameter.sessionId = customHapticInfo.parameter.sessionId;
    return VibratorJni::PlayPattern(info, vibratePattern, hapticInfo);
}

int32_t VibratorImpl::GetVibratorList(const VibratorIdentifierData &identifier,
                                      std::vector<VibratorInfo> &vibratorInfo)
{
    VibratorIdentifierTrans identifierInfo;
    identifierInfo.deviceId = identifier.deviceId;
    identifierInfo.isLocalVibrator = identifier.isLocalVibrator;
    identifierInfo.position = identifier.position;
    identifierInfo.vibratorId = identifier.vibratorId;
    std::vector<VibratorInfoTrans> vibratorInfoOut{};
    int32_t ret = VibratorJni::GetVibratorList(identifierInfo, vibratorInfoOut);
    vibratorInfo.reserve(vibratorInfoOut.size());
    for (auto &info : vibratorInfoOut) {
        VibratorInfo resInfo;
        resInfo.deviceId = info.deviceId;
        resInfo.vibratorId = info.vibratorId;
        resInfo.deviceName = info.deviceName;
        resInfo.isSupportHdHaptic = info.isSupportHdHaptic;
        resInfo.isLocalVibrator = info.isLocalVibrator;
        resInfo.position = info.position;
        vibratorInfo.push_back(resInfo);
    }
    return ret;
}

int32_t VibratorImpl::GetEffectInfo(const VibratorIdentifierData &identifier,
                                    const std::string &effectType, EffectInfoData &effectInfo)
{
    VibratorIdentifierTrans info;
    info.deviceId = identifier.deviceId;
    info.isLocalVibrator = identifier.isLocalVibrator;
    info.position = identifier.position;
    info.vibratorId = identifier.vibratorId;
    EffectInfoTrans effectInfoOut;
    int32_t ret = VibratorJni::GetEffectInfo(info, effectType, effectInfoOut);
    effectInfo.duration = effectInfoOut.duration;
    effectInfo.isSupportEffect = effectInfoOut.isSupportEffect;
    return ret;
}

int32_t VibratorImpl::GetVibratorCapacity(const VibratorIdentifierData &identifier, VibratorCapacity &capacity)
{
    VibratorIdentifierTrans info;
    info.deviceId = identifier.deviceId;
    info.isLocalVibrator = identifier.isLocalVibrator;
    info.position = identifier.position;
    info.vibratorId = identifier.vibratorId;
    VibratorCapacityTrans vibratorCapacity;
    int32_t ret = VibratorJni::GetVibratorCapacity(info, vibratorCapacity);
    capacity.isSupportHdHaptic = vibratorCapacity.isSupportHdHaptic;
    capacity.isSupportPresetMapping = vibratorCapacity.isSupportPresetMapping;
    capacity.isSupportTimeDelay = vibratorCapacity.isSupportTimeDelay;
    return ret;
}

VibrateCurvePointTrans VibratorImpl::ConvertCurvePoint(const VibrateCurvePoint& src)
{
    VibrateCurvePointTrans dest;
    dest.time = src.time;
    dest.intensity = src.intensity;
    dest.frequency = src.frequency;
    return dest;
}

VibrateEventTrans VibratorImpl::ConvertEvent(const VibrateEvent& src)
{
    VibrateEventTrans dest;
    dest.tag = static_cast<VibrateTagTrans>(src.tag);
    dest.time = src.time;
    dest.duration = src.duration;
    dest.intensity = src.intensity;
    dest.frequency = src.frequency;
    dest.index = src.index;
    dest.points.reserve(src.points.size());
    for (const auto& point : src.points) {
        dest.points.push_back(ConvertCurvePoint(point));
    }
    return dest;
}

VibratePatternTrans VibratorImpl::ConvertPattern(const VibratePattern& src)
{
    VibratePatternTrans dest;
    dest.startTime = src.startTime;
    dest.patternDuration = src.patternDuration;
    dest.events.reserve(src.events.size());
    for (const auto& event : src.events) {
        dest.events.push_back(ConvertEvent(event));
    }
    return dest;
}

std::vector<VibratePatternTrans> VibratorImpl::ConvertPatterns(const std::vector<VibratePattern>& srcPatterns)
{
    std::vector<VibratePatternTrans> destPatterns;
    destPatterns.reserve(srcPatterns.size());
    for (const auto& pattern : srcPatterns) {
        destPatterns.push_back(ConvertPattern(pattern));
    }
    return destPatterns;
}
}
}