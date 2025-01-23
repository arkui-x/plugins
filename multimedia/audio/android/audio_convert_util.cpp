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

#include "audio_convert_util.h"

#include <map>

#include "audio_log.h"

namespace OHOS::Plugin {
AudioFormatEncoding ConvertAudioSampleFormatToAn(AudioSampleFormat formatOh)
{
    static const std::map<AudioSampleFormat, AudioFormatEncoding> table = {
        { AudioSampleFormat::INVALID_WIDTH, ENCODING_INVALID },
        { AudioSampleFormat::SAMPLE_U8, ENCODING_PCM_8BIT },
        { AudioSampleFormat::SAMPLE_S16LE, ENCODING_PCM_16BIT },
        { AudioSampleFormat::SAMPLE_S24LE, ENCODING_PCM_24BIT_PACKED },
        { AudioSampleFormat::SAMPLE_S32LE, ENCODING_PCM_32BIT },
        { AudioSampleFormat::SAMPLE_F32LE, ENCODING_PCM_FLOAT }
    };

    auto it = table.find(formatOh);
    if (it != table.end()) {
        return it->second;
    }
    return ENCODING_INVALID;
}

AudioSampleFormat ConvertAudioSampleFormatToOh(AudioFormatEncoding formatAn)
{
    static const std::map<AudioFormatEncoding, AudioSampleFormat> table = {
        { ENCODING_INVALID, AudioSampleFormat::INVALID_WIDTH },
        { ENCODING_PCM_8BIT, AudioSampleFormat::SAMPLE_U8 },
        { ENCODING_PCM_16BIT, AudioSampleFormat::SAMPLE_S16LE },
        { ENCODING_PCM_24BIT_PACKED, AudioSampleFormat::SAMPLE_S24LE },
        { ENCODING_PCM_32BIT, AudioSampleFormat::SAMPLE_S32LE },
        { ENCODING_PCM_FLOAT, AudioSampleFormat::SAMPLE_F32LE }
    };

    auto it = table.find(formatAn);
    if (it != table.end()) {
        return it->second;
    }
    return AudioSampleFormat::INVALID_WIDTH;
}

void ConvertAudioUsageToAn(StreamUsage usageOh, AudioAttributesUsage& usage, AudioAttributesContenType& content)
{
    static const std::map<StreamUsage, AudioUsage> table = {
        { StreamUsage::STREAM_USAGE_UNKNOWN, { USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN } },
        { StreamUsage::STREAM_USAGE_MUSIC, { USAGE_MEDIA, CONTENT_TYPE_MUSIC } },
        { StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION, { USAGE_VOICE_COMMUNICATION, CONTENT_TYPE_UNKNOWN } },
        { StreamUsage::STREAM_USAGE_VOICE_ASSISTANT, { USAGE_ASSISTANT, CONTENT_TYPE_UNKNOWN } },
        { StreamUsage::STREAM_USAGE_ALARM, { USAGE_ALARM, CONTENT_TYPE_UNKNOWN } },
        { StreamUsage::STREAM_USAGE_VOICE_MESSAGE, { USAGE_UNKNOWN, CONTENT_TYPE_UNKNOWN } },
        { StreamUsage::STREAM_USAGE_RINGTONE, { USAGE_NOTIFICATION_RINGTONE, CONTENT_TYPE_UNKNOWN } },
        { StreamUsage::STREAM_USAGE_NOTIFICATION, { USAGE_NOTIFICATION, CONTENT_TYPE_UNKNOWN } },
        { StreamUsage::STREAM_USAGE_ACCESSIBILITY, { USAGE_ASSISTANCE_ACCESSIBILITY, CONTENT_TYPE_UNKNOWN } },
        { StreamUsage::STREAM_USAGE_MOVIE, { USAGE_MEDIA, CONTENT_TYPE_MOVIE } },
        { StreamUsage::STREAM_USAGE_GAME, { USAGE_GAME, CONTENT_TYPE_UNKNOWN } },
        { StreamUsage::STREAM_USAGE_AUDIOBOOK, { USAGE_MEDIA, CONTENT_TYPE_SPEECH } },
        { StreamUsage::STREAM_USAGE_NAVIGATION, { USAGE_ASSISTANCE_NAVIGATION_GUIDANCE, CONTENT_TYPE_UNKNOWN } },
    };

    usage = USAGE_UNKNOWN;
    content = CONTENT_TYPE_UNKNOWN;

    auto it = table.find(usageOh);
    if (it != table.end()) {
        usage = it->second.usage_an;
        content = it->second.content_an;
        return;
    }
}

void ConvertAudioUsageToOh(
    AudioAttributesUsage usage, AudioAttributesContenType content, StreamUsage& usageOh, ContentType& contentOh)
{
    static const std::map<AudioAttributesUsage, StreamUsage> table = {
        { USAGE_UNKNOWN, StreamUsage::STREAM_USAGE_UNKNOWN },
        { USAGE_VOICE_COMMUNICATION, StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION },
        { USAGE_VOICE_COMMUNICATION_SIGNALLING, StreamUsage::STREAM_USAGE_VOICE_COMMUNICATION },
        { USAGE_ALARM, StreamUsage::STREAM_USAGE_ALARM },
        { USAGE_NOTIFICATION, StreamUsage::STREAM_USAGE_NOTIFICATION },
        { USAGE_NOTIFICATION_RINGTONE, StreamUsage::STREAM_USAGE_RINGTONE },
        { USAGE_NOTIFICATION_EVENT, StreamUsage::STREAM_USAGE_NOTIFICATION },
        { USAGE_ASSISTANCE_ACCESSIBILITY, StreamUsage::STREAM_USAGE_ACCESSIBILITY },
        { USAGE_ASSISTANCE_NAVIGATION_GUIDANCE, StreamUsage::STREAM_USAGE_NAVIGATION },
        { USAGE_ASSISTANCE_SONIFICATION, StreamUsage::STREAM_USAGE_UNKNOWN },
        { USAGE_GAME, StreamUsage::STREAM_USAGE_GAME },
        { USAGE_ASSISTANT, StreamUsage::STREAM_USAGE_VOICE_ASSISTANT },
    };

    usageOh = StreamUsage::STREAM_USAGE_UNKNOWN;
    contentOh = ContentType::CONTENT_TYPE_UNKNOWN;

    auto it = table.find(usage);
    if (it != table.end()) {
        usageOh = it->second;
    }
    switch (usage) {
        case USAGE_MEDIA:
            if (content == CONTENT_TYPE_MOVIE) {
                usageOh = StreamUsage::STREAM_USAGE_MOVIE;
                contentOh = ContentType::CONTENT_TYPE_MOVIE;
            } else if (content == CONTENT_TYPE_SPEECH) {
                usageOh = StreamUsage::STREAM_USAGE_AUDIOBOOK;
                contentOh = ContentType::CONTENT_TYPE_SPEECH;
            } else if (content == CONTENT_TYPE_MUSIC) {
                usageOh = StreamUsage::STREAM_USAGE_MUSIC;
                contentOh = ContentType::CONTENT_TYPE_MUSIC;
            }
            break;
        case USAGE_NOTIFICATION_RINGTONE:
            contentOh = ContentType::CONTENT_TYPE_RINGTONE;
            break;
        case USAGE_GAME:
            contentOh = ContentType::CONTENT_TYPE_GAME;
            break;
        default:
            break;
    }
}

AudioFormatChannelLayoutIn ConvertCapturerAudioChannelLayoutToAn(AudioChannelLayout layoutOh)
{
    static const std::map<AudioChannelLayout, AudioFormatChannelLayoutIn> table = {
        { AudioChannelLayout::CH_LAYOUT_MONO, CHANNEL_IN_MONO },
        { AudioChannelLayout::CH_LAYOUT_STEREO, CHANNEL_IN_STEREO },
    };

    auto it = table.find(layoutOh);
    if (it != table.end()) {
        return it->second;
    }
    return CHANNEL_IN_DEFAULT;
}

CapturePolicy ConvertPrivacyTypeToAn(AudioPrivacyType privacyOh)
{
    static const std::map<AudioPrivacyType, CapturePolicy> table = {
        { AudioPrivacyType::PRIVACY_TYPE_PUBLIC, ALLOW_CAPTURE_BY_ALL },
        { AudioPrivacyType::PRIVACY_TYPE_PRIVATE, ALLOW_CAPTURE_BY_NONE },
    };

    auto it = table.find(privacyOh);
    if (it != table.end()) {
        return it->second;
    }
    return ALLOW_CAPTURE_BY_NONE;
}

int32_t ConvertChannelCountToLayout(AudioChannel channelCount)
{
    static const std::map<AudioChannel, int32_t> table = {
        { AudioChannel::MONO, CHANNEL_OUT_MONO },
        { AudioChannel::STEREO, CHANNEL_OUT_STEREO },
        { AudioChannel::CHANNEL_3, CHANNEL_OUT_STEREO | CHANNEL_OUT_FRONT_CENTER },
        { AudioChannel::CHANNEL_4, CHANNEL_OUT_QUAD },
        { AudioChannel::CHANNEL_5, CHANNEL_OUT_QUAD | CHANNEL_OUT_FRONT_CENTER },
        { AudioChannel::CHANNEL_6, CHANNEL_OUT_5POINT1 },
        { AudioChannel::CHANNEL_7, CHANNEL_OUT_5POINT1 | CHANNEL_OUT_BACK_CENTER },
        { AudioChannel::CHANNEL_8, CHANNEL_OUT_7POINT1_SURROUND },
        { AudioChannel::CHANNEL_9, CHANNEL_OUT_DEFAULT },
        { AudioChannel::CHANNEL_10, CHANNEL_OUT_7POINT1POINT2 },
        { AudioChannel::CHANNEL_11, CHANNEL_OUT_DEFAULT },
        { AudioChannel::CHANNEL_12, CHANNEL_OUT_7POINT1POINT4 },
        { AudioChannel::CHANNEL_13, CHANNEL_OUT_DEFAULT },
        { AudioChannel::CHANNEL_14, CHANNEL_OUT_9POINT1POINT4 },
        { AudioChannel::CHANNEL_15, CHANNEL_OUT_DEFAULT },
        { AudioChannel::CHANNEL_16, CHANNEL_OUT_9POINT1POINT6 },
    };

    auto it = table.find(channelCount);
    if (it != table.end()) {
        return it->second;
    }
    return CHANNEL_OUT_DEFAULT;
}

AudioDeviceType ConvertActiveDeviceTypeToAn(DeviceType deviceType)
{
    static const std::map<DeviceType, AudioDeviceType> table = {
        { DEVICE_TYPE_BLUETOOTH_SCO, TYPE_BLUETOOTH_SCO},
        { DEVICE_TYPE_EARPIECE, TYPE_BUILTIN_EARPIECE},
        { DEVICE_TYPE_SPEAKER, TYPE_BUILTIN_SPEAKER},
    };

    auto it = table.find(deviceType);
    if (it != table.end()) {
        return it->second;
    }
    return TYPE_UNKNOWN;
}

DeviceType ConvertDeviceTypeToOh(AudioDeviceType deviceType)
{
    static const std::map<AudioDeviceType, DeviceType> table = {
        { TYPE_UNKNOWN, DEVICE_TYPE_INVALID },
        { TYPE_AUX_LINE, DEVICE_TYPE_DEFAULT },
        { TYPE_BLE_BROADCAST, DEVICE_TYPE_DEFAULT },
        { TYPE_BLE_HEADSET, DEVICE_TYPE_DEFAULT },
        { TYPE_BLE_SPEAKER, DEVICE_TYPE_SPEAKER },
        { TYPE_BLUETOOTH_A2DP, DEVICE_TYPE_BLUETOOTH_A2DP },
        { TYPE_BLUETOOTH_SCO, DEVICE_TYPE_BLUETOOTH_SCO },
        { TYPE_BUILTIN_EARPIECE, DEVICE_TYPE_EARPIECE },
        { TYPE_BUILTIN_MIC, DEVICE_TYPE_MIC },
        { TYPE_BUILTIN_SPEAKER, DEVICE_TYPE_SPEAKER },
        { TYPE_BUILTIN_SPEAKER_SAFE, DEVICE_TYPE_SPEAKER },
        { TYPE_BUS, DEVICE_TYPE_DEFAULT },
        { TYPE_DOCK, DEVICE_TYPE_DEFAULT },
        { TYPE_DOCK_ANALOG, DEVICE_TYPE_DEFAULT },
        { TYPE_FM, DEVICE_TYPE_DEFAULT },
        { TYPE_FM_TUNER, DEVICE_TYPE_DEFAULT },
        { TYPE_HDMI, DEVICE_TYPE_DEFAULT },
        { TYPE_HDMI_ARC, DEVICE_TYPE_DEFAULT },
        { TYPE_HDMI_EARC, DEVICE_TYPE_DEFAULT },
        { TYPE_HEARING_AID, DEVICE_TYPE_DEFAULT },
        { TYPE_IP, DEVICE_TYPE_DEFAULT },
        { TYPE_LINE_ANALOG, DEVICE_TYPE_DEFAULT },
        { TYPE_LINE_DIGITAL, DEVICE_TYPE_DEFAULT },
        { TYPE_REMOTE_SUBMIX, DEVICE_TYPE_DEFAULT },
        { TYPE_TELEPHONY, DEVICE_TYPE_DEFAULT },
        { TYPE_TV_TUNER, DEVICE_TYPE_DEFAULT },
        { TYPE_USB_ACCESSORY, DEVICE_TYPE_DEFAULT },
        { TYPE_USB_DEVICE, DEVICE_TYPE_DEFAULT },
        { TYPE_USB_HEADSET, DEVICE_TYPE_USB_HEADSET },
        { TYPE_WIRED_HEADPHONES, DEVICE_TYPE_WIRED_HEADPHONES },
        { TYPE_WIRED_HEADSET, DEVICE_TYPE_WIRED_HEADSET },
    };

    auto it = table.find(deviceType);
    if (it != table.end()) {
        return it->second;
    }
    return DEVICE_TYPE_INVALID;
}

AudioSourceType ConvertSourceTypeToAn(SourceType sourceTypeOh)
{
    static const std::map<SourceType, AudioSourceType> table = {
        { SourceType::SOURCE_TYPE_INVALID, AUDIO_SOURCE_DEFAULT },
        { SourceType::SOURCE_TYPE_MIC, AUDIO_SOURCE_MIC },
        { SourceType::SOURCE_TYPE_VOICE_RECOGNITION, AUDIO_SOURCE_VOICE_RECOGNITION },
        { SourceType::SOURCE_TYPE_PLAYBACK_CAPTURE, AUDIO_SOURCE_REMOTE_SUBMIX },
        { SourceType::SOURCE_TYPE_WAKEUP, AUDIO_SOURCE_DEFAULT },
        { SourceType::SOURCE_TYPE_VOICE_COMMUNICATION, AUDIO_SOURCE_VOICE_COMMUNICATION },
    };

    auto it = table.find(sourceTypeOh);
    if (it != table.end()) {
        return it->second;
    }
    return AUDIO_SOURCE_DEFAULT;
}

SourceType ConvertSourceTypeToOh(AudioSourceType sourceTypeAn)
{
    static const std::map<AudioSourceType, SourceType> table = {
        { AUDIO_SOURCE_MIC, SourceType::SOURCE_TYPE_MIC },
        { AUDIO_SOURCE_VOICE_RECOGNITION, SourceType::SOURCE_TYPE_VOICE_RECOGNITION },
        { AUDIO_SOURCE_VOICE_COMMUNICATION, SourceType::SOURCE_TYPE_VOICE_COMMUNICATION },
        { AUDIO_SOURCE_REMOTE_SUBMIX, SourceType::SOURCE_TYPE_PLAYBACK_CAPTURE },
    };

    auto it = table.find(sourceTypeAn);
    if (it != table.end()) {
        return it->second;
    }
    return SourceType::SOURCE_TYPE_INVALID;
}

DualMonoMode ConvertBlendModeToAn(ChannelBlendMode blendModeOh)
{
    static const std::map<ChannelBlendMode, DualMonoMode> table = {
        { ChannelBlendMode::MODE_DEFAULT, DUAL_MONO_MODE_OFF },
        { ChannelBlendMode::MODE_BLEND_LR, DUAL_MONO_MODE_LR },
        { ChannelBlendMode::MODE_ALL_LEFT, DUAL_MONO_MODE_LL },
        { ChannelBlendMode::MODE_ALL_RIGHT, DUAL_MONO_MODE_RR },
    };

    auto it = table.find(blendModeOh);
    if (it != table.end()) {
        return it->second;
    }
    return DUAL_MONO_MODE_OFF;
}

bool IsPCMFormat(int32_t encode)
{
    if ((encode == ENCODING_PCM_8BIT) || (encode == ENCODING_PCM_16BIT) || (encode == ENCODING_PCM_24BIT_PACKED) ||
        (encode == ENCODING_PCM_32BIT) || (encode == ENCODING_PCM_FLOAT)) {
        return true;
    }
    return false;
}

AudioScene ConvertSceneToOh(AudioMode modeAn)
{
    static const std::map<AudioMode, AudioScene> table = {
        { MODE_NORMAL, AUDIO_SCENE_DEFAULT },
        { MODE_RINGTONE, AUDIO_SCENE_RINGING },
        { MODE_IN_CALL, AUDIO_SCENE_PHONE_CALL },
        { MODE_IN_COMMUNICATION, AUDIO_SCENE_PHONE_CHAT },
        { MODE_CALL_SCREENING, AUDIO_SCENE_INVALID },
        { MODE_CALL_REDIRECT, AUDIO_SCENE_INVALID },
        { MODE_COMMUNICATION_REDIRECT, AUDIO_SCENE_INVALID },
    };

    auto it = table.find(modeAn);
    if (it != table.end()) {
        return it->second;
    }
    return AUDIO_SCENE_INVALID;
}

StreamType ConvertVolumeTypeToAn(AudioVolumeType typeOh)
{
    static const std::map<AudioVolumeType, StreamType> table = {
        { AudioVolumeType::STREAM_VOICE_CALL, StreamType::STREAM_VOICE_CALL },
        { AudioVolumeType::STREAM_RING, StreamType::STREAM_RING },
        { AudioVolumeType::STREAM_MUSIC, StreamType::STREAM_MUSIC },
        { AudioVolumeType::STREAM_ALARM, StreamType::STREAM_ALARM },
        { AudioVolumeType::STREAM_ACCESSIBILITY, StreamType::STREAM_ACCESSIBILITY },
        { AudioVolumeType::STREAM_VOICE_ASSISTANT, StreamType::STREAM_INVALID },
    };

    auto it = table.find(typeOh);
    if (it != table.end()) {
        return it->second;
    }
    return StreamType::STREAM_INVALID;
}

AudioRingerMode ConvertRingerModeToOh(RingerMode modeAn)
{
    static const std::map<RingerMode, AudioRingerMode> table = {
        { RingerMode::RINGER_MODE_SILENT, AudioRingerMode::RINGER_MODE_SILENT },
        { RingerMode::RINGER_MODE_VIBRATE, AudioRingerMode::RINGER_MODE_VIBRATE },
        { RingerMode::RINGER_MODE_NORMAL, AudioRingerMode::RINGER_MODE_NORMAL },
    };

    auto it = table.find(modeAn);
    if (it != table.end()) {
        return it->second;
    }
    return AudioRingerMode::RINGER_MODE_NORMAL;
}

AudioDeviceType ConvertDeviceTypeToAn(DeviceType deviceType)
{
    static const std::map<DeviceType, AudioDeviceType> table = {
        { DEVICE_TYPE_NONE, TYPE_UNKNOWN },
        { DEVICE_TYPE_INVALID, TYPE_UNKNOWN },
        { DEVICE_TYPE_EARPIECE, TYPE_BUILTIN_EARPIECE },
        { DEVICE_TYPE_SPEAKER, TYPE_BUILTIN_SPEAKER },
        { DEVICE_TYPE_WIRED_HEADSET, TYPE_WIRED_HEADSET },
        { DEVICE_TYPE_WIRED_HEADPHONES, TYPE_WIRED_HEADPHONES },
        { DEVICE_TYPE_BLUETOOTH_SCO, TYPE_BLUETOOTH_SCO },
        { DEVICE_TYPE_BLUETOOTH_A2DP, TYPE_BLUETOOTH_A2DP },
        { DEVICE_TYPE_MIC, TYPE_BUILTIN_MIC },
        { DEVICE_TYPE_WAKEUP, TYPE_UNKNOWN },
        { DEVICE_TYPE_USB_HEADSET, TYPE_USB_HEADSET },
        { DEVICE_TYPE_USB_ARM_HEADSET, TYPE_UNKNOWN },
        { DEVICE_TYPE_FILE_SINK, TYPE_UNKNOWN },
        { DEVICE_TYPE_FILE_SOURCE, TYPE_UNKNOWN },
        { DEVICE_TYPE_EXTERN_CABLE, TYPE_UNKNOWN },
        { DEVICE_TYPE_DEFAULT, TYPE_UNKNOWN },
    };

    auto it = table.find(deviceType);
    if (it != table.end()) {
        return it->second;
    }
    return TYPE_UNKNOWN;
}

AudioDeviceFlags ConvertDeviceFlagToAn(DeviceFlag flag)
{
    static const std::map<DeviceFlag, AudioDeviceFlags> table = {
        { OUTPUT_DEVICES_FLAG, GET_DEVICES_OUTPUTS },
        { INPUT_DEVICES_FLAG, GET_DEVICES_INPUTS },
        { ALL_DEVICES_FLAG, GET_DEVICES_ALL },
    };

    auto it = table.find(flag);
    if (it != table.end()) {
        return it->second;
    }
    return GET_DEVICES_INVALID;
}
} // namespace OHOS::Plugin
