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

#include "audio_renderer_private.h"
#include "audio_log.h"
#import "audio_manager_impl.h"

namespace OHOS {
namespace AudioStandard {

AudioRenderer::~AudioRenderer() = default;
AudioRendererPrivate::~AudioRendererPrivate()
{
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    if (managerImpl) {
        [managerImpl removeRenderer:rendererImpl_];
    }
}

int32_t AudioRenderer::CheckMaxRendererInstances()
{
    return SUCCESS;
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(AudioStreamType audioStreamType)
{
    AppInfo appInfo = {};
    return Create(audioStreamType, appInfo);
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(AudioStreamType audioStreamType, const AppInfo &appInfo)
{
    return nullptr;
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(const AudioRendererOptions &rendererOptions)
{
    AppInfo appInfo = {};
    return Create("", rendererOptions, appInfo);
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(const AudioRendererOptions &rendererOptions,
    const AppInfo &appInfo)
{
    return Create("", rendererOptions, appInfo);
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(const std::string cachePath,
    const AudioRendererOptions &rendererOptions)
{
    AppInfo appInfo = {};
    return Create(cachePath, rendererOptions, appInfo);
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(const std::string cachePath,
    const AudioRendererOptions &rendererOptions, const AppInfo &appInfo)
{
    auto audioRenderer = std::make_unique<AudioRendererPrivate>();
    if (audioRenderer->CheckParams(rendererOptions) != SUCCESS) {
        AUDIO_ERR_LOG("CheckParams failed in renderer");
        audioRenderer = nullptr;
    } else {
        audioRenderer->CreateAudioTrack(rendererOptions);
    }
    return audioRenderer;
}

void AudioRendererPrivate::CreateAudioTrack(const AudioRendererOptions &rendererOptions)
{
    rendererImpl_ = [[AudioRendererImpl alloc] init];
    [rendererImpl_ initWithSampleRate: rendererOptions];

    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    if (managerImpl) {
        [managerImpl addRenderer:rendererImpl_];
    }
}

int32_t AudioRendererPrivate::CheckParams(const AudioRendererOptions &rendererOptions)
{
    ContentType contentType = rendererOptions.rendererInfo.contentType;
    CHECK_AND_RETURN_RET_LOG(contentType >= CONTENT_TYPE_UNKNOWN && contentType <= CONTENT_TYPE_ULTRASONIC,
        ERR_NOT_SUPPORTED, "Invalid content type");

    StreamUsage streamUsage = rendererOptions.rendererInfo.streamUsage;
    CHECK_AND_RETURN_RET_LOG(streamUsage >= STREAM_USAGE_UNKNOWN &&
        streamUsage <= STREAM_USAGE_VOICE_MODEM_COMMUNICATION, ERR_NOT_SUPPORTED, "Invalid stream usage");

    if (!IsFormatValid(rendererOptions.streamInfo.format) ||
        !IsSamplingRateValid(rendererOptions.streamInfo.samplingRate) ||
        !IsEncodingTypeValid(rendererOptions.streamInfo.encoding)) {
        AUDIO_ERR_LOG("Unsupported audio parameter");
        return ERR_NOT_SUPPORTED;
    }
    if (!IsPlaybackChannelRelatedInfoValid(rendererOptions.streamInfo.channels,
        rendererOptions.streamInfo.channelLayout)) {
        AUDIO_ERR_LOG("Unsupported channels or channelLayout");
        return ERR_NOT_SUPPORTED;
    }
    return SUCCESS;
}

int32_t AudioRendererPrivate::GetFrameCount(uint32_t &frameCount) const
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererPrivate::GetLatency(uint64_t &latency) const
{
    return ERR_NOT_SUPPORTED;
}

void AudioRendererPrivate::SetAudioPrivacyType(AudioPrivacyType privacyType) {}

int32_t AudioRendererPrivate::SetParams(const AudioRendererParams params)
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererPrivate::GetParams(AudioRendererParams &params) const
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererPrivate::GetRendererInfo(AudioRendererInfo &rendererInfo) const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return [rendererImpl_ getRendererInfo:rendererInfo];
}

int32_t AudioRendererPrivate::GetStreamInfo(AudioStreamInfo &streamInfo) const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return [rendererImpl_ getStreamInfo:streamInfo];
}

int32_t AudioRendererPrivate::SetRendererCallback(const std::shared_ptr<AudioRendererCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return [rendererImpl_ setRendererCallback:callback];
}

int32_t AudioRendererPrivate::SetRendererPositionCallback(int64_t markPosition,
    const std::shared_ptr<RendererPositionCallback> &callback)
{
    return ERR_NOT_SUPPORTED;
}

void AudioRendererPrivate::UnsetRendererPositionCallback() {}

int32_t AudioRendererPrivate::SetRendererPeriodPositionCallback(int64_t frameNumber,
    const std::shared_ptr<RendererPeriodPositionCallback> &callback)
{
    return ERR_NOT_SUPPORTED;
}

void AudioRendererPrivate::UnsetRendererPeriodPositionCallback() {}

bool AudioRendererPrivate::Start(StateChangeCmdType cmdType)
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, false, "rendererImpl_ == nullptr.");
    return [rendererImpl_ start];
}

int32_t AudioRendererPrivate::Write(uint8_t *buffer, size_t bufferSize)
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererPrivate::Write(uint8_t *pcmBuffer, size_t pcmSize, uint8_t *metaBuffer, size_t metaSize)
{
    return ERR_NOT_SUPPORTED;
}

RendererState AudioRendererPrivate::GetStatus() const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, RENDERER_INVALID, "rendererImpl_ == nullptr.");
    return [rendererImpl_ getStatus];
}

bool AudioRendererPrivate::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, false, "rendererImpl_ == nullptr.");
    return [rendererImpl_ getAudioTime:timestamp];
}

