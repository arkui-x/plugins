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

#include <condition_variable>
#include <mutex>
#include "audio_errors.h"
#include "format_convert_util.h"
#import "audio_renderer_impl.h"
#import "audio_manager_impl.h"

#define QUEUE_BUFFER_SIZE 2
#define MIN_STREAM_VOLUME 0.0
#define MAX_STREAM_VOLUME 1.0
#define WRITE_BUFFER_TIMEOUT_IN_MS 200
#define MIN_BUFFER_SIZE 20000
#define ALLOC_BUFFER_SIZE 200000

#pragma mark - AudioRendererImpl implementation
@implementation AudioRendererImpl
{
    OHOS::AudioStandard::AudioRendererOptions rendererOptions_;
    AudioStreamBasicDescription audioDescription_;
    AudioQueueRef audioQueue_;
    AudioQueueTimelineRef timeLine_;
    AudioQueueBufferRef audioQueueBuffers_[QUEUE_BUFFER_SIZE];
    BOOL audioQueueUsed_[QUEUE_BUFFER_SIZE];
    dispatch_queue_t playQueue_;
    uint8_t tempBuffer_[MIN_BUFFER_SIZE];
    OHOS::AudioStandard::RendererState playState_;
    std::mutex writeBufferLock_;
    std::condition_variable writeThreadCv_;
    bool isWrited_;
    OHOS::AudioStandard::InterruptMode interruptMode_;
    std::shared_ptr<OHOS::AudioStandard::AudioRendererCallback> stateCallback_;
    std::shared_ptr<OHOS::AudioStandard::AudioRendererWriteCallback> writeCallback_;
    std::shared_ptr<OHOS::AudioStandard::AudioRendererDeviceChangeCallback> deviceCallback_;
    std::shared_ptr<OHOS::AudioStandard::AudioRendererOutputDeviceChangeCallback> deviceWithInfoCallback_;
    OHOS::AudioStandard::AudioRenderMode renderMode_;
    std::mutex writeCbMutex_;
}

- (void)initChannelLayout:(const OHOS::AudioStandard::AudioChannelLayout)channelLayout
{
    AudioChannelLayout acl;
    bool isValid = ConvertChannelLayoutFromOh(channelLayout, acl);
    if (isValid) {
        NSLog(@"AudioChannelLayout.mChannelLayoutTag = %d", acl.mChannelLayoutTag);
        OSStatus status = AudioQueueSetProperty(audioQueue_, kAudioQueueProperty_ChannelLayout, &acl, sizeof(acl));
        if (status != noErr) {
            NSLog(@"Failed to AudioQueueSetProperty kAudioQueueProperty_ChannelLayout: status = %d", status);
        }
    } else {
        NSLog(@"Unknown channel layout.");
    }
}

- (void)initWithSampleRate:(const OHOS::AudioStandard::AudioRendererOptions &)rendererOptions
{
    NSLog(@"initWithSampleRate samplingRate = %d, channels = %d, format = %d",
        rendererOptions.streamInfo.samplingRate, rendererOptions.streamInfo.channels,
        rendererOptions.streamInfo.format);
    renderMode_ = OHOS::AudioStandard::RENDER_MODE_NORMAL;
    rendererOptions_ = rendererOptions;
    playQueue_ = dispatch_queue_create("audio queue play queue", DISPATCH_QUEUE_SERIAL);
    playState_ = OHOS::AudioStandard::RENDERER_NEW;
    ConvertStreamInfoFromOh(rendererOptions.streamInfo, audioDescription_);
    NSLog(@"AudioStreamBasicDescription mSampleRate = %f, mFormatID = %u, mFormatFlags = %u, \
        mChannelsPerFrame = %u, mFramesPerPacket = %u, mBitsPerChannel = %u, mBytesPerFrame = %u, \
        mBytesPerPacket = %u",
        audioDescription_.mSampleRate, audioDescription_.mFormatID, audioDescription_.mFormatFlags,
        audioDescription_.mChannelsPerFrame, audioDescription_.mFramesPerPacket, audioDescription_.mBitsPerChannel,
        audioDescription_.mBytesPerFrame, audioDescription_.mBytesPerPacket);

    [self setupAudioQueue];
    [self initChannelLayout:rendererOptions.streamInfo.channelLayout];

    UInt32 enable = 1;
    OSStatus status = AudioQueueSetProperty(audioQueue_, kAudioQueueProperty_EnableTimePitch, &enable, sizeof(enable));
    if (status != noErr) {
        NSLog(@"Failed to AudioQueueSetProperty kAudioQueueProperty_EnableTimePitch: status = %d", status);
    }
    UInt32 algorithm = kAudioQueueTimePitchAlgorithm_Spectral;
    status = AudioQueueSetProperty(audioQueue_, kAudioQueueProperty_TimePitchAlgorithm, &algorithm, sizeof(algorithm));
    if (status != noErr) {
        NSLog(@"Failed to AudioQueueSetProperty kAudioQueueProperty_TimePitchAlgorithm: status = %d", status);
    }

    playState_ = OHOS::AudioStandard::RENDERER_PREPARED;
    isWrited_ = false;
    interruptMode_ = OHOS::AudioStandard::SHARE_MODE;
    AVAudioSession *audioSession = [AVAudioSession sharedInstance];
    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    if (managerImpl) {
        if ([managerImpl getAllInterruptMode] == OHOS::AudioStandard::SHARE_MODE &&
            [audioSession.category isEqualToString:AVAudioSessionCategorySoloAmbient]) {
            NSLog(@"old category = %@",audioSession.category);
            [audioSession setCategory:AVAudioSessionCategoryAmbient error:nil];
            NSLog(@"new category = %@",audioSession.category);
            [audioSession setActive:YES error:nil];
            [managerImpl updateCategory:audioSession.category options:audioSession.categoryOptions];
        }
    }

    [[NSNotificationCenter defaultCenter] addObserver:self
                                            selector:@selector(handleRouteChange:)
                                            name:AVAudioSessionRouteChangeNotification
                                            object:audioSession];
}

