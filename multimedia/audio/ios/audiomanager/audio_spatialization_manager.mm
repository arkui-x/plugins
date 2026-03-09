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

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_spatialization_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

AudioSpatializationManager::AudioSpatializationManager()
{
    AUDIO_DEBUG_LOG("AudioSpatializationManager start");
}

AudioSpatializationManager::~AudioSpatializationManager()
{
    AUDIO_DEBUG_LOG("AudioSpatializationManager::~AudioSpatializationManager");
}

AudioSpatializationManager *AudioSpatializationManager::GetInstance()
{
    static AudioSpatializationManager audioSpatializationManager;
    return &audioSpatializationManager;
}

bool AudioSpatializationManager::IsSpatializationEnabled()
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return false;
}

bool AudioSpatializationManager::IsSpatializationEnabled(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return false;
}

bool AudioSpatializationManager::IsSpatializationEnabledForCurrentDevice()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return false;
}

int32_t AudioSpatializationManager::SetSpatializationEnabled(const bool enable)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSpatializationManager::SetSpatializationEnabled(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, const bool enable)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

bool AudioSpatializationManager::IsHeadTrackingEnabled()
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return false;
}

bool AudioSpatializationManager::IsHeadTrackingEnabled(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return false;
}

int32_t AudioSpatializationManager::SetHeadTrackingEnabled(const bool enable)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSpatializationManager::SetHeadTrackingEnabled(
    const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, const bool enable)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSpatializationManager::RegisterSpatializationEnabledEventListener(
    const std::shared_ptr<AudioSpatializationEnabledChangeCallback> &callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSpatializationManager::RegisterSpatializationEnabledForCurrentDeviceEventListener(
    const std::shared_ptr<AudioSpatializationEnabledChangeForCurrentDeviceCallback> &callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSpatializationManager::RegisterHeadTrackingEnabledEventListener(
    const std::shared_ptr<AudioHeadTrackingEnabledChangeCallback> &callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSpatializationManager::UnregisterSpatializationEnabledEventListener()
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSpatializationManager::UnregisterSpatializationEnabledForCurrentDeviceEventListener()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioSpatializationManager::UnregisterHeadTrackingEnabledEventListener()
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

bool AudioSpatializationManager::IsSpatializationSupported()
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return false;
}

bool AudioSpatializationManager::IsSpatializationSupportedForDevice(const std::shared_ptr<AudioDeviceDescriptor>
    &selectedAudioDevice)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return false;
}

bool AudioSpatializationManager::IsHeadTrackingSupported()
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return false;
}

bool AudioSpatializationManager::IsHeadTrackingSupportedForDevice(const std::shared_ptr<AudioDeviceDescriptor>
    &selectedAudioDevice)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return false;
}

int32_t AudioSpatializationManager::UpdateSpatialDeviceState(const AudioSpatialDeviceState audioSpatialDeviceState)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}

AudioSpatializationSceneType AudioSpatializationManager::GetSpatializationSceneType()
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return SPATIALIZATION_SCENE_TYPE_DEFAULT;
}

int32_t AudioSpatializationManager::SetSpatializationSceneType(
    const AudioSpatializationSceneType spatializationSceneType)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    return ERR_NOT_SUPPORTED;
}
} // namespace AudioStandard
} // namespace OHOS
