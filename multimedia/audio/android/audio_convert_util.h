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

#ifndef PLUGINS_MULTIMEDIA_AUDIO_AUDIO_UTIL_H
#define PLUGINS_MULTIMEDIA_AUDIO_AUDIO_UTIL_H

#include "audio_info.h"

namespace OHOS::Plugin {
using namespace OHOS::AudioStandard;

enum AudioFormatEncoding {
    ENCODING_INVALID = 0x00,
    ENCODING_PCM_8BIT = 0x03,
    ENCODING_PCM_16BIT = 0x02,
    ENCODING_PCM_24BIT_PACKED = 0x15,
    ENCODING_PCM_32BIT = 0x16,
    ENCODING_PCM_FLOAT = 0x04,
};

enum AudioFormatChannelLayout : int32_t{
    CHANNEL_OUT_DEFAULT = 0x00000001,
    CHANNEL_OUT_MONO = 0x00000004,
    CHANNEL_OUT_STEREO = 0x0000000c,
    CHANNEL_OUT_SURROUND = 0x0000041c,
    CHANNEL_OUT_QUAD = 0x000000cc,
    CHANNEL_OUT_5POINT1 = 0x000000fc,
    CHANNEL_OUT_6POINT1 = 0x000004fc,
    CHANNEL_OUT_7POINT1_SURROUND = 0x000018fc,
    CHANNEL_OUT_5POINT1POINT2 = 0x003000fc,
    CHANNEL_OUT_5POINT1POINT4 = 0x000b40fc,
    CHANNEL_OUT_7POINT1POINT2 = 0x003018fc,
    CHANNEL_OUT_7POINT1POINT4 = 0x000b58fc,
    CHANNEL_OUT_9POINT1POINT4 = 0x0c0b58fc,
    CHANNEL_OUT_9POINT1POINT6 = 0x0c3b58fc,
    CHANNEL_OUT_FRONT_CENTER = 0x00000010,
    CHANNEL_OUT_BACK_CENTER = 0x00000400,
};

enum AudioFormatChannelLayoutIn : int32_t{
    CHANNEL_IN_DEFAULT = 0x00000001,
    CHANNEL_IN_MONO = 0x00000010,
    CHANNEL_IN_STEREO = 0x0000000c,
};

enum AudioAttributesUsage {
    USAGE_UNKNOWN = 0x00000000,
    USAGE_MEDIA = 0x00000001,
    USAGE_VOICE_COMMUNICATION = 0x00000002,
    USAGE_VOICE_COMMUNICATION_SIGNALLING = 0x00000003,
    USAGE_ALARM = 0x00000004, 
    USAGE_NOTIFICATION = 0x00000005,
    USAGE_NOTIFICATION_RINGTONE = 0x00000006,
    USAGE_NOTIFICATION_EVENT = 0x0000000a,
    USAGE_ASSISTANCE_ACCESSIBILITY = 0x0000000b,
    USAGE_ASSISTANCE_NAVIGATION_GUIDANCE = 0x0000000c,
    USAGE_ASSISTANCE_SONIFICATION = 0x0000000d,
    USAGE_GAME = 0x0000000e,
    USAGE_ASSISTANT = 0x00000010,
};

enum AudioDeviceType {
    TYPE_UNKNOWN = 0x00000000,
    TYPE_AUX_LINE = 0x00000013,
    TYPE_BLE_BROADCAST = 0x0000001e,
    TYPE_BLE_HEADSET = 0x0000001a,
    TYPE_BLE_SPEAKER = 0x0000001b,
    TYPE_BLUETOOTH_A2DP = 0x00000008,
    TYPE_BLUETOOTH_SCO = 0x00000007,
    TYPE_BUILTIN_EARPIECE = 0x00000001,
    TYPE_BUILTIN_MIC = 0x0000000f,
    TYPE_BUILTIN_SPEAKER = 0x00000002,
    TYPE_BUILTIN_SPEAKER_SAFE = 0x00000018,
    TYPE_BUS = 0x00000015,
    TYPE_DOCK = 0x0000000d,
    TYPE_DOCK_ANALOG = 0x0000001f,
    TYPE_FM = 0x0000000e,
    TYPE_FM_TUNER = 0x00000010,
    TYPE_HDMI = 0x00000009,
    TYPE_HDMI_ARC = 0x0000000a,
    TYPE_HDMI_EARC = 0x0000001d,
    TYPE_HEARING_AID = 0x00000017,
    TYPE_IP = 0x00000014,
    TYPE_LINE_ANALOG = 0x00000005,
    TYPE_LINE_DIGITAL = 0x00000006,
    TYPE_REMOTE_SUBMIX = 0x00000019,
    TYPE_TELEPHONY = 0x00000012,
    TYPE_TV_TUNER = 0x00000011,
    TYPE_USB_ACCESSORY = 0x0000000c,
    TYPE_USB_DEVICE = 0x0000000b,
    TYPE_USB_HEADSET = 0x00000016,
    TYPE_WIRED_HEADPHONES = 0x00000004,
    TYPE_WIRED_HEADSET = 0x00000003,
};

enum AudioAttributesContenType {
    CONTENT_TYPE_UNKNOWN = 0x00000000,
    CONTENT_TYPE_SPEECH = 0x00000001,
    CONTENT_TYPE_MUSIC = 0x00000002,
    CONTENT_TYPE_MOVIE = 0x00000003,
    CONTENT_TYPE_SONIFICATION = 0x00000004,
};

enum CapturePolicy {
    ALLOW_CAPTURE_BY_ALL = 0x00000001,
    ALLOW_CAPTURE_BY_NONE = 0x00000003,
};

enum AudioSourceType {
    AUDIO_SOURCE_DEFAULT = 0x00000000,
    AUDIO_SOURCE_MIC = 0x00000001,
    AUDIO_SOURCE_VOICE_RECOGNITION = 0x00000006,
    AUDIO_SOURCE_VOICE_COMMUNICATION = 0x00000007,
    AUDIO_SOURCE_REMOTE_SUBMIX = 0x00000008,
};

enum DualMonoMode {
    DUAL_MONO_MODE_OFF = 0x00000000,
    DUAL_MONO_MODE_LR = 0x00000001,
    DUAL_MONO_MODE_LL = 0x00000002,
    DUAL_MONO_MODE_RR = 0x00000003,
};

enum AudioMode {
    MODE_NORMAL = 0x00000000,
    MODE_RINGTONE = 0x00000001,
    MODE_IN_CALL = 0x00000002,
    MODE_IN_COMMUNICATION = 0x00000003,
    MODE_CALL_SCREENING = 0x00000004,
    MODE_CALL_REDIRECT = 0x00000005,
    MODE_COMMUNICATION_REDIRECT =0x00000006,
};

enum StreamType {
    STREAM_INVALID = -1,
    STREAM_VOICE_CALL = 0x00000000,
    STREAM_SYSTEM = 0x00000001,
    STREAM_RING = 0x00000002,
    STREAM_MUSIC = 0x00000003,
    STREAM_ALARM = 0x00000004,
    STREAM_NOTIFICATION = 0x00000005,
    STREAM_DTMF = 0x00000008,
    STREAM_ACCESSIBILITY = 0x0000000a,
};

enum RingerMode {
    RINGER_MODE_SILENT = 0x00000000,
    RINGER_MODE_VIBRATE = 0x00000001,
    RINGER_MODE_NORMAL = 0x00000002,
};

enum AudioDeviceFlags {
    GET_DEVICES_INVALID = -1,
    GET_DEVICES_INPUTS = 0x00000001,
    GET_DEVICES_OUTPUTS = 0x00000002,
    GET_DEVICES_ALL = 0x00000003,
};

struct AudioUsage
{
    AudioAttributesUsage usage_an;
    AudioAttributesContenType content_an;
};

AudioFormatEncoding ConvertAudioSampleFormatToAn(AudioSampleFormat formatOh);
AudioSampleFormat ConvertAudioSampleFormatToOh(AudioFormatEncoding formatAn);
void ConvertAudioUsageToAn(StreamUsage usageOh, AudioAttributesUsage &usage, AudioAttributesContenType &content);
void ConvertAudioUsageToOh(AudioAttributesUsage usage, AudioAttributesContenType content,
    StreamUsage &usageOh, ContentType &contentOh);
AudioFormatChannelLayoutIn ConvertCapturerAudioChannelLayoutToAn(AudioChannelLayout layoutOh);
CapturePolicy ConvertPrivacyTypeToAn(AudioPrivacyType privacyOh);
int32_t ConvertChannelCountToLayout(AudioChannel channelCount);
AudioDeviceType ConvertActiveDeviceTypeToAn(DeviceType deviceType);
DeviceType ConvertDeviceTypeToOh(AudioDeviceType deviceType);
AudioSourceType ConvertSourceTypeToAn(SourceType sourceTypeOh);
SourceType ConvertSourceTypeToOh(AudioSourceType sourceTypeAn);
DualMonoMode ConvertBlendModeToAn(ChannelBlendMode blendModeOh);
bool IsPCMFormat(int32_t encode);
AudioScene ConvertSceneToOh(AudioMode modeAn);
StreamType ConvertVolumeTypeToAn(AudioVolumeType typeOh);
AudioRingerMode ConvertRingerModeToOh(RingerMode modeAn);
AudioDeviceType ConvertDeviceTypeToAn(DeviceType deviceType);
AudioDeviceFlags ConvertDeviceFlagToAn(DeviceFlag flag);
} // namespace OHOS::Plugin
#endif // PLUGINS_MULTIMEDIA_AUDIO_AUDIO_UTIL_H
