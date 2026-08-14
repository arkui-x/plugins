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

#include <list>
#include <memory>
#include <mutex>
#include <securec.h>
#include "audio_utils.h"
#include "format_convert_util.h"
#import "audio_capturer_impl.h"
#import "audio_manager_impl.h"

// Fixed callback frame duration, unified across platforms so that the
// buffer length observed by JS is identical for the same stream params.
#define CAPTURE_FRAME_DURATION_MS 20
#define MS_PER_SECOND 1000
// Number of rotating AudioQueue buffers. With only one buffer the queue starves while JS is
// consuming the previous frame, causing periodic capture gaps at a 20ms cadence.
#define CAPTURE_QUEUE_BUFFER_COUNT 3

#pragma mark - AudioCapturerImpl implementation
@implementation AudioCapturerImpl
{
    AudioStreamBasicDescription audioDescription_;
    AudioQueueRef audioQueue_;
    AudioQueueBufferRef audioQueueBuffers_[CAPTURE_QUEUE_BUFFER_COUNT];
    AudioQueueTimelineRef timeLine_;
    AudioChannelLayout acl_;
    AVAudioSession *audioSession_;
    OHOS::AudioStandard::AudioCapturerOptions capturerOptions_;
    OHOS::AudioStandard::CapturerState recordState_;
    std::shared_ptr<OHOS::AudioStandard::AudioCapturerReadCallback> readCallback_;
    std::shared_ptr<OHOS::AudioStandard::AudioCapturerCallback> stateCallback_;
    std::shared_ptr<OHOS::AudioStandard::AudioCapturerDeviceChangeCallback> deviceCallback_;
    std::shared_ptr<OHOS::AudioStandard::AudioCapturerInfoChangeCallback> deviceWithInfoCallback_;
    std::mutex readBufferLock_;
    bool isLastRead_;
    size_t bufferSize_;
    // Staging slots, one per AudioQueue buffer. A slot stays pending from readerCallback until
    // JS returns it via enqueue, which also re-enqueues the paired AudioQueue buffer.
    // pendingSlots_ is a FIFO pairing every OnReadData with exactly one getBufferDesc/enqueue.
    std::unique_ptr<uint8_t[]> stagingBuffers_[CAPTURE_QUEUE_BUFFER_COUNT];
    size_t stagingDataLengths_[CAPTURE_QUEUE_BUFFER_COUNT];
    bool stagingPending_[CAPTURE_QUEUE_BUFFER_COUNT];
    std::list<size_t> pendingSlots_;
}

#pragma mark - record audio

- (void)initChannelLayout:(const OHOS::AudioStandard::AudioChannelLayout)channelLayout
{
    NSLog(@"initChannelLayout %llu", channelLayout);
    bool isValid = ConvertChannelLayoutFromOh(channelLayout, acl_);
    if (isValid) {
        OSStatus status = AudioQueueSetProperty(audioQueue_,
                                            kAudioQueueProperty_ChannelLayout,
                                            &acl_,
                                            sizeof(acl_));
        if (status != noErr) {
            NSLog(@"Failed to Set channel layout: %d", (int)status);
        } else {
            NSLog(@"Channel layout Set successfully.");
        }
    } else {
        NSLog(@"Unknown channel layout.");
    }
}

