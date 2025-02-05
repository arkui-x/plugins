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

#ifndef PLUGINS_MULTIMEDIA_AUDIO_AUDIO_CAPTURER_ANDROID_JAVA_JNI_AUDIO_CAPTURER_JNI_H
#define PLUGINS_MULTIMEDIA_AUDIO_AUDIO_CAPTURER_ANDROID_JAVA_JNI_AUDIO_CAPTURER_JNI_H

#include <jni.h>
#include <list>
#include <map>
#include <memory>

#include "audio_capturer.h"

namespace OHOS::Plugin {
class AudioCapturerJni final {
public:
    AudioCapturerJni() = delete;
    ~AudioCapturerJni() = delete;
    static bool Register(void* env);
    // Called by Java
    static void NativeInit(JNIEnv* env, jobject jobj);
    static void NativeOnMarkerReached(JNIEnv* env, jobject jobj, jlong capturerPtr, jint position);
    static void NativeOnPeriodicNotification(JNIEnv* env, jobject jobj, jlong capturerPtr, jint period);
    static void NativeOnRoutingChanged(JNIEnv* env, jobject jobj, jlong capturerPtr, jobject deviceInfo);
    static void NativeOnInfoChanged(JNIEnv* env, jobject jobj, jlong capturerPtr, jobject deviceInfo);

    // Called by C++
    static int32_t CreateAudioRecord(long capturerPtr, const AudioStandard::AudioCapturerOptions& capturerOptions);
    static bool Start(long capturerPtr);
    static bool Stop(long capturerPtr);
    static bool Release(long capturerPtr);
    static int32_t GetBufferSize(
        long capturerPtr, const AudioStandard::AudioStreamInfo& formatInfo, size_t& bufferSize);
    static int32_t Read(long capturerPtr, uint8_t* buffer, size_t bufferSize);
    static int32_t GetCapturerInfo(long capturerPtr, AudioStandard::AudioCapturerInfo& capturerInfo);
    static int32_t SetNotificationMarkerPosition(long capturerPtr, int64_t markPosition);
    static int32_t SetCapturerPositionUpdateListener(
        long capturerPtr, const std::shared_ptr<AudioStandard::CapturerPositionCallback>& callback);
    static void UnsetCapturerPositionUpdateListener(long capturerPtr);
    static int32_t SetPositionNotificationPeriod(long capturerPtr, int64_t frameNumber);
    static int32_t SetPeriodPositionUpdateListener(
        long capturerPtr, const std::shared_ptr<AudioStandard::CapturerPeriodPositionCallback>& callback);
    static void UnsetPeriodPositionUpdateListener(long capturerPtr);
    static int32_t RegisterCapturerDeviceChangeListener(
        long capturerPtr, const std::shared_ptr<AudioStandard::AudioCapturerDeviceChangeCallback>& callback);
    static int32_t RemoveAudioCapturerDeviceChangeListener(
        long capturerPtr, const std::shared_ptr<AudioStandard::AudioCapturerDeviceChangeCallback>& callback);
    static int32_t GetStreamInfo(long capturerPtr, AudioStandard::AudioStreamInfo& streamInfo);
    static int32_t GetAudioSessionId(long capturerPtr, uint32_t& sessionID);
    static bool GetTimestamp(long capturerPtr, AudioStandard::Timestamp& timestamp);
    static int32_t GetCurrentInputDevices(long capturerPtr, AudioStandard::AudioDeviceDescriptor& deviceInfo);
    static void SetCapturerState(long capturerPtr, const AudioStandard::CapturerState capturerState);
    static int32_t SetAudioCapturerInfoChangeCallback(
        long capturerPtr, const std::shared_ptr<AudioStandard::AudioCapturerInfoChangeCallback>& callback);
    static int32_t RemoveAudioCapturerInfoChangeCallback(
        long capturerPtr, const std::shared_ptr<AudioStandard::AudioCapturerInfoChangeCallback>& callback);
    static void NotifyCapturerInfoChanged(long capturerPtr, AudioStandard::AudioCapturerChangeInfo changeInfo);
    static void Finalize(long capturerPtr);

private:
    static bool hasInit_;
    static std::map<long, std::shared_ptr<AudioStandard::CapturerPositionCallback>> capturerPositionObserver_;
    static std::map<long, std::shared_ptr<AudioStandard::CapturerPeriodPositionCallback>> periodPositionObserver_;
    static std::map<long, std::list<std::shared_ptr<AudioStandard::AudioCapturerDeviceChangeCallback>>>
        deviceChangeObserver_;
    static std::map<long, std::list<std::shared_ptr<AudioStandard::AudioCapturerInfoChangeCallback>>>
        capturerInfoChangeObserver_;
    static std::map<long, AudioStandard::CapturerState> capturerStateMap_;
    static void UnsetRecordPositionUpdateListener(long capturerPtr);
};
} // namespace OHOS::Plugin
#endif // PLUGINS_MULTIMEDIA_AUDIO_AUDIO_CAPTURER_ANDROID_JAVA_JNI_AUDIO_CAPTURER_JNI_H