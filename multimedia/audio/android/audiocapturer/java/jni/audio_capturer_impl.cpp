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

#include "audio_capturer_impl.h"

#include "audio_errors.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static const int32_t SHORT_TIMEOUT_IN_MS = 200;      // ms
static const int32_t READ_BUFFER_TIMEOUT_IN_MS = 20; // ms
static constexpr char READ_THREAD_NAME[] = "OS_AudioReadCB";
} // namespace
AudioCapturerImpl::AudioCapturerImpl() : minReadBufferSize_(0) {}

AudioCapturerImpl::~AudioCapturerImpl()
{
    Release();
    Plugin::AudioCapturerJni::Finalize(reinterpret_cast<long>(this));
}

int32_t AudioCapturerImpl::CreateAudioRecord(const AudioCapturerOptions& capturerOptions)
{
    TransferState(CAPTURER_NEW);
    streamInfo_.channelLayout = capturerOptions.streamInfo.channelLayout;
    streamInfo_.samplingRate = capturerOptions.streamInfo.samplingRate;
    streamInfo_.format = capturerOptions.streamInfo.format;
    streamInfo_.channels = capturerOptions.streamInfo.channels;
    int32_t ret = Plugin::AudioCapturerJni::CreateAudioRecord(reinterpret_cast<long>(this), capturerOptions);
    if (ret != SUCCESS) {
        TransferState(CAPTURER_INVALID);
        return ret;
    }
    TransferState(CAPTURER_PREPARED);
    Plugin::AudioCapturerJni::SetCapturerState(reinterpret_cast<long>(this), recordState_);
    return ret;
}

CapturerState AudioCapturerImpl::GetStatus() const
{
    return recordState_;
}

bool AudioCapturerImpl::Start()
{
    std::unique_lock<std::mutex> statusLock(statusLock_);
    CHECK_AND_RETURN_RET_LOG((recordState_ == CAPTURER_PREPARED) || (recordState_ == CAPTURER_STOPPED), false,
        "Start failed. Illegal state:%{public}u", recordState_);
    bool ret = Plugin::AudioCapturerJni::Start(reinterpret_cast<long>(this));
    if (ret) {
        TransferState(CAPTURER_RUNNING);
    }
    return ret;
}

bool AudioCapturerImpl::Stop()
{
    std::unique_lock<std::mutex> statusLock(statusLock_);
    CHECK_AND_RETURN_RET_LOG((recordState_ == CAPTURER_RUNNING), false,
        "State of stream is not running. Illegal state:%{public}u", recordState_);
    bool ret = Plugin::AudioCapturerJni::Stop(reinterpret_cast<long>(this));
    if (ret) {
        TransferState(CAPTURER_STOPPED);
    }
    return ret;
}

bool AudioCapturerImpl::Release()
{
    std::unique_lock<std::mutex> statusLock(statusLock_);
    CHECK_AND_RETURN_RET_LOG(recordState_ != CAPTURER_RELEASED, false, "Already released, do nothing.");
    bool ret = Plugin::AudioCapturerJni::Release(reinterpret_cast<long>(this));
    if (ret) {
        readThreadReleased_ = true;
        readThreadCv_.notify_all();
        if (callbackLoop_.joinable()) {
            callbackLoop_.join();
        }
        readBuffer_ = { nullptr, 0, 0 };
        cbBuffer_ = nullptr;
        isReaded_ = false;
        TransferState(CAPTURER_RELEASED);
    }
    return ret;
}

int32_t AudioCapturerImpl::SetCapturerCallback(const std::shared_ptr<AudioCapturerCallback>& callback)
{
    stateCallback_ = callback;
    return SUCCESS;
}

void AudioCapturerImpl::TransferState(CapturerState recordState)
{
    if (recordState_ != recordState) {
        AUDIO_DEBUG_LOG(
            "AudioCapturerImpl::TransferState state changed. old_state(%d), new_state(%d)", recordState_, recordState);
        recordState_ = recordState;
        if (stateCallback_) {
            stateCallback_->OnStateChange(recordState);
        }
        Plugin::AudioCapturerJni::SetCapturerState(reinterpret_cast<long>(this), recordState_);
        AudioCapturerChangeInfo changeInfo;
        int32_t ret = GetCurrentCapturerChangeInfo(changeInfo);
        if (ret == SUCCESS) {
            Plugin::AudioCapturerJni::NotifyCapturerInfoChanged(reinterpret_cast<long>(this), changeInfo);
        }
    }
}

