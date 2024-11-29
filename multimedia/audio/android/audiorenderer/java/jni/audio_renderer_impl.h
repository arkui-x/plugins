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

#ifndef AUDIO_RENDERER_IMPL_H
#define AUDIO_RENDERER_IMPL_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "audio_renderer_jni.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
class AudioRendererImpl {
public:
    AudioRendererImpl();
    ~AudioRendererImpl();

    int32_t CreateAudioTrack(const AudioRendererOptions& rendererOptions);
    void TransferState(RendererState playState);
    RendererState GetStatus() const;

    bool Start();
    bool Pause();
    bool Stop();
    bool Release();
    bool Flush();
    int32_t Write(uint8_t* buffer, size_t bufferSize);
    int32_t SetRenderMode(AudioRenderMode renderMode);
    int32_t SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback>& callback);
    int32_t GetBufferSize(size_t& bufferSize) const;
    int32_t GetBufferDesc(BufferDesc& bufDesc) const;
    int32_t Enqueue(const BufferDesc& bufDesc);
    int32_t SetSpeed(float speed);
    float GetSpeed();
    int32_t SetVolume(float volume);
    float GetMinStreamVolume();
    float GetMaxStreamVolume();
    uint32_t GetUnderflowCount();
    int32_t GetRendererInfo(AudioRendererInfo& rendererInfo) const;
    int32_t GetStreamInfo(AudioStreamInfo& streamInfo) const;
    int32_t GetCurrentOutputDevices(AudioDeviceDescriptor& deviceInfo) const;
    int32_t GetAudioStreamId(uint32_t& sessionID) const;
    bool GetAudioTime(Timestamp& timestamp, Timestamp::Timestampbase base) const;
    int32_t SetChannelBlendMode(ChannelBlendMode blendMode);

    int32_t SetRendererCallback(const std::shared_ptr<AudioRendererCallback>& callback);
    int32_t SetRendererPositionCallback(
        int64_t markPosition, const std::shared_ptr<RendererPositionCallback>& callback);
    void UnsetRendererPositionCallback();
    int32_t SetRendererPeriodPositionCallback(
        int64_t frameNumber, const std::shared_ptr<RendererPeriodPositionCallback>& callback);
    void UnsetRendererPeriodPositionCallback();
    int32_t RegisterRendererDeviceChangeListener(
        const std::shared_ptr<AudioRendererOutputDeviceChangeCallback>& callback);
    int32_t UnregisterRendererDeviceChangeListener();

private:
    void WriteCallbackFunc();
    std::thread callbackLoop_; // thread for callback to client and write.
    size_t minWriteBufferSize_;
    std::shared_ptr<AudioRendererWriteCallback> writeCallback_ = nullptr;
    std::atomic<bool> writeThreadReleased_ = true;
    std::shared_ptr<AudioRendererCallback> stateCallback_ = nullptr;

    std::unique_ptr<uint8_t[]> cbBuffer_ { nullptr };
    BufferDesc writeBuffer_;
    std::mutex writeBufferLock_;
    std::condition_variable writeThreadCv_;
    bool isWrited_ = false;
    std::mutex statusLock_;
    RendererState playState_ = RENDERER_INVALID;

    AudioStreamInfo formatInfo_;
    AudioEffectMode effectMode_ = EFFECT_NONE;
    AudioRenderMode renderMode_ = RENDER_MODE_NORMAL;
    std::mutex writeCbMutex_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_RENDERER_IMPL_H
