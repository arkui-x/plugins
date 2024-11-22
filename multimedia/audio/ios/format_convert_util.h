/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef FORMAT_CONVERT_UTIL_H
#define FORMAT_CONVERT_UTIL_H

#include "audio_info.h"
#import <AVFoundation/AVFoundation.h>

#define SEC_TO_NANOSECOND 1000000000
#define SEC_TO_MILLISECOND 1000

struct AudioInterruption {
    NSInteger interruptionType;
    NSInteger interruptionReason;
    NSInteger interruptionOption;
};

bool ConvertChannelLayoutFromOh(const OHOS::AudioStandard::AudioChannelLayout channelLayoutIn,
    AudioChannelLayout &channelLayoutOut);
void ConvertStreamInfoFromOh(const OHOS::AudioStandard::AudioStreamInfo streamInfo,
    AudioStreamBasicDescription &audioDescription);
void ConvertDeviceTypeToOh(const AVAudioSessionPort portType, OHOS::AudioStandard::DeviceType &deviceType);
void ConvertDeviceChangeReasonToOh(const NSInteger reason,
    OHOS::AudioStandard::AudioStreamDeviceChangeReason &changeReason);
void ConvertInterruptEventToOh(const AudioInterruption audioInterruption,
    OHOS::AudioStandard::InterruptEvent &interruptEvent);
int32_t ConvertErrorToOh(const OSStatus status);

#endif // FORMAT_CONVERT_UTIL_H