- (bool)initWithSampleRate:(const OHOS::AudioStandard::AudioCapturerOptions)capturerOptions
{
    capturerOptions_ = capturerOptions;
    recordState_ = OHOS::AudioStandard::CAPTURER_NEW;
    const OHOS::AudioStandard::AudioStreamInfo &streamInfo = capturerOptions.streamInfo;
    // Defensive validation: reject invalid stream params before any allocation.
    // The public entry (AudioCapturer::Create) already gates them via CheckParams;
    // this guards the impl against direct misuse.
    if (streamInfo.samplingRate <= 0 || streamInfo.channels <= 0) {
        NSLog(@"initWithSampleRate: invalid streamInfo, samplingRate=%d, channels=%d",
            streamInfo.samplingRate, streamInfo.channels);
        recordState_ = OHOS::AudioStandard::CAPTURER_INVALID;
        return false;
    }
    ConvertStreamInfoFromOh(streamInfo, audioDescription_);
    recordState_ = OHOS::AudioStandard::CAPTURER_PREPARED;
    isLastRead_ = false;
    // Use the same frame-length formula on all platforms so that identical stream
    // params produce identical buffer sizes. Frame-first, same as HPAE
    // spanSizeInFrame_: round the frame count down to whole samples before converting to
    // bytes, so odd rates (e.g. 11025Hz -> 220 frames) stay sample-aligned.
    uint32_t bytesPerSample = OHOS::AudioStandard::Util::GetSamplePerFrame(streamInfo.format);
    size_t framesPerSpan =
        static_cast<size_t>(streamInfo.samplingRate) * CAPTURE_FRAME_DURATION_MS / MS_PER_SECOND;
    bufferSize_ = framesPerSpan * static_cast<size_t>(streamInfo.channels) * bytesPerSample;
    for (size_t i = 0; i < CAPTURE_QUEUE_BUFFER_COUNT; i++) {
        stagingBuffers_[i] = std::make_unique<uint8_t[]>(bufferSize_);
        stagingDataLengths_[i] = 0;
        stagingPending_[i] = false;
    }
    pendingSlots_.clear();
    [self initChannelLayout:capturerOptions.streamInfo.channelLayout];
    NSLog(@"AudioStreamBasicDescription mSampleRate = %f, mFormatID = %u, mFormatFlags = %u, \
        mChannelsPerFrame = %u, mFramesPerPacket = %u, mBitsPerChannel = %u, mBytesPerFrame = %u, \
        mBytesPerPacket = %u",
        audioDescription_.mSampleRate, audioDescription_.mFormatID, audioDescription_.mFormatFlags,
        audioDescription_.mChannelsPerFrame, audioDescription_.mFramesPerPacket, audioDescription_.mBitsPerChannel,
        audioDescription_.mBytesPerFrame, audioDescription_.mBytesPerPacket);
    if (![self SetupAudioQueue]) {
        NSLog(@"SetupAudioQueue failed, abort init.");
        recordState_ = OHOS::AudioStandard::CAPTURER_INVALID;
        return false;
    }
    audioSession_ = [AVAudioSession sharedInstance];
    [audioSession_ setCategory:AVAudioSessionCategoryPlayAndRecord
                withOptions:AVAudioSessionCategoryOptionAllowBluetooth | AVAudioSessionCategoryOptionDefaultToSpeaker
                error:nil];
    [audioSession_ setActive:YES error:nil];
    // Align the hardware I/O period with the callback frame duration to reduce callback jitter.
    [audioSession_ setPreferredIOBufferDuration:(CAPTURE_FRAME_DURATION_MS / (double)MS_PER_SECOND) error:nil];
    NSLog(@"AVAudioSession IOBufferDuration = %f s", audioSession_.IOBufferDuration);
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    if (managerImpl) {
        [managerImpl updateCategory:audioSession_.category options:audioSession_.categoryOptions];
    }

    [[NSNotificationCenter defaultCenter] addObserver:self
                                            selector:@selector(handleRouteChange:)
                                            name:AVAudioSessionRouteChangeNotification
                                            object:audioSession_];
    return true;
}

- (int32_t)getCapturerInfo:(OHOS::AudioStandard::AudioCapturerInfo &)capturerInfo
{
    if (recordState_ != OHOS::AudioStandard::CAPTURER_RELEASED) {
        capturerInfo = capturerOptions_.capturerInfo;
        return OHOS::AudioStandard::SUCCESS;
    } else {
        return OHOS::AudioStandard::ERR_ILLEGAL_STATE;
    }
}

- (int32_t)getStreamInfo:(OHOS::AudioStandard::AudioStreamInfo &)streamInfo
{
    if (recordState_ != OHOS::AudioStandard::CAPTURER_RELEASED) {
        streamInfo = capturerOptions_.streamInfo;
        return OHOS::AudioStandard::SUCCESS;
    } else {
        return OHOS::AudioStandard::ERR_ILLEGAL_STATE;
    }
}