bool AudioRendererPrivate::GetAudioPosition(Timestamp &timestamp, Timestamp::Timestampbase base)
{
    return false;
}

bool AudioRendererPrivate::Drain() const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, false, "rendererImpl_ == nullptr.");
    return [rendererImpl_ drain];
}

bool AudioRendererPrivate::Flush() const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, false, "rendererImpl_ == nullptr.");
    return [rendererImpl_ flush];
}

bool AudioRendererPrivate::PauseTransitent(StateChangeCmdType cmdType)
{
    return false;
}

bool AudioRendererPrivate::Pause(StateChangeCmdType cmdType)
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, false, "rendererImpl_ == nullptr.");
    return [rendererImpl_ pause];
}

bool AudioRendererPrivate::Stop()
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, false, "rendererImpl_ == nullptr.");
    return [rendererImpl_ stop];
}

bool AudioRendererPrivate::Release()
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, false, "rendererImpl_ == nullptr.");
    return [rendererImpl_ releaseRenderer];
}

int32_t AudioRendererPrivate::GetBufferSize(size_t &bufferSize) const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return [rendererImpl_ getBufferSize:bufferSize];
}

int32_t AudioRendererPrivate::GetAudioStreamId(uint32_t &sessionID) const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return [rendererImpl_ getAudioStreamId: sessionID];
}
 
int32_t AudioRendererPrivate::SetAudioRendererDesc(AudioRendererDesc audioRendererDesc)
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererPrivate::SetStreamType(AudioStreamType audioStreamType)
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererPrivate::SetVolume(float volume) const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return [rendererImpl_ setVolume:volume];
}

float AudioRendererPrivate::GetVolume() const
{
    return 0.0f;
}

int32_t AudioRendererPrivate::SetRenderRate(AudioRendererRate renderRate) const
{
    return ERR_NOT_SUPPORTED;
}

AudioRendererRate AudioRendererPrivate::GetRenderRate() const
{
    return RENDER_RATE_NORMAL;
}

int32_t AudioRendererPrivate::SetRendererSamplingRate(uint32_t sampleRate) const
{
    return ERR_NOT_SUPPORTED;
}

uint32_t AudioRendererPrivate::GetRendererSamplingRate() const
{
    return 0;
}

