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

#include "audio_log.h"
#include "audio_manager_impl.h"
#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

const map<pair<ContentType, StreamUsage>, AudioStreamType> AudioSystemManager::streamTypeMap_
    = AudioSystemManager::CreateStreamMap();

AudioSystemManager::AudioSystemManager()
{
    AUDIO_DEBUG_LOG("AudioSystemManager start");
}

AudioSystemManager::~AudioSystemManager()
{
    AUDIO_DEBUG_LOG("AudioSystemManager end");
}

AudioSystemManager *AudioSystemManager::GetInstance()
{
    static AudioSystemManager audioManager;
    return &audioManager;
}

map<pair<ContentType, StreamUsage>, AudioStreamType> AudioSystemManager::CreateStreamMap()
{
    map<pair<ContentType, StreamUsage>, AudioStreamType> streamMap;
    // Mapping relationships from content and usage to stream type in design
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_UNKNOWN)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_MODEM_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_SYSTEM)] = STREAM_SYSTEM;
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_MEDIA)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_MOVIE, STREAM_USAGE_MEDIA)] = STREAM_MOVIE;
    streamMap[make_pair(CONTENT_TYPE_GAME, STREAM_USAGE_MEDIA)] = STREAM_GAME;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_MEDIA)] = STREAM_SPEECH;
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_ALARM)] = STREAM_ALARM;
    streamMap[make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_NOTIFICATION)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_PROMPT, STREAM_USAGE_ENFORCED_TONE)] = STREAM_SYSTEM_ENFORCED;
    streamMap[make_pair(CONTENT_TYPE_DTMF, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_DTMF;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[make_pair(CONTENT_TYPE_SPEECH, STREAM_USAGE_ACCESSIBILITY)] = STREAM_ACCESSIBILITY;
    streamMap[make_pair(CONTENT_TYPE_ULTRASONIC, STREAM_USAGE_SYSTEM)] = STREAM_ULTRASONIC;

    // Old mapping relationships from content and usage to stream type
    streamMap[make_pair(CONTENT_TYPE_MUSIC, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_UNKNOWN)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_MEDIA)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_SONIFICATION, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_UNKNOWN)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_MEDIA)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_RINGTONE, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;

    // Only use stream usage to choose stream type
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MEDIA)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MUSIC)] = STREAM_MUSIC;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_MODEM_COMMUNICATION)] = STREAM_VOICE_CALL;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_ASSISTANT)] = STREAM_VOICE_ASSISTANT;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ALARM)] = STREAM_ALARM;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_VOICE_MESSAGE)] = STREAM_VOICE_MESSAGE;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NOTIFICATION_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_RINGTONE)] = STREAM_RING;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NOTIFICATION)] = STREAM_NOTIFICATION;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ACCESSIBILITY)] = STREAM_ACCESSIBILITY;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_SYSTEM)] = STREAM_SYSTEM;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_MOVIE)] = STREAM_MOVIE;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_GAME)] = STREAM_GAME;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_AUDIOBOOK)] = STREAM_SPEECH;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_NAVIGATION)] = STREAM_NAVIGATION;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_DTMF)] = STREAM_DTMF;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ENFORCED_TONE)] = STREAM_SYSTEM_ENFORCED;
    streamMap[make_pair(CONTENT_TYPE_UNKNOWN, STREAM_USAGE_ULTRASONIC)] = STREAM_ULTRASONIC;

    return streamMap;
}

AudioStreamType AudioSystemManager::GetStreamType(ContentType contentType, StreamUsage streamUsage)
{
    AudioStreamType streamType = AudioStreamType::STREAM_MUSIC;
    auto pos = streamTypeMap_.find(make_pair(contentType, streamUsage));
    if (pos != streamTypeMap_.end()) {
        streamType = pos->second;
    } else {
        AUDIO_ERR_LOG("The pair of contentType and streamUsage is not in design. Use the default stream type");
    }

    if (streamType == AudioStreamType::STREAM_MEDIA) {
        streamType = AudioStreamType::STREAM_MUSIC;
    }

    return streamType;
}

int32_t AudioSystemManager::SetRingerMode(AudioRingerMode ringMode)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

