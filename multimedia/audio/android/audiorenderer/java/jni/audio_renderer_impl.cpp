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
#include "audio_renderer_impl.h"

#include "audio_errors.h"
#include "audio_log.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static const int32_t WRITE_BUFFER_TIMEOUT_IN_MS = 200; // ms
static constexpr char WRITE_THREAD_NAME[] = "OS_AudioWriteCB";
}

AudioRendererImpl::AudioRendererImpl() : playState_(RENDERER_NEW), minWriteBufferSize_(0) {}

AudioRendererImpl::~AudioRendererImpl()
{
    Release();
    Plugin::AudioRendererJni::Finalize(reinterpret_cast<long>(this));
}

int32_t AudioRendererImpl::CreateAudioTrack(const AudioRendererOptions& rendererOptions)
{
    formatInfo_.channelLayout = rendererOptions.streamInfo.channelLayout;
    formatInfo_.samplingRate = rendererOptions.streamInfo.samplingRate;
    formatInfo_.format = rendererOptions.streamInfo.format;
    formatInfo_.channels = rendererOptions.streamInfo.channels;
    int32_t ret = Plugin::AudioRendererJni::CreateAudioTrack(
        reinterpret_cast<long>(this), rendererOptions, MAX_STREAM_SPEED_LEVEL);
    if (ret == SUCCESS) {
        playState_ = RENDERER_PREPARED;
    }
    return ret;
}

bool AudioRendererImpl::Start()
{
    std::unique_lock<std::mutex> statusLock(statusLock_);
    CHECK_AND_RETURN_RET_LOG(
        (playState_ == RENDERER_PREPARED) || (playState_ == RENDERER_STOPPED) || (playState_ == RENDERER_PAUSED), false,
        "Start failed. Illegal state:%{public}u", playState_);
    bool ret = Plugin::AudioRendererJni::Start(reinterpret_cast<long>(this));
    if (!ret) {
        return false;
    }
    TransferState(RENDERER_RUNNING);
    return true;
}

bool AudioRendererImpl::Pause()
{
    std::unique_lock<std::mutex> statusLock(statusLock_);
    CHECK_AND_RETURN_RET_LOG(
        playState_ == RENDERER_RUNNING, false, "State of stream is not running. Illegal state:%{public}u", playState_);
    bool ret = Plugin::AudioRendererJni::Pause(reinterpret_cast<long>(this));
    if (!ret) {
        return false;
    }
    TransferState(RENDERER_PAUSED);
    return true;
}

bool AudioRendererImpl::Stop()
{
    std::unique_lock<std::mutex> statusLock(statusLock_);
    CHECK_AND_RETURN_RET_LOG((playState_ == RENDERER_RUNNING) || (playState_ == RENDERER_PAUSED), false,
        "State of stream is not running. Illegal state:%{public}u", playState_);
    bool ret = Plugin::AudioRendererJni::Stop(reinterpret_cast<long>(this));
    if (!ret) {
        return false;
    }
    TransferState(RENDERER_STOPPED);
    return true;
}

bool AudioRendererImpl::Release()
{
    std::unique_lock<std::mutex> statusLock(statusLock_);
    CHECK_AND_RETURN_RET_LOG(playState_ != RENDERER_RELEASED, false, "Already released, do nothing.");
    bool ret = Plugin::AudioRendererJni::Release(reinterpret_cast<long>(this));
    if (!ret) {
        return false;
    }

    TransferState(RENDERER_RELEASED);
    writeThreadReleased_ = true;
    writeBuffer_ = { nullptr, 0, 0 };
    writeCallback_ = nullptr;
    writeThreadCv_.notify_all();
    isWrited_ = false;
    statusLock.unlock();

    if (callbackLoop_.joinable()) {
        callbackLoop_.join();
    }
    return true;
}

bool AudioRendererImpl::Flush()
{
    CHECK_AND_RETURN_RET_LOG(
        (playState_ == RENDERER_RUNNING) || (playState_ == RENDERER_PAUSED) || (playState_ == RENDERER_STOPPED), false,
        "State is not RUNNING. Illegal state:%{public}u", playState_);

    return Plugin::AudioRendererJni::Flush(reinterpret_cast<long>(this));
}

int32_t AudioRendererImpl::Write(uint8_t* buffer, size_t bufferSize)
{
    CHECK_AND_RETURN_RET_LOG((buffer != nullptr) && (bufferSize > 0), ERR_INVALID_PARAM, "buffer is nullptr.");

    return Plugin::AudioRendererJni::Write(reinterpret_cast<long>(this), buffer, bufferSize);
}