int32_t AudioCapturerImpl::GetBufferSize(size_t& bufferSize) const
{
    int32_t result = Plugin::AudioCapturerJni::GetBufferSize(reinterpret_cast<long>(this), streamInfo_, bufferSize);
    return result;
}

int32_t AudioCapturerImpl::GetBufferDesc(BufferDesc& bufDesc)
{
    std::unique_lock<std::mutex> lock(readBufferLock_);
    bufDesc.buffer = cbBuffer_.get();
    bufDesc.bufLength = minReadBufferSize_;
    bufDesc.dataLength = minReadBufferSize_;

    return SUCCESS;
}

int32_t AudioCapturerImpl::SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback>& callback)
{
    std::lock_guard<std::mutex> lock(readCbMutex_);
    readCallback_ = callback;

    int32_t ret = GetBufferSize(minReadBufferSize_);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("AudioCapturerImpl::SetCapturerReadCallback Get buffer size failed.");
        return ret;
    }
    cbBuffer_ = std::make_unique<uint8_t[]>(minReadBufferSize_);
    readBuffer_ = { cbBuffer_.get(), minReadBufferSize_, minReadBufferSize_ };

    callbackLoop_ = std::thread(&AudioCapturerImpl::ReadCallbackFunc, this);
    pthread_setname_np(callbackLoop_.native_handle(), READ_THREAD_NAME);

    std::unique_lock<std::mutex> threadStartlock(readThreadLock_);
    readThreadCv_.wait_for(threadStartlock, std::chrono::milliseconds(SHORT_TIMEOUT_IN_MS),
        [this] { return readThreadReleased_ == false; });
    return SUCCESS;
}

void AudioCapturerImpl::ReadCallbackFunc()
{
    readThreadReleased_ = false;
    readThreadCv_.notify_all();

    while (!readThreadReleased_) {
        if (recordState_ != CAPTURER_RUNNING) {
            continue;
        }

        std::unique_lock<std::mutex> lockBuffer(readBufferLock_);
        memset(readBuffer_.buffer, 0, minReadBufferSize_ * sizeof(uint8_t));
        Plugin::AudioCapturerJni::Read(reinterpret_cast<long>(this), readBuffer_.buffer, readBuffer_.dataLength);
        lockBuffer.unlock();

        std::unique_lock<std::mutex> lockCb(readCbMutex_);
        isReaded_ = false;
        if (readCallback_) {
            readCallback_->OnReadData(minReadBufferSize_);
        }
        lockCb.unlock();

        std::unique_lock<std::mutex> bufferLock(readBufferLock_);
        cbBufferCv_.wait_for(
            bufferLock, std::chrono::milliseconds(READ_BUFFER_TIMEOUT_IN_MS), [this] { return isReaded_ == true; });
    }
}

int32_t AudioCapturerImpl::Enqueue(const BufferDesc& bufDesc)
{
    if (bufDesc.bufLength != minReadBufferSize_ || bufDesc.dataLength != minReadBufferSize_) {
        AUDIO_ERR_LOG("Enqueue invalid bufLength:%zu or dataLength:%zu, should be %zu", bufDesc.bufLength,
            bufDesc.dataLength, minReadBufferSize_);
        return ERR_INVALID_INDEX; // ERR_INVALID_INDEX(-17)
    }
    if (bufDesc.buffer != cbBuffer_.get()) {
        AUDIO_WARNING_LOG("AudioCapturerImpl::Enqueue buffer is not from us.");
    }
    readBuffer_.buffer = cbBuffer_.get();
    readBuffer_.bufLength = minReadBufferSize_;
    readBuffer_.dataLength = minReadBufferSize_;

    std::unique_lock<std::mutex> lock(readBufferLock_);
    isReaded_ = true;
    cbBufferCv_.notify_all();
    return SUCCESS;
}

int32_t AudioCapturerImpl::GetCapturerInfo(AudioCapturerInfo& capturerInfo) const
{
    return Plugin::AudioCapturerJni::GetCapturerInfo(reinterpret_cast<long>(this), capturerInfo);
}

int32_t AudioCapturerImpl::SetCapturerPositionCallback(
    int64_t markPosition, const std::shared_ptr<CapturerPositionCallback>& callback)
{
    int32_t ret = Plugin::AudioCapturerJni::SetNotificationMarkerPosition(reinterpret_cast<long>(this), markPosition);
    if (ret != SUCCESS) {
        return ret;
    }
    return Plugin::AudioCapturerJni::SetCapturerPositionUpdateListener(reinterpret_cast<long>(this), callback);
}

void AudioCapturerImpl::UnsetCapturerPositionCallback()
{
    Plugin::AudioCapturerJni::UnsetCapturerPositionUpdateListener(reinterpret_cast<long>(this));
}

