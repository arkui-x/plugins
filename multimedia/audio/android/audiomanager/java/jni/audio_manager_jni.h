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

#ifndef PLUGINS_MULTIMEDIA_AUDIO_ANDROID_AUDIOMANAGER_JAVA_JNI_AUDIO_MANAGER_JNI_H
#define PLUGINS_MULTIMEDIA_AUDIO_ANDROID_AUDIOMANAGER_JAVA_JNI_AUDIO_MANAGER_JNI_H

#include <jni.h>
#include <memory>

#include "audio_group_manager.h"
#include "audio_stream_manager.h"
#include "audio_system_manager.h"

namespace OHOS::Plugin {
using namespace OHOS::AudioStandard;
class AudioManagerJni final {
public:
    AudioManagerJni() = delete;
    ~AudioManagerJni() = delete;
    static bool Register(void* env);
    // Called by Java
    static void NativeInit(JNIEnv* env, jobject jobj);
    static void NativeOnAudioDeviceChanged(JNIEnv* env, jobject jobj, jboolean jIsAdded, jobjectArray jDeviceInfos);
    static AudioScene GetAudioScene();
    static int32_t GetVolume(AudioVolumeType volumeType);
    static int32_t GetMaxVolume(AudioVolumeType volumeType);
    static int32_t GetMinVolume(AudioVolumeType volumeType);
    static int32_t IsStreamMute(AudioVolumeType volumeType, bool& isMute);
    static AudioRingerMode GetRingerMode();
    static bool IsMicrophoneMute();
    static bool IsVolumeFixed();
    static float GetSystemVolumeInDb(AudioVolumeType volumeType, int32_t volumeLevel, DeviceType deviceType);
    static int32_t GetDevices(DeviceFlag deviceFlag, std::vector<std::shared_ptr<AudioDeviceDescriptor>>& desc);
    static int32_t AddDeviceChangeCallback(
        const DeviceFlag flag, const std::shared_ptr<AudioManagerDeviceChangeCallback>& callback);
    static int32_t RemoveDeviceChangeCallback();
    static void NativeOnAudioRendererChanged(JNIEnv* env, jobject jobj, jobject jRendererConf);
    static void NativeOnAudioCapturerChanged(JNIEnv* env, jobject jobj, jobject jCapturerConf);
    static std::vector<std::shared_ptr<AudioRendererChangeInfo>> GetAudioRendererChangeInfo(jobject jRendererConf);
    static std::vector<std::shared_ptr<AudioCapturerChangeInfo>> GetAudioCapturerChangeInfo(jobject jCapturerConf);
    static int32_t RegisterAudioRendererEventListener(
        const std::shared_ptr<AudioRendererStateChangeCallback>& callback);
    static int32_t UnregisterAudioRendererEventListener();
    static int32_t RegisterAudioCapturerEventListener(
        const std::shared_ptr<AudioCapturerStateChangeCallback>& callback);
    static int32_t UnregisterAudioCapturerEventListener();
    static int32_t GetCurrentRendererChangeInfos(
        std::vector<std::shared_ptr<AudioRendererChangeInfo>>& audioRendererChangeInfos);
    static int32_t GetCurrentCapturerChangeInfos(
        std::vector<std::shared_ptr<AudioCapturerChangeInfo>>& audioCapturerChangeInfos);
    static bool IsStreamActive(AudioVolumeType volumeType);
    static int32_t SetDeviceActive(DeviceType deviceType, bool flag);
    static bool IsDeviceActive(DeviceType deviceType);

private:
    static void ConvertDeviceInfoToAudioDeviceDescriptor(
        std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor, const AudioDeviceDescriptor& deviceInfo);
    static int32_t RegisterAudioDeviceCallback();
    static int32_t UnregisterAudioDeviceCallback();

    static bool hasInit_;
    static std::shared_ptr<AudioCapturerStateChangeCallback> audioCapturerEventCallback_;
    static std::shared_ptr<AudioRendererStateChangeCallback> audioRendererEventCallback_;
    static std::map<DeviceFlag, std::shared_ptr<AudioManagerDeviceChangeCallback>> deviceChangeCallbacks_;
};
} // namespace OHOS::Plugin
#endif // PLUGINS_MULTIMEDIA_AUDIO_ANDROID_AUDIOMANAGER_JAVA_JNI_AUDIO_MANAGER_JNI_H
