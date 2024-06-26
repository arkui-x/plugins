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

#include "audio_system_manager.h"

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_manager_jni.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

AudioSystemManager::AudioSystemManager() {}

AudioSystemManager::~AudioSystemManager() {}

AudioSystemManager* AudioSystemManager::GetInstance()
{
    AUDIO_DEBUG_LOG("Entered %{public}s", __func__);
    static AudioSystemManager audioManager;
    return &audioManager;
}

int32_t AudioSystemManager::GetCallingPid()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return 0;
}

map<pair<ContentType, StreamUsage>, AudioStreamType> AudioSystemManager::CreateStreamMap()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    map<pair<ContentType, StreamUsage>, AudioStreamType> streamMap;
    return streamMap;
}

AudioStreamType AudioSystemManager::GetStreamType(ContentType contentType, StreamUsage streamUsage)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return STREAM_DEFAULT;
}

void AudioSystemManager::AudioServerDied(pid_t pid)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
}

int32_t AudioSystemManager::SetRingerMode(AudioRingerMode ringMode)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

AudioRingerMode AudioSystemManager::GetRingerMode()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return RINGER_MODE_NORMAL;
}

int32_t AudioSystemManager::SetAudioScene(const AudioScene& scene)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

AudioScene AudioSystemManager::GetAudioScene() const
{
    return Plugin::AudioManagerJni::GetAudioScene();
}

int32_t AudioSystemManager::SetDeviceActive(ActiveDeviceType deviceType, bool flag) const
{
    switch (deviceType) {
        case EARPIECE:
        case SPEAKER:
        case BLUETOOTH_SCO:
        case FILE_SINK_DEVICE:
            break;
        default:
            AUDIO_ERR_LOG("device=%{public}d not supported", deviceType);
            return ERR_NOT_SUPPORTED;
    }

    return Plugin::AudioManagerJni::SetDeviceActive(deviceType, flag);
}

bool AudioSystemManager::IsDeviceActive(ActiveDeviceType deviceType) const
{
    switch (deviceType) {
        case EARPIECE:
        case SPEAKER:
        case BLUETOOTH_SCO:
        case FILE_SINK_DEVICE:
            break;
        default:
            AUDIO_ERR_LOG("device=%{public}d not supported", deviceType);
            return false;
    }

    return Plugin::AudioManagerJni::IsDeviceActive(deviceType);
}

DeviceType AudioSystemManager::GetActiveOutputDevice()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return DEVICE_TYPE_NONE;
}

DeviceType AudioSystemManager::GetActiveInputDevice()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return DEVICE_TYPE_NONE;
}

bool AudioSystemManager::IsStreamActive(AudioVolumeType volumeType) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return false;
}

const std::string AudioSystemManager::GetAudioParameter(const std::string key)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return "";
}

void AudioSystemManager::SetAudioParameter(const std::string& key, const std::string& value)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
}

int32_t AudioSystemManager::GetExtraParameters(const std::string& mainKey, const std::vector<std::string>& subKeys,
    std::vector<std::pair<std::string, std::string>>& result)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetExtraParameters(
    const std::string& key, const std::vector<std::pair<std::string, std::string>>& kvpairs)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

uint64_t AudioSystemManager::GetTransactionId(DeviceType deviceType, DeviceRole deviceRole)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return 0L;
}

int32_t AudioSystemManager::SetVolume(AudioVolumeType volumeType, int32_t volumeLevel) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::GetVolume(AudioVolumeType volumeType) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetLowPowerVolume(int32_t streamId, float volume) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

float AudioSystemManager::GetLowPowerVolume(int32_t streamId) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return 0.0f;
}

float AudioSystemManager::GetSingleStreamVolume(int32_t streamId) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return 0.0f;
}

int32_t AudioSystemManager::GetMaxVolume(AudioVolumeType volumeType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::GetMinVolume(AudioVolumeType volumeType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetMute(AudioVolumeType volumeType, bool mute) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

bool AudioSystemManager::IsStreamMute(AudioVolumeType volumeType) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return false;
}

int32_t AudioSystemManager::SetDeviceChangeCallback(
    const DeviceFlag flag, const std::shared_ptr<AudioManagerDeviceChangeCallback>& callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "input param is invalid");
    return Plugin::AudioManagerJni::AddDeviceChangeCallback(flag, callback);
}

int32_t AudioSystemManager::UnsetDeviceChangeCallback(DeviceFlag flag)
{
    return Plugin::AudioManagerJni::RemoveDeviceChangeCallback();
}

int32_t AudioSystemManager::SetRingerModeCallback(
    const int32_t clientId, const std::shared_ptr<AudioRingerModeCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnsetRingerModeCallback(const int32_t clientId) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetMicrophoneMute(bool isMute)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

bool AudioSystemManager::IsMicrophoneMute(API_VERSION api_v)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return false;
}

int32_t AudioSystemManager::SelectOutputDevice(std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SelectInputDevice(std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

std::string AudioSystemManager::GetSelectedDeviceInfo(int32_t uid, int32_t pid, AudioStreamType streamType) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return "";
}

int32_t AudioSystemManager::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<sptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

std::vector<sptr<AudioDeviceDescriptor>> AudioSystemManager::GetDevices(DeviceFlag deviceFlag)
{
    std::vector<sptr<AudioDeviceDescriptor>> devices;
    Plugin::AudioManagerJni::GetDevices(deviceFlag, devices);
    return devices;
}

std::vector<sptr<AudioDeviceDescriptor>> AudioSystemManager::GetActiveOutputDeviceDescriptors()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    std::vector<sptr<AudioDeviceDescriptor>> ret;
    return ret;
}