- (int32_t)getRendererInfo:(OHOS::AudioStandard::AudioRendererInfo &)rendererInfo
{
    rendererInfo = rendererOptions_.rendererInfo;
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)getStreamInfo:(OHOS::AudioStandard::AudioStreamInfo &)streamInfo
{
    if (playState_ != OHOS::AudioStandard::RENDERER_RELEASED) {
        streamInfo = rendererOptions_.streamInfo;
        return OHOS::AudioStandard::SUCCESS;
    } else {
        return OHOS::AudioStandard::ERR_OPERATION_FAILED;
    }
}

- (int32_t)setRendererCallback:(const std::shared_ptr<OHOS::AudioStandard::AudioRendererCallback> &)callback
{
    stateCallback_ = callback;
    [[NSNotificationCenter defaultCenter] addObserver:self
                                            selector:@selector(handleAudioSessionInterruption:)
                                            name:AVAudioSessionInterruptionNotification
                                            object:nil];
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)setRenderMode:(OHOS::AudioStandard::AudioRenderMode)renderMode
{
    NSLog(@"SetRenderMode to %s", renderMode == OHOS::AudioStandard::AudioRenderMode::RENDER_MODE_NORMAL ?
        "RENDER_MODE_NORMAL" : "RENDER_MODE_CALLBACK");
    if (renderMode_ == renderMode) {
        return OHOS::AudioStandard::SUCCESS;
    }

    // renderMode_ is inited as RENDER_MODE_NORMAL, can only be set to RENDER_MODE_CALLBACK.
    if (renderMode_ == OHOS::AudioStandard::RENDER_MODE_CALLBACK &&
        renderMode == OHOS::AudioStandard::RENDER_MODE_NORMAL) {
        NSLog(@"SetRenderMode from callback to normal is not supported.");
        return OHOS::AudioStandard::ERR_INCORRECT_MODE;
    }

    // state check
    if (playState_ != OHOS::AudioStandard::RENDERER_PREPARED && playState_ != OHOS::AudioStandard::RENDERER_NEW) {
        NSLog(@"SetRenderMode failed. invalid state");
        return OHOS::AudioStandard::ERR_ILLEGAL_STATE;
    }
    renderMode_ = renderMode;
    dispatch_async(playQueue_, ^{
        int32_t tryCount = QUEUE_BUFFER_SIZE;
        while (tryCount > 0) {
            isWrited_ = false;
            writeCallback_->OnWriteData(MIN_BUFFER_SIZE);
            std::unique_lock<std::mutex> lk(writeBufferLock_);
            writeThreadCv_.wait_for(lk, std::chrono::milliseconds(WRITE_BUFFER_TIMEOUT_IN_MS),
                [self]{ return isWrited_; });
            NSLog(@"queue isWrited_:%d", isWrited_);

            if (isWrited_) {
                tryCount--;
            }
        }
    });
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)setRendererWriteCallback:(const std::shared_ptr<OHOS::AudioStandard::AudioRendererWriteCallback> &)callback
{
    if (renderMode_ != OHOS::AudioStandard::RENDER_MODE_CALLBACK) {
        return OHOS::AudioStandard::ERR_INCORRECT_MODE;
        NSLog(@"incorrect render mode");
    }
    std::lock_guard<std::mutex> lock(writeCbMutex_);
    writeCallback_ = callback;
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)setRendererDeviceChangeCallback:
    (const std::shared_ptr<OHOS::AudioStandard::AudioRendererDeviceChangeCallback> &)callback
{
    deviceCallback_ = callback;
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)destroyRendererDeviceChangeCallback
{
    deviceCallback_ = nil;
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)setRendererDeviceChangeWithInfoCallback:
    (const std::shared_ptr<OHOS::AudioStandard::AudioRendererOutputDeviceChangeCallback> &)callback
{
    deviceWithInfoCallback_ = callback;
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)destroyRendererDeviceChangeWithInfoCallback
{
    deviceWithInfoCallback_ = nil;
    return OHOS::AudioStandard::SUCCESS;
}

