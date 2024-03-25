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

#import "recorder_adapter.h"

#include <fcntl.h>

@implementation RecorderAdapter
- (id)init
{
    NSLog(@"RecorderAdapter init!");
    self = [super init];
    if(self){
       self.videoSettings = [[NSMutableDictionary alloc] init];
       self.audioSettings = [[NSMutableDictionary alloc] init];
       self->audioRecorder_ = [[AudioRecorder alloc] init];
       self->isRecordAudio_ = NO;
    }
    return self;
}

- (void)setVideoDeviceInput
{
    if (self->videoDeviceTypes_.count == 0) {
        NSLog(@"videoDeviceTypes_ is not set!");
        return;
    }

    AVCaptureDeviceDiscoverySession *videoDeviceDiscoverySession = [
        AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:
            self->videoDeviceTypes_
            mediaType:AVMediaTypeVideo
            position:AVCaptureDevicePositionUnspecified];
    if (videoDeviceDiscoverySession) {
        NSLog(@"setVideoDeviceInput videoDeviceDiscoverySession is nil!");
        return;
    }
    AVCaptureDevice *videoDevice = videoDeviceDiscoverySession.devices.firstObject;
    NSError *error = nil;
    self->videoDeviceInput_ = [AVCaptureDeviceInput deviceInputWithDevice:videoDevice error:&error];
    if (error)
    {
        NSLog(@"Create vide input device. error:%@", error.localizedDescription);
    }
}

- (void)setAudioSession
{
    AVAudioSession *audioSession = [AVAudioSession sharedInstance];
    if (audioSession) {
        [audioSession setCategory:AVAudioSessionCategoryPlayAndRecord error:nil];
        [audioSession setActive:YES error:nil];
    }
}

- (void)startCapture
{
    NSLog(@"RecorderAdapter startCapture!");
    [self setAudioSession];
    if (self->avCaptureSession_) {
        [self->avCaptureSession_ startRunning];
    }
}

- (void)stopCapture
{
    NSLog(@"RecorderAdapter stopCapture!");
    if (self->avCaptureSession_) {
        [self->avCaptureSession_ stopRunning];
    }
}

- (void)setAudioDeviceInput
{
    NSLog(@"RecorderAdapter setAudioDeviceInput!");
    if (self->audioDeviceTypes_.count == 0) {
        NSLog(@"audioDeviceTypes_ is not set!");
        return;
    }
    AVCaptureDeviceDiscoverySession *audioDeviceDiscoverySession = [
        AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes:
            self->audioDeviceTypes_
            mediaType:AVMediaTypeAudio
            position:AVCaptureDevicePositionUnspecified];
    if (!audioDeviceDiscoverySession) {
        NSLog(@"setAudioDeviceInput audioDeviceDiscoverySession is nil!");
        return;
    }
    AVCaptureDevice *audioDevice = audioDeviceDiscoverySession.devices.firstObject;
    NSError *error = nil;
    // Create a audio input device.
    self->audioDeviceInput_ = [AVCaptureDeviceInput deviceInputWithDevice:audioDevice error:&error];
    if (error)
    {
        NSLog(@"Create audio inpt device. error:%@", error.localizedDescription);
    }
}

- (void)setOutputFile:(int)fd
{
    char path[PATH_MAX] = { '\0' };
    if (fcntl(fd, F_GETPATH, &path) == -1) {
        NSLog(@"RecorderAdapter setOutputFile. Failed to get path!");
        return;
    }

    NSString *str = [NSString stringWithUTF8String: path];
    self->outputURL_ = [NSURL fileURLWithPath:str];
    if (self->isRecordAudio_ && self->audioRecorder_) {
        [self->audioRecorder_ setOutputFile: self->outputURL_];
    }
}

- (void)setLocation:(float)latitude longitude:(float)longitude
{
    NSLog(@"RecorderAdapter setLocation latitude:%f longitude:%f", latitude, longitude);
}

- (void)setOrientationHint:(int)rotation
{
    NSLog(@"RecorderAdapter SetOrientationHint rotation:%d", rotation);
    self.videoRotationAngle = (CGFloat)rotation;
}

