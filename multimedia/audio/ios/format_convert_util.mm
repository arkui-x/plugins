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

#include "format_convert_util.h"
#include "audio_errors.h"

bool ConvertChannelLayoutFromOh(const OHOS::AudioStandard::AudioChannelLayout channelLayoutIn,
    AudioChannelLayout &channelLayoutOut)
{
    bool isValid = true;
    switch (channelLayoutIn) {
        /** Channel count: 1*/
        case OHOS::AudioStandard::CH_LAYOUT_MONO:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_Mono;
            break;
        /** Channel count: 2*/
        case OHOS::AudioStandard::CH_LAYOUT_STEREO:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_Stereo;
            break;
        /** Channel count: 3*/
        case OHOS::AudioStandard::CH_LAYOUT_2POINT1:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_WAVE_2_1;
            break;
        case OHOS::AudioStandard::CH_LAYOUT_3POINT0:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_MPEG_3_0_A;
            break;
        case OHOS::AudioStandard::CH_LAYOUT_SURROUND:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_DVD_2;
            break;
        /** Channel count: 4*/
        case OHOS::AudioStandard::CH_LAYOUT_3POINT1:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_MPEG_4_0_A;
            break;
        case OHOS::AudioStandard::CH_LAYOUT_4POINT0:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_MPEG_4_0_B;
            break;
        case OHOS::AudioStandard::CH_LAYOUT_QUAD:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_Quadraphonic;
            break;
        case OHOS::AudioStandard::CH_LAYOUT_2POINT0POINT2:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_ITU_2_2;
            break;
        /** Channel count: 5*/
        case OHOS::AudioStandard::CH_LAYOUT_4POINT1:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_DTS_4_1;
            break;
        case OHOS::AudioStandard::CH_LAYOUT_5POINT0:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_AudioUnit_5_0;
            break;
        /** Channel count: 6*/
        case OHOS::AudioStandard::CH_LAYOUT_5POINT1:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_AudioUnit_5_1;
            break;
        case OHOS::AudioStandard::CH_LAYOUT_6POINT0:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_AudioUnit_6_0;
            break;
        case OHOS::AudioStandard::CH_LAYOUT_HEXAGONAL:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_Hexagonal;
            break;
        /** Channel count: 7*/
        case OHOS::AudioStandard::CH_LAYOUT_6POINT1:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_AudioUnit_6_1;
            break;
        case OHOS::AudioStandard::CH_LAYOUT_7POINT0:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_AudioUnit_7_0;
            break;
        case OHOS::AudioStandard::CH_LAYOUT_7POINT0_FRONT:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_AudioUnit_7_0_Front;
            break;
        /** Channel count: 8*/
        case OHOS::AudioStandard::CH_LAYOUT_7POINT1:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_AudioUnit_7_1;
            break;
        case OHOS::AudioStandard::CH_LAYOUT_OCTAGONAL:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_Octagonal;
            break;
        case OHOS::AudioStandard::CH_LAYOUT_5POINT1POINT2:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_Atmos_5_1_2;
            break;
        /** Channel count: 10*/
        case OHOS::AudioStandard::CH_LAYOUT_5POINT1POINT4:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_Atmos_5_1_4;
            break;
        case OHOS::AudioStandard::CH_LAYOUT_7POINT1POINT2:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_Atmos_7_1_2;
            break;
        /** Channel count: 12*/
        case OHOS::AudioStandard::CH_LAYOUT_7POINT1POINT4:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_Atmos_7_1_4;
            break;
        case OHOS::AudioStandard::CH_LAYOUT_10POINT2:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_TMH_10_2_std;
            break;
        /** Channel count: 16*/
        case OHOS::AudioStandard::CH_LAYOUT_9POINT1POINT6:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_Atmos_9_1_6;
            break;
        case OHOS::AudioStandard::CH_LAYOUT_HEXADECAGONAL:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_Hexagonal;
            break;
        /** HOA: third order*/
        case OHOS::AudioStandard::CH_LAYOUT_HOA_ORDER3_ACN_N3D:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_HOA_ACN_N3D;
            break;
        case OHOS::AudioStandard::CH_LAYOUT_HOA_ORDER3_ACN_SN3D:
            channelLayoutOut.mChannelLayoutTag = kAudioChannelLayoutTag_HOA_ACN_SN3D;
            break;
        default:
            isValid = false;
            break;
    }
    return isValid;
}

void ConvertStreamInfoFromOh(const OHOS::AudioStandard::AudioStreamInfo streamInfo,
    AudioStreamBasicDescription &audioDescription)
{
    audioDescription.mSampleRate = streamInfo.samplingRate;
    audioDescription.mFormatID = kAudioFormatLinearPCM;
    audioDescription.mChannelsPerFrame = streamInfo.channels;
    audioDescription.mFramesPerPacket = 1;
    switch (streamInfo.format) {
        case OHOS::AudioStandard::SAMPLE_U8:
            audioDescription.mBitsPerChannel = 8;
            audioDescription.mFormatFlags = kAudioFormatFlagIsPacked;
            break;
        case OHOS::AudioStandard::SAMPLE_S16LE:
            audioDescription.mBitsPerChannel = 16;
            audioDescription.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
            break;
        case OHOS::AudioStandard::SAMPLE_S24LE:
            audioDescription.mBitsPerChannel = 24;
            audioDescription.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
            break;
        case OHOS::AudioStandard::SAMPLE_S32LE:
            audioDescription.mBitsPerChannel = 32;
            audioDescription.mFormatFlags = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
            break;
        case OHOS::AudioStandard::SAMPLE_F32LE:
            audioDescription.mBitsPerChannel = 32;
            audioDescription.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
            break;
        default:
            audioDescription.mBitsPerChannel = 8;
            audioDescription.mFormatFlags = kAudioFormatFlagIsPacked;
            break;
    }
    audioDescription.mBytesPerFrame = (audioDescription.mBitsPerChannel / 8) * audioDescription.mChannelsPerFrame;
    audioDescription.mBytesPerPacket = audioDescription.mBytesPerFrame * audioDescription.mFramesPerPacket;
}