static void AudioRecordAQInputCallback(
                                        void * __nullable               inUserData,
                                        AudioQueueRef                   inAQ,
                                        AudioQueueBufferRef             inBuffer,
                                        const AudioTimeStamp *          inStartTime,
                                        UInt32                          inNumberPacketDescriptions,
                                        const AudioStreamPacketDescription * __nullable inPacketDescs)
{
    AudioCapturerImpl *recorder = (__bridge AudioCapturerImpl *)(inUserData);
    bool ret = [recorder readerCallback:inBuffer];
    if(!ret) {
        return;
    }
}

- (size_t)indexOfQueueBuffer:(AudioQueueBufferRef)inBuffer
{
    for (size_t i = 0; i < CAPTURE_QUEUE_BUFFER_COUNT; i++) {
        if (audioQueueBuffers_[i] == inBuffer) {
            return i;
        }
    }
    return CAPTURE_QUEUE_BUFFER_COUNT;
}

- (bool)readerCallback:(AudioQueueBufferRef)inBuffer
{
    if(recordState_ != OHOS::AudioStandard::CAPTURER_RUNNING)
    {
        return false;
    }
    size_t index = [self indexOfQueueBuffer:inBuffer];
    if (index == CAPTURE_QUEUE_BUFFER_COUNT) {
        NSLog(@"readerCallback: unknown buffer, re-enqueue it.");
        AudioQueueEnqueueBuffer(audioQueue_, inBuffer, 0, NULL);
        return false;
    }
    {
        std::unique_lock<std::mutex> lk(readBufferLock_);
        if (stagingPending_[index]) {
            // JS has not consumed the previous frame in this slot. Drop this frame instead of
            // corrupting the data being read, and recycle the queue buffer immediately.
            NSLog(@"readerCallback: slot %zu still pending, frame dropped.", index);
            AudioQueueEnqueueBuffer(audioQueue_, inBuffer, 0, NULL);
            return true;
        }
        // Note: mAudioDataByteSize can be smaller than bufferSize_ for the final buffer on
        // stop/interrupt (short packet). It is passed through as-is; the napi layer clamps with
        // min(length, bufLength), so JS may observe a tail buffer shorter than bufferSize_.
        Byte* audiodata = (Byte*)inBuffer->mAudioData;
        errno_t cpyRet = memcpy_s(stagingBuffers_[index].get(), bufferSize_, audiodata,
                                  inBuffer->mAudioDataByteSize);
        if (cpyRet != EOK) {
            NSLog(@"readerCallback: memcpy_s failed %d, frame dropped.", (int)cpyRet);
            AudioQueueEnqueueBuffer(audioQueue_, inBuffer, 0, NULL);
            return true;
        }
        stagingDataLengths_[index] = inBuffer->mAudioDataByteSize;
        if (readCallback_ && !isLastRead_) {
            stagingPending_[index] = true;
            pendingSlots_.push_back(index);
        }
    }
    // OnReadData is invoked outside the lock: the napi layer answers it asynchronously on the
    // JS thread with getBufferDesc/enqueue, which take the same lock.
    if (readCallback_ && !isLastRead_) {
        readCallback_->OnReadData(inBuffer->mAudioDataByteSize);
        return true;
    } else {
        AudioQueueEnqueueBuffer(audioQueue_, inBuffer, 0, NULL);
        return false;
    }
}

- (bool)SetupAudioQueue
{
    OSStatus status = AudioQueueNewInput(&(audioDescription_), AudioRecordAQInputCallback,
                                        (__bridge void * _Nullable)(self), nil, nil, 0, &audioQueue_);
    if (status != 0) {
        NSLog(@"AudioQueueNewInput failed!==%d",(int)status);
        return false;
    }
    for (size_t i = 0; i < CAPTURE_QUEUE_BUFFER_COUNT; i++) {
        status = AudioQueueAllocateBuffer(audioQueue_, bufferSize_, &audioQueueBuffers_[i]);
        if (status != noErr) {
            NSLog(@"AudioQueueAllocateBuffer failed for buffer %zu: %d", i, (int)status);
            return false;
        }
        status = AudioQueueEnqueueBuffer(audioQueue_, audioQueueBuffers_[i], 0, NULL);
        if (status != noErr) {
            NSLog(@"AudioQueueEnqueueBuffer failed for buffer %zu: %d", i, (int)status);
            return false;
        }
    }
    AudioQueueCreateTimeline(audioQueue_, &timeLine_);
    return true;
}