int32_t AudioCapturerImpl::SetCapturerPeriodPositionCallback(
    int64_t frameNumber, const std::shared_ptr<CapturerPeriodPositionCallback>& callback)
{
    int32_t ret = Plugin::AudioCapturerJni::SetPositionNotificationPeriod(reinterpret_cast<long>(this), frameNumber);
    if (ret != SUCCESS) {
        return ret;
    }
    return Plugin::AudioCapturerJni::SetPeriodPositionUpdateListener(reinterpret_cast<long>(this), callback);
}

void AudioCapturerImpl::UnsetCapturerPeriodPositionCallback()
{
    Plugin::AudioCapturerJni::UnsetPeriodPositionUpdateListener(reinterpret_cast<long>(this));
}

int32_t AudioCapturerImpl::SetAudioCapturerDeviceChangeCallback(
    const std::shared_ptr<AudioCapturerDeviceChangeCallback>& callback)
{
    return Plugin::AudioCapturerJni::RegisterCapturerDeviceChangeListener(reinterpret_cast<long>(this), callback);
}

int32_t AudioCapturerImpl::RemoveAudioCapturerDeviceChangeCallback(
    const std::shared_ptr<AudioCapturerDeviceChangeCallback>& callback)
{
    return Plugin::AudioCapturerJni::RemoveAudioCapturerDeviceChangeListener(reinterpret_cast<long>(this), callback);
}

int32_t AudioCapturerImpl::GetStreamInfo(AudioStreamInfo& streamInfo) const
{
    int32_t ret = Plugin::AudioCapturerJni::GetStreamInfo(reinterpret_cast<long>(this), streamInfo);
    streamInfo.channelLayout = streamInfo_.channelLayout;
    return ret;
}

bool AudioCapturerImpl::GetAudioTime(Timestamp& timestamp, Timestamp::Timestampbase base) const
{
    if (recordState_ == CAPTURER_STOPPED) {
        return false;
    }
    return Plugin::AudioCapturerJni::GetTimestamp(reinterpret_cast<long>(this), timestamp);
}

int32_t AudioCapturerImpl::GetAudioStreamId(uint32_t& sessionID) const
{
    return Plugin::AudioCapturerJni::GetAudioSessionId(reinterpret_cast<long>(this), sessionID);
}

int32_t AudioCapturerImpl::GetCurrentInputDevices(AudioDeviceDescriptor& deviceInfo) const
{
    return Plugin::AudioCapturerJni::GetCurrentInputDevices(reinterpret_cast<long>(this), deviceInfo);
}

int32_t AudioCapturerImpl::GetCurrentCapturerChangeInfo(AudioCapturerChangeInfo& changeInfo) const
{
    uint32_t sessionID;
    AudioDeviceDescriptor deviceInfo;
    CapturerState capturerState;
    AudioCapturerInfo capturerInfo;
    int32_t ret = Plugin::AudioCapturerJni::GetAudioSessionId(reinterpret_cast<long>(this), sessionID);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("AudioCapturerImpl::GetCurrentCapturerChangeInfo GetAudioSessionId failed");
        return ret;
    }
    ret = Plugin::AudioCapturerJni::GetCurrentInputDevices(reinterpret_cast<long>(this), deviceInfo);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("AudioCapturerImpl::GetCurrentCapturerChangeInfo GetCurrentInputDevices failed");
        return ret;
    }
    ret = Plugin::AudioCapturerJni::GetCapturerInfo(reinterpret_cast<long>(this), capturerInfo);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("AudioCapturerImpl::GetCurrentCapturerChangeInfo GetCapturerInfo failed");
        return ret;
    }
    changeInfo.sessionId = sessionID;
    changeInfo.capturerInfo = capturerInfo;
    changeInfo.capturerState = recordState_;
    changeInfo.inputDeviceInfo = deviceInfo;

    return SUCCESS;
}

int32_t AudioCapturerImpl::SetAudioCapturerInfoChangeCallback(
    const std::shared_ptr<AudioCapturerInfoChangeCallback>& callback)
{
    return Plugin::AudioCapturerJni::SetAudioCapturerInfoChangeCallback(reinterpret_cast<long>(this), callback);
}

int32_t AudioCapturerImpl::RemoveAudioCapturerInfoChangeCallback(
    const std::shared_ptr<AudioCapturerInfoChangeCallback>& callback)
{
    return Plugin::AudioCapturerJni::RemoveAudioCapturerInfoChangeCallback(reinterpret_cast<long>(this), callback);
}
} // namespace AudioStandard
} // namespace OHOS