int32_t AudioRendererPrivate::SetBufferDuration(uint64_t bufferDuration) const
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererPrivate::SetChannelBlendMode(ChannelBlendMode blendMode)
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererPrivate::SetRenderMode(AudioRenderMode renderMode)
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return [rendererImpl_ setRenderMode:renderMode];
}

AudioRenderMode AudioRendererPrivate::GetRenderMode() const
{
    return RENDER_MODE_NORMAL;
}

int32_t AudioRendererPrivate::GetBufferDesc(BufferDesc &bufDesc)
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return [rendererImpl_ getBufferDesc:bufDesc];
}

int32_t AudioRendererPrivate::Enqueue(const BufferDesc &bufDesc)
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return [rendererImpl_ enqueue:bufDesc];
}

int32_t AudioRendererPrivate::Clear() const
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererPrivate::GetBufQueueState(BufferQueueState &bufState) const
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererPrivate::SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return [rendererImpl_ setRendererWriteCallback:callback];
}

int32_t AudioRendererPrivate::SetRendererFirstFrameWritingCallback(
    const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback)
{
    return ERR_NOT_SUPPORTED;
}

void AudioRendererPrivate::SetInterruptMode(InterruptMode mode)
{
    CHECK_AND_RETURN_LOG(rendererImpl_ != nullptr, "rendererImpl_ == nullptr.");
    [rendererImpl_ setInterruptMode:mode];
}

int32_t AudioRendererPrivate::SetParallelPlayFlag(bool parallelPlayFlag)
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererPrivate::SetLowPowerVolume(float volume) const
{
    return ERR_NOT_SUPPORTED;
}

float AudioRendererPrivate::GetLowPowerVolume() const
{
    return 0.0f;
}

int32_t AudioRendererPrivate::SetOffloadAllowed(bool isAllowed)
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererPrivate::SetOffloadMode(int32_t state, bool isAppBack) const
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererPrivate::UnsetOffloadMode() const
{
    return ERR_NOT_SUPPORTED;
}

float AudioRendererPrivate::GetSingleStreamVolume() const
{
    return 0.0f;
}

float AudioRendererPrivate::GetMinStreamVolume() const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, 0.0f, "rendererImpl_ == nullptr.");
    return [rendererImpl_ getMinStreamVolume];
}

float AudioRendererPrivate::GetMaxStreamVolume() const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, 0.0f, "rendererImpl_ == nullptr.");
    return [rendererImpl_ getMaxStreamVolume];
}

int32_t AudioRendererPrivate::GetCurrentOutputDevices(AudioDeviceDescriptor &deviceInfo) const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return [rendererImpl_ getCurrentOutputDevices:deviceInfo];
}

uint32_t AudioRendererPrivate::GetUnderflowCount() const
{
    return 0;
}

void AudioRendererPrivate::SetAudioRendererErrorCallback(std::shared_ptr<AudioRendererErrorCallback> errorCallback) {}

int32_t AudioRendererPrivate::RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
    const std::shared_ptr<AudioRendererPolicyServiceDiedCallback> &callback)
{
    return SUCCESS;
}

int32_t AudioRendererPrivate::UnregisterAudioPolicyServerDiedCb(const int32_t clientPid)
{
    return SUCCESS;
}

int32_t AudioRendererPrivate::RegisterOutputDeviceChangeWithInfoCallback(
    const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return [rendererImpl_ setRendererDeviceChangeWithInfoCallback:callback];
}

int32_t AudioRendererPrivate::UnregisterOutputDeviceChangeWithInfoCallback()
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return [rendererImpl_ destroyRendererDeviceChangeWithInfoCallback];
}

int32_t AudioRendererPrivate::UnregisterOutputDeviceChangeWithInfoCallback(
    const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return [rendererImpl_ destroyRendererDeviceChangeWithInfoCallback];
}

AudioEffectMode AudioRendererPrivate::GetAudioEffectMode() const
{
    AudioEffectMode effectMode;
    return effectMode;
}

int64_t AudioRendererPrivate::GetFramesWritten() const
{
    return 0;
}

int32_t AudioRendererPrivate::SetAudioEffectMode(AudioEffectMode effectMode) const
{
    return ERR_NOT_SUPPORTED;
}

