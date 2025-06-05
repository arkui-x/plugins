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

#include "audio_capturer_private.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
AudioCapturer::~AudioCapturer() = default;
AudioCapturerPrivate::~AudioCapturerPrivate()
{
    capturerImpl_ = nullptr;
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(AudioStreamType audioStreamType)
{
    AppInfo appInfo = {};
    return Create(audioStreamType, appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(AudioStreamType audioStreamType, const AppInfo& appInfo)
{
    return Create(audioStreamType, appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions& options)
{
    AppInfo appInfo = {};
    return Create(options, "", appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions& options, const AppInfo& appInfo)
{
    return Create(options, "", appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions& options, const std::string cachePath)
{
    AppInfo appInfo = {};
    return Create(options, cachePath, appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(
    const AudioCapturerOptions& capturerOptions, const std::string cachePath, const AppInfo& appInfo)
{
    AUDIO_DEBUG_LOG("AudioCapturer::Create");

    auto audioCapturer = std::make_unique<AudioCapturerPrivate>();
    if (audioCapturer->CheckParams(capturerOptions) != SUCCESS) {
        AUDIO_ERR_LOG("CheckParams failed in capturer");
        audioCapturer = nullptr;
    } else {
        if (audioCapturer->CreateAudioRecord(capturerOptions) != SUCCESS) {
            audioCapturer = nullptr;
        }
    }
    return audioCapturer;
}

int32_t AudioCapturerPrivate::CreateAudioRecord(const AudioCapturerOptions& capturerOptions)
{
    capturerImpl_ = std::make_shared<AudioCapturerImpl>();
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");
    return capturerImpl_->CreateAudioRecord(capturerOptions);
}

int32_t AudioCapturerPrivate::CheckParams(const AudioCapturerOptions& capturerOptions)
{
    auto sourceType = capturerOptions.capturerInfo.sourceType;
    CHECK_AND_RETURN_RET_LOG(sourceType >= SOURCE_TYPE_MIC && sourceType <= SOURCE_TYPE_MAX, ERR_NOT_SUPPORTED,
        "AudioCapturerPrivate::CheckParams.Invalid source type %{public}d!", sourceType);

    if (!IsFormatValid(capturerOptions.streamInfo.format) ||
        !IsCapturerChannelValid(capturerOptions.streamInfo.channels) ||
        !IsEncodingTypeValid(capturerOptions.streamInfo.encoding) ||
        !IsSamplingRateValid(capturerOptions.streamInfo.samplingRate)) {
        AUDIO_ERR_LOG("AudioCapturerPrivate::CheckParams Unsupported audio parameter");
        return ERR_NOT_SUPPORTED;
    }
    return SUCCESS;
}

bool AudioCapturerPrivate::IsFormatValid(uint8_t format)
{
    bool isValidFormat =
        (find(AUDIO_SUPPORTED_FORMATS.begin(), AUDIO_SUPPORTED_FORMATS.end(), format) != AUDIO_SUPPORTED_FORMATS.end());
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::IsFormatValid: %{public}s", isValidFormat ? "true" : "false");
    return isValidFormat;
}

bool AudioCapturerPrivate::IsCapturerChannelValid(uint8_t channel)
{
    bool isValidChannel = (find(CAPTURER_SUPPORTED_CHANNELS.begin(), CAPTURER_SUPPORTED_CHANNELS.end(), channel) !=
                           CAPTURER_SUPPORTED_CHANNELS.end());
    AUDIO_DEBUG_LOG("IsChannelValid: %{public}s", isValidChannel ? "true" : "false");
    return isValidChannel;
}

bool AudioCapturerPrivate::IsEncodingTypeValid(uint8_t encodingType)
{
    bool isValidEncodingType = (find(AUDIO_SUPPORTED_ENCODING_TYPES.begin(), AUDIO_SUPPORTED_ENCODING_TYPES.end(),
                                    encodingType) != AUDIO_SUPPORTED_ENCODING_TYPES.end());
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::IsEncodingTypeValid: %{public}s", isValidEncodingType ? "true" : "false");
    return isValidEncodingType;
}

bool AudioCapturerPrivate::IsSamplingRateValid(uint32_t samplingRate)
{
    bool isValidSamplingRate = (find(AUDIO_SUPPORTED_SAMPLING_RATES.begin(), AUDIO_SUPPORTED_SAMPLING_RATES.end(),
                                    samplingRate) != AUDIO_SUPPORTED_SAMPLING_RATES.end());
    AUDIO_DEBUG_LOG("AudioCapturerPrivate: IsSamplingRateValid: %{public}s", isValidSamplingRate ? "true" : "false");
    return isValidSamplingRate;
}

int32_t AudioCapturerPrivate::UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config)
{
    return 0;
}

int32_t AudioCapturerPrivate::SetCapturerCallback(const std::shared_ptr<AudioCapturerCallback>& callback)
{
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::SetCapturerCallback");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "input param is invalid.");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");

    return capturerImpl_->SetCapturerCallback(callback);
}

int32_t AudioCapturerPrivate::GetCapturerInfo(AudioCapturerInfo& capturerInfo) const
{
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::GetCapturerInfo");

    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");

    return capturerImpl_->GetCapturerInfo(capturerInfo);
}

int32_t AudioCapturerPrivate::GetStreamInfo(AudioStreamInfo& streamInfo) const
{
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");

    return capturerImpl_->GetStreamInfo(streamInfo);
}

int32_t AudioCapturerPrivate::SetCapturerPositionCallback(
    int64_t markPosition, const std::shared_ptr<CapturerPositionCallback>& callback)
{
    CHECK_AND_RETURN_RET_LOG((callback != nullptr) && (markPosition > 0), ERR_INVALID_PARAM, "input param is invalid");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");

    return capturerImpl_->SetCapturerPositionCallback(markPosition, callback);
}

void AudioCapturerPrivate::UnsetCapturerPositionCallback()
{
    CHECK_AND_RETURN_LOG(capturerImpl_ != nullptr, "capturerImpl_ == nullptr.");

    return capturerImpl_->UnsetCapturerPositionCallback();
}

int32_t AudioCapturerPrivate::SetCapturerPeriodPositionCallback(
    int64_t frameNumber, const std::shared_ptr<CapturerPeriodPositionCallback>& callback)
{
    CHECK_AND_RETURN_RET_LOG((callback != nullptr) && (frameNumber > 0), ERR_INVALID_PARAM, "input param is invalid");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");

    return capturerImpl_->SetCapturerPeriodPositionCallback(frameNumber, callback);
}

void AudioCapturerPrivate::UnsetCapturerPeriodPositionCallback()
{
    CHECK_AND_RETURN_LOG(capturerImpl_ != nullptr, "capturerImpl_ == nullptr.");

    return capturerImpl_->UnsetCapturerPeriodPositionCallback();
}

bool AudioCapturerPrivate::Start()
{
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::Start");

    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, false, "capturerImpl_ == nullptr.");

    return capturerImpl_->Start();
}

int32_t AudioCapturerPrivate::Read(uint8_t& buffer, size_t userSize, bool isBlockingRead)
{
    return 0;
}

CapturerState AudioCapturerPrivate::GetStatus() const
{
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, CAPTURER_INVALID, "capturerImpl_ == nullptr.");

    return capturerImpl_->GetStatus();
}

bool AudioCapturerPrivate::GetTimeStampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    return false;
}

bool AudioCapturerPrivate::GetAudioTime(Timestamp& timestamp, Timestamp::Timestampbase base) const
{
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, false, "capturerImpl_ == nullptr.");

    return capturerImpl_->GetAudioTime(timestamp, base);
}

bool AudioCapturerPrivate::GetFirstPkgTimeStampInfo(int64_t &firstTs) const
{
    return false;
}

bool AudioCapturerPrivate::Pause() const
{
    return true;
}

bool AudioCapturerPrivate::Stop() const
{
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::Stop");

    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, false, "capturerImpl_ == nullptr.");

    return capturerImpl_->Stop();
}

bool AudioCapturerPrivate::Flush() const
{
    return true;
}

bool AudioCapturerPrivate::Release()
{
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::Release");

    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, false, "capturerImpl_ == nullptr.");

    return capturerImpl_->Release();
}