- (void)handleRouteChange:(NSNotification *)notification
{
    NSDictionary *userInfo = notification.userInfo;

    if (deviceCallback_) {
        NSLog(@"OnStateChange");
        OHOS::AudioStandard::AudioDeviceDescriptor deviceInfo;
        [self getCurrentOutputDevices:deviceInfo];
        deviceCallback_->OnStateChange(deviceInfo);
    }

    if (deviceWithInfoCallback_) {
        NSLog(@"OnOutputDeviceChange");
        NSInteger reason = [userInfo[AVAudioSessionRouteChangeReasonKey] integerValue];
        NSLog(@"reason = %ld", reason);
        OHOS::AudioStandard::AudioDeviceDescriptor deviceInfo;
        [self getCurrentOutputDevices:deviceInfo];
        OHOS::AudioStandard::AudioStreamDeviceChangeReason changeReason;
        ConvertDeviceChangeReasonToOh(reason, changeReason);
        deviceWithInfoCallback_->OnOutputDeviceChange(deviceInfo, changeReason);
    }

    AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
    if (managerImpl) {
        [managerImpl updateRendererChangeInfos];
    }
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

    if(stateCallback_) {
        stateCallback_->OnInterrupt(interruptEvent);
    }
}

static void AudioPlayerAQInputCallback(void* inUserData,AudioQueueRef outQ, AudioQueueBufferRef outQB)
{
    AudioRendererImpl* player = (__bridge AudioRendererImpl*)inUserData;
    [player playerCallback:outQB];
}

- (void)setupAudioQueue
{
    OSStatus status = AudioQueueNewOutput(&audioDescription_, AudioPlayerAQInputCallback,
        (__bridge void*)self, nil, nil, 0, &audioQueue_);
    if (status != noErr) {
        NSLog(@"Failed to AudioQueueNewOutput: status = %d", status);
        return;
    }

    for (int i = 0; i < QUEUE_BUFFER_SIZE; i++) {
        status = AudioQueueAllocateBuffer(audioQueue_, ALLOC_BUFFER_SIZE, &audioQueueBuffers_[i]);
        if (status != noErr) {
            NSLog(@"Failed to AudioQueueAllocateBuffer: i = %d, status = %d", i, status);
        }
    }

    status = AudioQueueCreateTimeline(audioQueue_, &timeLine_);
    if(status != noErr) {
        NSLog(@"Failed to AudioQueueCreateTimeline: status = %d", status);
    }
}

- (void)playerCallback:(AudioQueueBufferRef)outQB
{
    for (int i = 0; i < QUEUE_BUFFER_SIZE; i++) {
        if (outQB == audioQueueBuffers_[i]) {
            audioQueueUsed_[i] = NO;
            NSLog(@"playerCallback buffer i = %d", i);
        }
    }

    if (writeCallback_) {
        writeCallback_->OnWriteData(MIN_BUFFER_SIZE);
    }
}

- (AudioQueueBufferRef)getNotUsedBuffer
{
    for (int i = 0; i < QUEUE_BUFFER_SIZE; i++) {
        if (NO == audioQueueUsed_[i]) {
            audioQueueUsed_[i] = YES;
            NSLog(@"getNotUsedBuffer i = %d", i);
            return audioQueueBuffers_[i];
        }
    }
    return nil;
}

