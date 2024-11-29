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

#ifndef AUDIO_CAPTURER_IMPL_H
#define AUDIO_CAPTURER_IMPL_H
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "audio_capturer_jni.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {

class AudioCapturerImpl {
public:
    AudioCapturerImpl();
    ~AudioCapturerImpl();

    void TransferState(CapturerState recordState);
    CapturerState GetStatus() const;

    int32_t CreateAudioRecord(const AudioCapturerOptions& capturerOptions);
    bool Start();
    bool Stop();
    bool Release();
    int32_t SetCapturerCallback(const std::shared_ptr<AudioCapturerCallback>& callback);
    int32_t SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback>& callback);
    int32_t GetBufferSize(size_t& bufferSize) const;
    int32_t GetBufferDesc(BufferDesc& bufDesc);
    int32_t Enqueue(const BufferDesc& bufDesc);
    int32_t GetCapturerInfo(AudioCapturerInfo& capturerInfo) const;
    int32_t SetCapturerPositionCallback(
        int64_t markPosition, const std::shared_ptr<CapturerPositionCallback>& callback);
    void UnsetCapturerPositionCallback();
    int32_t SetCapturerPeriodPositionCallback(
        int64_t frameNumber, const std::shared_ptr<CapturerPeriodPositionCallback>& callback);
    void UnsetCapturerPeriodPositionCallback();
    int32_t SetAudioCapturerDeviceChangeCallback(const std::shared_ptr<AudioCapturerDeviceChangeCallback>& callback);
    int32_t RemoveAudioCapturerDeviceChangeCallback(const std::shared_ptr<AudioCapturerDeviceChangeCallback>& callback);
    int32_t GetStreamInfo(AudioStreamInfo& streamInfo) const;
    int32_t GetAudioStreamId(uint32_t& sessionID) const;
    bool GetAudioTime(Timestamp& timestamp, Timestamp::Timestampbase base) const;
    int32_t GetCurrentInputDevices(AudioDeviceDescriptor& deviceInfo) const;
    int32_t GetCurrentCapturerChangeInfo(AudioCapturerChangeInfo& changeInfo) const;
    int32_t SetAudioCapturerInfoChangeCallback(const std::shared_ptr<AudioCapturerInfoChangeCallback>& callback);
    int32_t RemoveAudioCapturerInfoChangeCallback(const std::shared_ptr<AudioCapturerInfoChangeCallback>& callback);

private:
    void ReadCallbackFunc();

    CapturerState recordState_ = CAPTURER_INVALID;
    AudioStreamInfo streamInfo_;
    size_t minReadBufferSize_;
    std::shared_ptr<AudioCapturerReadCallback> readCallback_ = nullptr;
    std::shared_ptr<AudioCapturerCallback> stateCallback_ = nullptr;
    BufferDesc readBuffer_;
    std::mutex statusLock_;
    std::mutex readBufferLock_;
    std::mutex readThreadLock_;
    std::mutex readCbMutex_;
    std::condition_variable readThreadCv_;
    std::condition_variable cbBufferCv_;
    std::unique_ptr<uint8_t[]> cbBuffer_ { nullptr };
    std::thread callbackLoop_;
    std::atomic<bool> isReaded_ = false;
    std::atomic<bool> readThreadReleased_ = true;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_CAPTURER_IMPL_H