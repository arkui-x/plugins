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

#include "audio_log.h"
#include "audio_renderer_private.h"

namespace OHOS {
namespace AudioStandard {
std::mutex AudioRenderer::createRendererMutex_;

AudioRenderer::~AudioRenderer() = default;
AudioRendererPrivate::~AudioRendererPrivate()
{
    rendererImpl_ = nullptr;
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

std::unique_ptr<AudioRenderer> AudioRenderer::Create(AudioStreamType audioStreamType, const AppInfo& appInfo)
{
    return nullptr;
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(const AudioRendererOptions& rendererOptions)
{
    AppInfo appInfo = {};
    return Create("", rendererOptions, appInfo);
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(
    const AudioRendererOptions& rendererOptions, const AppInfo& appInfo)
{
    return Create("", rendererOptions, appInfo);
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(
    const std::string cachePath, const AudioRendererOptions& rendererOptions)
{
    AppInfo appInfo = {};
    return Create(cachePath, rendererOptions, appInfo);
}

std::unique_ptr<AudioRenderer> AudioRenderer::Create(
    const std::string cachePath, const AudioRendererOptions& rendererOptions, const AppInfo& appInfo)
{
    std::lock_guard<std::mutex> lock(createRendererMutex_);

    auto audioRenderer = std::make_unique<AudioRendererPrivate>();
    if (audioRenderer->CheckParams(rendererOptions) != SUCCESS) {
        AUDIO_ERR_LOG("CheckParams failed in renderer");
        audioRenderer = nullptr;
    } else {
        int32_t ret = audioRenderer->CreateAudioTrack(rendererOptions);
        if (ret != SUCCESS) {
            audioRenderer = nullptr;
        }
    }
    return audioRenderer;
}

int32_t AudioRendererPrivate::CreateAudioTrack(const AudioRendererOptions& rendererOptions)
{
    rendererImpl_ = std::make_shared<AudioRendererImpl>();
    return rendererImpl_->CreateAudioTrack(rendererOptions);
}

int32_t AudioRendererPrivate::CheckParams(const AudioRendererOptions& rendererOptions)
{
    ContentType contentType = rendererOptions.rendererInfo.contentType;
    CHECK_AND_RETURN_RET_LOG(contentType >= CONTENT_TYPE_UNKNOWN && contentType <= CONTENT_TYPE_ULTRASONIC,
        ERR_NOT_SUPPORTED, "Invalid content type");

    StreamUsage streamUsage = rendererOptions.rendererInfo.streamUsage;
    CHECK_AND_RETURN_RET_LOG(
        streamUsage >= STREAM_USAGE_UNKNOWN && streamUsage <= STREAM_USAGE_VOICE_MODEM_COMMUNICATION, ERR_NOT_SUPPORTED,
        "Invalid stream usage");

    if (!IsFormatValid(rendererOptions.streamInfo.format) ||
        !IsSamplingRateValid(rendererOptions.streamInfo.samplingRate) ||
        !IsEncodingTypeValid(rendererOptions.streamInfo.encoding)) {
        AUDIO_ERR_LOG("Unsupported audio parameter");
        return ERR_NOT_SUPPORTED;
    }
    if (!IsPlaybackChannelRelatedInfoValid(
            rendererOptions.streamInfo.channels, rendererOptions.streamInfo.channelLayout)) {
        AUDIO_ERR_LOG("Unsupported channels or channelLayout");
        return ERR_NOT_SUPPORTED;
    }
    return SUCCESS;
}

int32_t AudioRendererPrivate::GetRendererInfo(AudioRendererInfo& rendererInfo) const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");

    return rendererImpl_->GetRendererInfo(rendererInfo);
}

int32_t AudioRendererPrivate::GetStreamInfo(AudioStreamInfo& streamInfo) const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");

    return rendererImpl_->GetStreamInfo(streamInfo);
}

bool AudioRendererPrivate::Start(StateChangeCmdType cmdType)
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, false, "rendererImpl_ == nullptr.");

    return rendererImpl_->Start();
}

int32_t AudioRendererPrivate::Write(uint8_t* buffer, size_t bufferSize)
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, 0, "rendererImpl_ == nullptr.");

    return rendererImpl_->Write(buffer, bufferSize);
}

