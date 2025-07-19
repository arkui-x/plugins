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

#include "audio_errors.h"
#import "format_convert_util.h"
#import "audio_manager_impl.h"

#define MIN_VOLUME 0
#define MAX_VOLUME 100

#pragma mark - AudioManagerImpl implementation
@implementation AudioManagerImpl
{
    bool communicationDeviceActive_;
    AVAudioSessionCategory savedCategory_;
    AVAudioSessionCategoryOptions savedCategoryOptions_;
    std::vector<AudioRendererImpl *> rendererList_;
    std::vector<AudioCapturerImpl *> capturerList_;
    std::shared_ptr<OHOS::AudioStandard::VolumeKeyEventCallback> volumeCallback_;
    std::shared_ptr<OHOS::AudioStandard::AudioRendererStateChangeCallback> rendererChangeCallback_;
    std::shared_ptr<OHOS::AudioStandard::AudioCapturerStateChangeCallback> capturerChangeCallback_;
    bool isRouteChangeNotification_;
    std::shared_ptr<OHOS::AudioStandard::AudioPreferredOutputDeviceChangeCallback> outputDeviceCallback_;
    std::shared_ptr<OHOS::AudioStandard::AudioPreferredInputDeviceChangeCallback> inputDeviceCallback_;
}

static AudioManagerImpl *sharedInstance = nil;

+ (AudioManagerImpl *)sharedInstance {
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[AudioManagerImpl alloc] init];
    });
    return sharedInstance;
}

- (const OHOS::AudioStandard::AudioScene)getAudioScene
{
    AVAudioSession *audioSession = [AVAudioSession sharedInstance];
    NSLog(@"AVAudioSession mode = %@", audioSession.mode);
    if ([audioSession.mode isEqualToString:AVAudioSessionModeVoiceChat]) {
        return OHOS::AudioStandard::AUDIO_SCENE_PHONE_CHAT;
    } else {
        return OHOS::AudioStandard::AUDIO_SCENE_DEFAULT;
    }
}

- (int32_t)registerVolumeKeyEventCallback:
    (const std::shared_ptr<OHOS::AudioStandard::VolumeKeyEventCallback> &)callback
{
    volumeCallback_ = callback;
    AVAudioSession *session = [AVAudioSession sharedInstance];
    [session setActive:YES error:nil];
    [session addObserver:self forKeyPath:@"outputVolume" options:NSKeyValueObservingOptionNew context:nil];
    return OHOS::AudioStandard::SUCCESS;
}

- (void)observeValueForKeyPath:
    (NSString *)keyPath ofObject:(id)object change:(NSDictionary *)change context:(void *)context
{
    if (volumeCallback_ && [keyPath isEqual:@"outputVolume"]) {
        NSLog(@"observeValueForKeyPath outputVolume: %@", change);
        id newValue = change[NSKeyValueChangeNewKey];
        if ([newValue isKindOfClass:[NSNumber class]]) {
            float floatValue = [newValue floatValue];
            OHOS::AudioStandard::VolumeEvent volumeEvent;
            volumeEvent.volume = floatValue * MAX_VOLUME;
            volumeCallback_->OnVolumeKeyEvent(volumeEvent);
        } else {
            NSLog(@"Observe value is not NSNumber.");
        }
    }
}

- (std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>>)getDevices:
    (OHOS::AudioStandard::DeviceFlag)deviceFlag
{
    std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> descriptors = {};
    if (deviceFlag == OHOS::AudioStandard::INPUT_DEVICES_FLAG) {
        AVAudioSession *audioSession = [AVAudioSession sharedInstance];
        NSLog(@"AVAudioSession availableInputs count = %lu",[audioSession.availableInputs count]);
        for (AVAudioSessionPortDescription *portDescription in audioSession.availableInputs) {
            std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> descriptor =
                [self getDeviceInfo:portDescription role:OHOS::AudioStandard::DeviceRole::INPUT_DEVICE];
            descriptor->GetDeviceStreamInfo().samplingRate.clear();
            descriptors.push_back(descriptor);
        }
    }
    return descriptors;
}

