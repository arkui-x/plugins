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
#include "audio_renderer_impl.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
class AudioRendererPrivate : public AudioRenderer {
public:
    AudioRendererPrivate() = default;
    virtual ~AudioRendererPrivate();

    int32_t CheckParams(const AudioRendererOptions &rendererOptions);
    int32_t CreateAudioTrack(const AudioRendererOptions &rendererOptions);
    int32_t GetRendererInfo(AudioRendererInfo &rendererInfo) const override;
    int32_t GetStreamInfo(AudioStreamInfo &streamInfo) const override;
    bool Start(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    bool Pause(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override;
    bool Stop() override;
    bool Flush() const override;
    bool Release() override;
    int32_t Write(uint8_t *buffer, size_t bufferSize) override;
    RendererState GetStatus() const override;
    int32_t SetVolume(float volume) const override;
    int32_t SetSpeed(float speed) override;
    float GetSpeed() override;
    bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const override;
    int32_t GetBufferSize(size_t &bufferSize) const override;
    int32_t GetAudioStreamId(uint32_t &sessionID) const override;
    int32_t GetBufferDesc(BufferDesc &bufDesc) override;
    int32_t Enqueue(const BufferDesc &bufDesc) override;
    float GetMinStreamVolume() const override;
    float GetMaxStreamVolume() const override;
    int32_t GetCurrentOutputDevices(AudioDeviceDescriptor &deviceInfo) const override;
    uint32_t GetUnderflowCount() const override;
    int32_t SetRendererCallback(const std::shared_ptr<AudioRendererCallback> &callback) override;
    int32_t SetRendererPositionCallback(int64_t markPosition,
        const std::shared_ptr<RendererPositionCallback> &callback) override;
    void UnsetRendererPositionCallback() override;
    int32_t SetRendererPeriodPositionCallback(int64_t frameNumber,
        const std::shared_ptr<RendererPeriodPositionCallback> &callback) override;
    void UnsetRendererPeriodPositionCallback() override;
    int32_t SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback) override;
    int32_t SetChannelBlendMode(ChannelBlendMode blendMode) override;

    int32_t SetAudioEffectMode(AudioEffectMode effectMode) const override { return ERR_NOT_SUPPORTED; }
    AudioEffectMode GetAudioEffectMode() const override { return EFFECT_NONE; }
    void SetInterruptMode(InterruptMode mode) override {};
    int32_t SetVolumeWithRamp(float volume, int32_t duration) override { return ERR_NOT_SUPPORTED; }
    int32_t GetFrameCount(uint32_t &frameCount) const override { return 0; }
    int32_t GetLatency(uint64_t &latency) const override { return ERR_NOT_SUPPORTED; }
    void SetAudioPrivacyType(AudioPrivacyType privacyType) override {};
    AudioPrivacyType GetAudioPrivacyType() override { return PRIVACY_TYPE_PRIVATE; };
    int32_t SetParams(const AudioRendererParams params) override { return ERR_NOT_SUPPORTED; }
    int32_t GetParams(AudioRendererParams &params) const override { return ERR_NOT_SUPPORTED; }
    int32_t Write(uint8_t *pcmBuffer, size_t pcmSize, uint8_t *metaBuffer, size_t metaSize) override { return 0; }
    bool Drain() const override { return false; }
    bool PauseTransitent(StateChangeCmdType cmdType = CMD_FROM_CLIENT) override { return false; };
    int32_t SetAudioRendererDesc(AudioRendererDesc audioRendererDesc) override { return ERR_NOT_SUPPORTED; }
    int32_t SetStreamType(AudioStreamType audioStreamType) override { return ERR_NOT_SUPPORTED; }
    float GetVolume() const override { return 0.0f; }
    int32_t SetRenderRate(AudioRendererRate renderRate) const override { return ERR_NOT_SUPPORTED; }
    AudioRendererRate GetRenderRate() const override { return RENDER_RATE_NORMAL; }
    int32_t SetRendererSamplingRate(uint32_t sampleRate) const override { return ERR_NOT_SUPPORTED; }
    uint32_t GetRendererSamplingRate() const override { return 0; }
    int32_t SetBufferDuration(uint64_t bufferDuration) const override { return ERR_NOT_SUPPORTED; }
    int32_t SetRenderMode(AudioRenderMode renderMode) override;
    AudioRenderMode GetRenderMode() const override { return RENDER_MODE_NORMAL; }
    int32_t SetRendererFirstFrameWritingCallback(
        const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback) override { return ERR_NOT_SUPPORTED; }
    void SetPreferredFrameSize(int32_t frameSize) override {}
    int32_t Clear() const override  { return ERR_NOT_SUPPORTED; }
    int32_t GetBufQueueState(BufferQueueState &bufState) const override { return ERR_NOT_SUPPORTED; }
    int32_t SetParallelPlayFlag(bool parallelPlayFlag) override { return ERR_NOT_SUPPORTED; }
    int32_t SetLowPowerVolume(float volume) const override { return ERR_NOT_SUPPORTED; }
    float GetLowPowerVolume() const override { return 0.0f; }
    int32_t SetOffloadAllowed(bool isAllowed) override { return ERR_NOT_SUPPORTED; }
    int32_t SetOffloadMode(int32_t state, bool isAppBack) const override { return ERR_NOT_SUPPORTED; }
    int32_t UnsetOffloadMode() const override { return ERR_NOT_SUPPORTED; }
    float GetSingleStreamVolume() const override { return 0.0f; }
    int32_t RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
        const std::shared_ptr<AudioRendererPolicyServiceDiedCallback> &callback) override { return 0; };
    int32_t RegisterOutputDeviceChangeWithInfoCallback(
        const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback) override;
    int32_t UnregisterOutputDeviceChangeWithInfoCallback() override;
    int32_t UnregisterOutputDeviceChangeWithInfoCallback(
        const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback) override;
    int32_t UnregisterAudioPolicyServerDiedCb(const int32_t clientPid) override { return 0; };
    int64_t GetFramesWritten() const override { return 0; }
    void SetAudioRendererErrorCallback(std::shared_ptr<AudioRendererErrorCallback> errorCallback) override {}
    bool GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base) override { return false; }
    bool IsFastRenderer() override { return false; }

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

    std::shared_ptr<AudioRendererImpl> rendererImpl_ = nullptr;
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_RENDERER_PRIVATE_H