int32_t AudioSystemManager::GetPreferredInputDeviceDescriptors()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::GetAudioFocusInfoList(std::list<std::pair<AudioInterrupt, AudioFocuState>>& focusInfoList)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::RegisterFocusInfoChangeCallback(
    const std::shared_ptr<AudioFocusInfoChangeCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnregisterFocusInfoChangeCallback(
    const std::shared_ptr<AudioFocusInfoChangeCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::RegisterVolumeKeyEventCallback(
    const int32_t clientPid, const std::shared_ptr<VolumeKeyEventCallback>& callback, API_VERSION api_v)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnregisterVolumeKeyEventCallback(const int32_t clientPid)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

void AudioSystemManager::SetAudioMonoState(bool monoState)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
}

void AudioSystemManager::SetAudioBalanceValue(float balanceValue)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
}

int32_t AudioSystemManager::SetSystemSoundUri(const std::string& key, const std::string& uri)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

std::string AudioSystemManager::GetSystemSoundUri(const std::string& key)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return "";
}

int32_t AudioSystemManager::SetAudioManagerCallback(
    const AudioVolumeType streamType, const std::shared_ptr<AudioManagerCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnsetAudioManagerCallback(const AudioVolumeType streamType) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::ActivateAudioInterrupt(const AudioInterrupt& audioInterrupt)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::DeactivateAudioInterrupt(const AudioInterrupt& audioInterrupt) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetAudioManagerInterruptCallback(const std::shared_ptr<AudioManagerCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnsetAudioManagerInterruptCallback()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::RequestAudioFocus(const AudioInterrupt& audioInterrupt)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::AbandonAudioFocus(const AudioInterrupt& audioInterrupt)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::ReconfigureAudioChannel(const uint32_t& count, DeviceType deviceType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::GetVolumeGroups(std::string networkId, std::vector<sptr<VolumeGroupInfo>>& infos)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

std::shared_ptr<AudioGroupManager> AudioSystemManager::GetGroupManager(int32_t groupId)
{
    AUDIO_DEBUG_LOG("GetGroupManager groupId:%{public}d.", groupId);

    for (const auto& iter : groupManagerMap_) {
        if (iter->GetGroupId() == groupId) {
            return iter;
        }
    }
    std::shared_ptr<AudioGroupManager> groupManager = std::make_shared<AudioGroupManager>(groupId);
    groupManagerMap_.push_back(groupManager);
    return groupManager;
}

bool AudioSystemManager::RequestIndependentInterrupt(FocusType focusType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return false;
}

bool AudioSystemManager::AbandonIndependentInterrupt(FocusType focusType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return false;
}

int32_t AudioSystemManager::GetAudioLatencyFromXml() const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

uint32_t AudioSystemManager::GetSinkLatencyFromXml() const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UpdateStreamState(
    const int32_t clientUid, StreamSetState streamSetState, StreamUsage streamUsage)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

std::string AudioSystemManager::GetSelfBundleName()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return "";
}

int32_t AudioSystemManager::OffloadDrain()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::GetCapturePresentationPosition(
    const std::string& deviceClass, uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::GetRenderPresentationPosition(
    const std::string& deviceClass, uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::OffloadGetPresentationPosition(uint64_t& frames, int64_t& timeSec, int64_t& timeNanoSec)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::OffloadSetBufferSize(uint32_t sizeMs)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::OffloadSetVolume(float volume)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

void AudioSystemManager::RequestThreadPriority(uint32_t tid)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
}

int32_t AudioSystemManager::SetDeviceAbsVolumeSupported(const std::string& macAddress, const bool support)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetA2dpDeviceVolume(
    const std::string& macAddress, const int32_t volume, const bool updateUi)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

AudioPin AudioSystemManager::GetPinValueFromType(DeviceType deviceType, DeviceRole deviceRole) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return AUDIO_PIN_NONE;
}

DeviceType AudioSystemManager::GetTypeValueFromPin(AudioPin pin) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return DEVICE_TYPE_NONE;
}

int32_t AudioSystemManager::RegisterWakeupSourceCallback()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetAudioCapturerSourceCallback(const std::shared_ptr<AudioCapturerSourceCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetWakeUpSourceCloseCallback(const std::shared_ptr<WakeUpSourceCloseCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetAvailableDeviceChangeCallback(
    const AudioDeviceUsage usage, const std::shared_ptr<AudioManagerAvailableDeviceChangeCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnsetAvailableDeviceChangeCallback(AudioDeviceUsage usage)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::ConfigDistributedRoutingRole(AudioDeviceDescriptor* descriptor, CastType type)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetDistributedRoutingRoleCallback(
    const std::shared_ptr<AudioDistributedRoutingRoleCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::UnsetDistributedRoutingRoleCallback(
    const std::shared_ptr<AudioDistributedRoutingRoleCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SetCallDeviceActive(ActiveDeviceType deviceType, bool flag, std::string address) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::DisableSafeMediaVolume()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}
} // namespace AudioStandard
} // namespace OHOS
