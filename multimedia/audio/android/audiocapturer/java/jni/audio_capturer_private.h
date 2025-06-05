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

#ifndef AUDIO_CAPTURER_PRIVATE_H
#define AUDIO_CAPTURER_PRIVATE_H

#include <mutex>

#include "audio_capturer_impl.h"
#include "audio_capturer_jni.h"
#include "audio_errors.h"
#include "audio_interrupt_callback.h"

namespace OHOS {
namespace AudioStandard {
constexpr uint32_t INVALID_SESSION_ID = static_cast<uint32_t>(-1);
class AudioCapturerStateChangeCallbackImpl;
class CapturerPolicyServiceDiedCallback;

class AudioCapturerPrivate : public AudioCapturer {
public:
    AudioCapturerPrivate() = default;
    ~AudioCapturerPrivate();

    int32_t CreateAudioRecord(const AudioCapturerOptions& capturerOptions);
    int32_t CheckParams(const AudioCapturerOptions& capturerOptions);
    int32_t GetFrameCount(uint32_t& frameCount) const override { return 0; }
    int32_t SetParams(const AudioCapturerParams params) override { return 0; }
    int32_t UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config) override;
    int32_t SetCapturerCallback(const std::shared_ptr<AudioCapturerCallback>& callback) override;
    int32_t GetParams(AudioCapturerParams& params) const override { return 0; }
    int32_t GetCapturerInfo(AudioCapturerInfo& capturerInfo) const override;
    int32_t GetStreamInfo(AudioStreamInfo& streamInfo) const override;
    bool Start() override;
    int32_t Read(uint8_t& buffer, size_t userSize, bool isBlockingRead) override;
    CapturerState GetStatus() const override;
    bool GetTimeStampInfo(Timestamp &timestampNs, Timestamp::Timestampbase base) const override;
    bool GetAudioTime(Timestamp& timestamp, Timestamp::Timestampbase base) const override;
    bool GetFirstPkgTimeStampInfo(int64_t &firstTs) const override;
    bool Pause() const override;
    bool Stop() const override;
    bool Flush() const override;
    bool Release() override;
    int32_t GetBufferSize(size_t& bufferSize) const override;
    int32_t GetAudioStreamId(uint32_t& sessionID) const override;
    int32_t SetCapturerPositionCallback(
        int64_t markPosition, const std::shared_ptr<CapturerPositionCallback>& callback) override;
    void UnsetCapturerPositionCallback() override;
    int32_t SetCapturerPeriodPositionCallback(
        int64_t frameNumber, const std::shared_ptr<CapturerPeriodPositionCallback>& callback) override;
    void UnsetCapturerPeriodPositionCallback() override;
    int32_t SetBufferDuration(uint64_t bufferDuration) const override { return 0; }
    int32_t SetCaptureMode(AudioCaptureMode captureMode) override { return 0; }
    AudioCaptureMode GetCaptureMode() const override { return CAPTURE_MODE_NORMAL; }
    int32_t SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback>& callback) override;
    int32_t GetBufferDesc(BufferDesc& bufDesc) override;
    int32_t Enqueue(const BufferDesc& bufDesc) override;
    int32_t Clear() const override { return 0; }
    int32_t GetBufQueueState(BufferQueueState& bufState) const override { return 0; }
    void SetValid(bool valid) override {}
    int64_t GetFramesRead() const override { return 0; }
    int32_t GetCurrentInputDevices(AudioDeviceDescriptor& deviceInfo) const override;
    int32_t GetCurrentCapturerChangeInfo(AudioCapturerChangeInfo& changeInfo) const override;
    int32_t SetAudioCapturerDeviceChangeCallback(
        const std::shared_ptr<AudioCapturerDeviceChangeCallback>& callback) override;
    int32_t RemoveAudioCapturerDeviceChangeCallback(
        const std::shared_ptr<AudioCapturerDeviceChangeCallback>& callback) override;
    int32_t SetAudioCapturerInfoChangeCallback(
        const std::shared_ptr<AudioCapturerInfoChangeCallback>& callback) override;
    int32_t RemoveAudioCapturerInfoChangeCallback(
        const std::shared_ptr<AudioCapturerInfoChangeCallback>& callback) override;
    int32_t RegisterAudioCapturerEventListener() override { return SUCCESS; }
    int32_t UnregisterAudioCapturerEventListener() override { return SUCCESS; }
    void SetAudioCapturerErrorCallback(std::shared_ptr<AudioCapturerErrorCallback> errorCallback) override;
    int32_t RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
        const std::shared_ptr<AudioCapturerPolicyServiceDiedCallback> &callback) override;
    int32_t GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const override;
    std::vector<sptr<MicrophoneDescriptor>> GetCurrentMicrophones() const override
    { 
        std::vector<sptr<MicrophoneDescriptor>> emptyVector;
        return emptyVector;
    }
    uint32_t GetOverflowCount() const override { return 0; }
    int32_t SetInputDevice(DeviceType deviceType) const override;
    int32_t SetAudioSourceConcurrency(const std::vector<SourceType> &targetSources) override { return 0; }
    int32_t SetInterruptStrategy(InterruptStrategy strategy) override;
    void SetFastStatusChangeCallback(const std::shared_ptr<AudioCapturerFastStatusChangeCallback> &callback) override;

private:
    std::shared_ptr<AudioCapturerImpl> capturerImpl_ = nullptr;

    bool IsFormatValid(uint8_t format);
    bool IsEncodingTypeValid(uint8_t encodingType);
    bool IsSamplingRateValid(uint32_t samplingRate);
    bool IsCapturerChannelValid(uint8_t channel);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_CAPTURER_PRIVATE_H
