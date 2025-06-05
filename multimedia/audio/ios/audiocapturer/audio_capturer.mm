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

#include "audio_capturer.h"
#include "audio_capturer_private.h"
#include "audio_errors.h"
#include "audio_log.h"
#import "audio_manager_impl.h"

namespace OHOS {
namespace AudioStandard {

AudioCapturer::~AudioCapturer() = default;
AudioCapturerPrivate::~AudioCapturerPrivate()
{
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    if (managerImpl) {
        [managerImpl removeCapturer:capturerImpl_];
    }
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(AudioStreamType audioStreamType)
{
    AppInfo appInfo = {};
    return Create(audioStreamType, appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(AudioStreamType audioStreamType, const AppInfo &appInfo)
{
    return nullptr;
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions &options)
{
    AppInfo appInfo = {};
    return Create(options, "", appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions &options, const AppInfo &appInfo)
{
    return Create(options, "", appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions &options, const std::string cachePath)
{
    AppInfo appInfo = {};
    return Create(options, cachePath, appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions &capturerOptions,
    const std::string cachePath, const AppInfo &appInfo)
{
    AVAuthorizationStatus authorizationStatus = [AVCaptureDevice authorizationStatusForMediaType:AVMediaTypeAudio];
    if (authorizationStatus == AVAuthorizationStatusAuthorized) {
        auto audioCapturer = std::make_unique<AudioCapturerPrivate>();
        if (audioCapturer->CheckParams(capturerOptions) != SUCCESS) {
            AUDIO_ERR_LOG("CheckParams failed in renderer");
            audioCapturer = nullptr;
        } else {
            audioCapturer->CreateAudioRecord(capturerOptions);
        }
        return audioCapturer;
    } else {
        return nullptr;
    }
}

void AudioCapturerPrivate::CreateAudioRecord(const AudioCapturerOptions &capturerOptions)
{
    capturerImpl_ = [[AudioCapturerImpl alloc] init];
    [capturerImpl_ initWithSampleRate: capturerOptions];

    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    if (managerImpl) {
        [managerImpl addCapturer:capturerImpl_];
    }
}

int32_t AudioCapturerPrivate::CheckParams(const AudioCapturerOptions &capturerOptions)
{
    SourceType sourceType = capturerOptions.capturerInfo.sourceType;
    CHECK_AND_RETURN_RET_LOG(sourceType > SOURCE_TYPE_INVALID && sourceType <= SOURCE_TYPE_VOICE_CALL || 
    sourceType >= SOURCE_TYPE_VOICE_COMMUNICATION && sourceType <= SOURCE_TYPE_MAX,
        ERR_NOT_SUPPORTED, "Invalid source type");
    if (!IsFormatValid(capturerOptions.streamInfo.format) ||
        !IsSamplingRateValid(capturerOptions.streamInfo.samplingRate) ||
        !IsEncodingTypeValid(capturerOptions.streamInfo.encoding)) {
        AUDIO_ERR_LOG("Unsupported audio parameter");
        return ERR_NOT_SUPPORTED;
    }
    if (!IsCapturerChannelValid(capturerOptions.streamInfo.channels)) {
        AUDIO_ERR_LOG("Unsupported channels");
        return ERR_NOT_SUPPORTED;
    }
    return SUCCESS;
}

int32_t AudioCapturerPrivate::UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config)
{
    return 0;
}

int32_t AudioCapturerPrivate::SetCapturerCallback(const std::shared_ptr<AudioCapturerCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "input param is invalid");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");
    return [capturerImpl_ setCapturerCallback: callback];
}

int32_t AudioCapturerPrivate::GetCapturerInfo(AudioCapturerInfo &capturerInfo) const
{
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");
    return [capturerImpl_ getCapturerInfo: capturerInfo];
}

int32_t AudioCapturerPrivate::GetStreamInfo(AudioStreamInfo &streamInfo) const
{
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");
    return [capturerImpl_ getStreamInfo: streamInfo];
}

bool AudioCapturerPrivate::Start()
{
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, false, "capturerImpl_ == nullptr.");
    return [capturerImpl_ start];
}

CapturerState AudioCapturerPrivate::GetStatus() const
{
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, CAPTURER_INVALID, "capturerImpl_ == nullptr.");
    return [capturerImpl_ getStatus];
}

bool AudioCapturerPrivate::GetTimeStampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    return false;
}

bool AudioCapturerPrivate::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, false, "capturerImpl_ == nullptr.");
    return [capturerImpl_ getAudioTime: timestamp];
}

bool AudioCapturerPrivate::GetFirstPkgTimeStampInfo(int64_t &firstTs) const
{
    return false;
}

bool AudioCapturerPrivate::Stop() const
{
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, false, "capturerImpl_ == nullptr.");
    return [capturerImpl_ stop];
}