int32_t AudioRendererPrivate::SetRenderMode(AudioRenderMode renderMode)
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return rendererImpl_->SetRenderMode(renderMode);
}

int32_t AudioRendererPrivate::SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback>& callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "input param is invalid.");
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");

    return rendererImpl_->SetRendererWriteCallback(callback);
}

RendererState AudioRendererPrivate::GetStatus() const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, RENDERER_INVALID, "rendererImpl_ == nullptr.");

    return rendererImpl_->GetStatus();
}

bool AudioRendererPrivate::GetAudioTime(Timestamp& timestamp, Timestamp::Timestampbase base) const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, false, "rendererImpl_ == nullptr.");

    return rendererImpl_->GetAudioTime(timestamp, base);
}

bool AudioRendererPrivate::Flush() const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, false, "rendererImpl_ == nullptr.");

    return rendererImpl_->Flush();
}

bool AudioRendererPrivate::Pause(StateChangeCmdType cmdType)
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, false, "rendererImpl_ == nullptr.");

    return rendererImpl_->Pause();
}

bool AudioRendererPrivate::Stop()
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, false, "rendererImpl_ == nullptr.");

    return rendererImpl_->Stop();
}

bool AudioRendererPrivate::Release()
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, false, "rendererImpl_ == nullptr.");

    return rendererImpl_->Release();
}

int32_t AudioRendererPrivate::GetBufferSize(size_t& bufferSize) const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");

    return rendererImpl_->GetBufferSize(bufferSize);
}

int32_t AudioRendererPrivate::GetAudioStreamId(uint32_t& sessionID) const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");

    return rendererImpl_->GetAudioStreamId(sessionID);
}

int32_t AudioRendererPrivate::SetVolume(float volume) const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");

    return rendererImpl_->SetVolume(volume);
}

int32_t AudioRendererPrivate::SetChannelBlendMode(ChannelBlendMode blendMode)
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERR_ILLEGAL_STATE, "rendererImpl_ == nullptr.");

    return rendererImpl_->SetChannelBlendMode(blendMode);
}

int32_t AudioRendererPrivate::GetBufferDesc(BufferDesc& bufDesc)
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");

    return rendererImpl_->GetBufferDesc(bufDesc);
}

int32_t AudioRendererPrivate::Enqueue(const BufferDesc& bufDesc)
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");

    return rendererImpl_->Enqueue(bufDesc);
}

float AudioRendererPrivate::GetMinStreamVolume() const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, 0.0f, "rendererImpl_ == nullptr.");

    return rendererImpl_->GetMinStreamVolume();
}

float AudioRendererPrivate::GetMaxStreamVolume() const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, 0.0f, "rendererImpl_ == nullptr.");

    return rendererImpl_->GetMaxStreamVolume();
}

int32_t AudioRendererPrivate::GetCurrentOutputDevices(AudioDeviceDescriptor& deviceInfo) const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");

    return rendererImpl_->GetCurrentOutputDevices(deviceInfo);
}

uint32_t AudioRendererPrivate::GetUnderflowCount() const
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, 0, "rendererImpl_ == nullptr.");

    return rendererImpl_->GetUnderflowCount();
}

int32_t AudioRendererPrivate::SetSpeed(float speed)
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    CHECK_AND_RETURN_RET_LOG((speed >= MIN_STREAM_SPEED_LEVEL) && (speed <= MAX_STREAM_SPEED_LEVEL), ERR_INVALID_PARAM,
        "invaild speed index");

    return rendererImpl_->SetSpeed(speed);
}

float AudioRendererPrivate::GetSpeed()
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, 0.0f, "rendererImpl_ == nullptr.");

    return rendererImpl_->GetSpeed();
}

int32_t AudioRendererPrivate::SetRendererCallback(const std::shared_ptr<AudioRendererCallback>& callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "input param is invalid");
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");

    return rendererImpl_->SetRendererCallback(callback);
}

int32_t AudioRendererPrivate::SetRendererPositionCallback(
    int64_t markPosition, const std::shared_ptr<RendererPositionCallback>& callback)
{
    CHECK_AND_RETURN_RET_LOG((callback != nullptr) && (markPosition > 0), ERR_INVALID_PARAM, "input param is invalid");
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");

    return rendererImpl_->SetRendererPositionCallback(markPosition, callback);
}

