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

#include <mutex>
#include "format_convert_util.h"
#import "audio_capturer_impl.h"
#import "audio_manager_impl.h"

#define BUFFER_SIZE 20000

#pragma mark - AudioCapturerImpl implementation
@implementation AudioCapturerImpl
{
    AudioStreamBasicDescription audioDescription_;
    AudioQueueRef audioQueue_;
    AudioQueueBufferRef audioQueueBuffer_;
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
    uint8_t tempBuffer_[BUFFER_SIZE];
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

- (void)initWithSampleRate:(const OHOS::AudioStandard::AudioCapturerOptions)capturerOptions
{
    capturerOptions_ = capturerOptions;
    recordState_ = OHOS::AudioStandard::CAPTURER_NEW;
    ConvertStreamInfoFromOh(capturerOptions.streamInfo, audioDescription_);
    recordState_ = OHOS::AudioStandard::CAPTURER_PREPARED;
    isLastRead_ = false;
    [self initChannelLayout:capturerOptions.streamInfo.channelLayout];
    NSLog(@"AudioStreamBasicDescription mSampleRate = %f, mFormatID = %u, mFormatFlags = %u, \
        mChannelsPerFrame = %u, mFramesPerPacket = %u, mBitsPerChannel = %u, mBytesPerFrame = %u, \
        mBytesPerPacket = %u",
        audioDescription_.mSampleRate, audioDescription_.mFormatID, audioDescription_.mFormatFlags,
        audioDescription_.mChannelsPerFrame, audioDescription_.mFramesPerPacket, audioDescription_.mBitsPerChannel,
        audioDescription_.mBytesPerFrame, audioDescription_.mBytesPerPacket);
    [self SetupAudioQueue];
    audioSession_ = [AVAudioSession sharedInstance];
    [audioSession_ setCategory:AVAudioSessionCategoryPlayAndRecord
                withOptions:AVAudioSessionCategoryOptionAllowBluetooth | AVAudioSessionCategoryOptionDefaultToSpeaker
                error:nil];
    [audioSession_ setActive:YES error:nil];
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    if (managerImpl) {
        [managerImpl updateCategory:audioSession_.category options:audioSession_.categoryOptions];
    }

    [[NSNotificationCenter defaultCenter] addObserver:self
                                            selector:@selector(handleRouteChange:)
                                            name:AVAudioSessionRouteChangeNotification
                                            object:audioSession_];
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

- (bool)readerCallback:(AudioQueueBufferRef)inBuffer
{
    if(recordState_ != OHOS::AudioStandard::CAPTURER_RUNNING)
    {
        return false;
    }
    Byte* audiodata = (Byte*)inBuffer->mAudioData;
    memcpy(tempBuffer_, audiodata, inBuffer->mAudioDataByteSize);
    if (readCallback_ && !isLastRead_) {
        std::unique_lock<std::mutex> lk(readBufferLock_);
        readCallback_->OnReadData(inBuffer->mAudioDataByteSize);
        return true;
    } else {
        return false;
    }
}

- (void)SetupAudioQueue
{
    OSStatus status = AudioQueueNewInput(&(audioDescription_), AudioRecordAQInputCallback,
                                        (__bridge void * _Nullable)(self), nil, nil, 0, &audioQueue_);
    if (status != 0) {
        NSLog(@"AudioQueueNewInput failed!==%d",(int)status);
        return;
    }
    AudioQueueAllocateBuffer(audioQueue_, BUFFER_SIZE, &audioQueueBuffer_);
    AudioQueueCreateTimeline(audioQueue_, &timeLine_);
    AudioQueueEnqueueBuffer(audioQueue_, audioQueueBuffer_,0,NULL);
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
        bufferSize = BUFFER_SIZE;
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
    NSLog(@"getBufferDesc bufLength = %zu, tempBuffer_ =  %s", BUFFER_SIZE, tempBuffer_);
    bufDesc.buffer = tempBuffer_;
    bufDesc.bufLength = BUFFER_SIZE;
    bufDesc.dataLength = BUFFER_SIZE;
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)enqueue:(const OHOS::AudioStandard::BufferDesc &)bufDesc
{
    if (tempBuffer_ != bufDesc.buffer) {
        NSLog(@"buffer is not from us");
        return 0;
    }
    AudioQueueEnqueueBuffer(audioQueue_, audioQueueBuffer_,0,NULL);
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
