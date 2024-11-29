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

#ifndef AUDIO_RENDERER_IMPL_H
#define AUDIO_RENDERER_IMPL_H

#include "audio_info.h"
#include "audio_renderer.h"
#include "audio_device_descriptor.h"
#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>

@interface AudioRendererImpl : NSObject

- (const OHOS::AudioStandard::RendererState)getStatus;
- (bool)start;
- (bool)stop;
- (bool)drain;
- (bool)flush;
- (bool)pause;
- (bool)releaseRenderer;
- (bool)getAudioTime:(OHOS::AudioStandard::Timestamp &)time;
- (int32_t)getAudioStreamId:(uint32_t &)sessionID;
- (int32_t)getBufferSize:(size_t &)bufferSize;
- (int32_t)setSpeed:(float)speed;
- (float)getSpeed;
- (int32_t)setVolume:(float)volume;
- (float)getMinStreamVolume;
- (float)getMaxStreamVolume;
- (int32_t)getCurrentOutputDevices:(OHOS::AudioStandard::AudioDeviceDescriptor &)deviceInfo;
- (int32_t)setVolumeWithRamp:(float)volume rampTime:(int32_t)duration;
- (void)setInterruptMode:(OHOS::AudioStandard::InterruptMode)mode;
- (OHOS::AudioStandard::InterruptMode)getInterruptMode;
- (void)initWithSampleRate:(const OHOS::AudioStandard::AudioRendererOptions &)rendererOptions;
- (int32_t)getRendererInfo:(OHOS::AudioStandard::AudioRendererInfo &)rendererInfo;
- (int32_t)getStreamInfo:(OHOS::AudioStandard::AudioStreamInfo &)streamInfo;
- (int32_t)setRendererCallback:(const std::shared_ptr<OHOS::AudioStandard::AudioRendererCallback> &)callback;
- (int32_t)setRenderMode:(OHOS::AudioStandard::AudioRenderMode)renderMode;
- (int32_t)setRendererWriteCallback:(const std::shared_ptr<OHOS::AudioStandard::AudioRendererWriteCallback> &)callback;
- (int32_t)setRendererDeviceChangeCallback:
    (const std::shared_ptr<OHOS::AudioStandard::AudioRendererDeviceChangeCallback> &)callback;
- (int32_t)destroyRendererDeviceChangeCallback;
- (int32_t)setRendererDeviceChangeWithInfoCallback:
    (const std::shared_ptr<OHOS::AudioStandard::AudioRendererOutputDeviceChangeCallback> &)callback;
- (int32_t)destroyRendererDeviceChangeWithInfoCallback;
- (const int32_t)enqueue:(const OHOS::AudioStandard::BufferDesc &)bufDesc;
- (const int32_t)getBufferDesc:(OHOS::AudioStandard::BufferDesc &)bufDesc;
@end
#endif // AUDIO_RENDERER_IMPL_H