- (const int32_t)enqueue:(const OHOS::AudioStandard::BufferDesc &)bufDesc
{   
    std::unique_lock<std::mutex> lk(writeBufferLock_);
    isWrited_ = true;
    writeThreadCv_.notify_all();

    AudioQueueBufferRef audioQueueBuffer = nil;
    audioQueueBuffer = [self getNotUsedBuffer];
    if (audioQueueBuffer == nil) {
        NSLog(@"Find't no used buffer");
        return OHOS::AudioStandard::ERR_WRITE_BUFFER;
    }

    audioQueueBuffer->mAudioDataByteSize = bufDesc.dataLength;

    Byte* audiodata = (Byte*)audioQueueBuffer->mAudioData;

    memcpy(audiodata, bufDesc.buffer, bufDesc.dataLength);

    OSStatus status = AudioQueueEnqueueBuffer(audioQueue_, audioQueueBuffer, 0, nil);
    if (status != 0) {
        NSLog(@"Failed to AudioQueueEnqueueBuffer: status = %d", status);
        return ConvertErrorToOh(status);
    } else {
        return OHOS::AudioStandard::SUCCESS;
    }
}

- (const int32_t)getBufferDesc:(OHOS::AudioStandard::BufferDesc &)bufDesc
{
    bufDesc.buffer = tempBuffer_;
    bufDesc.bufLength = MIN_BUFFER_SIZE;
    bufDesc.dataLength = MIN_BUFFER_SIZE;
    return OHOS::AudioStandard::SUCCESS;
}

- (void)transferState:(OHOS::AudioStandard::RendererState) playState
{
    NSLog(@"transferState oldState = %d, newState = %d", playState_, playState);
    OHOS::AudioStandard::RendererState oldState = playState_;
    playState_ = playState;
    if (oldState != playState_) {
        if (stateCallback_) {
            stateCallback_->OnStateChange(playState);
        }
        AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
        if (managerImpl) {
            [managerImpl updateRendererChangeInfos];
        }
    }
}

- (const OHOS::AudioStandard::RendererState) getStatus
{
    NSLog(@"getStatus playState_:%d", playState_);
    return playState_;
}

- (bool)start
{
    if (audioQueue_ != nil) {
        if (!((playState_ == OHOS::AudioStandard::RENDERER_PREPARED) ||
            (playState_ == OHOS::AudioStandard::RENDERER_STOPPED) ||
            (playState_ == OHOS::AudioStandard::RENDERER_PAUSED))) {
            NSLog(@"Start failed. Illegal state:%d", playState_);
            return false;
        }
        OSStatus status = AudioQueueStart(audioQueue_, nil);
        if (status != noErr) {
            NSLog(@"Failed to AudioQueueStart: status = %d", status);
            return false;
        }
        [self transferState:OHOS::AudioStandard::RENDERER_RUNNING];
        return true;
    } else {
        return false;
    }
}

- (bool)stop
{
    if (audioQueue_ != nil) {
        if (!((playState_ == OHOS::AudioStandard::RENDERER_RUNNING) ||
            (playState_ == OHOS::AudioStandard::RENDERER_PAUSED))) {
            NSLog(@"State of stream is not running. Illegal state:%d", playState_);
            return false;
        }

        OSStatus status = AudioQueueStop(audioQueue_, true);
        if (status != noErr) {
            NSLog(@"Failed to AudioQueueStop: status = %d", status);
            return false;
        }
        [self transferState:OHOS::AudioStandard::RENDERER_STOPPED];
        return true;
    } else {
        return false;
    }
}

- (bool)drain
{
    if (playState_ != OHOS::AudioStandard::RENDERER_RUNNING) {
        NSLog(@"State is not RUNNING. Illegal state:%d", playState_);
        return false;
    }
    if (audioQueue_ != nil) {
        OSStatus status = AudioQueueReset(audioQueue_);
        if (status != noErr) {
            NSLog(@"Failed to AudioQueueReset: status = %d", status);
            return false;
        }
        return true;
    } else {
        return false;
    }
}

- (bool)flush
{
    if (!((playState_ == OHOS::AudioStandard::RENDERER_RUNNING) ||
        (playState_ == OHOS::AudioStandard::RENDERER_PAUSED) ||
        (playState_ == OHOS::AudioStandard::RENDERER_STOPPED))) {
        NSLog(@"State is not RUNNING. Illegal state:%d", playState_);
        return false;
    }
    if (audioQueue_ != nil) {
        OSStatus status = AudioQueueFlush(audioQueue_);
        if (status != noErr) {
            NSLog(@"Failed to AudioQueueFlush: %d", status);
            return false;
        }
        return true;
    } else {
        return false;
    }
}

