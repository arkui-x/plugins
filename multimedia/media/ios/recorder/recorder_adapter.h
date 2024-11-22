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

#ifndef PLUGINS_MULTIMEDIA_MEDIA_IOS_RECORDER_RECORDER_ADAPTER_H
#define PLUGINS_MULTIMEDIA_MEDIA_IOS_RECORDER_RECORDER_ADAPTER_H

#include <map>

#include "recorder.h"

#import <AVFoundation/AVFoundation.h>
#import "audio_recorder.h"

@interface RecorderAdapter: NSObject<AVCaptureFileOutputRecordingDelegate>
{
    std::shared_ptr<OHOS::Media::RecorderCallback> recorderCallbacks_;
    AudioRecorder *audioRecorder_;
    AVCaptureDeviceInput *audioDeviceInput_;
    AVCaptureDeviceInput *videoDeviceInput_;
    NSArray<AVCaptureDeviceType> *videoDeviceTypes_;
    NSArray<AVCaptureDeviceType> *audioDeviceTypes_;
    AVCaptureSession* avCaptureSession_;
    AVCaptureMovieFileOutput* movieFileOutput_;
    NSURL *outputURL_;
    bool isRecordAudio_;
}
@property (nonatomic) AVCaptureConnection *videoConnection;
@property (nonatomic) NSMutableDictionary *videoSettings;
@property (nonatomic) NSMutableDictionary *audioSettings;

@property(nonatomic) CGFloat videoRotationAngle;

- (id)init;

- (void)setVideoDeviceInput;
- (void)setAudioDeviceInput;
- (void)initAVCaptureSession;

- (void)setVideoSource:(NSArray<AVCaptureDeviceType> *)deviceTypes;
- (void)setAudioSource:(NSArray<AVCaptureDeviceType> *)deviceTypes;

- (void)setVideoWidth:(float)width;
- (void)setVideoHeigh:(float)height;

- (void)setVideoFrameRate:(int)frameRate;
- (void)setVideoBitRate:(int)rate;

- (void)setAudioEncoder:(int)encoder;
- (void)setAudioSampleRate:(int)rate;
- (void)setAudioChannels:(int)num;
- (void)setAudioEncodingBitRate:(int)bitRate;

- (void)setOutputFormat:(int)format;
- (void)setOutputFile:(int)fd;

- (void)setLocation:(float)latitude longitude:(float)longitude;
- (void)setOrientationHint:(int)rotation;

- (void)addDeviceInputToSession;
- (void)addOutputToSession;

- (void)configureSession;

- (void)setVideoEncoder:(AVVideoCodecType)videoCodecType;
- (void)setAudioCodecType;

- (void)prepare;
- (void)start;
- (void)pause;
- (void)resume;
- (void)stop;
- (void)reset;
- (void)releaseRecorder;

- (void)setAudioSession;
- (void)startCapture;
- (void)stopCapture;

- (id)GetSurface;
- (void)SetRecorderCallback:(const std::shared_ptr<OHOS::Media::RecorderCallback> &)callback;
@end
#endif // PLUGINS_MULTIMEDIA_MEDIA_IOS_RECORDER_RECORDER_ADAPTER_H