- (const OHOS::AudioStandard::CapturerState)getStatus
{
    NSLog(@"getStatus recordState_:%d", recordState_);
    return recordState_;
}

- (void)transferState:(OHOS::AudioStandard::CapturerState) recordState
{
    NSLog(@"transferState oldState:%d, newState:%d", recordState_, recordState);

    OHOS::AudioStandard::CapturerState oldState = recordState_;
    recordState_ = recordState;

    if (oldState != recordState_) {
        if (stateCallback_) {
            stateCallback_->OnStateChange(recordState);
        }
        AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
        if (managerImpl) {
            [managerImpl updateCapturerChangeInfos];
        }
    }

    if (deviceWithInfoCallback_) {
        OHOS::AudioStandard::AudioCapturerChangeInfo changeInfo;
        [self getCurrentCapturerChangeInfo: changeInfo];
        deviceWithInfoCallback_->OnStateChange(changeInfo);
    }
}

- (bool)start
{
    if (audioQueue_ != nil) {
        if (!((recordState_ == OHOS::AudioStandard::CAPTURER_PREPARED) ||
            (recordState_ == OHOS::AudioStandard::CAPTURER_STOPPED))) {
            return false;
        }
        OSStatus status = AudioQueueStart(audioQueue_, NULL);
        if (status != noErr) {
            return false;
        }
        [self transferState:OHOS::AudioStandard::CAPTURER_RUNNING];
        return true;
    } else {
        return false;
    }
}

- (bool)stop
{
    if (audioQueue_ != nil) {
        if (!(recordState_ == OHOS::AudioStandard::CAPTURER_RUNNING)) {
            return false;
        }
        OSStatus status = AudioQueueStop(audioQueue_, true);
        if (status != noErr) {
            NSLog(@"AudioQueueStop failed");
            return false;
        }
        [self transferState:OHOS::AudioStandard::CAPTURER_STOPPED];
        return true;
    } else {
        return false;
    }
}

- (bool)releaseCapturer
{
    std::unique_lock<std::mutex> lk(readBufferLock_);
    isLastRead_ = true;
    if(recordState_ != OHOS::AudioStandard::CAPTURER_STOPPED) {
        OSStatus status = AudioQueueStop(audioQueue_, true);
        if (status != noErr) {
            NSLog(@"AudioQueueStop failed status = %d", status);
            return false;
        }
    }
    if (audioQueue_ != nil) {
        if (recordState_ == OHOS::AudioStandard::CAPTURER_RELEASED) {
            return false;
        }
        OSStatus status = AudioQueueDispose(audioQueue_, true);
        if (status != noErr) {
            NSLog(@"AudioQueueDispose failed status = %d", status);
            return false;
        }
        audioQueue_ = nullptr;
        for (size_t i = 0; i < CAPTURE_QUEUE_BUFFER_COUNT; i++) {
            audioQueueBuffers_[i] = nullptr;
            stagingBuffers_[i].reset();
            stagingDataLengths_[i] = 0;
            stagingPending_[i] = false;
        }
        pendingSlots_.clear();
        [[NSNotificationCenter defaultCenter] removeObserver:self
                                            name:AVAudioSessionRouteChangeNotification
                                            object:audioSession_];
        deviceCallback_ = nullptr;
        deviceWithInfoCallback_ = nullptr;
        [self transferState:OHOS::AudioStandard::CAPTURER_RELEASED];
        return true;
    } else {
        return false;
    }
}

- (int32_t)setCapturerReadCallback:(const std::shared_ptr<OHOS::AudioStandard::AudioCapturerReadCallback> &)callback
{
    readCallback_ = callback;
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)getBufferSize:(size_t &)bufferSize
{
    if(audioQueue_ != nil) {
        bufferSize = bufferSize_;
        return OHOS::AudioStandard::SUCCESS;
    } else {
        return OHOS::AudioStandard::ERR_ILLEGAL_STATE;
    }
}

