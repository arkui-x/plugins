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

#ifndef AUDIO_CAPTURER_IMPL_H
#define AUDIO_CAPTURER_IMPL_H

#include "napi_audio_capturer_read_data_callback.h"
#include "audio_capturer.h"
#include "audio_info.h"
#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>

@interface AudioCapturerImpl : NSObject
- (bool)initWithSampleRate:(const OHOS::AudioStandard::AudioCapturerOptions)capturerOptions;
- (int32_t)getCapturerInfo:(OHOS::AudioStandard::AudioCapturerInfo &)capturerInfo;
- (int32_t)getStreamInfo:(OHOS::AudioStandard::AudioStreamInfo &)streamInfo;
- (const OHOS::AudioStandard::CapturerState)getStatus;
- (bool)start;
- (bool)stop;
- (bool)releaseCapturer;
- (int32_t)getBufferSize:(size_t &)bufferSize;
- (const int32_t)getBufferDesc:(OHOS::AudioStandard::BufferDesc &)bufDesc;
- (int32_t)enqueue:(const OHOS::AudioStandard::BufferDesc &)bufDesc;
- (bool)getAudioTime:(OHOS::AudioStandard::Timestamp &)timestamp;
- (int32_t)getAudioStreamId:(uint32_t &)sessionID;
- (int32_t)getCurrentInputDevices:(OHOS::AudioStandard::AudioDeviceDescriptor &)deviceInfo;
- (int32_t)getCurrentCapturerChangeInfo:(OHOS::AudioStandard::AudioCapturerChangeInfo &)changeInfo;
- (int32_t)setCapturerReadCallback:(const std::shared_ptr<OHOS::AudioStandard::AudioCapturerReadCallback> &)callback;
- (int32_t)setCapturerCallback:(const std::shared_ptr<OHOS::AudioStandard::AudioCapturerCallback> &)callback;
- (int32_t)setAudioCapturerDeviceChangeCallback:
    (const std::shared_ptr<OHOS::AudioStandard::AudioCapturerDeviceChangeCallback> &)callback;
- (int32_t)removeAudioCapturerDeviceChangeCallback:
    (const std::shared_ptr<OHOS::AudioStandard::AudioCapturerDeviceChangeCallback> &)callback;
- (int32_t)setAudioCapturerInfoChangeCallback:
    (const std::shared_ptr<OHOS::AudioStandard::AudioCapturerInfoChangeCallback> &)callback;
- (int32_t)removeAudioCapturerInfoChangeCallback:
    (const std::shared_ptr<OHOS::AudioStandard::AudioCapturerInfoChangeCallback> &)callback;
@end
#endif // AUDIO_CAOTURER_IMPL_H