int32_t AudioRendererPrivate::SetVolumeWithRamp(float volume, int32_t duration)
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return [rendererImpl_ setVolumeWithRamp:volume rampTime:duration];
}

void AudioRendererPrivate::SetPreferredFrameSize(int32_t frameSize) {}

int32_t AudioRendererPrivate::SetSpeed(float speed)
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return [rendererImpl_ setSpeed:speed];
}

float AudioRendererPrivate::GetSpeed()
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return [rendererImpl_ getSpeed];
}

bool AudioRendererPrivate::IsFastRenderer()
{
    return false;
}

bool AudioRendererPrivate::IsFormatValid(uint8_t format)
{
    bool isValidFormat = (find(AUDIO_SUPPORTED_FORMATS.begin(), AUDIO_SUPPORTED_FORMATS.end(), format)
                          != AUDIO_SUPPORTED_FORMATS.end());
    AUDIO_DEBUG_LOG("AudioRendererPrivate: IsFormatValid: %{public}s", isValidFormat ? "true" : "false");
    return isValidFormat;
}

bool AudioRendererPrivate::IsRendererChannelValid(uint8_t channel)
{
    bool isValidChannel = (find(RENDERER_SUPPORTED_CHANNELS.begin(), RENDERER_SUPPORTED_CHANNELS.end(), channel)
                           != RENDERER_SUPPORTED_CHANNELS.end());
    AUDIO_DEBUG_LOG("AudioRendererPrivate: IsChannelValid: %{public}s", isValidChannel ? "true" : "false");
    return isValidChannel;
}

bool AudioRendererPrivate::IsEncodingTypeValid(uint8_t encodingType)
{
    bool isValidEncodingType
            = (find(AUDIO_SUPPORTED_ENCODING_TYPES.begin(), AUDIO_SUPPORTED_ENCODING_TYPES.end(), encodingType)
               != AUDIO_SUPPORTED_ENCODING_TYPES.end());
    AUDIO_DEBUG_LOG("AudioRendererPrivate: IsEncodingTypeValid: %{public}s", isValidEncodingType ? "true" : "false");
    return isValidEncodingType;
}

bool AudioRendererPrivate::IsSamplingRateValid(uint32_t samplingRate)
{
    bool isValidSamplingRate
            = (find(AUDIO_SUPPORTED_SAMPLING_RATES.begin(), AUDIO_SUPPORTED_SAMPLING_RATES.end(), samplingRate)
               != AUDIO_SUPPORTED_SAMPLING_RATES.end());
    AUDIO_DEBUG_LOG("AudioRendererPrivate: IsSamplingRateValid: %{public}s", isValidSamplingRate ? "true" : "false");
    return isValidSamplingRate;
}

bool AudioRendererPrivate::IsRendererChannelLayoutValid(uint64_t channelLayout)
{
    bool isValidRendererChannelLayout = (find(RENDERER_SUPPORTED_CHANNELLAYOUTS.begin(),
        RENDERER_SUPPORTED_CHANNELLAYOUTS.end(), channelLayout) != RENDERER_SUPPORTED_CHANNELLAYOUTS.end());
    AUDIO_DEBUG_LOG("AudioRendererPrivate: isValidRendererChannelLayout: %{public}s",
        isValidRendererChannelLayout ? "true" : "false");
    return isValidRendererChannelLayout;
}

bool AudioRendererPrivate::IsPlaybackChannelRelatedInfoValid(uint8_t channels, uint64_t channelLayout)
{
    if (!IsRendererChannelValid(channels)) {
        AUDIO_ERR_LOG("AudioRendererPrivate: Invalid sink channel %{public}d", channels);
        return false;
    }
    if (!IsRendererChannelLayoutValid(channelLayout)) {
        AUDIO_ERR_LOG("AudioRendererPrivate: Invalid sink channel layout");
        return false;
    }
    return true;
}

int32_t AudioRendererPrivate::GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    return ERR_NOT_SUPPORTED;
}

void AudioRendererPrivate::SetFastStatusChangeCallback(
    const std::shared_ptr<AudioRendererFastStatusChangeCallback>& callback)
{}
}  // namespace AudioStandard
}  // namespace OHOS