- (std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>)getDeviceInfo:
    (AVAudioSessionPortDescription *)portDescription role:(OHOS::AudioStandard::DeviceRole)deviceRole
{
    AVAudioSession *audioSession = [AVAudioSession sharedInstance];
    NSLog(@"portDescription = %@",portDescription);
    std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor> descriptor =
        std::make_shared<OHOS::AudioStandard::AudioDeviceDescriptor>();
    descriptor->deviceRole_ = deviceRole;
    ConvertDeviceTypeToOh(portDescription.portType, descriptor->deviceType_);
    descriptor->deviceName_ = std::string([portDescription.portName UTF8String]);
    descriptor->displayName_ = std::string([portDescription.UID UTF8String]);
    NSLog(@"portType = %@, portName = %@, UID = %@", portDescription.portType, portDescription.portName,
        portDescription.UID);
    NSLog(@"channels = %lu",[portDescription.channels count]);
    for (AVAudioSessionChannelDescription *channelDescription in portDescription.channels) {
        descriptor->channelMasks_ |= channelDescription.channelLabel;
        NSLog(@"channelName = %@, channelNumber = %lu, owningPortUID = %@, channelLabel = %u",
            channelDescription.channelName,channelDescription.channelNumber,
            channelDescription.owningPortUID, channelDescription.channelLabel);
    }

    descriptor->GetDeviceStreamInfo().samplingRate.insert(
        static_cast<OHOS::AudioStandard::AudioSamplingRate>(audioSession.sampleRate));
    NSLog(@"sampleRate = %f", audioSession.sampleRate);
    return descriptor;
}

- (int32_t)setDeviceActive:(OHOS::AudioStandard::DeviceType)deviceType active:(bool)flag
{
    NSLog(@"setDeviceActive deviceType = %d, flag = %d",deviceType, flag);
    if (deviceType == OHOS::AudioStandard::DEVICE_TYPE_SPEAKER) {
        AVAudioSession *audioSession = [AVAudioSession sharedInstance];
        if (!communicationDeviceActive_) {
            savedCategory_ = audioSession.category;
            savedCategoryOptions_ = audioSession.categoryOptions;
            NSLog(@"Saved category = %@ categoryOptions = %lu",audioSession.category,audioSession.categoryOptions);
        }
        if (flag) {
            [audioSession setCategory:AVAudioSessionCategoryPlayAndRecord
                withOptions:AVAudioSessionCategoryOptionDefaultToSpeaker
                error:nil];
            [audioSession setActive:YES error:nil];
            communicationDeviceActive_ = true;
        } else {
            [audioSession setCategory:savedCategory_ withOptions:savedCategoryOptions_ error:nil];
            [audioSession setActive:YES error:nil];
            communicationDeviceActive_ = false;
        }
        return OHOS::AudioStandard::SUCCESS;
    } else {
        NSLog(@"deviceType = %d is not supported",deviceType);
        return OHOS::AudioStandard::ERR_NOT_SUPPORTED;
    }
}

- (bool)isDeviceActive:(OHOS::AudioStandard::DeviceType)deviceType
{
    if (deviceType == OHOS::AudioStandard::DEVICE_TYPE_SPEAKER) {
        AVAudioSession *audioSession = [AVAudioSession sharedInstance];
        AVAudioSessionRouteDescription *routeDescription = audioSession.currentRoute;
        for (AVAudioSessionPortDescription *portDescription in routeDescription.outputs) {
            if ([portDescription.portType isEqualToString:AVAudioSessionPortBuiltInSpeaker]) {
                return true;
            }
        }
    }
    return false;
}

- (void)updateCategory:(AVAudioSessionCategory)category options:(AVAudioSessionCategoryOptions)categoryOptions
{
    savedCategory_ = category;
    savedCategoryOptions_ = categoryOptions;
}

- (OHOS::AudioStandard::InterruptMode)getAllInterruptMode
{
    for (auto &renderer : rendererList_) {
        if (!renderer) {
            continue;
        }
        if ([renderer getInterruptMode] == OHOS::AudioStandard::INDEPENDENT_MODE) {
            return OHOS::AudioStandard::INDEPENDENT_MODE;
        }
    }
    return OHOS::AudioStandard::SHARE_MODE;
}