int32_t AudioCapturerPrivate::GetBufferSize(size_t& bufferSize) const
{
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::GetBufferSize");

    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");

    return capturerImpl_->GetBufferSize(bufferSize);
}

int32_t AudioCapturerPrivate::GetAudioStreamId(uint32_t& sessionID) const
{
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");

    return capturerImpl_->GetAudioStreamId(sessionID);
}

int32_t AudioCapturerPrivate::SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback>& callback)
{
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::SetCapturerReadCallback");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "input param is invalid");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");

    return capturerImpl_->SetCapturerReadCallback(callback);
}

int32_t AudioCapturerPrivate::GetBufferDesc(BufferDesc& bufDesc)
{
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::GetBufferDesc");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");

    return capturerImpl_->GetBufferDesc(bufDesc);
}

int32_t AudioCapturerPrivate::Enqueue(const BufferDesc& bufDesc)
{
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::Enqueue");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");

    return capturerImpl_->Enqueue(bufDesc);
}

int32_t AudioCapturerPrivate::GetCurrentInputDevices(AudioDeviceDescriptor& deviceInfo) const
{
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::GetCurrentInputDevices");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");

    return capturerImpl_->GetCurrentInputDevices(deviceInfo);
}

int32_t AudioCapturerPrivate::GetCurrentCapturerChangeInfo(AudioCapturerChangeInfo& changeInfo) const
{
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::GetCurrentCapturerChangeInfo");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");

    return capturerImpl_->GetCurrentCapturerChangeInfo(changeInfo);
}

