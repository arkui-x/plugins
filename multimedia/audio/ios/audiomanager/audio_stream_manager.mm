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
#include "audio_stream_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

AudioStreamManager *AudioStreamManager::GetInstance()
{
    static AudioStreamManager audioStreamManager;
    return &audioStreamManager;
}

int32_t AudioStreamManager::RegisterAudioRendererEventListener(const int32_t clientPid,
    const std::shared_ptr<AudioRendererStateChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, ERROR, "managerImpl == nullptr.");
    return [managerImpl registerAudioRendererEventListener:callback];
}

int32_t AudioStreamManager::UnregisterAudioRendererEventListener(const int32_t clientPid)
{
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, ERROR, "managerImpl == nullptr.");
    return [managerImpl unregisterAudioRendererEventListener];
}

int32_t AudioStreamManager::RegisterAudioCapturerEventListener(const int32_t clientPid,
    const std::shared_ptr<AudioCapturerStateChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, ERROR, "managerImpl == nullptr.");
    return [managerImpl registerAudioCapturerEventListener:callback];
}

int32_t AudioStreamManager::UnregisterAudioCapturerEventListener(const int32_t clientPid)
{
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, ERROR, "managerImpl == nullptr.");
    return [managerImpl unregisterAudioCapturerEventListener];
}

int32_t AudioStreamManager::GetCurrentRendererChangeInfos(
    vector<unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos)
{
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, ERROR, "managerImpl == nullptr.");
    return [managerImpl getCurrentRendererChangeInfos:audioRendererChangeInfos];
}

int32_t AudioStreamManager::GetCurrentCapturerChangeInfos(
    vector<unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, ERROR, "managerImpl == nullptr.");
    return [managerImpl getCurrentCapturerChangeInfos:audioCapturerChangeInfos];
}

bool AudioStreamManager::IsAudioRendererLowLatencySupported(const AudioStreamInfo &audioStreamInfo)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return false;
}

int32_t AudioStreamManager::GetEffectInfoArray(AudioSceneEffectInfo &audioSceneEffectInfo, StreamUsage streamUsage)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

bool AudioStreamManager::IsStreamActive(AudioVolumeType volumeType) const
{
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, false, "managerImpl == nullptr.");
    return [managerImpl isStreamActive:volumeType];
}

int32_t AudioStreamManager::GetHardwareOutputSamplingRate(sptr<AudioDeviceDescriptor> &desc)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return 0;
}
} // namespace AudioStandard
} // namespace OHOS