- (void)addRenderer:(AudioRendererImpl *)audioRenderer
{
    rendererList_.push_back(audioRenderer);
    [self updateRendererChangeInfos];
}

- (void)removeRenderer:(AudioRendererImpl *)audioRenderer
{
    auto it = std::find(rendererList_.begin(), rendererList_.end(), audioRenderer);
    if (it != rendererList_.end()) {
        rendererList_.erase(it);
        [self updateRendererChangeInfos];
    }
    if ([self getAllInterruptMode] == OHOS::AudioStandard::SHARE_MODE &&
        !capturerList_.size() && !communicationDeviceActive_) {
        AVAudioSession *audioSession = [AVAudioSession sharedInstance];
        [audioSession setCategory:AVAudioSessionCategoryAmbient error:nil];
        [audioSession setActive:YES error:nil];
        [self updateCategory:audioSession.category options:audioSession.categoryOptions];
    }
}

- (void)updateRendererChangeInfos
{
    if (rendererChangeCallback_) {
        std::vector<std::shared_ptr<OHOS::AudioStandard::AudioRendererChangeInfo>> audioRendererChangeInfos;
        [self getCurrentRendererChangeInfos:audioRendererChangeInfos];
        rendererChangeCallback_->OnRendererStateChange(audioRendererChangeInfos);
    }
}

- (void)addCapturer:(AudioCapturerImpl *)audioCapturer
{
    capturerList_.push_back(audioCapturer);
    [self updateCapturerChangeInfos];
}

- (void)removeCapturer:(AudioCapturerImpl *)audioCapturer
{
    auto it = std::find(capturerList_.begin(), capturerList_.end(), audioCapturer);
    if (it != capturerList_.end()) {
        capturerList_.erase(it);
        [self updateCapturerChangeInfos];
    }
    if ([self getAllInterruptMode] == OHOS::AudioStandard::SHARE_MODE &&
        !capturerList_.size() && !communicationDeviceActive_) {
        AVAudioSession *audioSession = [AVAudioSession sharedInstance];
        [audioSession setCategory:AVAudioSessionCategoryAmbient error:nil];
        [audioSession setActive:YES error:nil];
        [self updateCategory:audioSession.category options:audioSession.categoryOptions];
    }
}

- (void)updateCapturerChangeInfos
{
    if (capturerChangeCallback_) {
        std::vector<std::shared_ptr<OHOS::AudioStandard::AudioCapturerChangeInfo>> audioCapturerChangeInfos;
        [self getCurrentCapturerChangeInfos:audioCapturerChangeInfos];
        capturerChangeCallback_->OnCapturerStateChange(audioCapturerChangeInfos);
    }
}

- (int32_t)getCurrentRendererChangeInfos:
    (std::vector<std::shared_ptr<OHOS::AudioStandard::AudioRendererChangeInfo>> &)audioRendererChangeInfos
{
    NSLog(@"rendererList_ size = %lu", rendererList_.size());
    for (auto &renderer : rendererList_) {
        if (!renderer) {
            continue;
        }
        auto changeInfo = std::make_shared<OHOS::AudioStandard::AudioRendererChangeInfo>();
        uint32_t sessionId;
        int32_t ret = [renderer getAudioStreamId:sessionId];
        if (!ret) {
            changeInfo->sessionId = sessionId;
        }
        OHOS::AudioStandard::AudioRendererInfo rendererInfo;
        [renderer getRendererInfo:rendererInfo];
        changeInfo->rendererInfo = rendererInfo;

        OHOS::AudioStandard::AudioDeviceDescriptor deviceInfo;
        [renderer getCurrentOutputDevices:deviceInfo];
        changeInfo->outputDeviceInfo = deviceInfo;
        audioRendererChangeInfos.push_back(std::move(changeInfo));
    }
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)getCurrentCapturerChangeInfos:
    (std::vector<std::shared_ptr<OHOS::AudioStandard::AudioCapturerChangeInfo>> &)audioCapturerChangeInfos
{
    NSLog(@"capturerList_ size = %lu", capturerList_.size());
    for (auto &capturer : capturerList_) {
        if (!capturer) {
            continue;
        }
        auto changeInfo = std::make_shared<OHOS::AudioStandard::AudioCapturerChangeInfo>();
        int32_t ret = [capturer getCurrentCapturerChangeInfo: *changeInfo];
        if (!ret) {
            audioCapturerChangeInfos.push_back(std::move(changeInfo));
        }
    }
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)registerAudioRendererEventListener:
    (const std::shared_ptr<OHOS::AudioStandard::AudioRendererStateChangeCallback> &)callback
{
    rendererChangeCallback_ = callback;
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)unregisterAudioRendererEventListener
{
    rendererChangeCallback_ = nil;
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)registerAudioCapturerEventListener:
    (const std::shared_ptr<OHOS::AudioStandard::AudioCapturerStateChangeCallback> &)callback
{
    capturerChangeCallback_ = callback;
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)unregisterAudioCapturerEventListener
{
    capturerChangeCallback_ = nil;
    return OHOS::AudioStandard::SUCCESS;
}