- (bool)pause
{
    if (audioQueue_ != nil) {
        if (playState_ != OHOS::AudioStandard::RENDERER_RUNNING) {
            NSLog(@"State of stream is not running. Illegal state:%d", playState_);
            return false;
        }
        OSStatus status = AudioQueuePause(audioQueue_);
        if (status != noErr) {
            NSLog(@"Failed to AudioQueuePause: %d", status);
            return false;
        }
        [self transferState:OHOS::AudioStandard::RENDERER_PAUSED];
        return true;
    } else {
        return false;
    }
}

- (bool)releaseRenderer
{
    std::unique_lock<std::mutex> lk(writeBufferLock_);

    if (playState_ == OHOS::AudioStandard::RENDERER_RELEASED) {
        NSLog(@"Already released, do nothing.");
        return false;
    }
    if (audioQueue_ != nil) {
        OSStatus status = AudioQueueStop(audioQueue_, true);
        if (status != noErr) {
            NSLog(@"Failed to AudioQueueStop: %d", status);
            return false;
        }
        for (int i = 0; i < QUEUE_BUFFER_SIZE; i++) {
            status = AudioQueueFreeBuffer(audioQueue_, audioQueueBuffers_[i]);
            if (status != noErr) {
                NSLog(@"Failed to AudioQueueFreeBuffer: i = %d, status = %d", i, status);
                return false;
            }
            audioQueueBuffers_[i] = nil;
        }
        status = AudioQueueDispose(audioQueue_, true);
        if (status != noErr) {
            NSLog(@"Failed to AudioQueueDispose: status = %d", status);
            return false;
        }
        audioQueue_ = nil;
    }

    AVAudioSession *audioSession = [AVAudioSession sharedInstance];
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                            name:AVAudioSessionRouteChangeNotification
                                            object:audioSession];
    deviceCallback_ = nil;
    deviceWithInfoCallback_ = nil;

    [self transferState:OHOS::AudioStandard::RENDERER_RELEASED];
    return true;
}