- (int32_t)getAudioStreamId:(uint32_t &)sessionID
{
    if(audioQueue_ != nil) {
        sessionID = reinterpret_cast<uintptr_t>(audioQueue_);
        return OHOS::AudioStandard::SUCCESS;
    } else {
        return OHOS::AudioStandard::ERR_ILLEGAL_STATE;
    }
}

- (const int32_t)getBufferDesc:(OHOS::AudioStandard::BufferDesc &)bufDesc
{
    std::unique_lock<std::mutex> lk(readBufferLock_);
    if (pendingSlots_.empty()) {
        // No pending frame means abnormal call ordering (e.g. after release). Return an
        // error instead of SUCCESS so callers do not mistake a null buffer for valid data.
        NSLog(@"getBufferDesc: no pending buffer.");
        bufDesc.buffer = nullptr;
        bufDesc.bufLength = 0;
        bufDesc.dataLength = 0;
        return OHOS::AudioStandard::ERR_ILLEGAL_STATE;
    }
    size_t index = pendingSlots_.front();
    bufDesc.buffer = stagingBuffers_[index].get();
    bufDesc.bufLength = bufferSize_;
    // dataLength reflects the actual captured bytes, which may be smaller than bufLength
    // for the final short packet on stop/interrupt.
    bufDesc.dataLength = stagingDataLengths_[index];
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)enqueue:(const OHOS::AudioStandard::BufferDesc &)bufDesc
{
    size_t index = CAPTURE_QUEUE_BUFFER_COUNT;
    {
        std::unique_lock<std::mutex> lk(readBufferLock_);
        if (!pendingSlots_.empty() && stagingBuffers_[pendingSlots_.front()].get() == bufDesc.buffer) {
            index = pendingSlots_.front();
            pendingSlots_.pop_front();
            stagingPending_[index] = false;
        }
    }
    if (index == CAPTURE_QUEUE_BUFFER_COUNT) {
        NSLog(@"enqueue: buffer is not from us.");
        return OHOS::AudioStandard::ERR_ILLEGAL_STATE;
    }
    AudioQueueEnqueueBuffer(audioQueue_, audioQueueBuffers_[index], 0, NULL);
    return OHOS::AudioStandard::SUCCESS;
}

- (bool)getAudioTime:(OHOS::AudioStandard::Timestamp &)timestamp
{
    AudioTimeStamp avTimeStamp;
    OSStatus status = AudioQueueGetCurrentTime(audioQueue_, timeLine_, &avTimeStamp, NULL);
    if (status != noErr) {
        return false;
    }
    NSLog(@"AudioTimeStamp is %f",avTimeStamp.mSampleTime);
    timestamp.time.tv_nsec = avTimeStamp.mSampleTime * SEC_TO_NANOSECOND / audioDescription_.mSampleRate;
    timestamp.time.tv_sec = 0;
    return true;
}

- (int32_t)getCurrentInputDevices:(OHOS::AudioStandard::AudioDeviceDescriptor &)deviceInfo
{
    if (recordState_ != OHOS::AudioStandard::CAPTURER_RELEASED) {
        AVAudioSessionRouteDescription *routeDescription = audioSession_.currentRoute;
        NSLog(@"routeDescription count=%lu",[routeDescription.inputs count]);
        for (AVAudioSessionPortDescription *portDescription in routeDescription.inputs) {
            deviceInfo.deviceRole_ = OHOS::AudioStandard::DeviceRole::INPUT_DEVICE;
            ConvertDeviceTypeToOh(portDescription.portType, deviceInfo.deviceType_);
            deviceInfo.deviceName_ = std::string([portDescription.portName UTF8String]);
            deviceInfo.displayName_ = std::string([portDescription.UID UTF8String]);
            NSLog(@"deviceRole=%d,deviceType=%d, deviceName=%s, displayName=%s",
                deviceInfo.deviceRole_,deviceInfo.deviceType_, deviceInfo.deviceName_.c_str(), deviceInfo.displayName_.c_str());
            NSLog(@"channels=%lu",[portDescription.channels count]);
            for (AVAudioSessionChannelDescription *channelDescription in portDescription.channels) {
                deviceInfo.channelMasks_ |= channelDescription.channelLabel;
                NSLog(@"channelName=%@,channelNumber=%lu, owningPortUID=%@, channelLabel=%u",
                    channelDescription.channelName,channelDescription.channelNumber, channelDescription.owningPortUID,
                    channelDescription.channelLabel);
            }

            deviceInfo.GetDeviceStreamInfo().samplingRate.insert(
                static_cast<OHOS::AudioStandard::AudioSamplingRate>(audioSession_.sampleRate));
            NSLog(@"sampleRate=%f", audioSession_.sampleRate);
        }
        return OHOS::AudioStandard::SUCCESS;
    } else {
        return OHOS::AudioStandard::ERR_ILLEGAL_STATE;
    }
}