AudioRingerMode AudioSystemManager::GetRingerMode()
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ringModeBackup_;
}

int32_t AudioSystemManager::SetAudioScene(const AudioScene &scene)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

AudioScene AudioSystemManager::GetAudioScene() const
{
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, AUDIO_SCENE_DEFAULT, "managerImpl == nullptr.");
    return [managerImpl getAudioScene];
}

int32_t AudioSystemManager::SetDeviceActive(DeviceType deviceType, bool flag, const int32_t clientPid) const
{
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, ERROR, "managerImpl == nullptr.");
    return [managerImpl setDeviceActive:deviceType active:flag];
}

bool AudioSystemManager::IsDeviceActive(DeviceType deviceType) const
{
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, false, "managerImpl == nullptr.");
    return [managerImpl isDeviceActive:deviceType];
}

bool AudioSystemManager::IsStreamActive(AudioVolumeType volumeType) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return false;
}

const std::string AudioSystemManager::GetAudioParameter(const std::string key)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return "";
}

void AudioSystemManager::SetAudioParameter(const std::string &key, const std::string &value)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
}

int32_t AudioSystemManager::GetExtraParameters(const std::string &mainKey,
    const std::vector<std::string> &subKeys, std::vector<std::pair<std::string, std::string>> &result)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetExtraParameters(const std::string &key,
    const std::vector<std::pair<std::string, std::string>> &kvpairs)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetVolume(AudioVolumeType volumeType, int32_t volumeLevel) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::GetVolume(AudioVolumeType volumeType) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return 0;
}

int32_t AudioSystemManager::GetMaxVolume(AudioVolumeType volumeType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return 0;
}

int32_t AudioSystemManager::GetMinVolume(AudioVolumeType volumeType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return 0;
}

int32_t AudioSystemManager::SetMute(AudioVolumeType volumeType, bool mute, const DeviceType &deviceType) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

bool AudioSystemManager::IsStreamMute(AudioVolumeType volumeType) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return false;
}

int32_t AudioSystemManager::SetDeviceChangeCallback(const DeviceFlag flag,
    const std::shared_ptr<AudioManagerDeviceChangeCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnsetDeviceChangeCallback(DeviceFlag flag, std::shared_ptr<AudioManagerDeviceChangeCallback> cb)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetMicrophoneBlockedCallback(
    const std::shared_ptr<AudioManagerMicrophoneBlockedCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnsetMicrophoneBlockedCallback(
    const std::shared_ptr<AudioManagerMicrophoneBlockedCallback> callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetRingerModeCallback(const int32_t clientId,
                                                  const std::shared_ptr<AudioRingerModeCallback> &callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnsetRingerModeCallback(const int32_t clientId) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetMicrophoneMute(bool isMute)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

bool AudioSystemManager::IsMicrophoneMute()
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return false;
}

int32_t AudioSystemManager::SelectOutputDevice(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SelectInputDevice(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetExcludedDevices(
        AudioDeviceUsage audioDevUsage) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
    return ret;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetDevices(DeviceFlag deviceFlag)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptors = {};
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, descriptors, "managerImpl == nullptr.");
    return [managerImpl getDevices:deviceFlag];
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetActiveOutputDeviceDescriptors()
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descriptors;
    return descriptors;
}

int32_t AudioSystemManager::RegisterVolumeKeyEventCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback, API_VERSION api_v)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, ERROR, "managerImpl == nullptr.");
    return [managerImpl registerVolumeKeyEventCallback:callback];
}