- (void)addDeviceInputToSession
{
    if (!self->avCaptureSession_) {
        NSLog(@"addDeviceInputToSession avCaptureSession_ is nil!");
        return;
    }
    if ([self->avCaptureSession_ canAddInput:self->videoDeviceInput_])
    {
        [self->avCaptureSession_ addInput:self->videoDeviceInput_];
    } else {
        NSLog(@"unable to add video input!");
    }
    if ([self->avCaptureSession_ canAddInput:self->audioDeviceInput_])
    {
        [self->avCaptureSession_ addInput:self->audioDeviceInput_];
    } else {
        NSLog(@"unable to add audio input!");
    }
}

- (void)addOutputToSession
{
    if (!self->avCaptureSession_) {
        NSLog(@"addOutputToSession avCaptureSession_ is nil!");
        return;
    }
    self->movieFileOutput_ = [[AVCaptureMovieFileOutput alloc] init];
    if ([self->avCaptureSession_ canAddOutput:self->movieFileOutput_])
    {
        [self->avCaptureSession_ addOutput:self->movieFileOutput_];
        AVCaptureConnection *videoConnection = [self->movieFileOutput_ connectionWithMediaType:AVMediaTypeVideo];
        [self->movieFileOutput_ setOutputSettings:self.videoSettings forConnection:videoConnection];
        AVCaptureConnection *audioConnection = [self->movieFileOutput_ connectionWithMediaType:AVMediaTypeAudio];
        [self->movieFileOutput_ setOutputSettings:self.audioSettings forConnection:audioConnection];
    }
}

- (void)initAVCaptureSession
{
    NSLog(@"RecorderAdapter initAVCaptureSession");
    if (!self->avCaptureSession_) {
        self->avCaptureSession_ = [[AVCaptureSession alloc] init];
    }
}

- (void)setVideoSource:(NSArray<AVCaptureDeviceType> *)deviceTypes
{
    NSLog(@"RecorderAdapter setVideoSource");
    self->videoDeviceTypes_ = [NSArray arrayWithArray:deviceTypes];
}

- (void)setAudioSource:(NSArray<AVCaptureDeviceType> *)deviceTypes
{
    NSLog(@"RecorderAdapter setAudioSource");
    self->audioDeviceTypes_ = [NSArray arrayWithArray:deviceTypes];
}

- (void)configureSession
{
    NSLog(@"RecorderAdapter configureSession");
    [self initAVCaptureSession];
    if (self->avCaptureSession_) {
        [self->avCaptureSession_ beginConfiguration];
    }
    [self setVideoDeviceInput];
    [self setAudioDeviceInput];
    [self addDeviceInputToSession];
    [self addOutputToSession];
    if (self->avCaptureSession_) {
        [self->avCaptureSession_ commitConfiguration];
    }
    NSLog(@"RecorderAdapter configureSession end!");
    return;
}

- (void)setVideoEncoder:(AVVideoCodecType)videoCodecType
{
    NSLog(@"RecorderAdapter setVideoEncoder");
    self.videoSettings[AVVideoCodecKey] = videoCodecType;
}

- (void)setVideoWidth:(float)width
{
    NSLog(@"RecorderAdapter setVideoWidth width:%f", width);
    self.videoSettings[AVVideoWidthKey] = [NSNumber numberWithFloat:width];
}

- (void)setVideoHeigh:(float)height
{
    NSLog(@"RecorderAdapter setVideoWidth height:%f", height);
    self.videoSettings[AVVideoHeightKey] = [NSNumber numberWithFloat:height];
}

- (void)setVideoFrameRate:(int)frameRate
{
    NSLog(@"RecorderAdapter setVideoFrameRate frameRate:%d", frameRate);
    self.videoSettings[AVVideoAverageNonDroppableFrameRateKey] = [NSNumber numberWithInt:frameRate];
}

- (void)setVideoBitRate:(int)rate
{
    NSLog(@"RecorderAdapter setVideoBitRate rate:%d", rate);
    self.videoSettings[AVVideoAverageBitRateKey] = [NSNumber numberWithInt:rate];
}

- (void)setAudioEncoder:(int)encoder
{
    NSLog(@"RecorderAdapter setAudioEncoder encoder:%d", encoder);
    self.audioSettings[AVEncoderAudioQualityKey] = [NSNumber numberWithInt:encoder];
    [self->audioRecorder_ setAudioEncoder: encoder];
}

- (void)setAudioSampleRate:(int)rate
{
    NSLog(@"RecorderAdapter setAudioSampleRate rate:%d", rate);
    self.audioSettings[AVSampleRateKey] = [NSNumber numberWithInt:rate];
    [self->audioRecorder_ setAudioSampleRate: rate];
}

