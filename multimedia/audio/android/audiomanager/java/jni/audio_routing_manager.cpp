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

#include "audio_routing_manager.h"

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_manager_jni.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
AudioRoutingManager* AudioRoutingManager::GetInstance()
{
    static AudioRoutingManager audioRoutingManager;
    return &audioRoutingManager;
}

int32_t AudioRoutingManager::GetCallingPid()
{
    return getpid();
}

int32_t AudioRoutingManager::SetMicStateChangeCallback(
    const std::shared_ptr<AudioManagerMicStateChangeCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRoutingManager::GetPreferredOutputDeviceForRendererInfo(
    AudioRendererInfo rendererInfo, std::vector<std::shared_ptr<AudioDeviceDescriptor>>& desc)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRoutingManager::GetPreferredInputDeviceForCapturerInfo(
    AudioCapturerInfo captureInfo, std::vector<std::shared_ptr<AudioDeviceDescriptor>>& desc)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRoutingManager::SetPreferredOutputDeviceChangeCallback(AudioRendererInfo rendererInfo,
    const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback>& callback, const int32_t uid)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRoutingManager::SetPreferredInputDeviceChangeCallback(
    AudioCapturerInfo captureInfo, const std::shared_ptr<AudioPreferredInputDeviceChangeCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRoutingManager::UnsetPreferredOutputDeviceChangeCallback(
    const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRoutingManager::UnsetPreferredInputDeviceChangeCallback(
    const std::shared_ptr<AudioPreferredInputDeviceChangeCallback>& callback)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return ERR_NOT_SUPPORTED;
}

vector<sptr<MicrophoneDescriptor>> AudioRoutingManager::GetAvailableMicrophones()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    vector<sptr<MicrophoneDescriptor>> microphones;
    return microphones;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioRoutingManager::GetAvailableDevices(AudioDeviceUsage usage)
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> availableDevices;
    return availableDevices;
}

std::shared_ptr<AudioDeviceDescriptor> AudioRoutingManager::GetActiveBluetoothDevice()
{
    AUDIO_WARNING_LOG("%{public}s is not supported.", __func__);
    return nullptr;
}
} // namespace AudioStandard
} // namespace OHOS