int32_t AudioSystemManager::UnregisterVolumeKeyEventCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetAudioManagerInterruptCallback(const std::shared_ptr<AudioManagerCallback> &callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnsetAudioManagerInterruptCallback()
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::RequestAudioFocus(const AudioInterrupt &audioInterrupt)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::AbandonAudioFocus(const AudioInterrupt &audioInterrupt)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::GetVolumeGroups(std::string networkId, std::vector<sptr<VolumeGroupInfo>> &infos)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

std::shared_ptr<AudioGroupManager> AudioSystemManager::GetGroupManager(int32_t groupId)
{
    for (const auto& iter : groupManagerMap_) {
        if (iter->GetGroupId() == groupId) {
            return iter;
        }
    }

    std::shared_ptr<AudioGroupManager> groupManager = std::make_shared<AudioGroupManager>(groupId);
    if (groupManager->Init() == SUCCESS) {
        groupManagerMap_.push_back(groupManager);
    } else {
        groupManager = nullptr;
    }
    return groupManager;
}

bool AudioSystemManager::RequestIndependentInterrupt(FocusType focusType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return false;
}

bool AudioSystemManager::AbandonIndependentInterrupt(FocusType focusType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return false;
}

int32_t AudioSystemManager::SetAvailableDeviceChangeCallback(const AudioDeviceUsage usage,
    const std::shared_ptr<AudioManagerAvailableDeviceChangeCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnsetAvailableDeviceChangeCallback(AudioDeviceUsage usage)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::DisableSafeMediaVolume()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetOutputDevice(
    sptr<AudioRendererFilter> audioRendererFilter)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
    return ret;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetInputDevice(
    sptr<AudioCapturerFilter> audioCapturerFilter)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
    return ret;
}

int32_t AudioSystemManager::SetSelfAppVolume(int32_t volume, int32_t flag)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetAppVolume(int32_t appUid, int32_t volume, int32_t flag)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::GetAppVolume(int32_t appUid, int32_t &volumeLevel) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::GetSelfAppVolume(int32_t &volumeLevel) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetAppVolumeMuted(int32_t appUid, bool muted, int32_t volumeFlag)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnsetSelfAppVolumeCallback(
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetSelfAppVolumeCallback(
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetAppVolumeCallbackForUid(
    const int32_t appUid, const std::shared_ptr<AudioManagerAppVolumeChangeCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnsetAppVolumeCallbackForUid(
    const std::shared_ptr<AudioManagerAppVolumeChangeCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::IsAppVolumeMute(const int32_t appUid, const bool owned, bool &isMute)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnsetActiveVolumeTypeCallback(
    const std::shared_ptr<AudioManagerActiveVolumeTypeChangeCallback> &callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetActiveVolumeTypeCallback(
    const std::shared_ptr<AudioManagerActiveVolumeTypeChangeCallback> &callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetAudioSceneChangeCallback(
    const std::shared_ptr<AudioManagerAudioSceneChangedCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnsetAudioSceneChangeCallback(
    const std::shared_ptr<AudioManagerAudioSceneChangedCallback> callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

float AudioSystemManager::GetVolumeInUnitOfDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType device)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::GetMaxVolumeByUsage(StreamUsage streamUsage)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::GetMinVolumeByUsage(StreamUsage streamUsage)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::GetVolumeByUsage(StreamUsage streamUsage)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::IsStreamMuteByUsage(StreamUsage streamUsage, bool &isMute)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

float AudioSystemManager::GetVolumeInDbByStream(StreamUsage streamUsage, int32_t volumeLevel, DeviceType deviceType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

std::vector<AudioVolumeType> AudioSystemManager::GetSupportedAudioVolumeTypes()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    std::vector<AudioVolumeType> nullList = {};
    return nullList;
}

AudioVolumeType AudioSystemManager::GetAudioVolumeTypeByStreamUsage(StreamUsage streamUsage)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return AudioStreamType::STREAM_DEFAULT;
}

std::vector<StreamUsage> AudioSystemManager::GetStreamUsagesByVolumeType(AudioVolumeType audioVolumeType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    std::vector<StreamUsage> nullList = {};
    return nullList;
}

int32_t AudioSystemManager::RegisterStreamVolumeChangeCallback(const int32_t clientPid,
    const std::set<StreamUsage> &streamUsages, const std::shared_ptr<StreamVolumeChangeCallback> &callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnregisterStreamVolumeChangeCallback(const int32_t clientPid,
    const std::shared_ptr<StreamVolumeChangeCallback> &callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::RegisterSystemVolumeChangeCallback(const int32_t clientPid,
    const std::shared_ptr<SystemVolumeChangeCallback> &callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnregisterSystemVolumeChangeCallback(const int32_t clientPid,
    const std::shared_ptr<SystemVolumeChangeCallback> &callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::ForceVolumeKeyControlType(AudioVolumeType volumeType, int32_t duration)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}
} // namespace AudioStandard
} // namespace OHOS
