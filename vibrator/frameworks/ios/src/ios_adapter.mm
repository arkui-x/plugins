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
#import "vibratormanager.h"
#include "ios_adapter.h"
#include "raw_file_descriptor.h"
#include "ios_json_parser.h"
#include "ios_ivibrator_decoder.h"
#include "vibrator_decoder_creator.h"
#include <unistd.h>
#include <sys/stat.h>

namespace OHOS::Sensors {
namespace {
constexpr int32_t INTENSITY_ADJUST_MAX = 100;
constexpr int32_t INTENSITY_ADJUST_MIN = 0;
constexpr int32_t FREQUENCY_ADJUST_MIN = -100;
constexpr int32_t FREQUENCY_ADJUST_MAX = 100;
constexpr int32_t INTENSITY_MIN = 0;
constexpr int32_t INTENSITY_MAX = 100;
constexpr int32_t FREQUENCY_MIN = 0;
constexpr int32_t FREQUENCY_MAX = 100;
constexpr int32_t ERROR = -1;
constexpr int32_t SUCCESS = 0;
constexpr int32_t PARAMETER_ERROR = 401;
constexpr int32_t IS_NOT_SUPPORTED = 801;
} // namespace

int32_t VibratorImpl::Vibrate(const VibratorIdentifierData &identifier, int32_t timeOut, int32_t usage, bool systemUsage)
{
    NSLog(@"VibratorImpl::Vibrate enter");
    @autoreleasepool {
        VibratorManager *manager = [VibratorManager sharedInstance];
        if (manager == nil) {
            NSLog(@"Get VibratorManager instance failed");
            return ERROR;
        }
        return [manager vibrateWithTimeOut:timeOut];
    }
}

int32_t VibratorImpl::PlayPrimitiveEffect(const VibratorIdentifierData &identifier, const std::string &effect, const PrimitiveEffectData &primitiveEffectData)
{
    NSLog(@"VibratorImpl::PlayVibratorEffect enter");
    @autoreleasepool {
        VibratorManager *manager = [VibratorManager sharedInstance];
        if (manager == nil) {
            NSLog(@"Get VibratorManager instance failed");
            return ERROR;
        }
        NSString *effectStr = [NSString stringWithUTF8String:effect.c_str()];
        return [manager playVibratorEffect:effectStr loopCount:primitiveEffectData.count];
    }
}

int32_t VibratorImpl::PlayVibratorCustom(const VibratorIdentifierData &identifier, int32_t fd, int64_t offset, int64_t length, const CustomHapticInfoData &customHapticInfoData)
{
    NSLog(@"VibratorImpl::PlayVibratorCustom ");
    int32_t ret = CheckAuthAndParam(customHapticInfoData.usage, customHapticInfoData.parameter, identifier);
    if (ret != SUCCESS) {
        NSLog(@"CheckAuthAndParam failed, ret:%d", ret);
        return ret;
    }
    RawFileDescriptor rawFd;
    rawFd.fd = fd;
    rawFd.offset = offset;
    rawFd.length = length;

    IosJsonParser parser(rawFd);
    VibratorDecoderCreator creator;
    std::unique_ptr<IosIVibratorDecoder> decoder(creator.CreateDecoder(parser));
    if (decoder == nullptr) {
        NSLog(@"Failure to create a decoder");
        return ERROR;
    }
    VibratePackage package;
    ret = decoder->DecodeEffect(rawFd, parser, package);
    if (ret != 0 || package.patterns.empty()) {
        NSLog(@"Decode the vibration effect failed: %d", ret);
        return ERROR;
    }
    @autoreleasepool {
        VibratorManager *manager = [VibratorManager sharedInstance];
        if (manager == nil) {
            NSLog(@"Get VibratorManager instance failed");
            return ERROR;
        }
        [manager playPatterns:package.patterns];
        return SUCCESS;
    }
}

int32_t VibratorImpl::StopVibrator(const VibratorIdentifierData &identifier)
{
    NSLog(@"VibratorImpl::StopVibrator enter");
    @autoreleasepool {
        VibratorManager *manager = [VibratorManager sharedInstance];
        if (manager == nil) {
            NSLog(@"Get VibratorManager instance failed");
            return ERROR;
        }
        return [manager stopVibrator];
    }
}

int32_t VibratorImpl::StopVibratorByMode(const VibratorIdentifierData &identifier, const std::string &mode)
{
    NSLog(@"VibratorImpl::StopVibratorByMode enter, mode: %s", mode.c_str());
    @autoreleasepool {
        VibratorManager *manager = [VibratorManager sharedInstance];
        if (manager == nil) {
            NSLog(@"Get VibratorManager instance failed");
            return ERROR;
        }
        NSString *modeStr = [NSString stringWithUTF8String:mode.c_str()];
        return [manager stopVibratorByMode:modeStr];
    }
}

int32_t VibratorImpl::IsSupportEffect(const VibratorIdentifierData &identifier, const std::string &effect, bool &state)
{
    NSLog(@"VibratorImpl::IsSupportEffect enter, effect: %s", effect.c_str());
    @autoreleasepool {
        VibratorManager *manager = [VibratorManager sharedInstance];
        if (manager == nil) {
            NSLog(@"Get VibratorManager instance failed");
            return ERROR;
        }
        NSString *effectStr = [NSString stringWithUTF8String:effect.c_str()];
        state = [manager isSupportEffect:effectStr];
        return SUCCESS;
    }
}

int32_t VibratorImpl::PlayPattern(const VibratorIdentifierData &identifier, const VibratePattern &pattern,
    const CustomHapticInfoData &customHapticInfoData)
{
    NSLog(@"VibratorImpl::PlayPattern enter");
    @autoreleasepool {
        VibratorManager *manager = [VibratorManager sharedInstance];
        if (manager == nil) {
            NSLog(@"Get VibratorManager instance failed");
            return ERROR;
        }
        [manager playPattern:pattern];
        return SUCCESS;
    }
}

int32_t VibratorImpl::GetVibratorList(const VibratorIdentifierData &identifier, std::vector<VibratorInfo> &vibratorInfoData)
{
    NSLog(@"VibratorImpl::GetVibratorList enter");
    @autoreleasepool {
        VibratorManager *manager = [VibratorManager sharedInstance];
        if (manager == nil) {
            NSLog(@"Get VibratorManager instance failed");
            return ERROR;
        }
        NSArray<VibratorInfoObject *> *vibrators = [manager getVibratorList];
        for (VibratorInfoObject *vibrator in vibrators) {
            VibratorInfo info;
            info.deviceId = (int32_t)vibrator.deviceId;
            info.vibratorId = (int32_t)vibrator.vibratorId;
            info.deviceName = [vibrator.deviceName UTF8String];
            info.isSupportHdHaptic = vibrator.isSupportHdHaptic;
            info.isLocalVibrator = vibrator.isLocalVibrator;
            info.position = vibrator.position;
            vibratorInfoData.emplace_back(info);
        }
        return SUCCESS;
    }
}

int32_t VibratorImpl::GetEffectInfo(const VibratorIdentifierData &identifier, const std::string &effectType,
    EffectInfoData &effectInfoIPCData)
{
    NSLog(@"VibratorImpl::GetEffectInfo enter, effectType: %s", effectType.c_str());
    @autoreleasepool {
        VibratorManager *manager = [VibratorManager sharedInstance];
        if (manager == nil) {
            NSLog(@"Get VibratorManager instance failed");
            return ERROR;
        }
        NSString *effectTypeStr = [NSString stringWithUTF8String:effectType.c_str()];
        EffectInfo *info = [manager getEffectInfo:effectTypeStr];
        effectInfoIPCData.duration = [info.duration intValue];
        effectInfoIPCData.isSupportEffect = [manager isSupportEffect:effectTypeStr];
        return SUCCESS;
    }
}

int32_t VibratorImpl::GetVibratorCapacity(const VibratorIdentifierData &identifier, VibratorCapacity &capacity)
{
    NSLog(@"VibratorImpl::GetVibratorCapacity enter");
    @autoreleasepool {
        VibratorManager *manager = [VibratorManager sharedInstance];
        if (manager == nil) {
            NSLog(@"Get VibratorManager instance failed");
            return ERROR;
        }
        capacity.isSupportHdHaptic = manager.supportsCoreHaptics;
        capacity.isSupportPresetMapping = true;
        capacity.isSupportTimeDelay = true;
        return SUCCESS;
    }
}

int32_t VibratorImpl::CheckAuthAndParam(int32_t usage, const VibrateParameter &parameter,
    const VibratorIdentifierData &identifier)
{
    VibratorCapacity capacity;
    if (GetVibratorCapacity(identifier, capacity) != 0) {
        NSLog(@"GetVibratorCapacity failed");
        return ERROR;
    }
    if (!(capacity.isSupportHdHaptic || capacity.isSupportPresetMapping || capacity.isSupportTimeDelay)) {
        NSLog(@"The device does not support this operation");
        return IS_NOT_SUPPORTED;
    }
    if ((usage >= USAGE_MAX) || (usage < 0) || (!CheckVibratorParmeters(parameter))) {
        NSLog(@"Invalid parameter, usage:%d", usage);
        return PARAMETER_ERROR;
    }
    return SUCCESS;
}

bool VibratorImpl::CheckVibratorParmeters(const VibrateParameter &parameter)
{
    if ((parameter.intensity < INTENSITY_ADJUST_MIN) || (parameter.intensity > INTENSITY_ADJUST_MAX) ||
        (parameter.frequency < FREQUENCY_ADJUST_MIN) || (parameter.frequency > FREQUENCY_ADJUST_MAX)) {
        NSLog(@"Input invalid, intensity parameter is %d, frequency parameter is %d",
            parameter.intensity, parameter.frequency);
        return false;
    }
    return true;
}
} // namespace OHOS::Sensors