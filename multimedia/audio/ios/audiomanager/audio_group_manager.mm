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

#include "audio_group_manager.h"
#include "audio_manager_impl.h"

namespace OHOS {
namespace AudioStandard {
AudioGroupManager::AudioGroupManager(int32_t groupId) : groupId_(groupId) {}

AudioGroupManager::~AudioGroupManager() {}

int32_t AudioGroupManager::SetVolume(AudioVolumeType volumeType, int32_t volume, int32_t flag, int32_t uid)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

AudioStreamType AudioGroupManager::GetActiveVolumeType(const int32_t clientUid)
{
    return STREAM_DEFAULT;
}

int32_t AudioGroupManager::GetVolume(AudioVolumeType volumeType, int32_t uid)
{
    switch (volumeType) {
        case STREAM_MUSIC:
        case STREAM_VOICE_CALL:
        case STREAM_RING:
        case STREAM_VOICE_ASSISTANT:
        case STREAM_ALARM:
        case STREAM_ACCESSIBILITY:
            break;
        default:
            AUDIO_ERR_LOG("GetVolume volumeType=%{public}d not supported", volumeType);
            return ERR_NOT_SUPPORTED;
    }
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, 0, "managerImpl == nullptr.");
    return [managerImpl getVolume];
}

int32_t AudioGroupManager::GetMaxVolume(AudioVolumeType volumeType)
{
    switch (volumeType) {
        case STREAM_MUSIC:
        case STREAM_VOICE_CALL:
        case STREAM_RING:
        case STREAM_VOICE_ASSISTANT:
        case STREAM_ALARM:
        case STREAM_ACCESSIBILITY:
            break;
        default:
            AUDIO_ERR_LOG("GetMaxVolume volumeType=%{public}d not supported", volumeType);
            return ERR_NOT_SUPPORTED;
    }
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, 0, "managerImpl == nullptr.");
    return [managerImpl getMaxVolume];
}

int32_t AudioGroupManager::GetMinVolume(AudioVolumeType volumeType)
{
    switch (volumeType) {
        case STREAM_MUSIC:
        case STREAM_VOICE_CALL:
        case STREAM_RING:
        case STREAM_VOICE_ASSISTANT:
        case STREAM_ALARM:
        case STREAM_ACCESSIBILITY:
            break;
        default:
            AUDIO_ERR_LOG("GetMinVolume volumeType=%{public}d not supported", volumeType);
            return ERR_NOT_SUPPORTED;
    }
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, 0, "managerImpl == nullptr.");
    return [managerImpl getMinVolume];
}

int32_t AudioGroupManager::SetMute(AudioVolumeType volumeType, bool mute, const DeviceType &deviceType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioGroupManager::IsStreamMute(AudioVolumeType volumeType, bool &isMute)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioGroupManager::Init()
{
    return SUCCESS;
}

int32_t AudioGroupManager::GetGroupId()
{
    return groupId_;
}

int32_t AudioGroupManager::SetRingerModeCallback(const int32_t clientId,
    const std::shared_ptr<AudioRingerModeCallback> &callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioGroupManager::UnsetRingerModeCallback(const int32_t clientId) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioGroupManager::UnsetRingerModeCallback(
    const int32_t clientId, const std::shared_ptr<AudioRingerModeCallback>& callback) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioGroupManager::SetRingerMode(AudioRingerMode ringMode) const
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

AudioRingerMode AudioGroupManager::GetRingerMode() const
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return AudioRingerMode::RINGER_MODE_NORMAL;
}

int32_t AudioGroupManager::SetMicrophoneMute(bool isMute)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioGroupManager::SetMicrophoneMutePersistent(const bool isMute, const PolicyType type)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

bool AudioGroupManager::GetPersistentMicMuteState()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return false;
}

bool AudioGroupManager::IsMicrophoneMute()
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return false;
}

int32_t AudioGroupManager::SetMicStateChangeCallback(
    const std::shared_ptr<AudioManagerMicStateChangeCallback> &callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioGroupManager::UnsetMicStateChangeCallback(
    const std::shared_ptr<AudioManagerMicStateChangeCallback> &callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

bool AudioGroupManager::IsVolumeUnadjustable()
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return false;
}

int32_t AudioGroupManager::AdjustVolumeByStep(VolumeAdjustType adjustType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioGroupManager::AdjustSystemVolumeByStep(AudioVolumeType volumeType, VolumeAdjustType adjustType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

float AudioGroupManager::GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return 0.0f;
}

float AudioGroupManager::GetMaxAmplitude(const int32_t deviceId)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return 0.0f;
}
} // namespace AudioStandard
} // namespace OHOS