void AudioRendererPrivate::UnsetRendererPositionCallback()
{
    CHECK_AND_RETURN_LOG(rendererImpl_ != nullptr, "rendererImpl_ == nullptr.");

    rendererImpl_->UnsetRendererPositionCallback();
}

int32_t AudioRendererPrivate::SetRendererPeriodPositionCallback(
    int64_t frameNumber, const std::shared_ptr<RendererPeriodPositionCallback>& callback)
{
    CHECK_AND_RETURN_RET_LOG((callback != nullptr) && (frameNumber > 0), ERR_INVALID_PARAM, "input param is invalid");
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");

    return rendererImpl_->SetRendererPeriodPositionCallback(frameNumber, callback);
}

void AudioRendererPrivate::UnsetRendererPeriodPositionCallback()
{
    CHECK_AND_RETURN_LOG(rendererImpl_ != nullptr, "rendererImpl_ == nullptr.");

    rendererImpl_->UnsetRendererPeriodPositionCallback();
}

int32_t AudioRendererPrivate::RegisterOutputDeviceChangeWithInfoCallback(
        const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is null");
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return rendererImpl_->RegisterRendererDeviceChangeListener(callback);
}
    
int32_t AudioRendererPrivate::UnregisterOutputDeviceChangeWithInfoCallback()
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return rendererImpl_->UnregisterRendererDeviceChangeListener();
}

int32_t AudioRendererPrivate::UnregisterOutputDeviceChangeWithInfoCallback(
    const std::shared_ptr<AudioRendererOutputDeviceChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(rendererImpl_ != nullptr, ERROR, "rendererImpl_ == nullptr.");
    return rendererImpl_->UnregisterRendererDeviceChangeListener();
}

bool AudioRendererPrivate::IsFormatValid(uint8_t format)
{
    bool isValidFormat =
        (find(AUDIO_SUPPORTED_FORMATS.begin(), AUDIO_SUPPORTED_FORMATS.end(), format) != AUDIO_SUPPORTED_FORMATS.end());
    AUDIO_DEBUG_LOG("AudioRendererPrivate: IsFormatValid: %{public}s", isValidFormat ? "true" : "false");
    return isValidFormat;
}

bool AudioRendererPrivate::IsRendererChannelValid(uint8_t channel)
{
    bool isValidChannel = (find(RENDERER_SUPPORTED_CHANNELS.begin(), RENDERER_SUPPORTED_CHANNELS.end(), channel) !=
                           RENDERER_SUPPORTED_CHANNELS.end());
    AUDIO_DEBUG_LOG("AudioRendererPrivate: IsChannelValid: %{public}s", isValidChannel ? "true" : "false");
    return isValidChannel;
}

bool AudioRendererPrivate::IsEncodingTypeValid(uint8_t encodingType)
{
    bool isValidEncodingType = (find(AUDIO_SUPPORTED_ENCODING_TYPES.begin(), AUDIO_SUPPORTED_ENCODING_TYPES.end(),
                                    encodingType) != AUDIO_SUPPORTED_ENCODING_TYPES.end());
    AUDIO_DEBUG_LOG("AudioRendererPrivate: IsEncodingTypeValid: %{public}s", isValidEncodingType ? "true" : "false");
    return isValidEncodingType;
}

bool AudioRendererPrivate::IsSamplingRateValid(uint32_t samplingRate)
{
    bool isValidSamplingRate = (find(AUDIO_SUPPORTED_SAMPLING_RATES.begin(), AUDIO_SUPPORTED_SAMPLING_RATES.end(),
                                    samplingRate) != AUDIO_SUPPORTED_SAMPLING_RATES.end());
    AUDIO_DEBUG_LOG("AudioRendererPrivate: IsSamplingRateValid: %{public}s", isValidSamplingRate ? "true" : "false");
    return isValidSamplingRate;
}

bool AudioRendererPrivate::IsRendererChannelLayoutValid(uint64_t channelLayout)
{
    bool isValidRendererChannelLayout =
        (find(RENDERER_SUPPORTED_CHANNELLAYOUTS.begin(), RENDERER_SUPPORTED_CHANNELLAYOUTS.end(), channelLayout) !=
            RENDERER_SUPPORTED_CHANNELLAYOUTS.end());
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
} // namespace AudioStandard
} // namespace OHOS
