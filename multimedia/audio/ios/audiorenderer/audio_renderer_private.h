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

#ifndef AUDIO_RENDERER_PRIVATE_H
#define AUDIO_RENDERER_PRIVATE_H

#include "audio_errors.h"
#include "audio_interrupt_callback.h"
#include "audio_renderer.h"
#include "audio_utils.h"
#include "audio_renderer_impl.h"

namespace OHOS {
namespace AudioStandard {
constexpr uint32_t INVALID_SESSION_ID = static_cast<uint32_t>(-1);
class AudioRendererStateChangeCallbackImpl;
class RendererPolicyServiceDiedCallback;
class OutputDeviceChangeWithInfoCallbackImpl;

class AudioRendererPrivate : public AudioRenderer {
public:
    AudioRendererPrivate() = default;
    ~AudioRendererPrivate();
    int32_t CheckParams(const AudioRendererOptions &rendererOptions);
    void CreateAudioTrack(const AudioRendererOptions &rendererOptions);
    int32_t GetFrameCount(uint32_t &frameCount) const override;
    int32_t GetLatency(uint64_t &latency) const override;
    void SetAudioPrivacyType(AudioPrivacyType privacyType) override;
    AudioPrivacyType GetAudioPrivacyType() override { return PRIVACY_TYPE_PRIVATE; };
    int32_t SetParams(const AudioRendererParams params) override;
    int32_t GetParams(AudioRendererParams &params) const override;
    int32_t GetRendererInfo(AudioRendererInfo &rendererInfo) const override;
    int32_t GetStreamInfo(AudioStreamInfo &streamInfo) const override;
    bool Start(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    int32_t Write(uint8_t *buffer, size_t bufferSize) override;
    int32_t Write(uint8_t *pcmBuffer, size_t pcmSize, uint8_t *metaBuffer, size_t metaSize) override;
    RendererState GetStatus() const override;
    bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const override;
    bool GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base) override;
    bool Drain() const override;
    bool PauseTransitent(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    bool Pause(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    bool Stop() override;
    bool Flush() const override;
    bool Release() override;
    int32_t GetBufferSize(size_t &bufferSize) const override;
    int32_t GetAudioStreamId(uint32_t &sessionID) const override;
    int32_t SetAudioRendererDesc(AudioRendererDesc audioRendererDesc) override;
    int32_t SetStreamType(AudioStreamType audioStreamType) override;
    int32_t SetVolume(float volume) const override;
    float GetVolume() const override;
    int32_t SetRenderRate(AudioRendererRate renderRate) const override;
    AudioRendererRate GetRenderRate() const override;
    int32_t SetRendererSamplingRate(uint32_t sampleRate) const override;
    uint32_t GetRendererSamplingRate() const override;
    int32_t SetRendererCallback(const std::shared_ptr<AudioRendererCallback> &callback) override;
    int32_t SetRendererPositionCallback(int64_t markPosition,
        const std::shared_ptr<RendererPositionCallback> &callback) override;
    void UnsetRendererPositionCallback() override;
    int32_t SetRendererPeriodPositionCallback(int64_t frameNumber,
        const std::shared_ptr<RendererPeriodPositionCallback> &callback) override;
    void UnsetRendererPeriodPositionCallback() override;
    int32_t SetBufferDuration(uint64_t bufferDuration) const override;
    int32_t SetRenderMode(AudioRenderMode renderMode) override;
    AudioRenderMode GetRenderMode() const override;
    int32_t SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback) override;
    int32_t SetRendererFirstFrameWritingCallback(
        const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback) override;
    void SetPreferredFrameSize(int32_t frameSize) override;
    int32_t GetBufferDesc(BufferDesc &bufDesc) override;
    int32_t Enqueue(const BufferDesc &bufDesc) override;
    int32_t Clear() const override;
    int32_t GetBufQueueState(BufferQueueState &bufState) const override;
    void SetInterruptMode(InterruptMode mode) override;
    int32_t SetParallelPlayFlag(bool parallelPlayFlag) override;
    int32_t SetLowPowerVolume(float volume) const override;
    float GetLowPowerVolume() const override;
    int32_t SetOffloadAllowed(bool isAllowed) override;
    int32_t SetOffloadMode(int32_t state, bool isAppBack) const override;
    int32_t UnsetOffloadMode() const override;
    float GetSingleStreamVolume() const override;
    float GetMinStreamVolume() const override;
    float GetMaxStreamVolume() const override;
    int32_t GetCurrentOutputDevices(AudioDeviceDescriptor &deviceInfo) const override;
    uint32_t GetUnderflowCount() const override;
    int32_t RegisterOutputDeviceChangeWithInfoCallback(
        const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback) override;
    int32_t UnregisterOutputDeviceChangeWithInfoCallback() override;
    int32_t UnregisterOutputDeviceChangeWithInfoCallback(
        const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback) override;
    int32_t RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
        const std::shared_ptr<AudioRendererPolicyServiceDiedCallback> &callback) override;
    int32_t UnregisterAudioPolicyServerDiedCb(const int32_t clientPid) override;
    AudioEffectMode GetAudioEffectMode() const override;
    int64_t GetFramesWritten() const override;
    int32_t SetAudioEffectMode(AudioEffectMode effectMode) const override;
    int32_t SetChannelBlendMode(ChannelBlendMode blendMode) override;
    void SetAudioRendererErrorCallback(std::shared_ptr<AudioRendererErrorCallback> errorCallback) override;
    int32_t SetVolumeWithRamp(float volume, int32_t duration) override;

    int32_t RegisterRendererPolicyServiceDiedCallback();
    int32_t RemoveRendererPolicyServiceDiedCallback() const;

    int32_t SetSpeed(float speed) override;
    float GetSpeed() override;
    bool IsFastRenderer() override;

    void SetSilentModeAndMixWithOthers(bool on) override {};
    bool GetSilentModeAndMixWithOthers() override { return false; };

    void EnableVoiceModemCommunicationStartStream(bool enable) override {}

    bool IsNoStreamRenderer() const override { return false; }

    int32_t SetDefaultOutputDevice(DeviceType deviceType) override{ return ERR_NOT_SUPPORTED; }

    int32_t GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const override;

    void SetFastStatusChangeCallback(const std::shared_ptr<AudioRendererFastStatusChangeCallback> &callback) override;
    
private:
    bool IsFormatValid(uint8_t format);
    bool IsEncodingTypeValid(uint8_t encodingType);
    bool IsSamplingRateValid(uint32_t samplingRate);
    bool IsRendererChannelValid(uint8_t channel);
    bool IsRendererChannelLayoutValid(uint64_t channelLayout);
    bool IsPlaybackChannelRelatedInfoValid(uint8_t channels, uint64_t channelLayout);

    AudioRendererImpl *rendererImpl_;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_RENDERER_PRIVATE_H