- (int32_t)getCurrentCapturerChangeInfo:(OHOS::AudioStandard::AudioCapturerChangeInfo &)changeInfo
{
    if (recordState_ != OHOS::AudioStandard::CAPTURER_RELEASED) {
        uint32_t sessionId;
        [self getAudioStreamId: sessionId];
        changeInfo.sessionId = sessionId;
        changeInfo.capturerState = [self getStatus];
        changeInfo.capturerInfo = capturerOptions_.capturerInfo;
        [self getCurrentInputDevices: changeInfo.inputDeviceInfo];
        return OHOS::AudioStandard::SUCCESS;
    } else {
        return OHOS::AudioStandard::ERR_ILLEGAL_STATE;
    }
}

- (int32_t)setCapturerCallback:(const std::shared_ptr<OHOS::AudioStandard::AudioCapturerCallback> &)callback
{
    stateCallback_ = callback;
    [[NSNotificationCenter defaultCenter] addObserver:self
                                         selector:@selector(handleAudioSessionInterruption:)
                                             name:AVAudioSessionInterruptionNotification
                                           object:audioSession_];
    return OHOS::AudioStandard::SUCCESS;
}

- (void)handleAudioSessionInterruption:(NSNotification *)notification
{
    NSDictionary *userInfo = notification.userInfo;
    AudioInterruption interruption;
    interruption.interruptionType = [userInfo[AVAudioSessionInterruptionTypeKey] integerValue];
    interruption.interruptionReason = [userInfo[AVAudioSessionInterruptionReasonKey] integerValue];
    interruption.interruptionOption = [userInfo[AVAudioSessionInterruptionOptionKey] integerValue];
    OHOS::AudioStandard::InterruptEvent interruptEvent;
    ConvertInterruptEventToOh(interruption, interruptEvent);

    if(stateCallback_ != nullptr) {
        stateCallback_->OnInterrupt(interruptEvent);
    }
}

- (int32_t)setAudioCapturerDeviceChangeCallback:
    (const std::shared_ptr<OHOS::AudioStandard::AudioCapturerDeviceChangeCallback> &)callback
{
    deviceCallback_ = callback;
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)removeAudioCapturerDeviceChangeCallback:
    (const std::shared_ptr<OHOS::AudioStandard::AudioCapturerDeviceChangeCallback> &)callback
{
    deviceCallback_ = nil;
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)setAudioCapturerInfoChangeCallback:
    (const std::shared_ptr<OHOS::AudioStandard::AudioCapturerInfoChangeCallback> &)callback
{
    deviceWithInfoCallback_ = callback;
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)removeAudioCapturerInfoChangeCallback:
    (const std::shared_ptr<OHOS::AudioStandard::AudioCapturerInfoChangeCallback> &)callback
{
    deviceWithInfoCallback_ = nil;
    return OHOS::AudioStandard::SUCCESS;
}

- (void)handleRouteChange:(NSNotification *)notification
{
    NSDictionary *userInfo = notification.userInfo;
    if (deviceCallback_) {
        OHOS::AudioStandard::AudioDeviceDescriptor deviceInfo;
        [self getCurrentInputDevices: deviceInfo];
        deviceCallback_->OnStateChange(deviceInfo);
    }

    if (deviceWithInfoCallback_) {
        OHOS::AudioStandard::AudioCapturerChangeInfo changeInfo;
        [self getCurrentCapturerChangeInfo: changeInfo];
        deviceWithInfoCallback_->OnStateChange(changeInfo);
    }

    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    if (managerImpl) {
        [managerImpl updateCapturerChangeInfos];
    }
}
@end
