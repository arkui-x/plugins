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

#ifndef PLUGINS_MULTIMEDIA_AUDIO_AUDIO_RENDERER_ANDROID_JAVA_JNI_AUDIO_RENDERER_JNI_H
#define PLUGINS_MULTIMEDIA_AUDIO_AUDIO_RENDERER_ANDROID_JAVA_JNI_AUDIO_RENDERER_JNI_H

#include <jni.h>
#include <map>
#include <memory>

#include "audio_renderer.h"

namespace OHOS::Plugin {
using namespace OHOS::AudioStandard;

class AudioRendererJni final {
public:
    AudioRendererJni() = delete;
    ~AudioRendererJni() = delete;
    static bool Register(void* env);
    // Called by Java
    static void NativeInit(JNIEnv* env, jobject jobj);
    static void NativeOnMarkerReached(JNIEnv* env, jobject jobj, jlong rendererPtr, jint position);
    static void NativeOnPeriodicNotification(JNIEnv* env, jobject jobj, jlong rendererPtr, jint period);
    static void NativeOnRoutingChanged(JNIEnv* env, jobject jobj, jlong rendererPtr, jobject deviceInfo);
    static int32_t CreateAudioTrack(long rendererPtr, const AudioRendererOptions& rendererOptions, float maxSpeed);
    static bool Start(long rendererPtr);
    static bool Pause(long rendererPtr);
    static bool Stop(long rendererPtr);
    static bool Release(long rendererPtr);
    static bool Flush(long rendererPtr);
    static int32_t GetBufferSize(long rendererPtr, const AudioStreamInfo& formatInfo, size_t& bufferSize);
    static int32_t Write(long rendererPtr, uint8_t* buffer, size_t bufferSize);
    static int32_t SetSpeed(long rendererPtr, float speed);
    static float GetSpeed(long rendererPtr);
    static int32_t SetVolume(long rendererPtr, float volume);
    static float GetMinVolume(long rendererPtr);
    static float GetMaxVolume(long rendererPtr);
    static uint32_t GetUnderrunCount(long rendererPtr);
    static int32_t GetCurrentOutputDevices(long rendererPtr, AudioDeviceDescriptor& deviceInfo);
    static int32_t GetAudioSessionId(long rendererPtr, uint32_t& sessionID);
    static int32_t GetRendererInfo(long rendererPtr, AudioRendererInfo& rendererInfo);
    static int32_t GetStreamInfo(long rendererPtr, AudioStreamInfo& streamInfo);
    static bool GetTimestamp(long rendererPtr, Timestamp& timestamp);
    static int32_t SetDualMonoMode(long rendererPtr, ChannelBlendMode blendMode);
    static int32_t SetNotificationMarkerPosition(long rendererPtr, int64_t markPosition);
    static int32_t SetRendererPositionUpdateListener(
        long rendererPtr, const std::shared_ptr<RendererPositionCallback>& callback);
    static void UnsetRendererPositionUpdateListener(long rendererPtr);
    static int32_t SetPositionNotificationPeriod(long rendererPtr, int64_t frameNumber);
    static int32_t SetPeriodPositionUpdateListener(
        long rendererPtr, const std::shared_ptr<RendererPeriodPositionCallback>& callback);
    static void UnsetPeriodPositionUpdateListener(long rendererPtr);
    static int32_t RegisterRendererDeviceChangeListener(
        long rendererPtr, const std::shared_ptr<AudioRendererOutputDeviceChangeCallback>& callback);
    static int32_t UnregisterRendererDeviceChangeListener(long rendererPtr);
    static void Finalize(long rendererPtr);

private:
    static void UnregisterRendererPositionCallback(long rendererPtr);
    static void UnregisterPeriodPositionCallback(long rendererPtr);
    static void UnregisterDeviceChangeCallback(long rendererPtr);
    static void UnsetPlaybackPositionUpdateListener(long rendererPtr);
    static std::map<long, std::shared_ptr<RendererPositionCallback>> rendererPositionObserver_;
    static std::map<long, std::shared_ptr<RendererPeriodPositionCallback>> periodPositionObserver_;
    static std::map<long, std::shared_ptr<AudioRendererOutputDeviceChangeCallback>> deviceChangeObserver_;
    static bool hasInit_;
};
} // namespace OHOS::Plugin
#endif // PLUGINS_MULTIMEDIA_AUDIO_AUDIO_RENDERER_ANDROID_JAVA_JNI_AUDIO_RENDERER_JNI_H