- (bool)getAudioTime:(OHOS::AudioStandard::Timestamp &)time
{
    AudioTimeStamp timeStamp;
    OSStatus status = AudioQueueGetCurrentTime(audioQueue_, timeLine_, &timeStamp, nil);
    if (status != noErr) {
        NSLog(@"Failed to AudioQueueGetCurrentTime: status = %d", status);
        return false;
    }
    time.time.tv_nsec = timeStamp.mSampleTime * SEC_TO_NANOSECOND / audioDescription_.mSampleRate;
    time.time.tv_sec = 0;
    NSLog(@"tv_nsec = %lu", time.time.tv_nsec);
    return true;
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

- (int32_t)getBufferSize:(size_t &)bufferSize
{
    if(audioQueue_ != nil) {
        bufferSize = MIN_BUFFER_SIZE;
        return OHOS::AudioStandard::SUCCESS;
    } else {
        return OHOS::AudioStandard::ERR_ILLEGAL_STATE;
    }
}

- (int32_t)setSpeed:(float)speed
{
    OSStatus status = AudioQueueSetParameter(audioQueue_, kAudioQueueParam_PlayRate, speed);
    if (status != noErr) {
        NSLog(@"Failed to AudioQueueSetParameter kAudioQueueParam_PlayRate: status = %d", status);
        return ConvertErrorToOh(status);
    } else {
        return OHOS::AudioStandard::SUCCESS;
    }
}

- (float)getSpeed
{
    float speed;
    OSStatus status = AudioQueueGetParameter(audioQueue_, kAudioQueueParam_PlayRate, &speed);
    if (status != noErr) {
        NSLog(@"Failed to AudioQueueGetParameter kAudioQueueParam_PlayRate: status = %d", status);
    }
    return speed;
}

- (int32_t)setVolume:(float)volume
{
    OSStatus status = AudioQueueSetParameter(audioQueue_, kAudioQueueParam_Volume, volume);
    if (status != noErr) {
        NSLog(@"Failed to AudioQueueSetParameter kAudioQueueParam_Volume: status = %d", status);
        return ConvertErrorToOh(status);
    } else {
        return OHOS::AudioStandard::SUCCESS;
    }
}

- (float)getMinStreamVolume
{
    return MIN_STREAM_VOLUME;
}

- (float)getMaxStreamVolume
{
    return MAX_STREAM_VOLUME;
}

- (int32_t)getCurrentOutputDevices:(OHOS::AudioStandard::AudioDeviceDescriptor &)deviceInfo
{
    if (playState_ != OHOS::AudioStandard::RENDERER_RELEASED) {
        AVAudioSession *audioSession = [AVAudioSession sharedInstance];
        AVAudioSessionRouteDescription *routeDescription = audioSession.currentRoute;

        NSLog(@"routeDescription count = %lu",[routeDescription.outputs count]);
        for (AVAudioSessionPortDescription *portDescription in routeDescription.outputs) {
            deviceInfo.deviceRole_ = OHOS::AudioStandard::DeviceRole::OUTPUT_DEVICE;
            ConvertDeviceTypeToOh(portDescription.portType, deviceInfo.deviceType_);
            deviceInfo.deviceName_ = std::string([portDescription.portName UTF8String]);
            deviceInfo.displayName_ = std::string([portDescription.UID UTF8String]);
            NSLog(@"portType = %@, portName = %@, UID = %@", portDescription.portType, portDescription.portName,
                portDescription.UID);
            NSLog(@"channels = %lu",[portDescription.channels count]);
            for (AVAudioSessionChannelDescription *channelDescription in portDescription.channels) {
                deviceInfo.channelMasks_ |= channelDescription.channelLabel;
                NSLog(@"channelName = %@, channelNumber = %lu, owningPortUID = %@, channelLabel = %u",
                    channelDescription.channelName,channelDescription.channelNumber,
                    channelDescription.owningPortUID, channelDescription.channelLabel);
            }

            deviceInfo.GetDeviceStreamInfo().samplingRate.insert(
                static_cast<OHOS::AudioStandard::AudioSamplingRate>(audioSession.sampleRate));
            NSLog(@"sampleRate = %f", audioSession.sampleRate);
        }
    }

    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)setVolumeWithRamp:(float)volume rampTime:(int32_t)duration
{
    NSLog(@"setVolumeWithRamp: volume = %f, duration(sec) = %d", volume, duration / SEC_TO_MILLISECOND);
    OSStatus status = AudioQueueSetParameter(audioQueue_, kAudioQueueParam_VolumeRampTime, duration / SEC_TO_MILLISECOND);
    if (status != noErr) {
        NSLog(@"Failed to AudioQueueSetParameter kAudioQueueParam_VolumeRampTime: status = %d", status);
        return ConvertErrorToOh(status);
    }

    status = AudioQueueSetParameter(audioQueue_, kAudioQueueParam_Volume, volume);
    if (status != noErr) {
        NSLog(@"Failed to AudioQueueSetParameter kAudioQueueParam_Volume: status = %d", status);
        return ConvertErrorToOh(status);
    } else {
        return OHOS::AudioStandard::SUCCESS;
    }
}

- (void)setInterruptMode:(OHOS::AudioStandard::InterruptMode)mode
{
    if (mode == OHOS::AudioStandard::SHARE_MODE) {
        AVAudioSession *audioSession = [AVAudioSession sharedInstance];
        if ([audioSession.category isEqualToString:AVAudioSessionCategorySoloAmbient]) {
            NSLog(@"old category = %@",audioSession.category);
            [audioSession setCategory:AVAudioSessionCategoryAmbient error:nil];
            NSLog(@"new category = %@",audioSession.category);
            [audioSession setActive:YES error:nil];
            interruptMode_ = mode;
            AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
            if (managerImpl) {
                [managerImpl updateCategory:audioSession.category options:audioSession.categoryOptions];
            }
        }
    } else if (mode == OHOS::AudioStandard::INDEPENDENT_MODE) {
        AVAudioSession *audioSession = [AVAudioSession sharedInstance];
        if ([audioSession.category isEqualToString:AVAudioSessionCategoryAmbient]) {
            NSLog(@"old category = %@",audioSession.category);
            [audioSession setCategory:AVAudioSessionCategorySoloAmbient error:nil];
            NSLog(@"new category = %@",audioSession.category);
            [audioSession setActive:YES error:nil];
            interruptMode_ = mode;
            AudioManagerImpl *managerImpl = [AudioManagerImpl sharedInstance];
            if (managerImpl) {
                [managerImpl updateCategory:audioSession.category options:audioSession.categoryOptions];
            }
        }
    }
}

- (OHOS::AudioStandard::InterruptMode)getInterruptMode
{
    return interruptMode_;
}
@end
