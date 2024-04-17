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

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_manager_jni.h"

namespace OHOS {
namespace AudioStandard {
AudioGroupManager::AudioGroupManager(int32_t groupId) : groupId_(groupId) {}

AudioGroupManager::~AudioGroupManager() {}

int32_t AudioGroupManager::SetVolume(AudioVolumeType volumeType, int32_t volume)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioGroupManager::GetVolume(AudioVolumeType volumeType)
{
    return Plugin::AudioManagerJni::GetVolume(volumeType);
}

int32_t AudioGroupManager::GetMaxVolume(AudioVolumeType volumeType)
{
    return Plugin::AudioManagerJni::GetMaxVolume(volumeType);
}

int32_t AudioGroupManager::GetMinVolume(AudioVolumeType volumeType)
{
    return Plugin::AudioManagerJni::GetMinVolume(volumeType);
}

int32_t AudioGroupManager::SetMute(AudioVolumeType volumeType, bool mute)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioGroupManager::IsStreamMute(AudioVolumeType volumeType, bool& isMute)
{
    return Plugin::AudioManagerJni::IsStreamMute(volumeType, isMute);
}

int32_t AudioGroupManager::Init()
{
    return SUCCESS;
}

bool AudioGroupManager::IsAlived()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return false;
}

int32_t AudioGroupManager::GetGroupId()
{
    return groupId_;
}

int32_t AudioGroupManager::SetRingerModeCallback(
    const int32_t clientId, const std::shared_ptr<AudioRingerModeCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
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
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

AudioRingerMode AudioGroupManager::GetRingerMode() const
{
    return Plugin::AudioManagerJni::GetRingerMode();
}

int32_t AudioGroupManager::SetMicrophoneMute(bool isMute)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

bool AudioGroupManager::IsMicrophoneMute(API_VERSION api_v)
{
    return Plugin::AudioManagerJni::IsMicrophoneMute();
}

int32_t AudioGroupManager::SetMicStateChangeCallback(
    const std::shared_ptr<AudioManagerMicStateChangeCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

bool AudioGroupManager::IsVolumeUnadjustable()
{
    return Plugin::AudioManagerJni::IsVolumeFixed();
}

int32_t AudioGroupManager::AdjustVolumeByStep(VolumeAdjustType adjustType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioGroupManager::AdjustSystemVolumeByStep(AudioVolumeType volumeType, VolumeAdjustType adjustType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

float AudioGroupManager::GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType)
{
    return Plugin::AudioManagerJni::GetSystemVolumeInDb(volumeType, volumeLevel, deviceType);
}

float AudioGroupManager::GetMaxAmplitude(const int32_t deviceId)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return 0.0f;
}
} // namespace AudioStandard
} // namespace OHOS
