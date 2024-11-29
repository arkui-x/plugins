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

#ifndef AUDIO_MANAGER_IMPL_H
#define AUDIO_MANAGER_IMPL_H

#include "audio_info.h"
#include "audio_system_manager.h"
#include "audio_stream_manager.h"
#include "audio_routing_manager.h"
#include "audio_group_manager.h"
#include "audio_renderer_impl.h"
#include "audio_capturer_impl.h"
#import <AVFoundation/AVFoundation.h>

@interface AudioManagerImpl : NSObject

+ (AudioManagerImpl *)sharedInstance;

- (const OHOS::AudioStandard::AudioScene)getAudioScene;
- (int32_t)registerVolumeKeyEventCallback:
    (const std::shared_ptr<OHOS::AudioStandard::VolumeKeyEventCallback> &)callback;
- (std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>>)getDevices:
    (OHOS::AudioStandard::DeviceFlag)deviceFlag;
- (int32_t)setDeviceActive:(OHOS::AudioStandard::DeviceType)deviceType active:(bool)flag;
- (bool)isDeviceActive:(OHOS::AudioStandard::DeviceType)deviceType;
- (void)updateCategory:(AVAudioSessionCategory)category options:(AVAudioSessionCategoryOptions)categoryOptions;
- (OHOS::AudioStandard::InterruptMode)getAllInterruptMode;
- (void)addRenderer:(AudioRendererImpl *)audioRenderer;
- (void)removeRenderer:(AudioRendererImpl *)audioRenderer;
- (void)updateRendererChangeInfos;
- (void)addCapturer:(AudioCapturerImpl *)audioCapturer;
- (void)removeCapturer:(AudioCapturerImpl *)audioCapturer;
- (void)updateCapturerChangeInfos;
- (int32_t)getCurrentRendererChangeInfos:
    (std::vector<std::shared_ptr<OHOS::AudioStandard::AudioRendererChangeInfo>> &)audioRendererChangeInfos;
- (int32_t)getCurrentCapturerChangeInfos:
    (std::vector<std::shared_ptr<OHOS::AudioStandard::AudioCapturerChangeInfo>> &)audioCapturerChangeInfos;
- (int32_t)registerAudioRendererEventListener:
    (const std::shared_ptr<OHOS::AudioStandard::AudioRendererStateChangeCallback> &)callback;
- (int32_t)unregisterAudioRendererEventListener;
- (int32_t)registerAudioCapturerEventListener:
    (const std::shared_ptr<OHOS::AudioStandard::AudioCapturerStateChangeCallback> &)callback;
- (int32_t)unregisterAudioCapturerEventListener;
- (bool)isStreamActive:(OHOS::AudioStandard::AudioVolumeType)volumeType;
- (int32_t)getPreferredOutputDeviceForRendererInfo:
    (std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> &)desc;
- (int32_t)getPreferredInputDeviceForCapturerInfo:
    (std::vector<std::shared_ptr<OHOS::AudioStandard::AudioDeviceDescriptor>> &)desc;
- (int32_t)setPreferredOutputDeviceChangeCallback:
    (const std::shared_ptr<OHOS::AudioStandard::AudioPreferredOutputDeviceChangeCallback> &)callback;
- (int32_t)setPreferredInputDeviceChangeCallback:
    (const std::shared_ptr<OHOS::AudioStandard::AudioPreferredInputDeviceChangeCallback> &)callback;
- (int32_t)unsetPreferredOutputDeviceChangeCallback;
- (int32_t)unsetPreferredInputDeviceChangeCallback;
- (int32_t)getVolume;
- (int32_t)getMaxVolume;
- (int32_t)getMinVolume;
@end
#endif // AUDIO_MANAGER_IMPL_H