int32_t AudioRendererImpl::SetRenderMode(AudioRenderMode renderMode)
{
    AUDIO_INFO_LOG("SetRenderMode to %{public}s",
        renderMode == RENDER_MODE_NORMAL ? "RENDER_MODE_NORMAL" : "RENDER_MODE_CALLBACK");
    if (renderMode_ == renderMode) {
        return SUCCESS;
    }

    // renderMode_ is inited as RENDER_MODE_NORMAL, can only be set to RENDER_MODE_CALLBACK.
    if (renderMode_ == RENDER_MODE_CALLBACK && renderMode == RENDER_MODE_NORMAL) {
        AUDIO_ERR_LOG("SetRenderMode from callback to normal is not supported.");
        return ERR_INCORRECT_MODE;
    }

    // state check
    if (playState_ != RENDERER_PREPARED && playState_ != RENDERER_NEW) {
        AUDIO_ERR_LOG("SetRenderMode failed. invalid state");
        return ERR_ILLEGAL_STATE;
    }
    renderMode_ = renderMode;

    int32_t ret = GetBufferSize(minWriteBufferSize_);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Get buffer size fail.");
        return ret;
    }

    writeBuffer_.buffer = std::make_unique<uint8_t[]>(minWriteBufferSize_).get();
    writeBuffer_.bufLength = minWriteBufferSize_;
    writeBuffer_.dataLength = 0;
    cbBuffer_ = std::make_unique<uint8_t[]>(minWriteBufferSize_);

    callbackLoop_ = std::thread(&AudioRendererImpl::WriteCallbackFunc, this);
    pthread_setname_np(callbackLoop_.native_handle(), WRITE_THREAD_NAME);
    return SUCCESS;
}

int32_t AudioRendererImpl::SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback>& callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "Invalid null callback");
    CHECK_AND_RETURN_RET_LOG(renderMode_ == RENDER_MODE_CALLBACK, ERR_INCORRECT_MODE, "incorrect render mode");
    std::lock_guard<std::mutex> lock(writeCbMutex_);
    writeCallback_ = callback;
    return SUCCESS;
}

void AudioRendererImpl::WriteCallbackFunc()
{
    writeThreadReleased_ = false;

    while (!writeThreadReleased_) {
        AUDIO_DEBUG_LOG("AudioRendererJni::WriteCallbackFunc playState_:%d", playState_);
        std::unique_lock<std::mutex> statusLock(writeCbMutex_);
        if (playState_ != RENDERER_RUNNING) {
            continue;
        }
        if (writeBuffer_.dataLength > 0) {
            Write(writeBuffer_.buffer, writeBuffer_.dataLength);
            memset(writeBuffer_.buffer, 0, minWriteBufferSize_ * sizeof(uint8_t));
            writeBuffer_.dataLength = 0;
        }
        isWrited_ = false;
        if (writeCallback_) {
            writeCallback_->OnWriteData(minWriteBufferSize_);
        }
        statusLock.unlock();
        std::unique_lock<std::mutex> lk(writeBufferLock_);
        writeThreadCv_.wait_for(
            lk, std::chrono::milliseconds(WRITE_BUFFER_TIMEOUT_IN_MS), [this] { return isWrited_; });
    }
}

RendererState AudioRendererImpl::GetStatus() const
{
    AUDIO_DEBUG_LOG("AudioRendererImpl::GetStatus playState_:%{public}d", playState_);
    return playState_;
}

bool AudioRendererImpl::GetAudioTime(Timestamp& timestamp, Timestamp::Timestampbase base) const
{
    return Plugin::AudioRendererJni::GetTimestamp(reinterpret_cast<long>(this), timestamp);
}

int32_t AudioRendererImpl::GetBufferSize(size_t& bufferSize) const
{
    return Plugin::AudioRendererJni::GetBufferSize(reinterpret_cast<long>(this), formatInfo_, bufferSize);
}

int32_t AudioRendererImpl::GetBufferDesc(BufferDesc& bufDesc) const
{
    bufDesc.buffer = cbBuffer_.get();
    bufDesc.bufLength = minWriteBufferSize_;
    bufDesc.dataLength = minWriteBufferSize_;

    return SUCCESS;
}

int32_t AudioRendererImpl::Enqueue(const BufferDesc& bufDesc)
{
    CHECK_AND_RETURN_RET_LOG(bufDesc.buffer != nullptr && bufDesc.bufLength != 0, ERR_INVALID_PARAM, "Invalid buffer");

    writeBuffer_.buffer = bufDesc.buffer;
    writeBuffer_.bufLength = bufDesc.bufLength;
    writeBuffer_.dataLength = bufDesc.dataLength;

    std::unique_lock<std::mutex> lk(writeBufferLock_);
    isWrited_ = true;
    writeThreadCv_.notify_all();

    return SUCCESS;
}

int32_t AudioRendererImpl::SetChannelBlendMode(ChannelBlendMode blendMode)
{
    return Plugin::AudioRendererJni::SetDualMonoMode(reinterpret_cast<long>(this), blendMode);
}

