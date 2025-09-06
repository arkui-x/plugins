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

#include "audio_manager_impl.h"
#include "audio_routing_manager.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
AudioRoutingManager *AudioRoutingManager::GetInstance()
{
    static AudioRoutingManager audioRoutingManager;
    return &audioRoutingManager;
}

int32_t AudioRoutingManager::GetPreferredOutputDeviceForRendererInfo(AudioRendererInfo rendererInfo,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, ERROR, "managerImpl == nullptr.");
    return [managerImpl getPreferredOutputDeviceForRendererInfo:desc];
}

int32_t AudioRoutingManager::GetPreferredInputDeviceForCapturerInfo(AudioCapturerInfo captureInfo,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc)
{
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, ERROR, "managerImpl == nullptr.");
    return [managerImpl getPreferredInputDeviceForCapturerInfo:desc];
}

int32_t AudioRoutingManager::SetPreferredOutputDeviceChangeCallback(AudioRendererInfo rendererInfo,
    const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback>& callback, const int32_t uid)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, ERROR, "managerImpl == nullptr.");
    return [managerImpl setPreferredOutputDeviceChangeCallback:callback];
}

int32_t AudioRoutingManager::SetPreferredInputDeviceChangeCallback(AudioCapturerInfo captureInfo,
    const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, ERROR, "managerImpl == nullptr.");
    return [managerImpl setPreferredInputDeviceChangeCallback:callback];
}

int32_t AudioRoutingManager::UnsetPreferredOutputDeviceChangeCallback(
    const std::shared_ptr<AudioPreferredOutputDeviceChangeCallback> &callback)
{
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, ERROR, "managerImpl == nullptr.");
    return [managerImpl unsetPreferredOutputDeviceChangeCallback];
}

int32_t AudioRoutingManager::UnsetPreferredInputDeviceChangeCallback(
    const std::shared_ptr<AudioPreferredInputDeviceChangeCallback> &callback)
{
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    CHECK_AND_RETURN_RET_LOG(managerImpl != nullptr, ERROR, "managerImpl == nullptr.");
    return [managerImpl unsetPreferredInputDeviceChangeCallback];
}

vector<sptr<MicrophoneDescriptor>> AudioRoutingManager::GetAvailableMicrophones()
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    vector<sptr<MicrophoneDescriptor>> micDescs;
    return micDescs;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioRoutingManager::GetAvailableDevices(AudioDeviceUsage usage)
{
    AUDIO_WARNING_LOG("%{public}s is not supported", __func__);
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> availableDescs;
    return availableDescs;
}
} // namespace AudioStandard
} // namespace OHOS