- (bool)isStreamActive:(OHOS::AudioStandard::AudioVolumeType)volumeType
{
    for (auto &renderer : rendererList_) {
        if (!renderer) {
            continue;
        }
        if ([renderer getStatus] != OHOS::AudioStandard::RENDERER_RUNNING) {
            continue;
        }
        OHOS::AudioStandard::AudioRendererInfo rendererInfo;
        [renderer getRendererInfo:rendererInfo];
        if (volumeType == [self getVolumeTypeFromStreamUsage:rendererInfo.streamUsage]) {
            return true;
        }
    }
    return false;
}

- (OHOS::AudioStandard::AudioVolumeType)getVolumeTypeFromStreamUsage:(OHOS::AudioStandard::StreamUsage)streamUsage
{
    switch (streamUsage) {
        case OHOS::AudioStandard::STREAM_USAGE_VOICE_COMMUNICATION:
        case OHOS::AudioStandard::STREAM_USAGE_VOICE_MESSAGE:
        case OHOS::AudioStandard::STREAM_USAGE_VIDEO_COMMUNICATION:
            return OHOS::AudioStandard::STREAM_VOICE_CALL;
        case OHOS::AudioStandard::STREAM_USAGE_RINGTONE:
        case OHOS::AudioStandard::STREAM_USAGE_NOTIFICATION:
            return OHOS::AudioStandard::STREAM_RING;
        case OHOS::AudioStandard::STREAM_USAGE_MUSIC:
        case OHOS::AudioStandard::STREAM_USAGE_MOVIE:
        case OHOS::AudioStandard::STREAM_USAGE_GAME:
        case OHOS::AudioStandard::STREAM_USAGE_AUDIOBOOK:
        case OHOS::AudioStandard::STREAM_USAGE_NAVIGATION:
            return OHOS::AudioStandard::STREAM_MUSIC;
        case OHOS::AudioStandard::STREAM_USAGE_VOICE_ASSISTANT:
            return OHOS::AudioStandard::STREAM_VOICE_ASSISTANT;
        case OHOS::AudioStandard::STREAM_USAGE_ALARM:
            return OHOS::AudioStandard::STREAM_ALARM;
        case OHOS::AudioStandard::STREAM_USAGE_ACCESSIBILITY:
            return OHOS::AudioStandard::STREAM_ACCESSIBILITY;
        default:
            return OHOS::AudioStandard::STREAM_MUSIC;
    }
}