int32_t AudioRendererImpl::SetSpeed(float speed)
{
    return Plugin::AudioRendererJni::SetSpeed(reinterpret_cast<long>(this), speed);
}

float AudioRendererImpl::GetSpeed()
{
    return Plugin::AudioRendererJni::GetSpeed(reinterpret_cast<long>(this));
}

int32_t AudioRendererImpl::SetVolume(float volume)
{
    return Plugin::AudioRendererJni::SetVolume(reinterpret_cast<long>(this), volume);
}

float AudioRendererImpl::GetMinStreamVolume()
{
    return Plugin::AudioRendererJni::GetMinVolume(reinterpret_cast<long>(this));
}

float AudioRendererImpl::GetMaxStreamVolume()
{
    return Plugin::AudioRendererJni::GetMaxVolume(reinterpret_cast<long>(this));
}

uint32_t AudioRendererImpl::GetUnderflowCount()
{
    return Plugin::AudioRendererJni::GetUnderrunCount(reinterpret_cast<long>(this));
}

int32_t AudioRendererImpl::GetCurrentOutputDevices(AudioDeviceDescriptor& deviceInfo) const
{
    return Plugin::AudioRendererJni::GetCurrentOutputDevices(reinterpret_cast<long>(this), deviceInfo);
}

int32_t AudioRendererImpl::GetAudioStreamId(uint32_t& sessionID) const
{
    return Plugin::AudioRendererJni::GetAudioSessionId(reinterpret_cast<long>(this), sessionID);
}

int32_t AudioRendererImpl::GetRendererInfo(AudioRendererInfo& rendererInfo) const
{
    return Plugin::AudioRendererJni::GetRendererInfo(reinterpret_cast<long>(this), rendererInfo);
}

int32_t AudioRendererImpl::GetStreamInfo(AudioStreamInfo& streamInfo) const
{
    int32_t ret = Plugin::AudioRendererJni::GetStreamInfo(reinterpret_cast<long>(this), streamInfo);
    streamInfo.channelLayout = formatInfo_.channelLayout;
    return ret;
}

int32_t AudioRendererImpl::SetRendererCallback(const std::shared_ptr<AudioRendererCallback>& callback)
{
    stateCallback_ = callback;
    return SUCCESS;
}

void AudioRendererImpl::TransferState(RendererState playState)
{
    AUDIO_DEBUG_LOG("TransferState oldState:%{public}d, newState:%{public}d", playState_, playState);

    RendererState oldState = playState_;
    playState_ = playState;

    if (oldState == playState_) {
        AUDIO_DEBUG_LOG("No change in status.");
        return;
    }
    if (stateCallback_) {
        stateCallback_->OnStateChange(playState);
    }
}

int32_t AudioRendererImpl::SetRendererPositionCallback(
    int64_t markPosition, const std::shared_ptr<RendererPositionCallback>& callback)
{
    int32_t ret = Plugin::AudioRendererJni::SetNotificationMarkerPosition(reinterpret_cast<long>(this), markPosition);
    if (ret != SUCCESS) {
        return ret;
    }
    return Plugin::AudioRendererJni::SetRendererPositionUpdateListener(reinterpret_cast<long>(this), callback);
}

void AudioRendererImpl::UnsetRendererPositionCallback()
{
    Plugin::AudioRendererJni::UnsetRendererPositionUpdateListener(reinterpret_cast<long>(this));
}

int32_t AudioRendererImpl::SetRendererPeriodPositionCallback(
    int64_t frameNumber, const std::shared_ptr<RendererPeriodPositionCallback>& callback)
{
    int32_t ret = Plugin::AudioRendererJni::SetPositionNotificationPeriod(reinterpret_cast<long>(this), frameNumber);
    if (ret != SUCCESS) {
        return ret;
    }
    return Plugin::AudioRendererJni::SetPeriodPositionUpdateListener(reinterpret_cast<long>(this), callback);
}

void AudioRendererImpl::UnsetRendererPeriodPositionCallback()
{
    Plugin::AudioRendererJni::UnsetPeriodPositionUpdateListener(reinterpret_cast<long>(this));
}

int32_t AudioRendererImpl::RegisterRendererDeviceChangeListener(
    const std::shared_ptr<AudioRendererOutputDeviceChangeCallback>& callback)
{
    return Plugin::AudioRendererJni::RegisterRendererDeviceChangeListener(reinterpret_cast<long>(this), callback);
}

int32_t AudioRendererImpl::UnregisterRendererDeviceChangeListener()
{
    return Plugin::AudioRendererJni::UnregisterRendererDeviceChangeListener(reinterpret_cast<long>(this));
}
} // namespace AudioStandard
} // namespace OHOS