- (void)setAudioChannels:(int)num
{
    NSLog(@"RecorderAdapter setAudioChannels num:%d", num);
    self.audioSettings[AVNumberOfChannelsKey] = [NSNumber numberWithInt:num];
    [self->audioRecorder_ setAudioChannels: num];
}

- (void)setAudioEncodingBitRate:(int)bitRate
{
    NSLog(@"RecorderAdapter setAudioEncodingBitRate bitRate:%d", bitRate);
    self.audioSettings[AVEncoderBitRateKey] = [NSNumber numberWithInt:bitRate];
    [self->audioRecorder_ setAudioEncodingBitRate: bitRate];
}

- (void)setOutputFormat:(int)format
{
    if (format == OHOS::Media::FORMAT_MPEG_4) {
        NSLog(@"The format is video!");
    } else if (format == OHOS::Media::FORMAT_M4A){
        self->isRecordAudio_ = YES;
        [self->audioRecorder_ setOutputFormat: kAudioFormatMPEG4AAC];
    } else {
        NSLog(@"The format is error!");
    }
}

- (void)prepare
{
    if (self->isRecordAudio_) {
        [self->audioRecorder_ prepare];
    } else {
        [self configureSession];
    }
}

- (void)start
{
    if (!self->outputURL_) {
        NSLog(@"The output url is nil!");
        return;
    }
    if (self->isRecordAudio_) {
        [self->audioRecorder_ start];
    } else {
        if (!self->movieFileOutput_.isRecording) {
            [self startCapture];
            NSLog(@"RecorderAdapter start recoding movie!");
            [self->movieFileOutput_ startRecordingToOutputFileURL:self->outputURL_ recordingDelegate:self];
        }
    }
}

- (void)pause
{
    if (self->isRecordAudio_) {
        [self->audioRecorder_ pause];
    }
}

- (void)resume
{
    if (self->isRecordAudio_) {
        [self->audioRecorder_ resume];
    }
}

- (void)stop
{
    if (self->isRecordAudio_) {
        [self->audioRecorder_ stop];
    } else {
        if (self->movieFileOutput_.isRecording) {
            NSLog(@"RecorderAdapter stop recording!");
            [self->movieFileOutput_ stopRecording];
            [self stopCapture];
        }
    }
}

- (void)reset
{
    if (self->isRecordAudio_) {
        [self->audioRecorder_ reset];
    } else {
        if (self->avCaptureSession_) {
            [self->avCaptureSession_ removeInput:self->videoDeviceInput_];
            [self->avCaptureSession_ removeInput:self->audioDeviceInput_];
            [self->avCaptureSession_ removeOutput:self->movieFileOutput_];
        }
        [self configureSession];
    }

}

- (void)releaseRecorder
{
    if (self->isRecordAudio_) {
        [self->audioRecorder_ releaseAudioRecorder];
    } else {
        if (self->videoDeviceInput_) {
            self->videoDeviceInput_ = nil;
        }
        if (self->audioDeviceInput_) {
            self->audioDeviceInput_ = nil;
        }
        if (self->movieFileOutput_) {
            self->movieFileOutput_ = nil;
        }
        if (self->avCaptureSession_) {
            self->avCaptureSession_ = nil;
        }
        if (self->videoDeviceTypes_) {
            self->videoDeviceTypes_ = nil;
        }
        if (self->audioDeviceTypes_) {
            self->audioDeviceTypes_ = nil;
        }
    }
}

- (id)GetSurface
{
    return nil;
}

- (void)SetRecorderCallback:(const std::shared_ptr<OHOS::Media::RecorderCallback> &)callback
{
    self->recorderCallbacks_ = callback;
}

- (void) captureOutput:(AVCaptureFileOutput*)captureOutput
    didStartRecordingToOutputFileAtURL:(NSURL*)fileURL
    fromConnections:(NSArray*)connections
{
    NSLog(@"RecorderAdapter captureOutput1 fileURL:%@", fileURL);
}

- (void) captureOutput:(AVCaptureFileOutput*)captureOutput
    didFinishRecordingToOutputFileAtURL:(NSURL*)outputFileURL
    fromConnections:(NSArray*)connections
    error:(NSError*)error
{
    NSLog(@"RecorderAdapter captureOutput2 outputFileURL%@", outputFileURL);
    if (error) {
        NSLog(@"RecorderAdapter captureOutput2 error:%@", error);
    }
}
@end
