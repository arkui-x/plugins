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

#import "audio_recorder.h"
#import <UIKit/UIKit.h>

@implementation AudioRecorder
- (void)initDicSetting
{
    if (!self->dicSetting_) {
        self->dicSetting_ = [[NSDictionary alloc] initWithObjectsAndKeys:
                                [NSNumber numberWithFloat: self->audioSampleRate_],AVSampleRateKey,
                                [NSNumber numberWithInt: self->audioOutputFormat_],AVFormatIDKey,
                                [NSNumber numberWithInt: self->audioEncodingBitRate_],AVEncoderBitRateKey,
                                [NSNumber numberWithInt: self->audioChannels_], AVNumberOfChannelsKey,
                                [NSNumber numberWithInt: self->audioEncoder_],AVEncoderAudioQualityKey,
                                nil];
    }
}

- (void)setAudioEncoder:(int)encoder
{
    self->audioEncoder_ = encoder;
}

- (void)setAudioSampleRate:(int)rate
{
    self->audioSampleRate_ = (float)rate;
}

- (void)setAudioChannels:(int)num
{
    self->audioChannels_ = num;
}

- (void)setAudioEncodingBitRate:(int)bitRate
{
    self->audioEncodingBitRate_ = bitRate;
}

- (void)setOutputFormat:(int)format
{
    self->audioOutputFormat_ = format;
}

- (void)setOutputFile:(NSURL *)url
{
    self->strUrl_ = url;
}

- (void)setAudioSession
{
    AVAudioSession *audioSession=[AVAudioSession sharedInstance];
    if (audioSession) {
        [audioSession setCategory:AVAudioSessionCategoryPlayAndRecord error:nil];
        [audioSession setActive:YES error:nil];
    }
}

- (void)prepare
{
    [self setAudioSession];
    [self initDicSetting];
    NSError *error = nil;
    self->avAudioRecorder_ = [[AVAudioRecorder alloc]initWithURL:self->strUrl_ settings:self->dicSetting_ error:&error];
    self->avAudioRecorder_.delegate = self;
    if (error)
    {
        NSLog(@"Create AVAudioRecorder. error: %@", error.localizedDescription);
    }
    BOOL isPrepare = [self->avAudioRecorder_ prepareToRecord];
    if (isPrepare) {
        NSLog(@"AudioRecorder prepareToRecord success！");
    }
    self->isPause_ = NO;
}

- (void)start
{
    if (self->avAudioRecorder_ && !self->avAudioRecorder_.isRecording) {
        BOOL isRecord = [self->avAudioRecorder_ record];
        if (isRecord) {
            NSLog(@"AudioRecorder record success！");
        }
        self->isPause_ = NO;
    }
}

- (void)pause
{
    if (self->avAudioRecorder_ && self->avAudioRecorder_.isRecording) {
        [self->avAudioRecorder_ pause];
        self->isPause_ = YES;
    }
}

- (void)resume
{
    if (self->avAudioRecorder_ && self->isPause_) {
        [self->avAudioRecorder_ record];
        self->isPause_ = NO;
    }
}
- (void)stop
{
    if (self->avAudioRecorder_) {
        [self->avAudioRecorder_ stop];
        self->isPause_ = NO;
    }
}

- (void)reset
{
    if (self->avAudioRecorder_ && self->avAudioRecorder_.isRecording) {
        [self->avAudioRecorder_ stop];
    }
    self->isPause_ = NO;
}

- (void)releaseAudioRecorder
{
    if (self->avAudioRecorder_) {
        self->avAudioRecorder_ = nil;
    }
    if (self->dicSetting_) {
        self->dicSetting_ = nil;
    }
    self->strUrl_ = nil;
    self->isPause_ = NO;
}

- (void)audioRecorderDidFinishRecording:(AVAudioRecorder *)recorder
                           successfully:(BOOL)flag
{
    if (flag) {
        NSLog(@"Recording audio sucessfully!");
    } else {
        NSLog(@"Recording audio failed!");
    }
}

- (void)audioRecorderEncodeErrorDidOccur:(AVAudioRecorder *)recorder
                                   error:(NSError *)error
{
    if (error) {
        NSLog(@"AudioRecorder audioRecorderEncodeErrorDidOccur error:%@", error);
    }
}
@end
