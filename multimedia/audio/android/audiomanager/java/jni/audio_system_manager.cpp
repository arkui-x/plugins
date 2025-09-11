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

int32_t AudioSystemManager::GetCallingPid() const
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

void AudioSystemManager::AudioServerDied(pid_t pid, pid_t uid)
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

int32_t AudioSystemManager::SetDeviceActive(DeviceType deviceType, bool flag, const int32_t clientPid) const
{
    AUDIO_INFO_LOG("device: %{public}d", deviceType);
    if (!IsActiveDeviceType(deviceType)) {
        AUDIO_ERR_LOG("device=%{public}d not supported", deviceType);
        return ERR_NOT_SUPPORTED;
    }

    return Plugin::AudioManagerJni::SetDeviceActive(deviceType, flag);
}

bool AudioSystemManager::IsDeviceActive(DeviceType deviceType) const
{
    if (!IsActiveDeviceType(deviceType)) {
        AUDIO_ERR_LOG("device=%{public}d not supported", deviceType);
        return ERR_NOT_SUPPORTED;
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

int32_t AudioSystemManager::SetVolume(AudioVolumeType volumeType, int32_t volumeLevel, int32_t uid) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::GetVolume(AudioVolumeType volumeType, int32_t uid) const
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

int32_t AudioSystemManager::SetMute(AudioVolumeType volumeType, bool mute, const DeviceType &deviceType) const
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

int32_t AudioSystemManager::UnsetDeviceChangeCallback(DeviceFlag flag, std::shared_ptr<AudioManagerDeviceChangeCallback> cb)
{
    return Plugin::AudioManagerJni::RemoveDeviceChangeCallback();
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

bool AudioSystemManager::IsMicrophoneMute()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return false;
}

int32_t AudioSystemManager::SelectOutputDevice(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SelectInputDevice(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
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
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors,
    const int32_t audioDeviceSelectMode) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSystemManager::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
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
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    Plugin::AudioManagerJni::GetDevices(deviceFlag, devices);
    return devices;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioSystemManager::GetActiveOutputDeviceDescriptors()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;
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

int32_t AudioSystemManager::UnregisterVolumeKeyEventCallback(const int32_t clientPid,
    const std::shared_ptr<VolumeKeyEventCallback> &callback)
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

int32_t AudioSystemManager::ActivateAudioInterrupt(AudioInterrupt& audioInterrupt)
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

int32_t AudioSystemManager::ConfigDistributedRoutingRole(
    std::shared_ptr<AudioDeviceDescriptor> descriptor, CastType type)
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

int32_t AudioSystemManager::SetCallDeviceActive(DeviceType deviceType, bool flag, std::string address,
    const int32_t clientPid) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
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