void ConvertDeviceTypeToOh(const AVAudioSessionPort portType, OHOS::AudioStandard::DeviceType &deviceType)
{
    if ([portType isEqualToString:AVAudioSessionPortBuiltInReceiver]) {
        deviceType = OHOS::AudioStandard::DeviceType::DEVICE_TYPE_EARPIECE;
    } else if ([portType isEqualToString:AVAudioSessionPortBuiltInSpeaker]) {
        deviceType = OHOS::AudioStandard::DeviceType::DEVICE_TYPE_SPEAKER;
    } else if ([portType isEqualToString:AVAudioSessionPortHeadsetMic]) {
        deviceType = OHOS::AudioStandard::DeviceType::DEVICE_TYPE_WIRED_HEADSET;
    } else if ([portType isEqualToString:AVAudioSessionPortHeadphones]) {
        deviceType = OHOS::AudioStandard::DeviceType::DEVICE_TYPE_WIRED_HEADPHONES;
    } else if ([portType isEqualToString:AVAudioSessionPortBluetoothHFP]) {
        deviceType = OHOS::AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_SCO;
    } else if ([portType isEqualToString:AVAudioSessionPortBluetoothA2DP]) {
        deviceType = OHOS::AudioStandard::DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    } else if ([portType isEqualToString:AVAudioSessionPortBuiltInMic]) {
        deviceType = OHOS::AudioStandard::DeviceType::DEVICE_TYPE_MIC;
    } else if ([portType isEqualToString:AVAudioSessionPortUSBAudio]) {
        deviceType = OHOS::AudioStandard::DeviceType::DEVICE_TYPE_USB_HEADSET;
    } else {
        deviceType = OHOS::AudioStandard::DeviceType::DEVICE_TYPE_DEFAULT;
    }
}

void ConvertDeviceChangeReasonToOh(const NSInteger reason,
    OHOS::AudioStandard::AudioStreamDeviceChangeReason &changeReason)
{
    if (reason == AVAudioSessionRouteChangeReasonOldDeviceUnavailable) {
        changeReason = OHOS::AudioStandard::AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE;
    } else if (reason == AVAudioSessionRouteChangeReasonNewDeviceAvailable) {
        changeReason = OHOS::AudioStandard::AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE;
    } else if (reason == AVAudioSessionRouteChangeReasonOverride) {
        changeReason = OHOS::AudioStandard::AudioStreamDeviceChangeReason::OVERRODE;
    } else {
        changeReason = OHOS::AudioStandard::AudioStreamDeviceChangeReason::UNKNOWN;
    }
}

void ConvertInterruptEventToOh(const AudioInterruption audioInterruption,
    OHOS::AudioStandard::InterruptEvent &interruptEvent)
{
    if (audioInterruption.interruptionType == AVAudioSessionInterruptionTypeBegan) {
        interruptEvent.eventType = OHOS::AudioStandard::INTERRUPT_TYPE_BEGIN;
    } else if (audioInterruption.interruptionType == AVAudioSessionInterruptionTypeEnded) {
        interruptEvent.eventType = OHOS::AudioStandard::INTERRUPT_TYPE_END;
    }

    interruptEvent.forceType = OHOS::AudioStandard::INTERRUPT_FORCE;

    if (audioInterruption.interruptionOption == AVAudioSessionInterruptionOptionShouldResume) {
        interruptEvent.hintType = OHOS::AudioStandard::INTERRUPT_HINT_RESUME;
    } else {
        interruptEvent.hintType = OHOS::AudioStandard::INTERRUPT_HINT_NONE;
    }
}

int32_t ConvertErrorToOh(const OSStatus status)
{
    int32_t error = OHOS::AudioStandard::ERROR;
    switch (status) {
        case kAudioQueueErr_InvalidProperty:
        case kAudioQueueErr_InvalidParameter:
        case kAudioQueueErr_InvalidPropertySize:
        case kAudioQueueErr_InvalidPropertyValue:
            error = OHOS::AudioStandard::ERR_INVALID_PARAM;
            break;
        case kAudioQueueErr_InvalidDevice:
            error = OHOS::AudioStandard::ERR_DEVICE_NOT_SUPPORTED;
            break;
        case kAudioQueueErr_InvalidRunState:
            error = OHOS::AudioStandard::ERR_ILLEGAL_STATE;
            break;
        case kAudioQueueErr_Permissions:
            error = OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED;
            break;
        default:
            break;
    }
    return error;
}