int32_t AudioCapturerPrivate::SetAudioCapturerDeviceChangeCallback(
    const std::shared_ptr<AudioCapturerDeviceChangeCallback>& callback)
{
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::SetAudioCapturerDeviceChangeCallback");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "input param is invalid");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");

    return capturerImpl_->SetAudioCapturerDeviceChangeCallback(callback);
}

int32_t AudioCapturerPrivate::RemoveAudioCapturerDeviceChangeCallback(
    const std::shared_ptr<AudioCapturerDeviceChangeCallback>& callback)
{
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::RemoveAudioCapturerDeviceChangeCallback");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "input param is invalid");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");

    return capturerImpl_->RemoveAudioCapturerDeviceChangeCallback(callback);
}

int32_t AudioCapturerPrivate::SetAudioCapturerInfoChangeCallback(
    const std::shared_ptr<AudioCapturerInfoChangeCallback>& callback)
{
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::SetAudioCapturerInfoChangeCallback");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "input param is invalid");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");

    return capturerImpl_->SetAudioCapturerInfoChangeCallback(callback);
}

int32_t AudioCapturerPrivate::RemoveAudioCapturerInfoChangeCallback(
    const std::shared_ptr<AudioCapturerInfoChangeCallback>& callback)
{
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::RemoveAudioCapturerInfoChangeCallback");

    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "input param is invalid");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");

    return capturerImpl_->RemoveAudioCapturerInfoChangeCallback(callback);
}

void AudioCapturerPrivate::SetAudioCapturerErrorCallback(std::shared_ptr<AudioCapturerErrorCallback> errorCallback)
{

}

int32_t AudioCapturerPrivate::RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
    const std::shared_ptr<AudioCapturerPolicyServiceDiedCallback> &callback)
{
    return 0;
}

// diffrence from GetAudioPosition only when set speed
int32_t AudioCapturerPrivate::GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    return 0;
}

int32_t AudioCapturerPrivate::SetInputDevice(DeviceType deviceType) const
{
    return 0;
}

int32_t AudioCapturerPrivate::SetInterruptStrategy(InterruptStrategy strategy)
{
    return 0;
}

void AudioCapturerPrivate::SetFastStatusChangeCallback(
    const std::shared_ptr<AudioCapturerFastStatusChangeCallback>& callback)
{}
} // namespace AudioStandard
} // namespace OHOS