bool AudioCapturerPrivate::Release()
{
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, false, "capturerImpl_ == nullptr.");
    return [capturerImpl_ releaseCapturer];
}

int32_t AudioCapturerPrivate::GetBufferSize(size_t &bufferSize) const
{
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");
    return [capturerImpl_ getBufferSize: bufferSize];
}

int32_t AudioCapturerPrivate::GetAudioStreamId(uint32_t &sessionID) const
{
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");
    return [capturerImpl_ getAudioStreamId: sessionID];
}

int32_t AudioCapturerPrivate::SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "input param is invalid");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");
    return [capturerImpl_ setCapturerReadCallback: callback];
}

int32_t AudioCapturerPrivate::GetBufferDesc(BufferDesc &bufDesc)
{
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");
    return [capturerImpl_ getBufferDesc: bufDesc];
}

int32_t AudioCapturerPrivate::Enqueue(const BufferDesc &bufDesc)
{
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");
    return [capturerImpl_ enqueue: bufDesc];
}

int32_t AudioCapturerPrivate::GetCurrentInputDevices(AudioDeviceDescriptor &deviceInfo) const
{
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");
    return [capturerImpl_ getCurrentInputDevices: deviceInfo];
}

int32_t AudioCapturerPrivate::GetCurrentCapturerChangeInfo(AudioCapturerChangeInfo &changeInfo) const
{
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");
    return [capturerImpl_ getCurrentCapturerChangeInfo: changeInfo];
}

int32_t AudioCapturerPrivate::SetAudioCapturerDeviceChangeCallback(
    const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "input param is invalid");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");
    return [capturerImpl_ setAudioCapturerDeviceChangeCallback: callback];
}

int32_t AudioCapturerPrivate::RemoveAudioCapturerDeviceChangeCallback(
    const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "input param is invalid");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");
    return [capturerImpl_ removeAudioCapturerDeviceChangeCallback: callback];
}

int32_t AudioCapturerPrivate::SetAudioCapturerInfoChangeCallback(
    const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "input param is invalid");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");
    return [capturerImpl_ setAudioCapturerInfoChangeCallback: callback];
}

int32_t AudioCapturerPrivate::RemoveAudioCapturerInfoChangeCallback(
    const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "input param is invalid");
    CHECK_AND_RETURN_RET_LOG(capturerImpl_ != nullptr, ERROR, "capturerImpl_ == nullptr.");
    return [capturerImpl_ removeAudioCapturerInfoChangeCallback: callback];
}

bool AudioCapturerPrivate::IsFormatValid(uint8_t format)
{
    bool isValidFormat = (find(AUDIO_SUPPORTED_FORMATS.begin(), AUDIO_SUPPORTED_FORMATS.end(), format)
                          != AUDIO_SUPPORTED_FORMATS.end());
    AUDIO_DEBUG_LOG("AudioCapturerPrivate: IsFormatValid: %{public}s", isValidFormat ? "true" : "false");
    return isValidFormat;
}

bool AudioCapturerPrivate::IsCapturerChannelValid(uint8_t channel)
{
    bool isValidChannel = (find(CAPTURER_SUPPORTED_CHANNELS.begin(), CAPTURER_SUPPORTED_CHANNELS.end(), channel)
                           != CAPTURER_SUPPORTED_CHANNELS.end());
    AUDIO_DEBUG_LOG("AudioCapturerPrivate: IsChannelValid: %{public}s", isValidChannel ? "true" : "false");
    return isValidChannel;
}

