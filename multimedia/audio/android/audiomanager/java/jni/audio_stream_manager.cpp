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

#include "audio_stream_manager.h"

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_manager_jni.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
AudioStreamManager* AudioStreamManager::GetInstance()
{
    static AudioStreamManager audioStreamManager;
    return &audioStreamManager;
}

int32_t AudioStreamManager::RegisterAudioRendererEventListener(
    const int32_t clientPid, const std::shared_ptr<AudioRendererStateChangeCallback>& callback)
{
    return Plugin::AudioManagerJni::RegisterAudioRendererEventListener(callback);
}

int32_t AudioStreamManager::UnregisterAudioRendererEventListener(const int32_t clientPid)
{
    return Plugin::AudioManagerJni::UnregisterAudioRendererEventListener();
}

int32_t AudioStreamManager::RegisterAudioCapturerEventListener(
    const int32_t clientPid, const std::shared_ptr<AudioCapturerStateChangeCallback>& callback)
{
    return Plugin::AudioManagerJni::RegisterAudioCapturerEventListener(callback);
}

int32_t AudioStreamManager::UnregisterAudioCapturerEventListener(const int32_t clientPid)
{
    return Plugin::AudioManagerJni::UnregisterAudioCapturerEventListener();
}

int32_t AudioStreamManager::GetCurrentRendererChangeInfos(
    vector<unique_ptr<AudioRendererChangeInfo>>& audioRendererChangeInfos)
{
    return Plugin::AudioManagerJni::GetCurrentRendererChangeInfos(audioRendererChangeInfos);
}

int32_t AudioStreamManager::GetCurrentCapturerChangeInfos(
    vector<unique_ptr<AudioCapturerChangeInfo>>& audioCapturerChangeInfos)
{
    return Plugin::AudioManagerJni::GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
}

bool AudioStreamManager::IsAudioRendererLowLatencySupported(const AudioStreamInfo& audioStreamInfo)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return false;
}

int32_t AudioStreamManager::GetEffectInfoArray(AudioSceneEffectInfo& audioSceneEffectInfo, StreamUsage streamUsage)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

bool AudioStreamManager::IsStreamActive(AudioVolumeType volumeType) const
{
    return Plugin::AudioManagerJni::IsStreamActive(volumeType);
}

int32_t AudioStreamManager::GetHardwareOutputSamplingRate(sptr<AudioDeviceDescriptor>& desc)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}
} // namespace AudioStandard
} // namespace OHOS