- (int32_t)getPreferredOutputDeviceForRendererInfo:
    (std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> &)desc
{
    AVAudioSession *audioSession = [AVAudioSession sharedInstance];
    AVAudioSessionRouteDescription *routeDescription = audioSession.currentRoute;
    for (AVAudioSessionPortDescription *portDescription in routeDescription.outputs) {
        desc.push_back([self getDeviceInfo:portDescription role:OHOS::AudioStandard::DeviceRole::OUTPUT_DEVICE]);
    }
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)getPreferredInputDeviceForCapturerInfo:
    (std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> &)desc
{
    AVAudioSession *audioSession = [AVAudioSession sharedInstance];
    AVAudioSessionRouteDescription *routeDescription = audioSession.currentRoute;
    for (AVAudioSessionPortDescription *portDescription in routeDescription.inputs) {
        desc.push_back([self getDeviceInfo:portDescription role:OHOS::AudioStandard::DeviceRole::INPUT_DEVICE]);
    }
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)setPreferredOutputDeviceChangeCallback:
    (const std::shared_ptr<OHOS::AudioStandard::AudioPreferredOutputDeviceChangeCallback> &)callback
{
    AVAudioSession *audioSession = [AVAudioSession sharedInstance];
    if (!isRouteChangeNotification_) {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                selector:@selector(handleRouteChange:)
                                                name:AVAudioSessionRouteChangeNotification
                                                object:audioSession];
        isRouteChangeNotification_ = true;
    }
    outputDeviceCallback_ = callback;
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)setPreferredInputDeviceChangeCallback:
    (const std::shared_ptr<OHOS::AudioStandard::AudioPreferredInputDeviceChangeCallback> &)callback
{
    AVAudioSession *audioSession = [AVAudioSession sharedInstance];
    if (!isRouteChangeNotification_) {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                                selector:@selector(handleRouteChange:)
                                                name:AVAudioSessionRouteChangeNotification
                                                object:audioSession];
        isRouteChangeNotification_ = true;
    }
    inputDeviceCallback_ = callback;
    return OHOS::AudioStandard::SUCCESS;
}

- (void)handleRouteChange:(NSNotification *)notification
{
    AVAudioSession *audioSession = [AVAudioSession sharedInstance];
    AVAudioSessionRouteDescription *routeDescription = audioSession.currentRoute;
    if (outputDeviceCallback_) {
        std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> descriptors = {};
        for (AVAudioSessionPortDescription *portDescription in routeDescription.outputs) {
            descriptors.push_back(
                [self getDeviceInfo:portDescription role:OHOS::AudioStandard::DeviceRole::OUTPUT_DEVICE]);
        }
        outputDeviceCallback_->OnPreferredOutputDeviceUpdated(descriptors);
    }

    if (inputDeviceCallback_) {
        std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> descriptors = {};
        for (AVAudioSessionPortDescription *portDescription in routeDescription.inputs) {
            descriptors.push_back(
                [self getDeviceInfo:portDescription role:OHOS::AudioStandard::DeviceRole::INPUT_DEVICE]);
        }
        inputDeviceCallback_->OnPreferredInputDeviceUpdated(descriptors);
    }
}

- (int32_t)unsetPreferredOutputDeviceChangeCallback
{
    outputDeviceCallback_ = nil;
    if (!inputDeviceCallback_) {
        AVAudioSession *audioSession = [AVAudioSession sharedInstance];
        [[NSNotificationCenter defaultCenter] removeObserver:self
                                                name:AVAudioSessionRouteChangeNotification
                                                object:audioSession];
        isRouteChangeNotification_ = false;
    }
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)unsetPreferredInputDeviceChangeCallback
{
    inputDeviceCallback_ = nil;
    if (!outputDeviceCallback_) {
        AVAudioSession *audioSession = [AVAudioSession sharedInstance];
        [[NSNotificationCenter defaultCenter] removeObserver:self
                                                name:AVAudioSessionRouteChangeNotification
                                                object:audioSession];
        isRouteChangeNotification_ = false;
    }
    return OHOS::AudioStandard::SUCCESS;
}

- (int32_t)getVolume
{
    AVAudioSession *audioSession = [AVAudioSession sharedInstance];
    [audioSession setActive:YES error:nil];
    NSLog(@"AVAudioSession outputVolume = %f", audioSession.outputVolume);
    return audioSession.outputVolume * MAX_VOLUME;
}

- (int32_t)getMaxVolume
{
    return MAX_VOLUME;
}

- (int32_t)getMinVolume
{
    return MIN_VOLUME;
}
@end