bool AudioCapturerPrivate::IsEncodingTypeValid(uint8_t encodingType)
{
    bool isValidEncodingType
            = (find(AUDIO_SUPPORTED_ENCODING_TYPES.begin(), AUDIO_SUPPORTED_ENCODING_TYPES.end(), encodingType)
               != AUDIO_SUPPORTED_ENCODING_TYPES.end());
    AUDIO_DEBUG_LOG("AudioCapturerPrivate: IsEncodingTypeValid: %{public}s", isValidEncodingType ? "true" : "false");
    return isValidEncodingType;
}

bool AudioCapturerPrivate::IsSamplingRateValid(uint32_t samplingRate)
{
    bool isValidSamplingRate
            = (find(AUDIO_SUPPORTED_SAMPLING_RATES.begin(), AUDIO_SUPPORTED_SAMPLING_RATES.end(), samplingRate)
               != AUDIO_SUPPORTED_SAMPLING_RATES.end());
    AUDIO_DEBUG_LOG("AudioCapturerPrivate: IsSamplingRateValid: %{public}s", isValidSamplingRate ? "true" : "false");
    return isValidSamplingRate;
}
int32_t AudioCapturerPrivate::SetCapturerPositionCallback(
    int64_t markPosition, const std::shared_ptr<CapturerPositionCallback>& callback)
{
    return ERR_NOT_SUPPORTED;
}

void AudioCapturerPrivate::UnsetCapturerPositionCallback() {}

int32_t AudioCapturerPrivate::SetCapturerPeriodPositionCallback(
    int64_t frameNumber, const std::shared_ptr<CapturerPeriodPositionCallback>& callback)
{
    return ERR_NOT_SUPPORTED;
}

void AudioCapturerPrivate::UnsetCapturerPeriodPositionCallback() {}

bool AudioCapturerPrivate::Pause() const
{
    return false;
}

bool AudioCapturerPrivate::Flush() const
{
    return false;
}

AudioCaptureMode AudioCapturerPrivate::GetCaptureMode() const
{
    return (AudioCaptureMode)0;
}

int32_t AudioCapturerPrivate::Clear() const
{
    return 0;
}

void AudioCapturerPrivate::SetValid(bool valid) {}

int64_t AudioCapturerPrivate::GetFramesRead() const
{
    return 0;
}

int32_t AudioCapturerPrivate::GetFrameCount(uint32_t& frameCount) const
{
    return 0;
}

int32_t AudioCapturerPrivate::SetParams(const AudioCapturerParams params)
{
    return 0;
}

int32_t AudioCapturerPrivate::SetBufferDuration(uint64_t bufferDuration) const
{
    return 0;
}

int32_t AudioCapturerPrivate::SetCaptureMode(AudioCaptureMode captureMode)
{
    return 0;
}

int32_t AudioCapturerPrivate::GetBufQueueState(BufferQueueState& bufState) const
{
    return 0;
}

int32_t AudioCapturerPrivate::GetParams(AudioCapturerParams& params) const
{
    return 0;
}

uint32_t AudioCapturerPrivate::GetOverflowCount() const
{
    return 0;
}

int32_t AudioCapturerPrivate::SetInputDevice(DeviceType deviceType) const
{
    return 0;
}

int32_t AudioCapturerPrivate::SetAudioSourceConcurrency(const std::vector<SourceType> &targetSources) 
{ 
    return 0; 
}

int32_t AudioCapturerPrivate::SetInterruptStrategy(InterruptStrategy strategy)
{
    return 0;
}

std::vector<sptr<MicrophoneDescriptor>> AudioCapturerPrivate::GetCurrentMicrophones() const
{
    std::vector<sptr<MicrophoneDescriptor>> emptyVector;
    return emptyVector;
}

int32_t AudioCapturerPrivate::Read(uint8_t& buffer, size_t userSize, bool isBlockingRead)
{
    return 0;
}

int32_t AudioCapturerPrivate::RegisterAudioCapturerEventListener()
{
    return 0;
}

int32_t AudioCapturerPrivate::UnregisterAudioCapturerEventListener()
{
    return 0;
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

void AudioCapturerPrivate::SetFastStatusChangeCallback(
    const std::shared_ptr<AudioCapturerFastStatusChangeCallback>& callback)
{}
}  // namespace AudioStandard
}  // namespace OHOS
